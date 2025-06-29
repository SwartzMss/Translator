#ifndef NETWORKPROXY_H
#define NETWORKPROXY_H

#include <QString>
#include <QNetworkAccessManager>

struct ProxySettings
{
    QString host;
    quint16 port = 0;
    QString user;
    QString password;
    QString certPath;
};

namespace NetworkProxy {
void applyProxy(QNetworkAccessManager *manager, const ProxySettings &settings);
bool loadFromEnvironment(ProxySettings &settings);
void installCaCertificate(const QString &path);
}

#endif // NETWORKPROXY_H
