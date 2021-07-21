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

#ifndef BLUETOOTHLOWENERGYMANAGERIMPLEMENTATION_H
#define BLUETOOTHLOWENERGYMANAGERIMPLEMENTATION_H

#ifdef WITH_BLUETOOTH

#include <QTimer>
#include <QObject>
#include <QPointer>
#include <QBluetoothDeviceInfo>
#include <QBluetoothLocalDevice>
#include <QBluetoothDeviceDiscoveryAgent>

#include "plugintimer.h"

#include "hardware/bluetoothlowenergy/bluetoothlowenergymanager.h"
#include "hardware/bluetoothlowenergy/bluetoothdiscoveryreplyimplementation.h"
#include "hardware/bluetoothlowenergy/bluetoothlowenergydeviceimplementation.h"

namespace nymeaserver {

class BluetoothLowEnergyManagerImplementation : public BluetoothLowEnergyManager
{
    Q_OBJECT

    friend class HardwareManagerImplementation;

public:
    explicit BluetoothLowEnergyManagerImplementation(PluginTimer *reconnectTimer, QObject *parent = nullptr);

    BluetoothDiscoveryReply *discoverDevices(int interval = 5000) override;

    // Bluetooth device registration methods
    BluetoothLowEnergyDevice *registerDevice(const QBluetoothDeviceInfo &deviceInfo, const QLowEnergyController::RemoteAddressType &addressType = QLowEnergyController::RandomAddress) override;
    void unregisterDevice(BluetoothLowEnergyDevice *bluetoothDevice) override;

    bool available() const override;
    bool enabled() const override;

protected:
    void setEnabled(bool enabled) override;

private:
    PluginTimer *m_reconnectTimer = nullptr;
    QTimer *m_timer = nullptr;
    QList<QPointer<BluetoothLowEnergyDeviceImplementation>> m_devices;

    bool m_available = false;
    bool m_enabled = false;

    QList<QBluetoothDeviceDiscoveryAgent *> m_bluetoothDiscoveryAgents;
    QList<QBluetoothDeviceInfo> m_discoveredDevices;
    QPointer<BluetoothDiscoveryReplyImplementation> m_currentReply;

private slots:
    void onReconnectTimeout();
    void onDeviceDiscovered(const QBluetoothDeviceInfo &deviceInfo);
    void onDiscoveryError(const QBluetoothDeviceDiscoveryAgent::Error &error);
    void onDiscoveryTimeout();

public slots:
    bool enable();
    bool disable();

};

}

#endif // WITH_BLUETOOTH

#endif // BLUETOOTHLOWENERGYMANAGERIMPLEMENTATION_H
