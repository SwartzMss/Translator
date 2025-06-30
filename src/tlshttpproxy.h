#ifndef TLSHTTPPROXY_H
#define TLSHTTPPROXY_H

#include <QObject>
#include <QThread>
#include <QTimer>
#include <QStringList>
#include <QByteArray>

class TlsHttpProxy : public QObject
{
    Q_OBJECT
public:
    explicit TlsHttpProxy(QObject *parent = nullptr);
    ~TlsHttpProxy();

    void setProxy(const QString &host, int port,
                  const QString &user = QString(),
                  const QString &password = QString());
    void setCaCertificate(const QString &path);

    void fetch(const QString &url);
    void post(const QString &url, const QByteArray &data, const QString &contentType = "application/json");
    void cancel();

signals:
    void started();
    void finished(bool success, const QString &message);
    void debugMessage(const QString &msg);

private:
    static size_t headerCallback(char *buffer, size_t size, size_t nitems, void *userdata);
    static size_t writeCallback(char *ptr, size_t size, size_t nmemb, void *userdata);
    void perform();
    void appendDebug(const QString &msg);
    void finishError(const QString &msg);

    QString m_proxyHost;
    int m_proxyPort = 0;
    QString m_proxyUser;
    QString m_proxyPass;
    QString m_caPath;

    QString m_targetUrl;
    QByteArray m_headerBuf;
    QByteArray m_bodyBuf;
    QStringList m_debugLines;

    bool m_connecting = false;
    QThread *m_worker = nullptr;
    QTimer *m_timer;

    QByteArray m_postData;
    QString m_contentType;
    bool m_isPost = false;
};

#endif // TLSHTTPPROXY_H
