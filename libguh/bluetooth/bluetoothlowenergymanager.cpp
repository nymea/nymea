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

bool BluetoothLowEnergyManager::discoverDevices(QPointer<QObject> caller, const QString &callbackMethod)
{
    if (!available()) {
        qCWarning(dcHardware()) << name() << "is not avilable.";
        return false;
    }

    if (!enabled()) {
        qCWarning(dcHardware()) << name() << "is not enabled.";
        return false;
    }

    if (m_timer->isActive()) {
        qCWarning(dcHardware()) << name() << "discovery already running.";
        return false;
    }

    m_discoveredDevices.clear();
    m_currentReply.caller = caller;
    m_currentReply.callbackMethod = callbackMethod;

    // Start discovery on all adapters
    qCDebug(dcHardware()) << name() << "Start bluetooth discovery";
    foreach (QBluetoothDeviceDiscoveryAgent *discoveryAgent, m_bluetoothDiscoveryAgents) {
        discoveryAgent->start();
    }
    return true;
}

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
        qCDebug(dcHardware()) << name() << "Using adapter:" << hostInfo.name() << hostInfo.address().toString();
        QBluetoothLocalDevice localDevice(hostInfo.address());
        localDevice.powerOn();
        localDevice.setHostMode(QBluetoothLocalDevice::HostDiscoverable);

        QBluetoothDeviceDiscoveryAgent *discoveryAgent = new QBluetoothDeviceDiscoveryAgent(hostInfo.address(), this);
        connect(discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered, this, &BluetoothLowEnergyManager::onDeviceDiscovered);
        connect(discoveryAgent, SIGNAL(error(QBluetoothDeviceDiscoveryAgent::Error)), this, SLOT(onDiscoveryError(QBluetoothDeviceDiscoveryAgent::Error)));

        m_bluetoothDiscoveryAgents.append(discoveryAgent);
    }

    // Discovery timer
    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    m_timer->setInterval(5000);

    connect(m_timer, &QTimer::timeout, this, &BluetoothLowEnergyManager::discoveryTimeout);

    qCDebug(dcHardware()) << "-->" << name() << "created successfully.";
    setAvailable(true);
}

void BluetoothLowEnergyManager::discoveryTimeout()
{
    // Start discovery on all adapters
    qCDebug(dcHardware()) << name() << "Stop bluetooth discovery";
    foreach (QBluetoothDeviceDiscoveryAgent *discoveryAgent, m_bluetoothDiscoveryAgents) {
        discoveryAgent->stop();
    }

    qCDebug(dcHardware()) << name() << "Discovery finished. Found"<< m_discoveredDevices.count() << "Bluetooth devices.";

    if (m_currentReply.caller.isNull()) {
        qCWarning(dcHardware()) << name() << "The reciver of the discovery request does not exist any more.";
    } else {
        // Invoke the method containing the discovered devicelist
        // FIXME: check the callback method paramters during compililation
        QMetaObject::invokeMethod(m_currentReply.caller.data(), m_currentReply.callbackMethod.toLatin1().data(), Q_ARG(QList<QBluetoothDeviceInfo>, m_discoveredDevices));
    }
}

void BluetoothLowEnergyManager::onDeviceDiscovered(const QBluetoothDeviceInfo &deviceInfo)
{
    // Add the device to the list if not already added
    bool alreadyAdded = false;
    foreach (const QBluetoothDeviceInfo &device, m_discoveredDevices) {
        if (device.address() == deviceInfo.address()) {
            alreadyAdded = true;
        }
    }

    if (!alreadyAdded) {
        qCDebug(dcHardware()) << name() << "device discovered" << deviceInfo.name() <<deviceInfo.address().toString();
        m_discoveredDevices.append(deviceInfo);
    }
}

void BluetoothLowEnergyManager::onDiscoveryError(const QBluetoothDeviceDiscoveryAgent::Error &error)
{
    QBluetoothDeviceDiscoveryAgent *discoveryAgent = static_cast<QBluetoothDeviceDiscoveryAgent *>(sender());
    qCWarning(dcHardware()) << name() << "Discovery error:" << error << discoveryAgent->errorString();
}

bool BluetoothLowEnergyManager::enable()
{
    // TODO: enable all devices and trigger reconnect
    setEnabled(true);
    return true;
}

bool BluetoothLowEnergyManager::disable()
{
    // TODO: disable reconnect and disconnect all devices
    setEnabled(false);
    return true;
}
