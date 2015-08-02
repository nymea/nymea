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

#include "websocketserver.h"

namespace guhserver {

WebSocketServer::WebSocketServer(QObject *parent) :
    TransportInterface(parent)
{
}

WebSocketServer::~WebSocketServer()
{
}

void WebSocketServer::sendData(const QUuid &clientId, const QVariantMap &data)
{
    Q_UNUSED(clientId)
    Q_UNUSED(data)
}

void WebSocketServer::sendData(const QList<QUuid> &clients, const QVariantMap &data)
{
    Q_UNUSED(clients)
    Q_UNUSED(data)
}

bool WebSocketServer::startServer()
{
    return false;
}

bool WebSocketServer::stopServer()
{
    return false;
}

}
