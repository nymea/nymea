/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2017 Simon Stürz <simon.stuerz@guh.io>                   *
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

#include "bluetoothlowenergymanager.h"
#include "loggingcategories.h"

BluetoothLowEnergyManager::BluetoothLowEnergyManager(QObject *parent) :
    HardwareResource(HardwareResource::TypeBluetoothLE, "Bluetooth LE manager", parent)
{
    // Check which bluetooth adapter are available
    QList<QBluetoothHostInfo> bluetoothAdapters = QBluetoothLocalDevice::allDevices();
    if (bluetoothAdapters.isEmpty()) {
        qCWarning(dcHardware()) << name() << "No bluetooth adapter found. Resource not available.";
        setAvailable(false);
        return;
    }

    // Create a scanner for each adapter
    foreach (const QBluetoothHostInfo &hostInfo, bluetoothAdapters) {
        qCDebug(dcHardware()) << name() << "Adapter:" << hostInfo.name() << hostInfo.address().toString();
        m_bluetoothScanners.append(new BluetoothScanner(hostInfo.address(), this));
    }



//    // Check if Bluetooth is available on this device
//    if (!localDevice.isValid()) {
//        qCWarning(dcHardware()) << "No Bluetooth device found.";
//        setAvailable(false);
//        return false;
//    }

//    // Turn Bluetooth on
//    localDevice.powerOn();

//    // Make it visible to others
//    localDevice.setHostMode(QBluetoothLocalDevice::HostDiscoverable);

    qCDebug(dcHardware()) << "-->" << name() << "created successfully.";

    setAvailable(true);
}

bool BluetoothLowEnergyManager::enable()
{
    // TODO: enable all devices created by this

    return true;
}

bool BluetoothLowEnergyManager::disable()
{
    return true;
}
