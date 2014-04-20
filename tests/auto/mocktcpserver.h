/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
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

class MockTcpServer : public QObject
{
    Q_OBJECT
public:
    explicit MockTcpServer(QObject *parent = 0);
    ~MockTcpServer();

    void sendData(const QUuid &clientId, const QByteArray &data);
    void sendData(const QList<QUuid> &clients, const QByteArray &data);

/************** Used for testing **************************/
    static QList<MockTcpServer*> servers();
    void injectData(const QUuid &clientId, const QByteArray &data);
signals:
    void outgoingData(const QUuid &clientId, const QByteArray &data);
/************** Used for testing **************************/

signals:
    void dataAvailable(const QUuid &clientId, const QByteArray &data);

public slots:
    bool startServer();
    bool stopServer();

private:
    static QList<MockTcpServer*> s_allServers;
};

#endif // TCPSERVER_H

