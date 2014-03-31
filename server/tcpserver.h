/****************************************************************************
 *                                                                          *
 *  This file is part of guh.                                               *
 *                                                                          *
 *  Guh is free software: you can redistribute it and/or modify             *
 *  it under the terms of the GNU General Public License as published by    *
 *  the Free Software Foundation, version 2 of the License.                 *
 *                                                                          *
 *  Guh is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *  GNU General Public License for more details.                            *
 *                                                                          *
 *  You should have received a copy of the GNU General Public License       *
 *  along with guh.  If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                          *
 ***************************************************************************/

#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QObject>
#include <QNetworkInterface>
#include <QTcpServer>
#include <QTcpSocket>
#include <QUuid>

class TcpServer : public QObject
{
    Q_OBJECT
public:
    explicit TcpServer(QObject *parent = 0);
    
    void sendData(const QUuid &clientId, const QByteArray &data);
    void sendData(const QList<QUuid> &clients, const QByteArray &data);

private:
    QHash<QUuid, QTcpServer*> m_serverList;
    QHash<QUuid, QTcpSocket*> m_clientList;

signals:
    void clientConnected(const QUuid &clientId);
    void clientDisconnected(const QUuid &clientId);
    void dataAvailable(const QUuid &clientId, const QByteArray &data);
    
private slots:
    void newClientConnected();
    void readPackage();
    void slotClientDisconnected();

public slots:
    bool startServer();
    bool stopServer();
};

#endif // TCPSERVER_H
