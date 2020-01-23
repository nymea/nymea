/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2020, nymea GmbH
* Contact: contact@nymea.io
*
* This file is part of nymea.
* This project including source code and documentation is protected by
* copyright law, and remains the property of nymea GmbH. All rights, including
* reproduction, publication, editing and translation, are reserved. The use of
* this project is subject to the terms of a license agreement to be concluded
* with nymea GmbH in accordance with the terms of use of nymea GmbH, available
* under https://nymea.io/license
*
* GNU General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU General Public License as published by the Free Software
* Foundation, GNU version 3. This project is distributed in the hope that it
* will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
* of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
* Public License for more details.
*
* You should have received a copy of the GNU General Public License along with
* this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
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
