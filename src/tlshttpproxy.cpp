#include "tlshttpproxy.h"
#include <curl/curl.h>
#include <QDateTime>
#include <QMetaObject>
#include <QUrl>
#include <QDebug>

TlsHttpProxy::TlsHttpProxy(QObject *parent)
    : QObject(parent)
    , m_timer(new QTimer(this))
{
    m_timer->setSingleShot(true);
    connect(m_timer, &QTimer::timeout, this, [this]() {
        if (m_connecting) {
            finishError(tr("连接超时"));
        }
    });
}

TlsHttpProxy::~TlsHttpProxy()
{
    cancel();
}

void TlsHttpProxy::setProxy(const QString &host, int port,
                            const QString &user,
                            const QString &password)
{
    m_proxyHost = host;
    m_proxyPort = port;
    m_proxyUser = user;
    m_proxyPass = password;
}

void TlsHttpProxy::setCaCertificate(const QString &path)
{
    m_caPath = path;
}

void TlsHttpProxy::appendDebug(const QString &msg)
{
    const QString stamped = QString("[%1] %2")
            .arg(QDateTime::currentDateTime().toString("hh:mm:ss.zzz"), msg);
    m_debugLines << stamped;
    emit debugMessage(stamped);
    qDebug() << stamped;
}

void TlsHttpProxy::finishError(const QString &msg)
{
    appendDebug("ERROR: " + msg);
    m_connecting = false;
    m_timer->stop();
    emit finished(false, msg);
}

void TlsHttpProxy::fetch(const QString &url)
{
    if (m_connecting)
        return;

    if (m_proxyHost.isEmpty() || m_proxyPort <= 0) {
        emit finished(false, tr("请填写有效的代理地址和端口"));
        return;
    }

    const QUrl u(url);
    if (!u.isValid() || u.host().isEmpty()) {
        emit finished(false, tr("无效的目标URL"));
        return;
    }

    m_targetUrl = url;
    m_connecting = true;
    m_headerBuf.clear();
    m_bodyBuf.clear();
    m_debugLines.clear();

    emit started();
    appendDebug(tr("开始连接流程 -> %1 via %2:%3").arg(m_targetUrl).arg(m_proxyHost).arg(m_proxyPort));

    m_timer->start(30000); // 30s timeout

    m_worker = QThread::create([this]() { perform(); });
    connect(m_worker, &QThread::finished, m_worker, &QObject::deleteLater);
    m_worker->start();
}

void TlsHttpProxy::cancel()
{
    m_connecting = false;
    m_timer->stop();
    if (m_worker) {
        m_worker->quit();
        m_worker->wait();
        m_worker = nullptr;
    }
}

size_t TlsHttpProxy::headerCallback(char *buffer, size_t size, size_t nitems, void *userdata)
{
    TlsHttpProxy *self = static_cast<TlsHttpProxy *>(userdata);
    QByteArray data(buffer, size * nitems);
    self->m_headerBuf.append(data);
    QMetaObject::invokeMethod(self, [self, data]() {
        self->appendDebug(QString::fromUtf8(data).trimmed());
    }, Qt::QueuedConnection);
    return size * nitems;
}

size_t TlsHttpProxy::writeCallback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    TlsHttpProxy *self = static_cast<TlsHttpProxy *>(userdata);
    self->m_bodyBuf.append(ptr, size * nmemb);
    return size * nmemb;
}

void TlsHttpProxy::perform()
{
    CURL *curl = curl_easy_init();
    if (!curl) {
        QMetaObject::invokeMethod(this, [this]() { finishError(tr("初始化curl失败")); }, Qt::QueuedConnection);
        return;
    }

    curl_easy_setopt(curl, CURLOPT_URL, m_targetUrl.toUtf8().constData());
    curl_easy_setopt(curl, CURLOPT_PROXY, m_proxyHost.toUtf8().constData());
    curl_easy_setopt(curl, CURLOPT_PROXYPORT, m_proxyPort);
    curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_HTTPS);
    curl_easy_setopt(curl, CURLOPT_HTTPPROXYTUNNEL, 1L);

    if (!m_proxyUser.isEmpty()) {
        QByteArray auth = QString("%1:%2").arg(m_proxyUser, m_proxyPass).toUtf8();
        curl_easy_setopt(curl, CURLOPT_PROXYUSERPWD, auth.constData());
    }

    if (!m_caPath.isEmpty()) {
        curl_easy_setopt(curl, CURLOPT_CAINFO, m_caPath.toUtf8().constData());
        curl_easy_setopt(curl, CURLOPT_PROXY_CAINFO, m_caPath.toUtf8().constData());
        appendDebug("使用CA证书: " + m_caPath);
        curl_easy_setopt(curl, CURLOPT_PROXY_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(curl, CURLOPT_PROXY_SSL_VERIFYHOST, 2L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        appendDebug("代理服务器SSL验证已启用，目标服务器SSL验证已禁用");
        curl_easy_setopt(curl, CURLOPT_SSL_OPTIONS, CURLSSLOPT_ALLOW_BEAST | CURLSSLOPT_NO_REVOKE | CURLSSLOPT_NO_PARTIALCHAIN);
        curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);
        curl_easy_setopt(curl, CURLOPT_PROXY_SSLVERSION, CURL_SSLVERSION_TLSv1_2);
    } else {
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        curl_easy_setopt(curl, CURLOPT_PROXY_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_PROXY_SSL_VERIFYHOST, 0L);
        appendDebug("警告: 未提供CA证书，SSL验证已禁用");
    }

    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, &TlsHttpProxy::headerCallback);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, this);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &TlsHttpProxy::writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);

    CURLcode res = curl_easy_perform(curl);
    long response = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response);
    curl_easy_cleanup(curl);

    QMetaObject::invokeMethod(this, [this, res, response]() {
        m_timer->stop();
        m_connecting = false;
        if (res != CURLE_OK) {
            QString errorMsg = QString::fromUtf8(curl_easy_strerror(res));
            appendDebug("CURL错误代码: " + QString::number(res));
            appendDebug("CURL错误描述: " + errorMsg);
            if (res == CURLE_SSL_CONNECT_ERROR || res == CURLE_SSL_CERTPROBLEM ||
                res == CURLE_PEER_FAILED_VERIFICATION) {
                errorMsg += "\n\n可能的解决方案:\n";
                errorMsg += "1. 检查CA证书文件是否正确\n";
                errorMsg += "2. 确认证书文件格式为PEM格式\n";
                errorMsg += "3. 验证证书是否与代理服务器匹配\n";
                errorMsg += "4. 尝试使用不同的SSL版本";
            }
            finishError(errorMsg);
            return;
        }
        if (response < 200 || response >= 300) {
            finishError(tr("HTTP 状态码 %1").arg(response));
            return;
        }
        QString result;
        result += "=== 连接成功 ===\n";
        result += QString("HTTP 状态 %1\n\n").arg(response);
        if (m_bodyBuf.startsWith("<!DOCTYPE") || m_bodyBuf.startsWith("<html")) {
            result += QString::fromUtf8(m_bodyBuf);
        } else {
            result += QString("[二进制内容, 前 128 字节十六进制]\n%1")
                          .arg(QString(m_bodyBuf.left(128).toHex(' ')));
        }
        emit finished(true, result);
    }, Qt::QueuedConnection);
}

