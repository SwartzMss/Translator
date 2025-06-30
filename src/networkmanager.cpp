#include "networkmanager.h"

NetworkManager* NetworkManager::instance()
{
    static NetworkManager mgr;
    return &mgr;
}

NetworkManager::NetworkManager(QObject *parent) : QObject(parent) {}

void NetworkManager::get(const QString &url, const QString &proxyHost, int proxyPort,
                         const QString &user, const QString &password, const QString &caPath,
                         std::function<void(bool, const QByteArray&)> callback)
{
    TlsHttpProxy *proxy = new TlsHttpProxy(this);
    proxy->setProxy(proxyHost, proxyPort, user, password);
    if (!caPath.isEmpty())
        proxy->setCaCertificate(caPath);
    connect(proxy, &TlsHttpProxy::finished, this, [proxy, callback](bool ok, const QString &result) {
        callback(ok, result.toUtf8());
        proxy->deleteLater();
    });
    proxy->fetch(url);
}

void NetworkManager::post(const QString &url, const QByteArray &data, const QString &proxyHost, int proxyPort,
                          const QString &user, const QString &password, const QString &caPath,
                          std::function<void(bool, const QByteArray&)> callback)
{
    TlsHttpProxy *proxy = new TlsHttpProxy(this);
    proxy->setProxy(proxyHost, proxyPort, user, password);
    if (!caPath.isEmpty())
        proxy->setCaCertificate(caPath);
    connect(proxy, &TlsHttpProxy::finished, this, [proxy, callback](bool ok, const QString &result) {
        callback(ok, result.toUtf8());
        proxy->deleteLater();
    });
    proxy->post(url, data);
} 