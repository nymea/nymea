#include "cloudtransport.h"
#include "loggingcategories.h"

using namespace remoteproxyclient;

namespace nymeaserver {

CloudTransport::CloudTransport(const ServerConfiguration &config, QObject *parent):
    TransportInterface(config, parent)
{
}

void CloudTransport::sendData(const QUuid &clientId, const QByteArray &data)
{
    qCDebug(dcCloud) << "Should send data" << clientId << data;
    foreach (const ConnectionContext &ctx, m_connections) {
        if (ctx.clientId == clientId) {
            ctx.proxyConnection->sendData(data);
            return;
        }
    }
    qCWarning(dcCloud()) << "Error sending data. No such clientId";
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

void CloudTransport::connectToCloud(const QString &token)
{
    qCDebug(dcCloud) << "Should connect to cloud";
    ConnectionContext context;
    context.clientId = QUuid::createUuid();
    context.token = token;
    context.proxyConnection = new RemoteProxyConnection(QUuid::createUuid(), "nymea:core", this);
    m_connections.insert(context.proxyConnection, context);

    connect(context.proxyConnection, &RemoteProxyConnection::ready, this, &CloudTransport::transportReady);
    connect(context.proxyConnection, &RemoteProxyConnection::stateChanged, this, &CloudTransport::remoteConnectionStateChanged);
    connect(context.proxyConnection, &RemoteProxyConnection::dataReady, this, &CloudTransport::transportDataReady);

    context.proxyConnection->connectServer(QUrl("wss://dev-remoteproxy.nymea.io"));
}

void CloudTransport::remoteConnectionStateChanged(RemoteProxyConnection::State state)
{
    qCDebug(dcCloud) << "Remote connection state changed" << state;
    RemoteProxyConnection *proxyConnection = qobject_cast<RemoteProxyConnection*>(sender());
    ConnectionContext context = m_connections.value(proxyConnection);

    if (state == RemoteProxyConnection::StateRemoteConnected) {
        emit clientConnected(context.clientId);
    } else if (state ==RemoteProxyConnection::StateDisconnected) {
        emit clientDisconnected(context.clientId);
    }
}

void CloudTransport::transportReady()
{
    qCDebug(dcCloud) << "Transport ready";
    RemoteProxyConnection *proxyConnection = qobject_cast<RemoteProxyConnection*>(sender());
    ConnectionContext context = m_connections.value(proxyConnection);

    context.proxyConnection->authenticate(context.token);
}

void CloudTransport::transportDataReady(const QByteArray &data)
{
    RemoteProxyConnection *proxyConnection = qobject_cast<RemoteProxyConnection*>(sender());
    ConnectionContext context = m_connections.value(proxyConnection);

    emit dataAvailable(context.clientId, data);
}

}
