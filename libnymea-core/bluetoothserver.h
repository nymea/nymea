/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2016-2018 Simon Stürz <simon.stuerz@guh.io>              *
 *                                                                         *
 *  This file is part of nymea.                                            *
 *                                                                         *
 *  nymea is free software: you can redistribute it and/or modify          *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  nymea is distributed in the hope that it will be useful,               *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with nymea. If not, see <http://www.gnu.org/licenses/>.          *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef BLUETOOTHSERVER_H
#define BLUETOOTHSERVER_H

#include <QObject>
#include <QBluetoothSocket>
#include <QBluetoothServer>
#include <QBluetoothLocalDevice>

#include "transportinterface.h"

namespace nymeaserver {

class BluetoothServer : public TransportInterface
{
    Q_OBJECT
public:
    explicit BluetoothServer(QObject *parent = 0);
    ~BluetoothServer();

    static bool hardwareAvailable();

    void sendData(const QUuid &clientId, const QByteArray &data) override;
    void sendData(const QList<QUuid> &clients, const QByteArray &data) override;

private:
    QBluetoothServer *m_server = nullptr;
    QBluetoothLocalDevice *m_localDevice = nullptr;
    QBluetoothServiceInfo m_serviceInfo;

    // Client storage
    QHash<QUuid, QBluetoothSocket *> m_clientList;
    QByteArray m_receiveBuffer;

private slots:
    void onHostModeChanged(const QBluetoothLocalDevice::HostMode &mode);

    void onClientConnected();
    void onClientDisconnected();
    void onError(QBluetoothSocket::SocketError error);
    void readData();

public slots:
    bool startServer() override;
    bool stopServer() override;

};

}

#endif // BLUETOOTHSERVER_H
