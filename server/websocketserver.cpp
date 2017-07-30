/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
 *                                                                         *
 *  This file is part of guh.                                              *
 *                                                                         *
 *  Guh is free software: you can redistribute it and/or modify            *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  Guh is distributed in the hope that it will be useful,                 *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with guh. If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*!
    \class guhserver::WebSocketServer
    \brief This class represents the websocket server for guhd.

    \ingroup server
    \inmodule core

    \note The WebSocketServer is only available for builds with Qt version greater than Qt 5.3.0!

    The websocket server provides a server for websocket clients based on
    \l{http://tools.ietf.org/html/rfc6455}{Protocol Version 13}. The default
    port for the websocket server is 4444, which is according to this
    \l{https://en.wikipedia.org/wiki/List_of_TCP_and_UDP_port_numbers}{list}
    officially free.

    The URL for the insecure websocket:
    \code ws://localhost:4444\endcode

    The URL for the secure websocket (TLS 1.2):
    \code wss://localhost:4444\endcode

    You can turn on the \tt wss server in the \tt WebServerServer section of the \tt /etc/guh/guhd.conf file.

    \note For \tt wss you need to have a certificate and configure it in the \tt SSL-configuration
    section of the \tt /etc/guh/guhd.conf file.

    \sa WebServer, TcpServer, TransportInterface
*/

#include "guhsettings.h"
#include "guhcore.h"
#include "websocketserver.h"
#include "loggingcategories.h"

#include <QSslConfiguration>

namespace guhserver {

/*! Constructs a \l{WebSocketServer} with the given \a address, \a port \a sslEnabled and \a parent.
 *
 *  \sa ServerManager
 */
WebSocketServer::WebSocketServer(const QHostAddress &address, const uint &port, const bool &sslEnabled, QObject *parent) :
    TransportInterface(parent),
    m_server(0),
    m_host(address),
    m_port(port),
    m_useSsl(sslEnabled),
    m_enabled(false)
{
#ifndef TESTING_ENABLED
    m_avahiService = new QtAvahiService(this);
    connect(m_avahiService, &QtAvahiService::serviceStateChanged, this, &WebSocketServer::onAvahiServiceStateChanged);
#endif
}

/*! Destructor of this \l{WebSocketServer}. */
WebSocketServer::~WebSocketServer()
{
    qCDebug(dcApplication) << "Shutting down \"Websocket server\"";
    stopServer();
}

/*! Send the given \a data map to the client with the given \a clientId.
 *
 * \sa TransportInterface::sendData()
 */
void WebSocketServer::sendData(const QUuid &clientId, const QByteArray &data)
{
    QWebSocket *client = 0;
    client = m_clientList.value(clientId);
    if (client) {
        client->sendTextMessage(data);
    }
}

/*! Send the given \a data map to the given list of \a clients.
 *
 * \sa TransportInterface::sendData()
 */
void WebSocketServer::sendData(const QList<QUuid> &clients, const QByteArray &data)
{
    foreach (const QUuid &client, clients) {
        sendData(client, data);
    }
}

void WebSocketServer::onClientConnected()
{
    // got a new client connected
    QWebSocket *client = m_server->nextPendingConnection();
    qCDebug(dcConnection) << "Websocket server: new client connected:" << client->peerAddress().toString();

    // check websocket version
    if (client->version() != QWebSocketProtocol::Version13) {
        qCWarning(dcWebSocketServer) << "Client with invalid protocol version" << client->version() << ". Rejecting.";
        client->close(QWebSocketProtocol::CloseCodeProtocolError, QString("invalid protocol version: %1 != Supported Version 13").arg(client->version()));
        delete client;
        return;
    }

    QUuid clientId = QUuid::createUuid();

    // append the new client to the client list
    m_clientList.insert(clientId, client);

    connect(client, SIGNAL(pong(quint64,QByteArray)), this, SLOT(onPing(quint64,QByteArray)));
    connect(client, SIGNAL(binaryMessageReceived(QByteArray)), this, SLOT(onBinaryMessageReceived(QByteArray)));
    connect(client, SIGNAL(textMessageReceived(QString)), this, SLOT(onTextMessageReceived(QString)));
    connect(client, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onClientError(QAbstractSocket::SocketError)));
    connect(client, SIGNAL(disconnected()), this, SLOT(onClientDisconnected()));

    emit clientConnected(clientId);
}

void WebSocketServer::onClientDisconnected()
{
    QWebSocket *client = qobject_cast<QWebSocket *>(sender());
    qCDebug(dcConnection) << "Websocket server: client disconnected:" << client->peerAddress().toString();
    QUuid clientId = m_clientList.key(client);
    m_clientList.take(clientId)->deleteLater();
}

void WebSocketServer::onBinaryMessageReceived(const QByteArray &data)
{
    QWebSocket *client = qobject_cast<QWebSocket *>(sender());
    qCDebug(dcWebSocketServer) << "Binary message from" << client->peerAddress().toString() << ":" << data;
}

void WebSocketServer::onTextMessageReceived(const QString &message)
{
    QWebSocket *client = qobject_cast<QWebSocket *>(sender());
    qCDebug(dcWebSocketServer) << "Text message from" << client->peerAddress().toString() << ":" << message;
    emit dataAvailable(m_clientList.key(client), message.toUtf8());
}

void WebSocketServer::onClientError(QAbstractSocket::SocketError error)
{
    QWebSocket *client = qobject_cast<QWebSocket *>(sender());
    qCWarning(dcConnection) << "Websocket client error:" << error << client->errorString();
}

void WebSocketServer::onServerError(QAbstractSocket::SocketError error)
{
    qCWarning(dcConnection) << "Websocket server error:" << error << m_server->errorString();
}

void WebSocketServer::onPing(quint64 elapsedTime, const QByteArray &payload)
{
    QWebSocket *client = qobject_cast<QWebSocket *>(sender());
    qCDebug(dcWebSocketServer) << "ping response" << client->peerAddress() << elapsedTime << payload;
}

void WebSocketServer::onAvahiServiceStateChanged(const QtAvahiService::QtAvahiServiceState &state)
{
    if (state == QtAvahiService::QtAvahiServiceStateEstablished) {
        qCDebug(dcAvahi()) << "Service" << m_avahiService->name() << m_avahiService->serviceType() << "established successfully";
    }
}

/*! Returns true if this \l{WebSocketServer} could be reconfigured with the given \a address and \a port. */
bool WebSocketServer::reconfigureServer(const QHostAddress &address, const uint &port)
{
    if (m_host == address && m_port == (qint16)port && m_server->isListening()) {
        qCDebug(dcWebSocketServer()) << "Configuration unchanged. Not restarting the server.";
        return true;
    }

    stopServer();
    qCDebug(dcWebSocketServer()) << "Stopped server for reconfiguration.";

    QWebSocketServer *server;
    if (m_useSsl) {
        server = new QWebSocketServer("guh", QWebSocketServer::SecureMode, this);
        server->setSslConfiguration(m_sslConfiguration);
    } else {
        server = new QWebSocketServer("guh", QWebSocketServer::NonSecureMode, this);
    }

    if(!server->listen(address, port)) {
        qCWarning(dcConnection) << "Websocket server error: can not listen on" << address.toString() << port;
        delete server;
        // Restart the server with the old configuration
        qCDebug(dcWebSocketServer()) << "Restart server with old configuration.";
        startServer();
        return false;
    }
    // Remove the test server..
    server->close();
    delete server;

    // Start server with new configuration
    m_host = address;
    m_port = port;
    qCDebug(dcWebSocketServer()) << "Restart server with new configuration.";
    return startServer();
}

/*! Returns true if this \l{WebSocketServer} started successfully.
 *
 * \sa TransportInterface::startServer()
 */
bool WebSocketServer::startServer()
{
    if (m_useSsl) {
        m_server = new QWebSocketServer("guh", QWebSocketServer::SecureMode, this);
        m_server->setSslConfiguration(m_sslConfiguration);
    } else {
        m_server = new QWebSocketServer("guh", QWebSocketServer::NonSecureMode, this);
    }
    connect (m_server, &QWebSocketServer::newConnection, this, &WebSocketServer::onClientConnected);
    connect (m_server, &QWebSocketServer::acceptError, this, &WebSocketServer::onServerError);

    if (!m_server->listen(m_host, m_port)) {
        qCWarning(dcConnection) << "Websocket server" << m_server->serverName() << QString("could not listen on %1:%2").arg(m_server->serverAddress().toString()).arg(m_port);
        return false;
    }

    if (m_server->secureMode() == QWebSocketServer::NonSecureMode) {
        qCDebug(dcConnection) << "Started websocket server" << m_server->serverName() << QString("on ws://%1:%2").arg(m_server->serverAddress().toString()).arg(m_port);
    } else {
        qCDebug(dcConnection) << "Started websocket server" << m_server->serverName() << QString("on wss://%1:%2").arg(m_server->serverAddress().toString()).arg(m_port);
    }

#ifndef TESTING_ENABLED
    // Note: reversed order
    QHash<QString, QString> txt;
    txt.insert("sslEnabled", GuhCore::instance()->configuration()->sslEnabled() ? "true" : "false");
    txt.insert("jsonrpcVersion", JSON_PROTOCOL_VERSION);
    txt.insert("serverVersion", GUH_VERSION_STRING);
    txt.insert("manufacturer", "guh GmbH");
    txt.insert("uuid", GuhCore::instance()->configuration()->serverUuid().toString());
    txt.insert("name", GuhCore::instance()->configuration()->serverName());
    m_avahiService->registerService("guhIO", m_port, "_ws._tcp", txt);
#endif

    return true;
}

/*! Returns true if this \l{WebSocketServer} stopped successfully.
 *
 * \sa TransportInterface::stopServer()
 */
bool WebSocketServer::stopServer()
{
#ifndef TESTING_ENABLED
    if (m_avahiService)
        m_avahiService->resetService();
#endif

    foreach (QWebSocket *client, m_clientList.values()) {
        client->close(QWebSocketProtocol::CloseCodeNormal, "Stop server");
    }

    m_server->close();
    delete m_server;
    m_server = 0;
    return true;
}

}
