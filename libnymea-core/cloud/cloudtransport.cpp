#include "cloudtransport.h"
#include "loggingcategories.h"

#include "libnymea-remoteproxyclient/remoteproxyconnection.h"

using namespace remoteproxyclient;

namespace nymeaserver {

CloudTransport::CloudTransport(const ServerConfiguration &config, QObject *parent):
    TransportInterface(config, parent),
    m_remoteProxy(new RemoteProxyConnection(QUuid::createUuid(), "nymea:core", RemoteProxyConnection::ConnectionTypeWebSocket, this))
{

}

void CloudTransport::sendData(const QUuid &clientId, const QByteArray &data)
{
    qCDebug(dcCloud) << "Should send data" << clientId << data;
}

void CloudTransport::sendData(const QList<QUuid> &clientIds, const QByteArray &data)
{
    foreach (const QUuid &clientId, clientIds) {
        sendData(clientId, data);
    }
}

bool CloudTransport::startServer()
{
    qCDebug(dcCloud) << "Should start cloud server";
    return true;
}

bool CloudTransport::stopServer()
{
    qCDebug(dcCloud) << "Should stop cloud server";
    return true;
}

void CloudTransport::connectToCloud()
{
    qCDebug(dcCloud) << "Should connect to cloud";
    m_remoteProxy->connectServer(QHostAddress("127.0.0.1"), 1212);
}

}
