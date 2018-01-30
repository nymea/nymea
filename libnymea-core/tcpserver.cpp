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
    \class nymeaserver::TcpServer
    \brief This class represents the tcp server for nymead.

    \ingroup server
    \inmodule core

    \inherits TransportInterface

    The TCP server allows clients to connect to the JSON-RPC API.

    \sa WebSocketServer, TransportInterface
*/

#include "tcpserver.h"
#include "nymeacore.h"

#include <QDebug>

namespace nymeaserver {

/*! Constructs a \l{TcpServer} with the given \a host, \a port and \a parent.
 *
 *  \sa ServerManager
 */
TcpServer::TcpServer(const ServerConfiguration &configuration, const QSslConfiguration &sslConfiguration, QObject *parent) :
    TransportInterface(configuration, parent),
    m_server(NULL),
    m_sslConfig(sslConfiguration)
{
    m_avahiService = new QtAvahiService(this);
    connect(m_avahiService, &QtAvahiService::serviceStateChanged, this, &TcpServer::onAvahiServiceStateChanged);
}

/*! Destructor of this \l{TcpServer}. */
TcpServer::~TcpServer()
{
    qCDebug(dcTcpServer()) << "Shutting down \"TCP Server\"" << serverUrl().toString();
    stopServer();
}

QUrl TcpServer::serverUrl() const
{
    return QUrl(QString("%1://%2:%3").arg((configuration().sslEnabled ? "guhs" : "guh")).arg(configuration().address.toString()).arg(configuration().port));
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
        client->write(data + '\n');
    } else {
        qWarning(dcTcpServer()) << "Client" << clientId << "unknown to this transport";
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
    qCDebug(dcTcpServerTraffic()) << "Emitting data available";
    QUuid clientId = m_clientList.key(socket);
    emit dataAvailable(clientId, data);
}

void TcpServer::onAvahiServiceStateChanged(const QtAvahiService::QtAvahiServiceState &state)
{
    Q_UNUSED(state)
}

void TcpServer::resetAvahiService()
{
    if (m_avahiService)
        m_avahiService->resetService();

    // Note: reversed order
    QHash<QString, QString> txt;
    txt.insert("jsonrpcVersion", JSON_PROTOCOL_VERSION);
    txt.insert("serverVersion", NYMEA_VERSION_STRING);
    txt.insert("manufacturer", "guh GmbH");
    txt.insert("uuid", NymeaCore::instance()->configuration()->serverUuid().toString());
    txt.insert("name", NymeaCore::instance()->configuration()->serverName());
    txt.insert("sslEnabled", configuration().sslEnabled ? "true" : "false");
    if (!m_avahiService->registerService(QString("guhIO-tcp-%1").arg(configuration().id), configuration().port, "_jsonrpc._tcp", txt)) {
        qCWarning(dcTcpServer()) << "Could not register avahi service for" << configuration();
    }
}


/*! Returns true if this \l{TcpServer} could be reconfigured with the given \a address and \a port. */
void TcpServer::reconfigureServer(const ServerConfiguration &config)
{
    if (configuration().address == config.address &&
            configuration().port == config.port &&
            configuration().sslEnabled == config.sslEnabled &&
            configuration().authenticationEnabled == config.authenticationEnabled &&
            m_server->isListening())
        return;

    stopServer();
    setConfiguration(config);
    startServer();
}

void TcpServer::setServerName(const QString &serverName)
{
    m_serverName = serverName;
    resetAvahiService();
}

/*! Returns true if this \l{TcpServer} started successfully.
 *
 * \sa TransportInterface::startServer()
 */
bool TcpServer::startServer()
{
    m_server = new SslServer(configuration().sslEnabled, m_sslConfig);
    if(!m_server->listen(configuration().address, configuration().port)) {
        qCWarning(dcConnection) << "Tcp server error: can not listen on" << configuration().address.toString() << configuration().port;
        delete m_server;
        m_server = NULL;
        return false;
    }

    connect(m_server, SIGNAL(clientConnected(QSslSocket *)), SLOT(onClientConnected(QSslSocket *)));
    connect(m_server, SIGNAL(clientDisconnected(QSslSocket *)), SLOT(onClientDisconnected(QSslSocket *)));
    connect(m_server, &SslServer::dataAvailable, this, &TcpServer::onDataAvailable);

    qCDebug(dcConnection) << "Started Tcp server" << serverUrl().toString();
    resetAvahiService();

    return true;
}

/*! Returns true if this \l{TcpServer} stopped successfully.
 *
 * \sa TransportInterface::startServer()
 */
bool TcpServer::stopServer()
{
    if (m_avahiService)
        m_avahiService->resetService();

    if (!m_server)
        return true;

    m_server->close();
    m_server->deleteLater();
    m_server = NULL;
    return true;
}

void SslServer::incomingConnection(qintptr socketDescriptor)
{
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
    m_receiveBuffer.append(socket->readAll());
    int splitIndex = m_receiveBuffer.indexOf("}\n{");
    while (splitIndex > -1) {
        emit dataAvailable(socket, m_receiveBuffer.left(splitIndex + 1));
        m_receiveBuffer = m_receiveBuffer.right(m_receiveBuffer.length() - splitIndex - 2);
        splitIndex = m_receiveBuffer.indexOf("}\n{");
    }
    if (m_receiveBuffer.endsWith("}\n")) {
        emit dataAvailable(socket, m_receiveBuffer);
        m_receiveBuffer.clear();
    }
}

}
