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
    explicit Translator(QObject *parent = nullptr);
    ~Translator();

    // 翻译文本
    void translateText(const QString &text, const QString &fromLang, const QString &toLang);
    
    // 设置API密钥
    void setApiCredentials(const QString &appId, const QString &secretKey);
    
    // 检查API密钥是否已设置
    bool isApiConfigured() const;

signals:
    // 翻译完成信号
    void translationFinished(const QString &translatedText, const QString &detectedLang);
    
    // 翻译错误信号
    void translationError(const QString &errorMessage);

private slots:
    void onTranslationFinished();
    void onNetworkError(QNetworkReply::NetworkError error);

private:
    QNetworkAccessManager *m_networkManager;
    QString m_appId;
    QString m_secretKey;
    
    // 生成签名
    QString generateSignature(const QString &query, const QString &salt, const QString &appId, const QString &secretKey);
    
    // 解析翻译结果
    void parseTranslationResult(const QByteArray &data);
};

#endif // TRANSLATOR_H 