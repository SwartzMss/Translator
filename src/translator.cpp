#include "translator.h"
#include "config.h"
#include "logger.h"
#include <QNetworkRequest>
#include <QUrl>
#include <QUrlQuery>
#include <QDateTime>
#include <QCryptographicHash>
#include <QJsonParseError>
#include <QDebug>
#include "networkmanager.h"

Translator::Translator(QObject *parent)
    : QObject(parent)
{
    LOG_INFO("翻译器初始化完成");
}

Translator::~Translator()
{
    LOG_INFO("翻译器销毁");
}

void Translator::setApiCredentials(const QString &appId, const QString &secretKey)
{
    m_appId = appId;
    m_secretKey = secretKey;
    
    LOG_INFO(QString("设置新的API凭据 - App ID: %1***").arg(appId.left(4)));
}

bool Translator::isApiConfigured() const
{
    return !m_appId.isEmpty() && !m_secretKey.isEmpty();
}

void Translator::setNetworkProxy(const QString &host, quint16 port,
                                 const QString &user,
                                 const QString &password)
{
    m_proxyHost = host;
    m_proxyPort = port;
    m_proxyUser = user;
    m_proxyPassword = password;
}

void Translator::addCaCertificate(const QString &certPath)
{
    m_caPath = certPath;
}

void Translator::translateText(const QString &text, const QString &fromLang, const QString &toLang)
{
    LOG_INFO(QString("开始翻译 - 文本长度: %1, 从: %2, 到: %3")
             .arg(text.length()).arg(fromLang).arg(toLang));
    
    if (!isApiConfigured()) {
        QString errorMsg = "请先配置百度翻译API密钥";
        LOG_ERROR(QString("翻译失败 - 文本长度: %1, 错误: %2")
                  .arg(text.length()).arg(errorMsg));
        emit translationError(errorMsg);
        return;
    }

    if (text.trimmed().isEmpty()) {
        QString errorMsg = "请输入要翻译的文本";
        LOG_ERROR(QString("翻译失败 - 文本长度: %1, 错误: %2")
                  .arg(text.length()).arg(errorMsg));
        emit translationError(errorMsg);
        return;
    }

    // 构建请求参数
    QString salt = QString::number(QDateTime::currentMSecsSinceEpoch());
    QString sign = generateSignature(text, salt, m_appId, m_secretKey);

    QUrl url(QString::fromUtf8(Config::BAIDU_API_URL));
    QUrlQuery query;
    query.addQueryItem("q", text);
    query.addQueryItem("from", fromLang);
    query.addQueryItem("to", toLang);
    query.addQueryItem("appid", m_appId);
    query.addQueryItem("salt", salt);
    query.addQueryItem("sign", sign);
    url.setQuery(query);
    QString urlStr = url.toString();
    NetworkManager::instance()->get(urlStr, m_proxyHost, m_proxyPort, m_proxyUser, m_proxyPassword, m_caPath,
        [this](bool ok, const QByteArray &data) {
            if (!ok) {
                emit translationError(QString::fromUtf8(data));
                return;
            }
            parseTranslationResult(data);
        });
}

QString Translator::generateSignature(const QString &query, const QString &salt, 
                                    const QString &appId, const QString &secretKey)
{
    QString signStr = appId + query + salt + secretKey;
    QByteArray hash = QCryptographicHash::hash(signStr.toUtf8(), QCryptographicHash::Md5);
    QString signature = hash.toHex();
    
    LOG_DEBUG(QString("生成签名 - 原文长度: %1, 签名: %2...")
              .arg(signStr.length()).arg(signature.left(8)));
    return signature;
}

void Translator::onTranslationFinished(QNetworkReply *reply)
{
    if (!reply) {
        LOG_ERROR("无法获取网络响应对象");
        return;
    }

    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        QString errorMsg = "网络请求失败: " + reply->errorString();
        LOG_ERROR(QString("翻译失败: %1").arg(errorMsg));
        emit translationError(errorMsg);
        return;
    }

    QByteArray data = reply->readAll();
    QString response = QString::fromUtf8(data);
    
    LOG_DEBUG(QString("API响应 - 成功, 长度: %1").arg(response.length()));
    
    parseTranslationResult(data);
}

void Translator::onNetworkError(QNetworkReply::NetworkError error)
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) {
        LOG_ERROR("无法获取网络错误响应对象");
        return;
    }

    QString errorMsg;
    switch (error) {
    case QNetworkReply::ConnectionRefusedError:
        errorMsg = "连接被拒绝，请检查网络连接";
        break;
    case QNetworkReply::RemoteHostClosedError:
        errorMsg = "远程服务器关闭连接";
        break;
    case QNetworkReply::HostNotFoundError:
        errorMsg = "找不到服务器";
        break;
    case QNetworkReply::TimeoutError:
        errorMsg = "请求超时";
        break;
    default:
        errorMsg = reply->errorString();
        break;
    }

    QString fullMsg = QStringLiteral("网络错误: %1").arg(errorMsg);
    LOG_ERROR(fullMsg);
    emit translationError(fullMsg);
}

void Translator::parseTranslationResult(const QByteArray &data)
{
    // 新增：打印原始响应内容
    LOG_DEBUG(QString("原始API响应内容: %1").arg(QString::fromUtf8(data.left(512))));
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    
    if (parseError.error != QJsonParseError::NoError) {
        QString errorMsg = "解析响应数据失败: " + parseError.errorString();
        // 新增：解析失败时也输出原始内容
        LOG_ERROR(QString("翻译失败: %1, 原始内容: %2").arg(errorMsg).arg(QString::fromUtf8(data.left(512))));
        emit translationError(errorMsg);
        return;
    }

    QJsonObject obj = doc.object();
    
    // 检查是否有错误
    if (obj.contains("error_code")) {
        int errorCode = obj["error_code"].toInt();
        QString errorMsg = obj["error_msg"].toString();
        
        QString userFriendlyMsg;
        switch (errorCode) {
        case 52001: userFriendlyMsg = "请求超时，请重试"; break;
        case 52002: userFriendlyMsg = "系统错误，请重试"; break;
        case 52003: userFriendlyMsg = "未授权用户，请检查API密钥"; break;
        case 54000: userFriendlyMsg = "必填参数为空"; break;
        case 54001: userFriendlyMsg = "签名错误，请检查API密钥"; break;
        case 54003: userFriendlyMsg = "访问频率受限"; break;
        case 54004: userFriendlyMsg = "账户余额不足"; break;
        case 54005: userFriendlyMsg = "长query请求频繁"; break;
        case 58000: userFriendlyMsg = "不支持该语种的翻译"; break;
        case 58001: userFriendlyMsg = "翻译文本过长"; break;
        case 58002: userFriendlyMsg = "不支持该语种的翻译"; break;
        default: userFriendlyMsg = QString("API错误 %1: %2").arg(errorCode).arg(errorMsg); break;
        }
        
        LOG_ERROR(QString("翻译失败: %1").arg(userFriendlyMsg));
        emit translationError(userFriendlyMsg);
        return;
    }

    // 解析翻译结果
    if (obj.contains("trans_result")) {
        QJsonArray transResult = obj["trans_result"].toArray();
        if (!transResult.isEmpty()) {
            QStringList translatedTexts;
            for (const QJsonValue &value : transResult) {
                if (value.isObject()) {
                    QJsonObject resultObj = value.toObject();
                    translatedTexts.append(resultObj["dst"].toString());
                }
            }

            QString combinedText = translatedTexts.join('\n');
            QString detectedLang = obj["from"].toString();

            LOG_INFO(QString("翻译成功 - 检测语言: %1, 译文长度: %2")
                     .arg(detectedLang).arg(combinedText.length()));
            emit translationFinished(combinedText, detectedLang);
            return;
        }
    }

    QString errorMsg = "无法解析翻译结果";
    LOG_ERROR(QString("翻译失败: %1").arg(errorMsg));
    emit translationError(errorMsg);
} 