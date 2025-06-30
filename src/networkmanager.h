#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>
#include <functional>
#include "tlshttpproxy.h"

class NetworkManager : public QObject
{
    Q_OBJECT
public:
    static NetworkManager* instance();

    void get(const QString &url, const QString &proxyHost, int proxyPort,
             const QString &user, const QString &password, const QString &caPath,
             std::function<void(bool, const QByteArray&)> callback);

    void post(const QString &url, const QByteArray &data, const QString &proxyHost, int proxyPort,
              const QString &user, const QString &password, const QString &caPath,
              std::function<void(bool, const QByteArray&)> callback);

private:
    explicit NetworkManager(QObject *parent = nullptr);
};

#endif // NETWORKMANAGER_H 