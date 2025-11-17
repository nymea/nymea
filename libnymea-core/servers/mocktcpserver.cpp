// SPDX-License-Identifier: LGPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2025, chargebyte austria GmbH
*
* This file is part of nymea.
*
* nymea is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* as published by the Free Software Foundation, either version 3
* of the License, or (at your option) any later version.
*
* nymea is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with nymea. If not, see <https://www.gnu.org/licenses/>.
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "mocktcpserver.h"
#include "loggingcategories.h"

#include <QUuid>
#include <QHash>
#include <QJsonDocument>

using namespace nymeaserver;

QList<MockTcpServer*> MockTcpServer::s_allServers;

MockTcpServer::MockTcpServer(QObject *parent):
    TransportInterface(ServerConfiguration(), parent)
{
    s_allServers.append(this);

    connect(this, &TransportInterface::clientConnected, this, [this](const QUuid &clientId){
        m_connectedClients.append(clientId);
    });

    connect(this, &TransportInterface::clientDisconnected, this, [this](const QUuid &clientId){
        m_connectedClients.removeAll(clientId);
    });
}

MockTcpServer::~MockTcpServer()
{
    s_allServers.removeAll(this);
}

void MockTcpServer::sendData(const QUuid &clientId, const QByteArray &data)
{
    emit outgoingData(clientId, data);
}

void MockTcpServer::sendData(const QList<QUuid> &clients, const QByteArray &data)
{
    foreach (const QUuid &clientId, clients) {
        sendData(clientId, data + '\n');
    }
}

void MockTcpServer::terminateClientConnection(const QUuid &clientId)
{
    emit connectionTerminated(clientId);
    emit clientDisconnected(clientId);
}

QList<MockTcpServer *> MockTcpServer::servers()
{
    return s_allServers;
}

void MockTcpServer::injectData(const QUuid &clientId, const QByteArray &data)
{
    Q_ASSERT_X(m_connectedClients.contains(clientId), "MockTcpServer", "Cannot inject data. Client is not connected");
    emit dataAvailable(clientId, data);
}

bool MockTcpServer::reconfigureServer(const QHostAddress &address, const uint &port)
{
    Q_UNUSED(address)
    Q_UNUSED(port)
    return true;
}

bool MockTcpServer::startServer()
{
    return true;
}

bool MockTcpServer::stopServer()
{
    return true;
}
