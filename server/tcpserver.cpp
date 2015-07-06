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

#include "tcpserver.h"
#include "loggingcategories.h"
#include "guhsettings.h"

#include <QDebug>
#include <QJsonDocument>

namespace guhserver {

TcpServer::TcpServer(QObject *parent) :
    QObject(parent)
{       
    qCDebug(dcConnection) << "----------------------------";
    qCDebug(dcConnection) << "network interfaces:";
    foreach(const QNetworkInterface &interface, QNetworkInterface::allInterfaces()){
        qCDebug(dcConnection) << "   -------------------------";
        qCDebug(dcConnection) << "   name :" << interface.name();
        if(!interface.addressEntries().isEmpty()){
            qCDebug(dcConnection) << "   ip   :" << interface.addressEntries().first().ip().toString();
        }
        qCDebug(dcConnection) << "   mac  : " << interface.hardwareAddress();
    }
    qCDebug(dcConnection) << "----------------------------";

    // load settings
    bool ok;
    GuhSettings settings(GuhSettings::SettingsRoleGlobal);
    settings.beginGroup("JSONRPC");

    // TODO: handle interfaces in settings (enable just localhost ecc...)

    uint port = settings.value("port", 1234).toUInt(&ok);
    settings.endGroup();
    if(ok){
        m_port = port;
    } else {
        m_port = 1234;
    }
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
    qCDebug(dcConnection) << "new client connected:" << newConnection->peerAddress().toString();

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
    qCDebug(dcConnection) << "data comming from" << client->peerAddress().toString();
    QByteArray message;
    while(client->canReadLine()){
        QByteArray dataLine = client->readLine();
        qCDebug(dcConnection) << "line in:" << dataLine;
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
    qCDebug(dcConnection) << "client disconnected:" << client->peerAddress().toString();
    QUuid clientId = m_clientList.key(client);
    m_clientList.take(clientId)->deleteLater();
}

bool TcpServer::startServer()
{
    // Listen on all Networkinterfaces
    foreach(const QHostAddress &address, QNetworkInterface::allAddresses()){
        QTcpServer *server = new QTcpServer(this);
        if(server->listen(address, m_port)) {
            qCDebug(dcConnection) << "JSON-RPC server listening on" << address.toString() << ":" << m_port;
            connect(server, SIGNAL(newConnection()), SLOT(newClientConnected()));
            m_serverList.insert(QUuid::createUuid(), server);
        } else {
            qCWarning(dcConnection) << "can not listening to" << address.toString() << ":" << m_port;
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
        qCDebug(dcConnection) << "close server " << server->serverAddress().toString();
        server->close();
        delete server;
    }
    if(!m_serverList.empty()){
        return false;
    }
    return true;
}

}
