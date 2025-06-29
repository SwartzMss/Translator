#include "deepseekclient.h"
#include "config.h"
#include "logger.h"
#include <QNetworkRequest>
#include <QNetworkProxy>
#include <QJsonParseError>
#include <QJsonArray>
#include <QUrl>
#include <QSslSocket>
#include <QSslCertificate>
#include <QFile>

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

void DeepSeekClient::setNetworkProxy(const QString &host, quint16 port,
                                      const QString &user,
                                      const QString &password)
{
    if (host.isEmpty() || port == 0) {
        m_networkManager->setProxy(QNetworkProxy());
        LOG_INFO("取消网络代理设置(DeepSeek)");
        return;
    }

    QNetworkProxy proxy(QNetworkProxy::HttpProxy, host, port, user, password);
    m_networkManager->setProxy(proxy);
    LOG_INFO(QString("设置DeepSeek网络代理 - %1:%2").arg(host).arg(port));
}

void DeepSeekClient::addCaCertificate(const QString &certPath)
{
    if (certPath.isEmpty()) {
        return;
    }

    if (!QFile::exists(certPath)) {
        LOG_WARNING(QString("证书文件不存在: %1").arg(certPath));
        return;
    }

    const QList<QSslCertificate> certs = QSslCertificate::fromPath(certPath);
    if (certs.isEmpty()) {
        LOG_WARNING(QString("加载证书失败: %1").arg(certPath));
        return;
    }

    QSslSocket::addDefaultCaCertificates(certs);
    LOG_INFO(QString("导入CA证书(DeepSeek): %1").arg(certPath));
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
    request.setRawHeader("Authorization", QString("Bearer %1").arg(m_apiKey).toUtf8());

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
    
    // 检查是否有错误
    if (obj.contains("error")) {
        QString errorMsg = "API错误: " + obj["error"].toObject()["message"].toString();
        LOG_ERROR(QString("润色失败: %1").arg(errorMsg));
        emit polishError(errorMsg);
        return;
    }
    
    // 解析标准 OpenAI 兼容格式的响应
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
    
    // 如果没有找到标准格式，尝试其他可能的字段
    if (resultText.isEmpty()) {
        if (obj.contains("text")) {
            resultText = obj["text"].toString();
        } else if (obj.contains("result")) {
            resultText = obj["result"].toString();
        } else {
            resultText = QString::fromUtf8(data);
        }
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

