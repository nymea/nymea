/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2020, nymea GmbH
* Contact: contact@nymea.io
*
* This file is part of nymea.
* This project including source code and documentation is protected by
* copyright law, and remains the property of nymea GmbH. All rights, including
* reproduction, publication, editing and translation, are reserved. The use of
* this project is subject to the terms of a license agreement to be concluded
* with nymea GmbH in accordance with the terms of use of nymea GmbH, available
* under https://nymea.io/license
*
* GNU General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU General Public License as published by the Free Software
* Foundation, GNU version 3. This project is distributed in the hope that it
* will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
* of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
* Public License for more details.
*
* You should have received a copy of the GNU General Public License along with
* this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef BLUETOOTHSERVER_H
#define BLUETOOTHSERVER_H

#ifdef WITH_BLUETOOTH

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
    QBluetoothServiceInfo m_serviceInfo;

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

#endif // WITH_BLUETOOTH

#endif // BLUETOOTHSERVER_H
