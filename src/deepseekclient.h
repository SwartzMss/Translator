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

signals:
    void polishFinished(const QString &polishedText);
    void polishError(const QString &errorMessage);

private slots:
    void onPolishFinished(QNetworkReply *reply);
    void onNetworkError(QNetworkReply::NetworkError error);

private:
    QNetworkAccessManager *m_networkManager;
    QString m_apiKey;
};

#endif // DEEPSEEKCLIENT_H
