/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
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
    \class guhserver::TcpServer
    \brief This class represents the tcp server for guhd.

    \ingroup server
    \inmodule core

    \inherits TransportInterface

    The TCP server allows clients to connect to the JSON-RPC API.

    \sa WebSocketServer, TransportInterface
*/

#include "tcpserver.h"
#include "guhsettings.h"
#include "guhcore.h"

#include <QDebug>

namespace guhserver {

/*! Constructs a \l{TcpServer} with the given \a host, \a port and \a parent.
 *
 *  \sa ServerManager
 */
TcpServer::TcpServer(const QHostAddress &host, const uint &port, bool sslEnabled, const QSslConfiguration &sslConfiguration, QObject *parent) :
    TransportInterface(parent),
    m_server(NULL),
    m_host(host),
    m_port(port),
    m_sslEnabled(sslEnabled),
    m_sslConfig(sslConfiguration)
{       
#ifndef TESTING_ENABLED
    m_avahiService = new QtAvahiService(this);
    connect(m_avahiService, &QtAvahiService::serviceStateChanged, this, &TcpServer::onAvahiServiceStateChanged);
#endif
}

/*! Destructor of this \l{TcpServer}. */
TcpServer::~TcpServer()
{
    qCDebug(dcApplication) << "Shutting down \"TCP Server\"";
    stopServer();
}

/*! Sending \a data to a list of \a clients.*/
void TcpServer::sendData(const QList<QUuid> &clients, const QByteArray &data)
{
    foreach (const QUuid &client, clients) {
        sendData(client, data);
    }
}

/*! Sending \a data to the client with the given \a clientId.*/
void TcpServer::sendData(const QUuid &clientId, const QByteArray &data)
{
    QTcpSocket *client = 0;
    client = m_clientList.value(clientId);
    if (client) {
        qWarning() << "send data:" << data;
        client->write(data);
    }
}

void TcpServer::onClientConnected(QSslSocket *socket)
{
    qCDebug(dcConnection) << "Tcp server: new client connected:" << socket->peerAddress().toString();
    QUuid clientId = QUuid::createUuid();
    m_clientList.insert(clientId, socket);
    emit clientConnected(clientId);
}

void TcpServer::onClientDisconnected(QSslSocket *socket)
{
    qCDebug(dcConnection) << "Tcp server: client disconnected:" << socket->peerAddress().toString();
    QUuid clientId = m_clientList.key(socket);
    m_clientList.take(clientId);
    emit clientDisconnected(clientId);
}

void TcpServer::onError(QAbstractSocket::SocketError error)
{
    QTcpServer *server = qobject_cast<QTcpServer *>(sender());
    qCWarning(dcTcpServer) << server->serverAddress().toString() << "error:" << error << server->errorString();
    stopServer();
}

void TcpServer::onEncrypted()
{
    qCDebug(dcTcpServer) << "TCP Server connection encrypted";
}

void TcpServer::onDataAvailable(QSslSocket * socket, const QByteArray &data)
{
    QUuid clientId = m_clientList.key(socket);
    emit dataAvailable(clientId, data);
}

void TcpServer::onAvahiServiceStateChanged(const QtAvahiService::QtAvahiServiceState &state)
{
    if (state == QtAvahiService::QtAvahiServiceStateEstablished) {
        qCDebug(dcAvahi()) << "Service" << m_avahiService->name() << m_avahiService->serviceType() << "established successfully";
    }
}


/*! Returns true if this \l{TcpServer} could be reconfigured with the given \a address and \a port. */
bool TcpServer::reconfigureServer(const QHostAddress &address, const uint &port)
{
    if (m_host == address && m_port == (qint16)port && m_server->isListening())
        return true;

    stopServer();

    SslServer *server = new SslServer(m_sslEnabled, m_sslConfig);
    if(!server->listen(address, port)) {
        qCWarning(dcConnection) << "Tcp server error: can not listen on" << address.toString() << port;
        delete server;
        // Restart the server with the old configuration
        qCDebug(dcTcpServer()) << "Restart server with old configuration.";
        startServer();
        return false;
    }
    // Remove the test server..
    server->close();
    delete server;

    // Start server with new configuration
    m_host = address;
    m_port = port;
    return startServer();
}

/*! Returns true if this \l{TcpServer} started successfully.
 *
 * \sa TransportInterface::startServer()
 */
bool TcpServer::startServer()
{
    m_server = new SslServer(m_sslEnabled, m_sslConfig);
    if(!m_server->listen(m_host, m_port)) {
        qCWarning(dcConnection) << "Tcp server error: can not listen on" << m_host.toString() << m_port;
        delete m_server;
        m_server = NULL;
        return false;
    }
    qWarning() << "tcp listening";

#ifndef TESTING_ENABLED
    // Note: reversed order
    QHash<QString, QString> txt;
    txt.insert("jsonrpcVersion", JSON_PROTOCOL_VERSION);
    txt.insert("serverVersion", GUH_VERSION_STRING);
    txt.insert("manufacturer", "guh GmbH");
    txt.insert("uuid", GuhCore::instance()->configuration()->serverUuid().toString());
    txt.insert("name", GuhCore::instance()->configuration()->serverName());
    m_avahiService->registerService("guhIO", m_port, "_jsonrpc._tcp", txt);
#endif

    qCDebug(dcConnection) << "Started Tcp server on" << m_server->serverAddress().toString() << m_server->serverPort();
    connect(m_server, SIGNAL(clientConnected(QSslSocket *)), SLOT(onClientConnected(QSslSocket *)));
    connect(m_server, SIGNAL(clientDisconnected(QSslSocket *)), SLOT(onClientDisconnected(QSslSocket *)));
    connect(m_server, &SslServer::dataAvailable, this, &TcpServer::onDataAvailable);
    return true;
}

/*! Returns true if this \l{TcpServer} stopped successfully.
 *
 * \sa TransportInterface::startServer()
 */
bool TcpServer::stopServer()
{
#ifndef TESTING_ENABLED
    if (m_avahiService)
        m_avahiService->resetService();
#endif

    if (!m_server)
        return true;

    m_server->close();
    m_server->deleteLater();
    m_server = NULL;
    return true;
}

void SslServer::incomingConnection(qintptr socketDescriptor)
{
    qWarning() << "incoming";
    QSslSocket *sslSocket = new QSslSocket(this);

    connect(sslSocket, &QSslSocket::encrypted, [this, sslSocket](){ emit clientConnected(sslSocket); });
    connect(sslSocket, &QSslSocket::readyRead, this, &SslServer::onSocketReadyRead);
    connect(sslSocket, &QSslSocket::disconnected, this, &SslServer::onClientDisconnected);

    if (!sslSocket->setSocketDescriptor(socketDescriptor)) {
        qCWarning(dcConnection) << "Failed to set SSL socket descriptor.";
        delete sslSocket;
        return;
    }
    if (m_sslEnabled) {
        sslSocket->setSslConfiguration(m_config);
        sslSocket->startServerEncryption();
    } else {
        emit clientConnected(sslSocket);
    }
}

void SslServer::onClientDisconnected()
{
    QSslSocket *socket = static_cast<QSslSocket*>(sender());
    emit clientDisconnected(socket);
    socket->deleteLater();
}

void SslServer::onSocketReadyRead()
{
    QSslSocket *socket = static_cast<QSslSocket*>(sender());
    QByteArray data = socket->readAll();
    emit dataAvailable(socket, data);
}

}
