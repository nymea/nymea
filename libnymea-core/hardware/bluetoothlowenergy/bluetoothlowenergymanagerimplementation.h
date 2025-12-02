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

#ifndef BLUETOOTHLOWENERGYMANAGERIMPLEMENTATION_H
#define BLUETOOTHLOWENERGYMANAGERIMPLEMENTATION_H

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

class NymeaBluetoothAgent;

class BluetoothLowEnergyManagerImplementation : public BluetoothLowEnergyManager
{
    Q_OBJECT

    friend class HardwareManagerImplementation;

public:
    explicit BluetoothLowEnergyManagerImplementation(PluginTimer *reconnectTimer, QObject *parent = nullptr);

    BluetoothDiscoveryReply *discoverDevices(int timeout = 5000) override;

    // Bluetooth device registration methods
    BluetoothPairingJob *pairDevice(const QBluetoothAddress &device, const QBluetoothAddress &adapter) override;
    void unpairDevice(const QBluetoothAddress &device, const QBluetoothAddress &adapter) override;
    BluetoothLowEnergyDevice *registerDevice(const QBluetoothDeviceInfo &deviceInfo, const QLowEnergyController::RemoteAddressType &addressType = QLowEnergyController::RandomAddress) override;
    void unregisterDevice(BluetoothLowEnergyDevice *bluetoothDevice) override;

    bool available() const override;
    bool enabled() const override;

protected:
    void setEnabled(bool enabled) override;


private slots:
    void onReconnectTimeout();

private:
    PluginTimer *m_reconnectTimer = nullptr;
    QList<QPointer<BluetoothLowEnergyDeviceImplementation>> m_devices;

    bool m_available = false;
    bool m_enabled = false;
    NymeaBluetoothAgent *m_agent = nullptr;


};

}

#endif // BLUETOOTHLOWENERGYMANAGERIMPLEMENTATION_H
