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

#ifndef TUNEMANAGER_H
#define TUNEMANAGER_H

#include <QObject>
#include <QNetworkInterface>
#include <QTcpServer>
#include <QTcpSocket>
#include <QStringList>
#include <QProcess>

class TuneManager : public QObject
{
    Q_OBJECT
public:
    explicit TuneManager(int port = 31337, QObject *parent = 0);

    bool tuneAvailable();
    bool sendData(const QByteArray &data);

private:
    QTcpServer *m_server;
    QTcpSocket *m_tune;

    int m_port;
    int m_connected;

signals:
    void dataReady(const QByteArray &data);
    void tuneConnectionStatusChanged(const bool &connectionStatus);

private slots:
    void tuneConnected();
    void tuneDisconnected();
    void readData();

public slots:
    bool start();
    void stop();
};

#endif // TUNEMANAGER_H
