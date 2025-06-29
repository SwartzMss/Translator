#include "networkproxy.h"
#include "logger.h"
#include <QNetworkProxy>
#include <QUrl>
#include <QFile>
#include <QSslCertificate>
#include <QSslConfiguration>

void NetworkProxy::applyProxy(QNetworkAccessManager *manager, const ProxySettings &settings)
{
    if (!manager)
        return;

    if (settings.host.isEmpty() || settings.port == 0) {
        manager->setProxy(QNetworkProxy());
        LOG_INFO("取消网络代理设置");
    } else {
        QNetworkProxy proxy(QNetworkProxy::HttpProxy,
                            settings.host,
                            settings.port,
                            settings.user,
                            settings.password);
        manager->setProxy(proxy);
        LOG_INFO(QString("设置网络代理 - %1:%2").arg(settings.host).arg(settings.port));
    }

    if (!settings.certPath.isEmpty()) {
        NetworkProxy::installCaCertificate(settings.certPath);
    }
}

bool NetworkProxy::loadFromEnvironment(ProxySettings &settings)
{
    QByteArray val = qgetenv("EASYPROXY_URL");
    if (val.isEmpty())
        val = qgetenv("HTTP_PROXY");
    if (val.isEmpty())
        val = qgetenv("http_proxy");
    if (val.isEmpty())
        return false;

    QUrl url(QString::fromUtf8(val));
    if (!url.isValid() || url.host().isEmpty() || url.port() <= 0) {
        LOG_WARNING(QString("忽略无效的代理环境变量: %1").arg(QString::fromUtf8(val)));
        return false;
    }

    settings.host = url.host();
    settings.port = static_cast<quint16>(url.port());
    settings.user = url.userName();
    settings.password = url.password();
    return true;
}

void NetworkProxy::installCaCertificate(const QString &path)
{
    if (path.isEmpty())
        return;

    if (!QFile::exists(path)) {
        LOG_WARNING(QString("证书文件不存在: %1").arg(path));
        return;
    }

    const QList<QSslCertificate> certs = QSslCertificate::fromPath(path);
    if (certs.isEmpty()) {
        LOG_WARNING(QString("加载证书失败: %1").arg(path));
        return;
    }

    QSslConfiguration sslConfig = QSslConfiguration::defaultConfiguration();
    QList<QSslCertificate> caCerts = sslConfig.caCertificates();
    caCerts.append(certs);
    sslConfig.setCaCertificates(caCerts);
    QSslConfiguration::setDefaultConfiguration(sslConfig);

    LOG_INFO(QString("导入CA证书: %1").arg(path));
}
