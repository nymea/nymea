#ifndef CLOUDTRANSPORT_H
#define CLOUDTRANSPORT_H

#include <QObject>
#include "../transportinterface.h"

namespace remoteproxyclient {
    class RemoteProxyConnection;
}
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
    void connectToCloud();

private:
    remoteproxyclient::RemoteProxyConnection *m_remoteProxy = nullptr;
};

}

#endif // CLOUDTRANSPORT_H
