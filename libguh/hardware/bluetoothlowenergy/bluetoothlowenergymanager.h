/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2017-2018 Simon Stürz <simon.stuerz@guh.io>              *
 *                                                                         *
 *  This file is part of guh.                                              *
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

#ifndef BLUETOOTHLOWENERGYMANAGER_H
#define BLUETOOTHLOWENERGYMANAGER_H

#include <QTimer>
#include <QObject>
#include <QPointer>
#include <QBluetoothDeviceInfo>
#include <QBluetoothLocalDevice>
#include <QBluetoothDeviceDiscoveryAgent>

#include "hardwareresource.h"

#include "bluetoothdiscoveryreply.h"
#include "bluetoothlowenergydevice.h"

#include "libguh.h"

class LIBGUH_EXPORT BluetoothLowEnergyManager : public HardwareResource
{
    Q_OBJECT

public:
    explicit BluetoothLowEnergyManager(QObject *parent = nullptr);
    virtual ~BluetoothLowEnergyManager() = default;

    virtual BluetoothDiscoveryReply *discoverDevices(int interval = 5000) = 0;

    // Bluetooth device registration methods
    virtual BluetoothLowEnergyDevice *registerDevice(const QBluetoothDeviceInfo &deviceInfo, const QLowEnergyController::RemoteAddressType &addressType = QLowEnergyController::RandomAddress) = 0;
    virtual void unregisterDevice(BluetoothLowEnergyDevice *bluetoothDevice) = 0;
};

#endif // BLUETOOTHLOWENERGYMANAGER_H
