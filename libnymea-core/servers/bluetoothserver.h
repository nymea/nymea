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
    explicit BluetoothServer(QObject *parent = nullptr);
    ~BluetoothServer();

    static bool hardwareAvailable();

    void sendData(const QUuid &clientId, const QByteArray &data) override;
    void sendData(const QList<QUuid> &clients, const QByteArray &data) override;

    void terminateClientConnection(const QUuid &clientId) override;

private:
    QBluetoothServer *m_server = nullptr;
    QBluetoothLocalDevice *m_localDevice = nullptr;
    QBluetoothServiceInfo *m_serviceInfo = nullptr;

    // Client storage
    QHash<QUuid, QBluetoothSocket *> m_clientList;

private slots:
    void onHostModeChanged(const QBluetoothLocalDevice::HostMode &mode);

    void onClientConnected();
    void onClientDisconnected();
    void onClientError(QBluetoothSocket::SocketError error);
    void onClientStateChanged(QBluetoothSocket::SocketState state);
    void readData();

public slots:
    bool startServer() override;
    bool stopServer() override;

};

}

#endif // BLUETOOTHSERVER_H
