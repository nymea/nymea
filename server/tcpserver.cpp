/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stuerz <simon.stuerz@guh.guru>                *
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

    The TCP server allowes clients to connect to the JSON-RPC API.

    \sa WebSocketServer, TransportInterface
*/

#include "tcpserver.h"
#include "loggingcategories.h"
#include "guhsettings.h"
#include "guhcore.h"
#include "jsonrpcserver.h"

#include <QDebug>
#include <QJsonDocument>

namespace guhserver {

/*! Constructs a \l{TcpServer} with the given \a parent.
 *
 *  \sa ServerManager
 */
TcpServer::TcpServer(QObject *parent) :
    TransportInterface(parent)
{       
    // Timer for scanning network interfaces ever 5 seconds
    // Note: QNetworkConfigurationManager does not work on embedded platforms
    m_timer = new QTimer(this);
    m_timer->setSingleShot(false);
    m_timer->setInterval(5000);
    connect (m_timer, &QTimer::timeout, this, &TcpServer::onTimeout);

    // load JSON-RPC server settings
    GuhSettings settings(GuhSettings::SettingsRoleGlobal);
    qCDebug(dcTcpServer) << "Loading Tcp server settings from" << settings.fileName();
    settings.beginGroup("JSONRPC");

    // load port
    // 2222 Official free according to https://en.wikipedia.org/wiki/List_of_TCP_and_UDP_port_numbers
    m_port = settings.value("port", 2222).toUInt();

    // load interfaces
    QStringList interfaceList = settings.value("interfaces", QStringList("all")).toStringList();
    if (interfaceList.contains("all")) {
        m_networkInterfaces = QNetworkInterface::allInterfaces();
    } else {
        foreach (const QNetworkInterface &interface, QNetworkInterface::allInterfaces()) {
            if (interfaceList.contains(interface.name())) {
                m_networkInterfaces.append(interface);
            }
        }
    }

    // load IP versions (IPv4, IPv6 or any)
    m_ipVersions = settings.value("ip", QStringList("any")).toStringList();
    settings.endGroup();
}

/*! Destructor of this \l{TcpServer}. */
TcpServer::~TcpServer()
{
    qCDebug(dcApplication) << "Shutting down \"TCP Server\"";
    stopServer();
}

/*! Sending \a data to a list of \a clients.*/
void TcpServer::sendData(const QList<QUuid> &clients, const QVariantMap &data)
{
    foreach (const QUuid &client, clients) {
        sendData(client, data);
    }
}

/*! Sending \a data to the client with the given \a clientId.*/
void TcpServer::sendData(const QUuid &clientId, const QVariantMap &data)
{
    QTcpSocket *client = 0;
    client = m_clientList.value(clientId);
    if (client) {
        client->write(QJsonDocument::fromVariant(data).toJson());
    }
}

void TcpServer::reloadNetworkInterfaces()
{
    GuhSettings settings(GuhSettings::SettingsRoleGlobal);
    settings.beginGroup("JSONRPC");

    // reload network interfaces
    m_networkInterfaces.clear();

    QStringList interfaceList = settings.value("interfaces", QStringList("all")).toStringList();
    if (interfaceList.contains("all")) {
        m_networkInterfaces = QNetworkInterface::allInterfaces();
    } else {
        foreach (const QNetworkInterface &interface, QNetworkInterface::allInterfaces()) {
            if (interfaceList.contains(interface.name())) {
                m_networkInterfaces.append(interface);
            }
        }
    }
    settings.endGroup();
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
    qCDebug(dcTcpServer) << "data comming from" << client->peerAddress().toString();
    QByteArray message;
    while (client->canReadLine()) {
        QByteArray dataLine = client->readLine();
        qCDebug(dcTcpServer) << "line in:" << dataLine;
        message.append(dataLine);
        if (dataLine.endsWith('\n')) {
            validateMessage(m_clientList.key(client), message);
            message.clear();
        }
    }
}

void TcpServer::onClientDisconnected()
{
    QTcpSocket *client = qobject_cast<QTcpSocket*>(sender());
    qCDebug(dcConnection) << "Tcp server: client disconnected:" << client->peerAddress().toString();
    QUuid clientId = m_clientList.key(client);
    m_clientList.take(clientId)->deleteLater();
}

void TcpServer::onError(QAbstractSocket::SocketError error)
{
    QTcpServer *server = qobject_cast<QTcpServer *>(sender());
    QUuid uuid = m_serverList.key(server);
    qCWarning(dcTcpServer) << "Tcp server" << server->serverAddress().toString() << "error:" << error << server->errorString();
    qCWarning(dcTcpServer) << "shutting down Tcp server" << server->serverAddress().toString();

    server->close();
    m_serverList.take(uuid)->deleteLater();
}

void TcpServer::onTimeout()
{
    bool ipV4 = m_ipVersions.contains("IPv4");
    bool ipV6 = m_ipVersions.contains("IPv6");
    if (m_ipVersions.contains("any")) {
        ipV4 = ipV6 = true;
    }

    reloadNetworkInterfaces();

    // get all available host addresses
    QList<QHostAddress> allAddresses;
    foreach (const QNetworkInterface &interface, m_networkInterfaces) {
        QList<QNetworkAddressEntry> addresseEntries = interface.addressEntries();

        foreach (QNetworkAddressEntry entry, addresseEntries) {
            if (ipV4 && entry.ip().protocol() == QAbstractSocket::IPv4Protocol) {
                allAddresses.append(entry.ip());
            }
            if (ipV6 && entry.ip().protocol() == QAbstractSocket::IPv6Protocol) {
                allAddresses.append(entry.ip());
            }
        }
    }

    // check if a new server should be created
    QList<QHostAddress> serversToCreate;
    QList<QTcpServer *> serversToDelete;

    foreach (const QHostAddress &address, allAddresses) {
        // check if there is a server for this host address
        bool found = false;
        foreach (QTcpServer *s, m_serverList) {
            if (s->serverAddress() == address) {
                found = true;
                break;
            }
        }
        if (!found)
            serversToCreate.append(address);
    }

    // delete every server which has no longer an interface
    foreach (QTcpServer *s, m_serverList) {
        if (!allAddresses.contains(s->serverAddress())) {
            qCDebug(dcConnection) << "Closing Tcp server on" << s->serverAddress().toString() << s->serverPort();
            QUuid uuid = m_serverList.key(s);
            s->close();
            m_serverList.take(uuid)->deleteLater();
        }

    }

    foreach(const QHostAddress &address, serversToCreate){
        QTcpServer *server = new QTcpServer(this);
        if(server->listen(address, m_port)) {
            qCDebug(dcConnection) << "Started TCP server on" << address.toString() << m_port;
            connect(server, &QTcpServer::newConnection, this, &TcpServer::onClientConnected);
            m_serverList.insert(QUuid::createUuid(), server);
        } else {
            qCWarning(dcConnection) << "Tcp server error: can not listen on" << address.toString() << m_port;
            delete server;
        }
    }

    if (!serversToCreate.isEmpty() || !serversToDelete.isEmpty()) {
        qCDebug(dcConnection) << "Current server list:";
        foreach (QTcpServer *s, m_serverList) {
            qCDebug(dcConnection) << "  - Tcp server on" << s->serverAddress().toString() << s->serverPort();
        }
    }
}

/*! Returns true if this \l{TcpServer} started successfully.
 *
 * \sa TransportInterface::startServer()
 */
bool TcpServer::startServer()
{
    bool ipV4 = m_ipVersions.contains("IPv4");
    bool ipV6 = m_ipVersions.contains("IPv6");
    if (m_ipVersions.contains("any")) {
        ipV4 = ipV6 = true;
    }

    foreach (const QNetworkInterface &interface, m_networkInterfaces) {
        QList<QNetworkAddressEntry> addresseEntries = interface.addressEntries();
        QList<QHostAddress> addresses;

        foreach (QNetworkAddressEntry entry, addresseEntries) {
            if (ipV4 && entry.ip().protocol() == QAbstractSocket::IPv4Protocol) {
                addresses.append(entry.ip());
            }
            if (ipV6 && entry.ip().protocol() == QAbstractSocket::IPv6Protocol) {
                addresses.append(entry.ip());
            }
        }

        // create a tcp server for each address found for the corresponding settings
        foreach(const QHostAddress &address, addresses){
            QTcpServer *server = new QTcpServer(this);
            if(server->listen(address, m_port)) {
                qCDebug(dcConnection) << "Started Tcp server on" << server->serverAddress().toString() << server->serverPort();
                connect(server, SIGNAL(newConnection()), SLOT(onClientConnected()));
                m_serverList.insert(QUuid::createUuid(), server);
            } else {
                qCWarning(dcConnection) << "Tcp server error: can not listen on" << interface.name() << address.toString() << m_port;
                delete server;
            }
        }
    }

    m_timer->start();

    if (m_serverList.empty())
        return false;

    return true;
}

/*! Returns true if this \l{TcpServer} stopped successfully.
 *
 * \sa TransportInterface::startServer()
 */
bool TcpServer::stopServer()
{
    // Listen on all Networkinterfaces
    foreach (QTcpServer *s, m_serverList) {
        QUuid uuid = m_serverList.key(s);
        s->close();
        m_serverList.take(uuid)->deleteLater();
    }

    if (!m_serverList.empty())
        return false;

    return true;
}

}
