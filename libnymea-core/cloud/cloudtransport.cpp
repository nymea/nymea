/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2020, nymea GmbH
* Contact: contact@nymea.io
*
* This file is part of nymea.
* This project including source code and documentation is protected by
* copyright law, and remains the property of nymea GmbH. All rights, including
* reproduction, publication, editing and translation, are reserved. The use of
* this project is subject to the terms of a license agreement to be concluded
* with nymea GmbH in accordance with the terms of use of nymea GmbH, available
* under https://nymea.io/license
*
* GNU General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU General Public License as published by the Free Software
* Foundation, GNU version 3. This project is distributed in the hope that it
* will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
* of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
* Public License for more details.
*
* You should have received a copy of the GNU General Public License along with
* this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "cloudtransport.h"
#include "loggingcategories.h"

#include "nymeacore.h"

using namespace remoteproxyclient;

namespace nymeaserver {

CloudTransport::CloudTransport(const ServerConfiguration &config, QObject *parent):
    TransportInterface(config, parent)
{
    m_defaultProxyUrl = "ssl://remoteproxy.nymea.io:4433";
}

void CloudTransport::sendData(const QUuid &clientId, const QByteArray &data)
{
    qCDebug(dcCloudTraffic()) << "Sending data" << clientId << data;
    foreach (const ConnectionContext &ctx, m_connections) {
        if (ctx.clientId == clientId) {
            ctx.proxyConnection->sendData(data + '\n');
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

void CloudTransport::terminateClientConnection(const QUuid &clientId)
{
    foreach (const ConnectionContext &ctx, m_connections) {
        if (ctx.clientId == clientId) {
            ctx.proxyConnection->disconnectServer();
            return;
        }
    }
}

bool CloudTransport::startServer()
{
    qCDebug(dcCloud()) << "Started cloud transport";
    return true;
}

bool CloudTransport::stopServer()
{
    qCDebug(dcCloud()) << "Stopped cloud transport";
    return true;
}

void CloudTransport::connectToCloud(const QString &token, const QString &nonce, const QString &serverUrl)
{
    QString proxyUrl = serverUrl.isEmpty() ? m_defaultProxyUrl : serverUrl;
    qCDebug(dcCloud()) << "Connecting to remote proxy server" << proxyUrl;

    RemoteProxyConnection::ConnectionType connectionType = RemoteProxyConnection::ConnectionTypeTcpSocket;
    QUrl url(proxyUrl);
    if (url.scheme() == "wss" || url.scheme() == "ws") {
        connectionType = RemoteProxyConnection::ConnectionTypeWebSocket;
    }

    ConnectionContext context;
    context.clientId = QUuid::createUuid();
    context.token = token;
    context.nonce = nonce;
    QString identifier = QString("nymea:core (%1)").arg(NymeaCore::instance()->configuration()->serverName());
    context.proxyConnection = new RemoteProxyConnection(NymeaCore::instance()->configuration()->serverUuid().toString(), identifier, connectionType, this);
    m_connections.insert(context.proxyConnection, context);

    connect(context.proxyConnection, &RemoteProxyConnection::ready, this, &CloudTransport::transportReady);
    connect(context.proxyConnection, &RemoteProxyConnection::stateChanged, this, &CloudTransport::remoteConnectionStateChanged);
    connect(context.proxyConnection, &RemoteProxyConnection::dataReady, this, &CloudTransport::transportDataReady);
    connect(context.proxyConnection, &RemoteProxyConnection::remoteConnectionEstablished, this, &CloudTransport::transportConnected);
    connect(context.proxyConnection, &RemoteProxyConnection::disconnected, this, &CloudTransport::transportDisconnected);

    context.proxyConnection->connectServer(url);
}

void CloudTransport::remoteConnectionStateChanged(RemoteProxyConnection::State state)
{
    qCDebug(dcCloudTraffic()) << "Remote connection state changed" << state;
}

void CloudTransport::transportConnected()
{
    RemoteProxyConnection *proxyConnection = qobject_cast<RemoteProxyConnection*>(sender());
    ConnectionContext context = m_connections.value(proxyConnection);

    qCDebug(dcCloud()) << "The remote client connected successfully" << proxyConnection->tunnelPartnerName() << proxyConnection->tunnelPartnerUuid();
    emit clientConnected(context.clientId);
}

void CloudTransport::transportDisconnected()
{
    RemoteProxyConnection *proxyConnection = qobject_cast<RemoteProxyConnection*>(sender());
    ConnectionContext context = m_connections.take(proxyConnection);
    proxyConnection->deleteLater();

    qCDebug(dcCloud()) << "The remote connection disconnected." << context.clientId;
    emit clientDisconnected(context.clientId);
}

void CloudTransport::transportReady()
{
    RemoteProxyConnection *proxyConnection = static_cast<RemoteProxyConnection *>(sender());
    qCDebug(dcCloud()) << "Connected successfully to remote proxy server" << proxyConnection->proxyServerName() <<
                          proxyConnection->proxyServerVersion() << "API version:" << proxyConnection->proxyServerApiVersion();

    ConnectionContext context = m_connections.value(proxyConnection);
    context.proxyConnection->authenticate(context.token, context.nonce);
}

void CloudTransport::transportDataReady(const QByteArray &data)
{
    RemoteProxyConnection *proxyConnection = qobject_cast<RemoteProxyConnection*>(sender());
    ConnectionContext context = m_connections.value(proxyConnection);
    qCDebug(dcCloudTraffic()) << "Data received:" << context.clientId.toString() << data;
    emit dataAvailable(context.clientId, data);
}

}
