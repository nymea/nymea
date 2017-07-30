/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
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

#include "mocktcpserver.h"
#include "loggingcategories.h"
#include "jsonrpcserver.h"
#include "guhcore.h"

#include <QUuid>
#include <QHash>
#include <QJsonDocument>

using namespace guhserver;

QList<MockTcpServer*> MockTcpServer::s_allServers;

MockTcpServer::MockTcpServer(QObject *parent):
    TransportInterface(parent)
{
    s_allServers.append(this);
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
        sendData(clientId, data);
    }
}

QList<MockTcpServer *> MockTcpServer::servers()
{
    return s_allServers;
}

void MockTcpServer::injectData(const QUuid &clientId, const QByteArray &data)
{
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
