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

#ifndef MOCKTCPSERVER_H
#define MOCKTCPSERVER_H

#include <QObject>
#include <QNetworkInterface>
#include <QDebug>

#include "transportinterface.h"

class JsonRPCServer;

namespace nymeaserver {

class MockTcpServer : public TransportInterface
{
    Q_OBJECT
public:
    explicit MockTcpServer(QObject *parent = nullptr);
    ~MockTcpServer() override;

    void sendData(const QUuid &clientId, const QByteArray &data) override;
    void sendData(const QList<QUuid> &clients, const QByteArray &data) override;
    void terminateClientConnection(const QUuid &clientId) override;

/************** Used for testing **************************/
    static QList<MockTcpServer*> servers();
    void injectData(const QUuid &clientId, const QByteArray &data);
signals:
    void outgoingData(const QUuid &clientId, const QByteArray &data);
    void connectionTerminated(const QUuid &clientId);
/************** Used for testing **************************/

public slots:
    bool reconfigureServer(const QHostAddress &address, const uint &port);
    bool startServer() override;
    bool stopServer() override;

private:
    static QList<MockTcpServer*> s_allServers;

    QList<QUuid> m_connectedClients;
};

}

#endif // TCPSERVER_H

