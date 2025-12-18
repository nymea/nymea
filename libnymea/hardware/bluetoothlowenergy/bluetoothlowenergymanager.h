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

#ifndef BLUETOOTHLOWENERGYMANAGER_H
#define BLUETOOTHLOWENERGYMANAGER_H

#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothDeviceInfo>
#include <QBluetoothLocalDevice>
#include <QObject>
#include <QPointer>
#include <QTimer>

#include "hardwareresource.h"

#include "bluetoothdiscoveryreply.h"
#include "bluetoothlowenergydevice.h"

#include "libnymea.h"

class NymeaBluetoothAgent;

class LIBNYMEA_EXPORT BluetoothPairingJob : public QObject
{
    Q_OBJECT
public:
    explicit BluetoothPairingJob(const QBluetoothAddress &address, QObject *parent = nullptr);
    virtual ~BluetoothPairingJob() = default;

    QBluetoothAddress address() const;

    virtual bool isFinished() const = 0;
    virtual bool success() const = 0;

    virtual void passKeyEntered(const QString passKey) = 0;

signals:
    void finished(bool success);
    void passKeyRequested();
    void displayPinCode(const QString &pinCode);

private:
    QBluetoothAddress m_address;
};

class LIBNYMEA_EXPORT BluetoothLowEnergyManager : public HardwareResource
{
    Q_OBJECT

public:
    explicit BluetoothLowEnergyManager(QObject *parent = nullptr);
    virtual ~BluetoothLowEnergyManager() = default;

    virtual BluetoothDiscoveryReply *discoverDevices(int interval = 5000) = 0;

    // Bluetooth device registration methods
    virtual BluetoothPairingJob *pairDevice(const QBluetoothAddress &device, const QBluetoothAddress &adapter) = 0;
    virtual void unpairDevice(const QBluetoothAddress &device, const QBluetoothAddress &adapter) = 0;
    virtual BluetoothLowEnergyDevice *registerDevice(const QBluetoothDeviceInfo &deviceInfo,
                                                     const QLowEnergyController::RemoteAddressType &addressType = QLowEnergyController::RandomAddress)
        = 0;
    virtual void unregisterDevice(BluetoothLowEnergyDevice *bluetoothDevice) = 0;

public slots:
    Q_SCRIPTABLE void EnableBluetooth(bool enabled);
};

#endif // BLUETOOTHLOWENERGYMANAGER_H
