/****************************************************************************
 *                                                                          *
 *  This file is part of guh.                                               *
 *                                                                          *
 *  Guh is free software: you can redistribute it and/or modify             *
 *  it under the terms of the GNU General Public License as published by    *
 *  the Free Software Foundation, version 2 of the License.                 *
 *                                                                          *
 *  Guh is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *  GNU General Public License for more details.                            *
 *                                                                          *
 *  You should have received a copy of the GNU General Public License       *
 *  along with guh.  If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                          *
 ***************************************************************************/

#include "tcpserver.h"
#include <QDebug>
#include <QJsonDocument>

TcpServer::TcpServer(QObject *parent) :
    QObject(parent)
{       

    qDebug() << "----------------------------";
    qDebug() << "network interfaces:";
    foreach(const QNetworkInterface &interface, QNetworkInterface::allInterfaces()){
        qDebug() << "   -------------------------";
        qDebug() << "   name:" << interface.name();
        qDebug() << "   mac: " << interface.hardwareAddress();
    }
    qDebug() << "----------------------------";

}

void TcpServer::sendData(const QList<QUuid> &clients, const QByteArray &data)
{
    foreach (const QUuid &client, clients) {
        sendData(client, data);
    }
}

void TcpServer::sendData(const QUuid &clientId, const QByteArray &data)
{
    QTcpSocket *client = m_clientList.value(clientId);
    if (client) {
        client->write(data);
    }
}

void TcpServer::newClientConnected()
{
    // got a new client connected
    QTcpServer *server = qobject_cast<QTcpServer*>(sender());
    QTcpSocket *newConnection = server->nextPendingConnection();
    qDebug() << "new client connected:" << newConnection->peerAddress().toString();

    QUuid clientId = QUuid::createUuid();

    // append the new client to the client list
    m_clientList.insert(clientId, newConnection);

    connect(newConnection, SIGNAL(readyRead()),this,SLOT(readPackage()));
    connect(newConnection,SIGNAL(disconnected()),this,SLOT(slotClientDisconnected()));

    emit clientConnected(clientId);
}


void TcpServer::readPackage()
{
    QTcpSocket *client = qobject_cast<QTcpSocket*>(sender());
    qDebug() << "-----------> data comming from" << client->peerAddress().toString();
    QByteArray message;
    while(client->canReadLine()){
        QByteArray dataLine = client->readLine();
        qDebug() << "line in:" << dataLine;
        message.append(dataLine);
        if(dataLine.endsWith('\n')){
            emit dataAvailable(m_clientList.key(client), message);
            message.clear();
        }
    }
}

void TcpServer::slotClientDisconnected()
{
    QTcpSocket *client = qobject_cast<QTcpSocket*>(sender());
    qDebug() << "client disconnected:" << client->peerAddress().toString();
    QUuid clientId = m_clientList.key(client);
    m_clientList.take(clientId)->deleteLater();
}

bool TcpServer::startServer()
{
    // Listen on all Networkinterfaces
    foreach(const QHostAddress &address, QNetworkInterface::allAddresses()){
        QTcpServer *server = new QTcpServer(this);
        if(server->listen(address, 1234)) {
            qDebug() << "server listening on" << address.toString();
            connect(server, SIGNAL(newConnection()), SLOT(newClientConnected()));
            m_serverList.insert(QUuid::createUuid(), server);
        } else {
            qDebug() << "ERROR: can not listening to" << address.toString();
            delete server;
        }
    }
    if(m_serverList.empty()){
        return false;
    }
    return true;
}

bool TcpServer::stopServer()
{
    // Listen on all Networkinterfaces
    foreach(QTcpServer *server, m_serverList){
        qDebug() << "close server " << server->serverAddress().toString();
        server->close();
        delete server;
    }
    if(!m_serverList.empty()){
        return false;
    }
    return true;
}


