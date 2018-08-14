#ifndef CLOUDTRANSPORT_H
#define CLOUDTRANSPORT_H

#include <QObject>
#include "../transportinterface.h"
#include "libnymea-remoteproxyclient/remoteproxyconnection.h"

namespace nymeaserver {

class CloudTransport : public TransportInterface
{
    Q_OBJECT
public:
    explicit CloudTransport(const ServerConfiguration &config, QObject *parent = nullptr);

    void sendData(const QUuid &clientId, const QByteArray &data) override;
    void sendData(const QList<QUuid> &clientIds, const QByteArray &data) override;

    bool startServer() override;
    bool stopServer() override;
signals:

public slots:
    void connectToCloud(const QString &token);
    void remoteConnectionStateChanged(remoteproxyclient::RemoteProxyConnection::State state);

private slots:
    void transportReady();
    void transportDataReady(const QByteArray &data);

private:
    class ConnectionContext {
    public:
        QUuid clientId;
        QString token;
        remoteproxyclient::RemoteProxyConnection* proxyConnection;
    };
    QHash<remoteproxyclient::RemoteProxyConnection*, ConnectionContext> m_connections;

};

}

#endif // CLOUDTRANSPORT_H
