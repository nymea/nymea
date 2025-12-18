// SPDX-License-Identifier: LGPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2025, chargebyte austria GmbH
*
* This file is part of nymea.
*
* nymea is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* as published by the Free Software Foundation, either version 3
* of the License, or (at your option) any later version.
*
* nymea is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with nymea. If not, see <https://www.gnu.org/licenses/>.
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "tunnelproxyserver.h"
#include "loggingcategories.h"

NYMEA_LOGGING_CATEGORY(dcTunnelProxyServer, "TunnelProxyServer")

namespace nymeaserver {

TunnelProxyServer::TunnelProxyServer(const QString &serverName, const QUuid &serverUuid, const TunnelProxyServerConfiguration &configuration, QObject *parent)
    : TransportInterface(configuration, parent)
    , m_tunnelProxyConfig(configuration)
    , m_serverName(serverName)
    , m_serverUuid(serverUuid)
{
    if (!configuration.authenticationEnabled) {
        qCWarning(dcTunnelProxyServer()) << "=====================================================================================================================";
        qCWarning(dcTunnelProxyServer()) << "WARNING! Tunnel proxy connection has authentication disabled! The system will be publicly accessible on the internet.";
        qCWarning(dcTunnelProxyServer()) << "=====================================================================================================================";
    }

    initConfiguration();

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

void TunnelProxyServer::setTunnelProxyConfig(const TunnelProxyServerConfiguration &tunnelProxyConfig)
{
    m_tunnelProxyConfig = tunnelProxyConfig;
}

bool TunnelProxyServer::startServer()
{
    initConfiguration();

    qCDebug(dcTunnelProxyServer()) << "Connecting to tunnel proxy server at:" << m_serverUrl;
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
        qCDebug(dcTunnelProxyServer()) << "Connected with" << m_tunnelProxySocketServer->remoteProxyServer() << m_tunnelProxySocketServer->remoteProxyServerName()
                                       << m_tunnelProxySocketServer->remoteProxyServerVersion() << "API:" << m_tunnelProxySocketServer->remoteProxyApiVersion();
    }
}

void TunnelProxyServer::onServerRunningChanged(bool running)
{
    qCDebug(dcTunnelProxyServer()).noquote() << "The server is" << (running ? "now listening for incoming connections on " + m_serverUuid.toString() : "not running any more.");
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
    if (m_tunnelProxyConfig.ignoreSslErrors) {
        qCWarning(dcTunnelProxyServer()) << "Ingoring SSL errors on tunnel proxy connection:" << errors;
        m_tunnelProxySocketServer->ignoreSslErrors();
    } else {
        qCWarning(dcTunnelProxyServer()) << "The remote proxy connection failed due to SSL errors:" << errors;
    }
}

void TunnelProxyServer::onClientConnected(TunnelProxySocket *tunnelProxySocket)
{
    QUuid clientId = QUuid::createUuid();
    qCDebug(dcTunnelProxyServer()) << "Client connected:" << clientId.toString() << tunnelProxySocket->clientName()
                                   << "(Remote address:" << tunnelProxySocket->clientPeerAddress().toString() << ")";
    m_clients.insert(clientId, tunnelProxySocket);

    connect(tunnelProxySocket, &TunnelProxySocket::dataReceived, this, [this, clientId](const QByteArray &data) { emit dataAvailable(clientId, data); });

    emit clientConnected(clientId);
}

void TunnelProxyServer::onClientDisconnected(TunnelProxySocket *tunnelProxySocket)
{
    QUuid clientId = m_clients.key(tunnelProxySocket);
    qCDebug(dcTunnelProxyServer()) << "Client disconnected:" << clientId.toString() << tunnelProxySocket->clientName()
                                   << "(Remote address:" << tunnelProxySocket->clientPeerAddress().toString() << ")";
    m_clients.remove(clientId);
    emit clientDisconnected(clientId);
}

void TunnelProxyServer::initConfiguration()
{
    qCDebug(dcTunnelProxyServer()) << "Init configuration" << m_tunnelProxyConfig;
    m_serverUrl.setScheme(m_tunnelProxyConfig.sslEnabled ? "ssl" : "tcp");
    m_serverUrl.setHost(m_tunnelProxyConfig.address);
    m_serverUrl.setPort(m_tunnelProxyConfig.port);
    qCDebug(dcTunnelProxyServer()) << "Using server URL" << m_serverUrl.toString();
}

} // namespace nymeaserver
