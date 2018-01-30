/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2017 Simon Stürz <simon.stuerz@guh.io>                   *
 *                                                                         *
 *  This file is part of nymea.                                            *
 *                                                                         *
 *  This library is free software; you can redistribute it and/or          *
 *  modify it under the terms of the GNU Lesser General Public             *
 *  License as published by the Free Software Foundation; either           *
 *  version 2.1 of the License, or (at your option) any later version.     *
 *                                                                         *
 *  This library is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
 *  Lesser General Public License for more details.                        *
 *                                                                         *
 *  You should have received a copy of the GNU Lesser General Public       *
 *  License along with this library; If not, see                           *
 *  <http://www.gnu.org/licenses/>.                                        *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

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

#endif // BLUETOOTHLOWENERGYMANAGERIMPLEMENTATION_H
