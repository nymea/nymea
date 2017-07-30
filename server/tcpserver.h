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

#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QNetworkInterface>
#include <QUuid>
#include <QTimer>

#include "transportinterface.h"
#include "network/avahi/qtavahiservice.h"

namespace guhserver {

class TcpServer : public TransportInterface
{
    Q_OBJECT
public:
    explicit TcpServer(const QHostAddress &host, const uint &port, QObject *parent = 0);
    ~TcpServer();

    void sendData(const QUuid &clientId, const QByteArray &data) override;
    void sendData(const QList<QUuid> &clients, const QByteArray &data) override;

private:
    QTimer *m_timer;

    QtAvahiService *m_avahiService;

    QTcpServer * m_server;
    QHash<QUuid, QTcpSocket *> m_clientList;

    QHostAddress m_host;
    qint16 m_port;

private slots:
    void onClientConnected();
    void onClientDisconnected();
    void readPackage();
    void onError(QAbstractSocket::SocketError error);

    void onAvahiServiceStateChanged(const QtAvahiService::QtAvahiServiceState &state);

public slots:
    bool reconfigureServer(const QHostAddress &address, const uint &port);
    bool startServer() override;
    bool stopServer() override;
};

}

#endif // TCPSERVER_H
