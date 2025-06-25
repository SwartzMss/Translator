#include "deepseekclient.h"
#include "config.h"
#include "logger.h"
#include <QNetworkRequest>
#include <QJsonParseError>

DeepSeekClient::DeepSeekClient(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
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
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject payload;
    payload["prompt"] = QString::fromUtf8(Config::DEEPSEEK_PROMPT) + text;
    payload["api_key"] = m_apiKey;

    QByteArray data = QJsonDocument(payload).toJson();

    QNetworkReply *reply = m_networkManager->post(request, data);

    connect(reply, &QNetworkReply::finished, [this, reply]() {
        this->onPolishFinished(reply);
    });
    connect(reply, &QNetworkReply::errorOccurred,
            this, &DeepSeekClient::onNetworkError);

    LOG_INFO("发送DeepSeek润色请求");
}

void DeepSeekClient::onPolishFinished(QNetworkReply *reply)
{
    if (!reply) {
        LOG_ERROR("无法获取网络响应对象");
        return;
    }

    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        QString errorMsg = "网络请求失败: " + reply->errorString();
        LOG_ERROR(QString("润色失败: %1").arg(errorMsg));
        emit polishError(errorMsg);
        return;
    }

    QByteArray data = reply->readAll();
    QString response = QString::fromUtf8(data);

    LOG_DEBUG(QString("DeepSeek API响应长度: %1").arg(response.length()));

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        QString errorMsg = "解析响应数据失败: " + parseError.errorString();
        LOG_ERROR(QString("润色失败: %1").arg(errorMsg));
        emit polishError(errorMsg);
        return;
    }

    QJsonObject obj = doc.object();
    QString resultText;
    if (obj.contains("text")) {
        resultText = obj["text"].toString();
    } else if (obj.contains("result")) {
        resultText = obj["result"].toString();
    } else {
        resultText = QString::fromUtf8(data);
    }

    LOG_INFO("润色成功");
    emit polishFinished(resultText);
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

