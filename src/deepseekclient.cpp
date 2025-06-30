#include "deepseekclient.h"
#include "config.h"
#include "logger.h"
#include <QNetworkRequest>
#include <QJsonParseError>
#include <QJsonArray>
#include <QUrl>
#include "networkmanager.h"

DeepSeekClient::DeepSeekClient(QObject *parent)
    : QObject(parent)
{
    LOG_INFO("DeepSeek客户端初始化完成");
}

DeepSeekClient::~DeepSeekClient()
{
    LOG_INFO("DeepSeek客户端销毁");
}

void DeepSeekClient::setApiKey(const QString &apiKey)
{
    m_apiKey = apiKey;
    LOG_INFO("设置DeepSeek API密钥");
}

bool DeepSeekClient::isApiConfigured() const
{
    return !m_apiKey.isEmpty();
}

void DeepSeekClient::setProxy(const QString &host, quint16 port, const QString &user, const QString &password, const QString &caPath)
{
    m_proxyHost = host;
    m_proxyPort = port;
    m_proxyUser = user;
    m_proxyPassword = password;
    m_caPath = caPath;
}

void DeepSeekClient::addCaCertificate(const QString &certPath)
{
    m_caPath = certPath;
}

void DeepSeekClient::polishText(const QString &text)
{
    if (!isApiConfigured()) {
        emit polishError("请先配置DeepSeek API密钥");
        return;
    }

    if (text.trimmed().isEmpty()) {
        emit polishError("请输入要润色的文本");
        return;
    }

    QUrl url(QString::fromUtf8(Config::DEEPSEEK_API_URL));
    QJsonObject payload;
    payload["model"] = "deepseek-chat";
    payload["messages"] = QJsonArray{
        QJsonObject{
            {"role", "user"},
            {"content", QString::fromUtf8(Config::DEEPSEEK_PROMPT) + " " + text}
        }
    };
    payload["max_tokens"] = 2000;
    payload["temperature"] = 0.7;
    QByteArray data = QJsonDocument(payload).toJson();
    NetworkManager::instance()->post(url.toString(), data, m_proxyHost, m_proxyPort, m_proxyUser, m_proxyPassword, m_caPath,
        [this](bool ok, const QByteArray &data) {
            if (!ok) {
                emit polishError(QString::fromUtf8(data));
                return;
            }
            QJsonParseError parseError;
            QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
            if (parseError.error != QJsonParseError::NoError) {
                emit polishError("解析响应数据失败: " + parseError.errorString());
                return;
            }
            QJsonObject obj = doc.object();
            QString resultText;
            if (obj.contains("choices") && obj["choices"].isArray()) {
                QJsonArray choices = obj["choices"].toArray();
                if (!choices.isEmpty()) {
                    QJsonObject choice = choices.first().toObject();
                    if (choice.contains("message")) {
                        QJsonObject message = choice["message"].toObject();
                        resultText = message["content"].toString();
                    }
                }
            }
            if (resultText.isEmpty()) {
                if (obj.contains("text")) {
                    resultText = obj["text"].toString();
                } else if (obj.contains("result")) {
                    resultText = obj["result"].toString();
                } else {
                    resultText = QString::fromUtf8(data);
                }
            }
            emit polishFinished(resultText);
        });

    LOG_INFO("发送DeepSeek润色请求");
}

void DeepSeekClient::onNetworkError(QNetworkReply::NetworkError)
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) {
        LOG_ERROR("无法获取网络错误响应对象");
        return;
    }

    QString errorMsg = "网络错误: " + reply->errorString();
    LOG_ERROR(QString("润色失败: %1").arg(errorMsg));
    emit polishError(errorMsg);
}

