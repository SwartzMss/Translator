#ifndef DEEPSEEKCLIENT_H
#define DEEPSEEKCLIENT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>

class DeepSeekClient : public QObject
{
    Q_OBJECT
public:
    explicit DeepSeekClient(QObject *parent = nullptr);
    ~DeepSeekClient();

    void setApiKey(const QString &apiKey);
    bool isApiConfigured() const;

    void polishText(const QString &text);

    // 设置网络代理
    void setProxy(const QString &host, quint16 port, const QString &user = QString(), const QString &password = QString(), const QString &caPath = QString());

    // 导入自定义CA证书
    void addCaCertificate(const QString &certPath);

signals:
    void polishFinished(const QString &polishedText);
    void polishError(const QString &errorMessage);

private slots:
    void onNetworkError(QNetworkReply::NetworkError error);

private:
    QNetworkAccessManager *m_networkManager;
    QString m_apiKey;
    // 全局代理参数
    QString m_proxyHost;
    quint16 m_proxyPort = 0;
    QString m_proxyUser;
    QString m_proxyPassword;
    QString m_caPath;
};

#endif // DEEPSEEKCLIENT_H
