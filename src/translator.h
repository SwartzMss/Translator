#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

class Translator : public QObject
{
    Q_OBJECT

public:
    Translator(QObject *parent = nullptr);
    ~Translator();

    // 翻译文本
    void translateText(const QString &text, const QString &fromLang, const QString &toLang);
    
    // 设置API密钥
    void setApiCredentials(const QString &appId, const QString &secretKey);
    
    // 检查API密钥是否已设置
    bool isApiConfigured() const;

    // 设置网络代理
    void setNetworkProxy(const QString &host, quint16 port,
                         const QString &user = QString(),
                         const QString &password = QString());

    // 导入自定义CA证书
    void addCaCertificate(const QString &certPath);

    void setProxy(const QString &host, quint16 port, const QString &user = QString(), const QString &password = QString(), const QString &caPath = QString()) {
        m_proxyHost = host;
        m_proxyPort = port;
        m_proxyUser = user;
        m_proxyPassword = password;
        m_caPath = caPath;
    }

signals:
    // 翻译完成信号
    void translationFinished(const QString &translatedText, const QString &detectedLang);
    
    // 翻译错误信号
    void translationError(const QString &errorMessage);

private slots:
    void onTranslationFinished(QNetworkReply *reply);
    void onNetworkError(QNetworkReply::NetworkError error);

private:
    QNetworkAccessManager *m_networkManager;
    QString m_appId;
    QString m_secretKey;
    // 全局代理参数
    QString m_proxyHost;
    quint16 m_proxyPort = 0;
    QString m_proxyUser;
    QString m_proxyPassword;
    QString m_caPath;
    
    // 生成签名
    QString generateSignature(const QString &query, const QString &salt, const QString &appId, const QString &secretKey);
    
    // 解析翻译结果
    void parseTranslationResult(const QByteArray &data);
};

#endif // TRANSLATOR_H 