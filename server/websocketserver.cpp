/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stuerz <simon.stuerz@guh.guru>                *
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

#include "websocketserver.h"
#include "guhsettings.h"
#include "loggingcategories.h"

#include <QJsonDocument>
#include <QSslConfiguration>

namespace guhserver {

/*! Constructs a \l{WebSocketServer} with the given \a sslConfiguration and \a parent.
 *
 *  \sa ServerManager
 */
WebSocketServer::WebSocketServer(const QSslConfiguration &sslConfiguration, QObject *parent) :
    TransportInterface(parent),
    m_server(0),
    m_sslConfiguration(sslConfiguration),
    m_useSsl(false),
    m_enabled(false)
{
    // load webserver settings
    GuhSettings settings(GuhSettings::SettingsRoleGlobal);
    qCDebug(dcWebServer) << "Loading webserver settings from" << settings.fileName();

    settings.beginGroup("WebSocketServer");
    // 4444 Official free according to https://en.wikipedia.org/wiki/List_of_TCP_and_UDP_port_numbers
    m_port = settings.value("port", 4444).toInt();
    m_useSsl = settings.value("https", false).toBool();
    settings.endGroup();

    // check SSL
    if (m_useSsl && m_sslConfiguration.isNull())
        m_useSsl = false;
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
void WebSocketServer::sendData(const QUuid &clientId, const QVariantMap &data)
{
    QWebSocket *client = 0;
    client = m_clientList.value(clientId);
    if (client) {
        client->sendTextMessage(QJsonDocument::fromVariant(data).toJson());
    }
}

/*! Send the given \a data map to the given list of \a clients.
 *
 * \sa TransportInterface::sendData()
 */
void WebSocketServer::sendData(const QList<QUuid> &clients, const QVariantMap &data)
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
    validateMessage(m_clientList.key(client), message.toUtf8());
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

/*! Returns true if this \l{WebSocketServer} started successfully.
 *
 * \sa TransportInterface::startServer()
 */
bool WebSocketServer::startServer()
{
    if (m_server) {
        qCWarning(dcConnection) << "There is allready a websocket server instance. This should never happen!!! Please report this bug!";
        return false;
    }

    if (m_useSsl) {
        m_server = new QWebSocketServer("guh", QWebSocketServer::SecureMode, this);
        m_server->setSslConfiguration(m_sslConfiguration);
    } else {
        m_server = new QWebSocketServer("guh", QWebSocketServer::NonSecureMode, this);
    }
    connect (m_server, &QWebSocketServer::newConnection, this, &WebSocketServer::onClientConnected);
    connect (m_server, &QWebSocketServer::acceptError, this, &WebSocketServer::onServerError);

    if (!m_server->listen(QHostAddress::Any, m_port)) {
        qCWarning(dcConnection) << "Websocket server" << m_server->serverName() << QString("could not listen on %1:%2").arg(m_server->serverAddress().toString()).arg(m_port);
        return false;
    }

    if (m_server->secureMode() == QWebSocketServer::NonSecureMode) {
        qCDebug(dcConnection) << "Started websocket server" << m_server->serverName() << QString("on ws://%1:%2").arg(m_server->serverAddress().toString()).arg(m_port);
    } else {
        qCDebug(dcConnection) << "Started websocket server" << m_server->serverName() << QString("on wss://%1:%2").arg(m_server->serverAddress().toString()).arg(m_port);
    }
    return true;
}

/*! Returns true if this \l{WebSocketServer} stopped successfully.
 *
 * \sa TransportInterface::stopServer()
 */
bool WebSocketServer::stopServer()
{
    m_server->close();
    delete m_server;
    m_server = 0;
    return true;
}

}
