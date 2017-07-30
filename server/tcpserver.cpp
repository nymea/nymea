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
#include "loggingcategories.h"
#include "guhsettings.h"
#include "guhcore.h"

#include <QDebug>

namespace guhserver {

/*! Constructs a \l{TcpServer} with the given \a host, \a port and \a parent.
 *
 *  \sa ServerManager
 */
TcpServer::TcpServer(const QHostAddress &host, const uint &port, QObject *parent) :
    TransportInterface(parent),
    m_server(NULL),
    m_host(host),
    m_port(port)
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
        client->write(data);
    }
}

void TcpServer::onClientConnected()
{
    // got a new client connected
    QTcpServer *server = qobject_cast<QTcpServer*>(sender());
    QTcpSocket *newConnection = server->nextPendingConnection();
    qCDebug(dcConnection) << "Tcp server: new client connected:" << newConnection->peerAddress().toString();

    QUuid clientId = QUuid::createUuid();

    // append the new client to the client list
    m_clientList.insert(clientId, newConnection);

    connect(newConnection, SIGNAL(readyRead()),this,SLOT(readPackage()));
    connect(newConnection,SIGNAL(disconnected()),this,SLOT(onClientDisconnected()));

    emit clientConnected(clientId);
}

void TcpServer::readPackage()
{
    QTcpSocket *client = qobject_cast<QTcpSocket*>(sender());
    qCDebug(dcTcpServer) << "Data comming from" << client->peerAddress().toString();
    QByteArray message;
    while (client->canReadLine()) {
        QByteArray dataLine = client->readLine();
        qCDebug(dcTcpServer) << "Line in:" << dataLine;
        message.append(dataLine);
        if (dataLine.endsWith('\n')) {
            emit dataAvailable(m_clientList.key(client), message);
            message.clear();
        }
    }
}

void TcpServer::onClientDisconnected()
{
    QPointer<QTcpSocket> client = qobject_cast<QTcpSocket *>(sender());
    if (client.isNull())
        return;

    qCDebug(dcConnection) << "Tcp server: client disconnected:" << client->peerAddress().toString();
    QUuid clientId = m_clientList.key(client);
    m_clientList.take(clientId)->deleteLater();
}

void TcpServer::onError(QAbstractSocket::SocketError error)
{
    QTcpServer *server = qobject_cast<QTcpServer *>(sender());
    qCWarning(dcTcpServer) << server->serverAddress().toString() << "error:" << error << server->errorString();
    stopServer();
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

    QTcpServer *server = new QTcpServer(this);
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
    m_server = new QTcpServer(this);
    if(!m_server->listen(m_host, m_port)) {
        qCWarning(dcConnection) << "Tcp server error: can not listen on" << m_host.toString() << m_port;
        delete m_server;
        m_server = NULL;
        return false;
    }

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
    connect(m_server, SIGNAL(newConnection()), SLOT(onClientConnected()));
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

}
