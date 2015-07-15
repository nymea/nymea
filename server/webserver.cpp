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

#include "webserver.h"
#include "loggingcategories.h"

#include <QTcpServer>
#include <QTcpSocket>
#include <QUuid>

namespace guhserver {

WebServer::WebServer(QObject *parent) :
    TransportInterface(parent),
    m_enabled(false)
{
    m_server = new QTcpServer(this);

    connect(m_server, &QTcpServer::newConnection, this, &WebServer::onNewConnection);
}

WebServer::~WebServer()
{
    m_server->close();
}

void WebServer::sendData(const QUuid &clientId, const QByteArray &data)
{
    Q_UNUSED(clientId)
    Q_UNUSED(data)


}

void WebServer::sendData(const QList<QUuid> &clients, const QByteArray &data)
{
    Q_UNUSED(clients)
    Q_UNUSED(data)
}

QString WebServer::createContentHeader()
{
    QString contentHeader(
        "HTTP/1.1 200 OK\r\n"
       "Content-Type: application/json; charset=\"utf-8\"\r\n"
       "\r\n"
    );
    return contentHeader;
}

void WebServer::onNewConnection()
{
    QTcpSocket* socket = m_server->nextPendingConnection();
    connect(socket, &QTcpSocket::readyRead, this, &WebServer::readClient);
    connect(socket, &QTcpSocket::disconnected, this, &WebServer::discardClient);
    qCDebug(dcConnection) << "Webserver client connected" << socket->peerName() << socket->peerAddress().toString() << socket->peerPort();
}

void WebServer::readClient()
{

}

void WebServer::discardClient()
{    
    QTcpSocket* socket = static_cast<QTcpSocket *>(sender());
    qCDebug(dcConnection) << "Webserver client disonnected" << socket->peerName() << socket->peerAddress().toString() << socket->peerPort();

}

bool WebServer::startServer()
{
    if (!m_server->listen(QHostAddress::Any, m_port)) {
        qCWarning(dcConnection) << "Webserver could not listen on" << m_server->serverAddress().toString() << m_port;
        m_enabled = false;
        return false;
    }
    qCDebug(dcConnection) << "Started webserver on" << m_server->serverAddress().toString() << m_port;
    return true;
}

bool WebServer::stopServer()
{
    m_server->close();
    qCDebug(dcConnection) << "Webserver closed.";
    return true;
}

}
