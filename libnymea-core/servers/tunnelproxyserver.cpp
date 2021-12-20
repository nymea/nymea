/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2021, nymea GmbH
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

#include "tunnelproxyserver.h"
#include "loggingcategories.h"

NYMEA_LOGGING_CATEGORY(dcTunnelProxyServer, "TunnelProxyServer")

namespace nymeaserver {

TunnelProxyServer::TunnelProxyServer(const QString &serverName, const QUuid &serverUuid, const ServerConfiguration &configuration, QObject *parent) :
    TransportInterface(configuration, parent),
    m_serverName(serverName),
    m_serverUuid(serverUuid)
{
    // Note: the authentication must always be enabled on the tunnel proxy server
    if (!configuration.authenticationEnabled) {
        qCWarning(dcTunnelProxyServer()) << "=============================================================================================================================";
        qCWarning(dcTunnelProxyServer()) << "WARNING! The tunnel proxy server has authentication disabled! This is very dangerous and exposes your system to the internet.";
        qCWarning(dcTunnelProxyServer()) << "=============================================================================================================================";
    }

    // Default to ssl
    m_serverUrl.setScheme("ssl");
    m_serverUrl.setHost(configuration.address.toString());
    m_serverUrl.setPort(configuration.port);

    m_tunnelProxySocketServer = new TunnelProxySocketServer(m_serverUuid, m_serverName, this);
    connect(m_tunnelProxySocketServer, &TunnelProxySocketServer::stateChanged, this, &TunnelProxyServer::onStateChanged);
    connect(m_tunnelProxySocketServer, &TunnelProxySocketServer::runningChanged, this, &TunnelProxyServer::onServerRunningChanged);
    connect(m_tunnelProxySocketServer, &TunnelProxySocketServer::errorOccurred, this, &TunnelProxyServer::onErrorOccurred);
    connect(m_tunnelProxySocketServer, &TunnelProxySocketServer::serverErrorOccurred, this, &TunnelProxyServer::onServerErrorOccurred);
    connect(m_tunnelProxySocketServer, &TunnelProxySocketServer::sslErrors, this, &TunnelProxyServer::onSslErrors);
    connect(m_tunnelProxySocketServer, &TunnelProxySocketServer::clientConnected, this, &TunnelProxyServer::onClientConnected);
    connect(m_tunnelProxySocketServer, &TunnelProxySocketServer::clientDisconnected, this, &TunnelProxyServer::onClientDisconnected);

}

TunnelProxyServer::~TunnelProxyServer()
{
    stopServer();
}

void TunnelProxyServer::sendData(const QUuid &clientId, const QByteArray &data)
{
    TunnelProxySocket *tunnelProxySocket = m_clients.value(clientId);
    if (!tunnelProxySocket) {
        qCWarning(dcTunnelProxyServer()) << "Failed to send data to client" << clientId.toString() << "because there is no tunnel socket for this client UUID.";
        return;
    }

    // Note: we add a \n at the end of the data for easier json parsing on the other end
    tunnelProxySocket->writeData(data + '\n');
}

void TunnelProxyServer::sendData(const QList<QUuid> &clients, const QByteArray &data)
{
    foreach (const QUuid &client, clients) {
        sendData(client, data);
    }
}

void TunnelProxyServer::terminateClientConnection(const QUuid &clientId)
{
    TunnelProxySocket *tunnelProxySocket = m_clients.value(clientId);
    if (tunnelProxySocket) {
        tunnelProxySocket->disconnectSocket();
    }
}

void TunnelProxyServer::setServerName(const QString &serverName)
{
    m_serverName = serverName;
}

bool TunnelProxyServer::startServer()
{

    m_tunnelProxySocketServer->startServer(m_serverUrl);
    return true;
}

bool TunnelProxyServer::stopServer()
{
    qCDebug(dcTunnelProxyServer()) << "Stopping server";
    m_tunnelProxySocketServer->stopServer();
    return true;
}

void TunnelProxyServer::onStateChanged(TunnelProxySocketServer::State state)
{
    if (state == TunnelProxySocketServer::StateRegister) {
        qCDebug(dcTunnelProxyServer()) << "Connected with" << m_tunnelProxySocketServer->remoteProxyServer()
                                       << m_tunnelProxySocketServer->remoteProxyServerName()
                                       << m_tunnelProxySocketServer->remoteProxyServerVersion()
                                       << "API:" << m_tunnelProxySocketServer->remoteProxyApiVersion();
    }
}

void TunnelProxyServer::onServerRunningChanged(bool running)
{
    qCDebug(dcTunnelProxyServer()) << "The server is" << (running ? "now listening for incomming connections on " + m_serverUuid.toString() : "not running any more.");
    emit runningChanged(running);
}

void TunnelProxyServer::onErrorOccurred(QAbstractSocket::SocketError error)
{
    qCDebug(dcTunnelProxyServer()) << "Remote proxy connection error occurred" << error;
}

void TunnelProxyServer::onServerErrorOccurred(TunnelProxySocketServer::Error error)
{
    qCWarning(dcTunnelProxyServer()) << "Error occurred on server" << m_serverUrl.toString() << error;
}

void TunnelProxyServer::onSslErrors(const QList<QSslError> &errors)
{
    qCDebug(dcTunnelProxyServer()) << "Remote proxy connection SSL errors occurred" << errors;
    // FIXME: make this configurable
    m_tunnelProxySocketServer->ignoreSslErrors();
}

void TunnelProxyServer::onClientConnected(TunnelProxySocket *tunnelProxySocket)
{
    QUuid clientId = QUuid::createUuid();
    qCDebug(dcTunnelProxyServer()) << "Client connected:" << clientId.toString() << tunnelProxySocket->clientName() << "(Remote address:" << tunnelProxySocket->clientPeerAddress().toString() << ")";
    m_clients.insert(clientId, tunnelProxySocket);

    connect(tunnelProxySocket, &TunnelProxySocket::dataReceived, this, [this, clientId](const QByteArray &data){
        emit dataAvailable(clientId, data);
    });

    emit clientConnected(clientId);
}

void TunnelProxyServer::onClientDisconnected(TunnelProxySocket *tunnelProxySocket)
{
    QUuid clientId = m_clients.key(tunnelProxySocket);
    qCDebug(dcTunnelProxyServer()) << "Client disconnected:" << clientId.toString() << tunnelProxySocket->clientName() << "(Remote address:" << tunnelProxySocket->clientPeerAddress().toString() << ")";
    m_clients.remove(clientId);
    emit clientDisconnected(clientId);
}

}
