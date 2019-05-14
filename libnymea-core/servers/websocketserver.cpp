/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 - 2018 Simon Stürz <simon.stuerz@guh.io>            *
 *                                                                         *
 *  This file is part of nymea.                                            *
 *                                                                         *
 *  nymea is free software: you can redistribute it and/or modify          *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  nymea is distributed in the hope that it will be useful,               *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with nymea. If not, see <http://www.gnu.org/licenses/>.          *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*!
    \class nymeaserver::WebSocketServer
    \brief This class represents the websocket server for nymead.

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

    You can turn on the \tt wss server in the \tt WebServerServer section of the \tt /etc/nymea/nymead.conf file.

    \note For \tt wss you need to have a certificate and configure it in the \tt SSL-configuration
    section of the \tt /etc/nymea/nymead.conf file.

    \sa WebServer, TcpServer, TransportInterface
*/

#include "nymeasettings.h"
#include "nymeacore.h"
#include "websocketserver.h"
#include "loggingcategories.h"

#include <QSslConfiguration>

namespace nymeaserver {

/*! Constructs a \l{WebSocketServer} with the given \a configuration, \a sslConfiguration and \a parent.
 *
 *  \sa ServerManager, ServerConfiguration
 */
WebSocketServer::WebSocketServer(const ServerConfiguration &configuration, const QSslConfiguration &sslConfiguration, QObject *parent) :
    TransportInterface(configuration, parent),
    m_sslConfiguration(sslConfiguration),
    m_enabled(false)
{
}

/*! Destructor of this \l{WebSocketServer}. */
WebSocketServer::~WebSocketServer()
{
    qCDebug(dcWebSocketServer()) << "Shutting down \"Websocket server\"" << serverUrl().toString();
    stopServer();
}

/*! Returns the url of this server. */
QUrl WebSocketServer::serverUrl() const
{
    return QUrl(QString("%1://%2:%3").arg((configuration().sslEnabled ? "wss" : "ws")).arg(configuration().address.toString()).arg(configuration().port));
}

/*! Send the given \a data map to the client with the given \a clientId.
 *
 * \sa TransportInterface::sendData()
 */
void WebSocketServer::sendData(const QUuid &clientId, const QByteArray &data)
{
    QWebSocket *client = nullptr;
    client = m_clientList.value(clientId);
    if (client) {
        qCDebug(dcWebSocketServerTraffic()) << "Sending data to client" << data;
        client->sendTextMessage(data + '\n');
    } else {
        qCWarning(dcWebSocketServer()) << "Client" << clientId << "unknown to this transport";
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

void WebSocketServer::terminateClientConnection(const QUuid &clientId)
{
    QWebSocket *client = m_clientList.value(clientId);
    if (client) {
        client->abort();
    }
}

void WebSocketServer::onClientConnected()
{
    // got a new client connected
    QWebSocket *client = m_server->nextPendingConnection();

    // check websocket version
    if (client->version() != QWebSocketProtocol::Version13) {
        qCWarning(dcWebSocketServer) << "Client with invalid protocol version" << client->version() << ". Rejecting.";
        client->close(QWebSocketProtocol::CloseCodeProtocolError, QString("invalid protocol version: %1 != Supported Version 13").arg(client->version()));
        delete client;
        return;
    }

    QUuid clientId = QUuid::createUuid();

    qCDebug(dcWebSocketServer()) << "New client connected:" << clientId.toString() << "(Remote address:" << client->peerAddress().toString() << ")";

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
    QUuid clientId = m_clientList.key(client);
    qCDebug(dcWebSocketServer()) << "Client" << clientId.toString() << "disconnected. (Remote address:" << client->peerAddress().toString() << ")" ;
    m_clientList.take(clientId)->deleteLater();
    emit clientDisconnected(clientId);
}

void WebSocketServer::onBinaryMessageReceived(const QByteArray &data)
{
    QWebSocket *client = qobject_cast<QWebSocket *>(sender());
    QUuid clientId = m_clientList.key(client);
    qCDebug(dcWebSocketServerTraffic()) << "Binary message from" << clientId.toString() << ":" << data;
}

void WebSocketServer::onTextMessageReceived(const QString &message)
{
    QWebSocket *client = qobject_cast<QWebSocket *>(sender());
    QUuid clientId = m_clientList.key(client);
    qCDebug(dcWebSocketServerTraffic()) << "Text message from" << clientId.toString() << ":" << message;
    emit dataAvailable(clientId, message.toUtf8());
}

void WebSocketServer::onClientError(QAbstractSocket::SocketError error)
{
    QWebSocket *client = qobject_cast<QWebSocket *>(sender());
    QUuid clientId = m_clientList.key(client);
    qCWarning(dcWebSocketServer()) << "Client error from" << clientId.toString() << ":" << error << client->errorString();
}

void WebSocketServer::onServerError(QAbstractSocket::SocketError error)
{
    qCWarning(dcWebSocketServer()) << "Server error " << error << m_server->errorString();
}

void WebSocketServer::onPing(quint64 elapsedTime, const QByteArray &payload)
{
    QWebSocket *client = qobject_cast<QWebSocket *>(sender());
    QUuid clientId = m_clientList.key(client);
    qCDebug(dcWebSocketServer) << "Ping response from" << clientId.toString() << elapsedTime << payload;
}


/*! Returns true if this \l{WebSocketServer} could be reconfigured with the given \a config. */
void WebSocketServer::reconfigureServer(const ServerConfiguration &config)
{
    if (configuration() == config && m_server->isListening()) {
        qCDebug(dcWebSocketServer()) << "Configuration unchanged. Not restarting the server.";
        return;
    }

    stopServer();
    qCDebug(dcWebSocketServer()) << "Stopped server for reconfiguration.";

    setConfiguration(config);

    // Start server with new configuration
    qCDebug(dcWebSocketServer()) << "Restart server with new configuration.";
    startServer();
}

/*! Sets the server name to the given \a serverName. */
void WebSocketServer::setServerName(const QString &serverName)
{
    m_serverName = serverName;
}

/*! Returns true if this \l{WebSocketServer} started successfully.
 *
 * \sa TransportInterface::startServer()
 */
bool WebSocketServer::startServer()
{
    if (configuration().sslEnabled) {
        m_server = new QWebSocketServer("nymea", QWebSocketServer::SecureMode, this);
        m_server->setSslConfiguration(m_sslConfiguration);
    } else {
        m_server = new QWebSocketServer("nymea", QWebSocketServer::NonSecureMode, this);
    }
    connect (m_server, &QWebSocketServer::newConnection, this, &WebSocketServer::onClientConnected);
    connect (m_server, &QWebSocketServer::acceptError, this, &WebSocketServer::onServerError);

    if (!m_server->listen(configuration().address, static_cast<quint16>(configuration().port))) {
        qCWarning(dcWebSocketServer()) << "Error listening on" << serverUrl().toString();
        return false;
    }

    qCDebug(dcWebSocketServer()) << "Server started on" << serverUrl().toString();
    return true;
}

/*! Returns true if this \l{WebSocketServer} stopped successfully.
 *
 * \sa TransportInterface::stopServer()
 */
bool WebSocketServer::stopServer()
{
    foreach (QWebSocket *client, m_clientList.values()) {
        client->close(QWebSocketProtocol::CloseCodeNormal, "Stop server");
    }

    if (m_server) {
        m_server->close();
        delete m_server;
        m_server = nullptr;
    }
    return true;
}

}
