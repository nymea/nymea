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

#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <QObject>
#include <QHash>

#include "transportinterface.h"

class QTcpServer;
class QTcpSocket;
class QUuid;

namespace guhserver {

class WebServer :  public TransportInterface
{
    Q_OBJECT
public:
    explicit WebServer(QObject *parent = 0);
    ~WebServer();
    void sendData(const QUuid &clientId, const QByteArray &data);
    void sendData(const QList<QUuid> &clients, const QByteArray &data);

private:
    QTcpServer *m_server;
    QHash<QUuid, QTcpSocket *> m_clientList;

    bool m_enabled;
    qint16 m_port;

    QString createContentHeader();

signals:

private slots:
    void onNewConnection();
    void readClient();
    void discardClient();

public slots:
    bool startServer();
    bool stopServer();

};

}

#endif // WEBSERVER_H
