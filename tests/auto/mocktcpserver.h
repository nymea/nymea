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

#ifndef MOCKTCPSERVER_H
#define MOCKTCPSERVER_H

#include <QObject>
#include <QNetworkInterface>
#include <QDebug>

#include "transportinterface.h"

using namespace guhserver;

class JsonRPCServer;

class MockTcpServer : public TransportInterface
{
    Q_OBJECT
public:
    explicit MockTcpServer(QObject *parent = 0);
    ~MockTcpServer();

    void sendData(const QUuid &clientId, const QByteArray &data) override;
    void sendData(const QList<QUuid> &clients, const QByteArray &data) override;

/************** Used for testing **************************/
    static QList<MockTcpServer*> servers();
    void injectData(const QUuid &clientId, const QByteArray &data);
signals:
    void outgoingData(const QUuid &clientId, const QByteArray &data);
/************** Used for testing **************************/

public slots:
    bool reconfigureServer(const QHostAddress &address, const uint &port);
    bool startServer() override;
    bool stopServer() override;

private:
    static QList<MockTcpServer*> s_allServers;
};

#endif // TCPSERVER_H

