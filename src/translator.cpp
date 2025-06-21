#include "translator.h"
#include "config.h"
#include "logger.h"
#include <QNetworkRequest>
#include <QUrlQuery>
#include <QCryptographicHash>
#include <QJsonParseError>
#include <QDebug>

Translator::Translator(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
{
    LOG_INFO("翻译器初始化完成", "Translator");
}

Translator::~Translator()
{
    LOG_INFO("翻译器销毁", "Translator");
}

void Translator::setApiCredentials(const QString &appId, const QString &secretKey)
{
    m_appId = appId;
    m_secretKey = secretKey;
    
    LOG_SETTINGS_CHANGE("API凭据", "设置新的API凭据", "App ID: " + appId.left(4) + "***");
}

bool Translator::isApiConfigured() const
{
    return !m_appId.isEmpty() && !m_secretKey.isEmpty();
}

void Translator::translateText(const QString &text, const QString &fromLang, const QString &toLang)
{
    LOG_TRANSLATION_START(text, fromLang, toLang);
    
    if (!isApiConfigured()) {
        QString errorMsg = "请先配置百度翻译API密钥";
        LOG_TRANSLATION_ERROR(text, errorMsg, fromLang, toLang);
        emit translationError(errorMsg);
        return;
    }

    if (text.trimmed().isEmpty()) {
        QString errorMsg = "请输入要翻译的文本";
        LOG_TRANSLATION_ERROR(text, errorMsg, fromLang, toLang);
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

    QString params = query.toString();
    LOG_API_CALL(url.toString(), params);

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QNetworkReply *reply = m_networkManager->get(request);
    
    // 使用lambda表达式处理网络响应
    connect(reply, &QNetworkReply::finished, [this, reply]() {
        this->onTranslationFinished(reply);
    });
    
    connect(reply, &QNetworkReply::errorOccurred,
            this, &Translator::onNetworkError);
            
    LOG_INFO("网络请求已发送", "Translator");
}

QString Translator::generateSignature(const QString &query, const QString &salt, 
                                    const QString &appId, const QString &secretKey)
{
    QString signStr = appId + query + salt + secretKey;
    QByteArray hash = QCryptographicHash::hash(signStr.toUtf8(), QCryptographicHash::Md5);
    QString signature = hash.toHex();
    
    LOG_DEBUG(QString("生成签名 - 原文长度: %1, 签名: %2...").arg(signStr.length()).arg(signature.left(8)), "Translator");
    return signature;
}

void Translator::onTranslationFinished(QNetworkReply *reply)
{
    if (!reply) {
        LOG_ERROR("无法获取网络响应对象", "Translator");
        return;
    }

    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        QString errorMsg = "网络请求失败: " + reply->errorString();
        LOG_TRANSLATION_ERROR("", errorMsg, "", "");
        LOG_API_RESPONSE("", false);
        emit translationError(errorMsg);
        return;
    }

    QByteArray data = reply->readAll();
    QString response = QString::fromUtf8(data);
    
    LOG_API_RESPONSE(response, true);
    LOG_DEBUG(QString("收到API响应，长度: %1").arg(response.length()), "Translator");
    
    parseTranslationResult(data);
}

void Translator::onNetworkError(QNetworkReply::NetworkError error)
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) {
        LOG_ERROR("无法获取网络错误响应对象", "Translator");
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
        errorMsg = "网络错误: " + reply->errorString();
        break;
    }

    LOG_ERROR(QString("网络错误: %1").arg(errorMsg), "Translator");
    emit translationError(errorMsg);
}

void Translator::parseTranslationResult(const QByteArray &data)
{
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    
    if (parseError.error != QJsonParseError::NoError) {
        QString errorMsg = "解析响应数据失败: " + parseError.errorString();
        LOG_TRANSLATION_ERROR("", errorMsg, "", "");
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
        
        LOG_TRANSLATION_ERROR("", userFriendlyMsg, "", "");
        emit translationError(userFriendlyMsg);
        return;
    }

    // 解析翻译结果
    if (obj.contains("trans_result")) {
        QJsonArray transResult = obj["trans_result"].toArray();
        if (!transResult.isEmpty()) {
            QJsonObject firstResult = transResult.first().toObject();
            QString translatedText = firstResult["dst"].toString();
            QString detectedLang = obj["from"].toString();
            
            LOG_TRANSLATION_SUCCESS("", translatedText, "", "", detectedLang);
            emit translationFinished(translatedText, detectedLang);
            return;
        }
    }

    QString errorMsg = "无法解析翻译结果";
    LOG_TRANSLATION_ERROR("", errorMsg, "", "");
    emit translationError(errorMsg);
} 