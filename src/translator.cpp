#include "translator.h"
#include "config.h"
#include <QNetworkRequest>
#include <QUrlQuery>
#include <QCryptographicHash>
#include <QJsonParseError>
#include <QDebug>

Translator::Translator(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
{
    connect(m_networkManager, &QNetworkAccessManager::finished,
            this, &Translator::onTranslationFinished);
}

Translator::~Translator()
{
}

void Translator::setApiCredentials(const QString &appId, const QString &secretKey)
{
    m_appId = appId;
    m_secretKey = secretKey;
}

bool Translator::isApiConfigured() const
{
    return !m_appId.isEmpty() && !m_secretKey.isEmpty();
}

void Translator::translateText(const QString &text, const QString &fromLang, const QString &toLang)
{
    if (!isApiConfigured()) {
        emit translationError("请先配置百度翻译API密钥");
        return;
    }

    if (text.trimmed().isEmpty()) {
        emit translationError("请输入要翻译的文本");
        return;
    }

    // 构建请求参数
    QString salt = QString::number(QDateTime::currentMSecsSinceEpoch());
    QString sign = generateSignature(text, salt, m_appId, m_secretKey);

    QUrl url(Config::BAIDU_API_URL);
    QUrlQuery query;
    query.addQueryItem("q", text);
    query.addQueryItem("from", fromLang);
    query.addQueryItem("to", toLang);
    query.addQueryItem("appid", m_appId);
    query.addQueryItem("salt", salt);
    query.addQueryItem("sign", sign);
    url.setQuery(query);

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QNetworkReply *reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::errorOccurred,
            this, &Translator::onNetworkError);
}

QString Translator::generateSignature(const QString &query, const QString &salt, 
                                    const QString &appId, const QString &secretKey)
{
    QString signStr = appId + query + salt + secretKey;
    QByteArray hash = QCryptographicHash::hash(signStr.toUtf8(), QCryptographicHash::Md5);
    return hash.toHex();
}

void Translator::onTranslationFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;

    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        emit translationError("网络请求失败: " + reply->errorString());
        return;
    }

    QByteArray data = reply->readAll();
    parseTranslationResult(data);
}

void Translator::onNetworkError(QNetworkReply::NetworkError error)
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;

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

    emit translationError(errorMsg);
}

void Translator::parseTranslationResult(const QByteArray &data)
{
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    
    if (parseError.error != QJsonParseError::NoError) {
        emit translationError("解析响应数据失败: " + parseError.errorString());
        return;
    }

    QJsonObject obj = doc.object();
    
    // 检查是否有错误
    if (obj.contains("error_code")) {
        int errorCode = obj["error_code"].toInt();
        QString errorMsg = obj["error_msg"].toString();
        
        QString userFriendlyMsg;
        switch (errorCode) {
        case 52001:
            userFriendlyMsg = "请求超时，请重试";
            break;
        case 52002:
            userFriendlyMsg = "系统错误，请重试";
            break;
        case 52003:
            userFriendlyMsg = "未授权用户，请检查API密钥";
            break;
        case 54000:
            userFriendlyMsg = "必填参数为空";
            break;
        case 54001:
            userFriendlyMsg = "签名错误，请检查API密钥";
            break;
        case 54003:
            userFriendlyMsg = "访问频率受限";
            break;
        case 54004:
            userFriendlyMsg = "账户余额不足";
            break;
        case 54005:
            userFriendlyMsg = "长query请求频繁";
            break;
        case 58000:
            userFriendlyMsg = "不支持该语种的翻译";
            break;
        case 58001:
            userFriendlyMsg = "翻译文本过长";
            break;
        case 58002:
            userFriendlyMsg = "不支持该语种的翻译";
            break;
        default:
            userFriendlyMsg = QString("API错误 %1: %2").arg(errorCode).arg(errorMsg);
            break;
        }
        
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
            
            emit translationFinished(translatedText, detectedLang);
            return;
        }
    }

    emit translationError("无法解析翻译结果");
} 