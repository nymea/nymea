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

#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QNetworkInterface>
#include <QUuid>
#include <QTimer>

#include "transportinterface.h"

namespace guhserver {

class TcpServer : public TransportInterface
{
    Q_OBJECT
public:
    explicit TcpServer(QObject *parent = 0);
    
    void sendData(const QUuid &clientId, const QVariantMap &data) override;
    void sendData(const QList<QUuid> &clients, const QVariantMap &data) override;

private:
    QTimer *m_timer;

    QHash<QUuid, QTcpServer*> m_serverList;
    QHash<QUuid, QTcpSocket*> m_clientList;

    uint m_port;
    QList<QNetworkInterface> m_networkInterfaces;
    QStringList m_ipVersions;

    void reloadNetworkInterfaces();
    void validateMessage(const QUuid &clientId, const QByteArray &data);

public:
    void sendResponse(const QUuid &clientId, int commandId, const QVariantMap &params = QVariantMap());
    void sendErrorResponse(const QUuid &clientId, int commandId, const QString &error);

private slots:
    void onClientConnected();
    void onClientDisconnected();
    void readPackage();
    void onError(QAbstractSocket::SocketError error);
    void onTimeout();

public slots:
    bool startServer() override;
    bool stopServer() override;
};

}

#endif // TCPSERVER_H
