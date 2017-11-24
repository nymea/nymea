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

BluetoothDiscoveryReply *BluetoothLowEnergyManager::discoverDevices(const int &interval)
{
    // Create the reply for this discovery request
    QPointer<BluetoothDiscoveryReply> reply = new BluetoothDiscoveryReply(this);
    if (!available()) {
        qCWarning(dcHardware()) << name() << "is not avilable.";
        reply->setError(BluetoothDiscoveryReply::BluetoothDiscoveryReplyErrorNotAvailable);
        reply->setFinished();
        return reply.data();
    }

    if (!enabled()) {
        qCWarning(dcHardware()) << name() << "is not enabled.";
        reply->setError(BluetoothDiscoveryReply::BluetoothDiscoveryReplyErrorNotEnabled);
        reply->setFinished();
        return reply.data();
    }

    if (!m_currentReply.isNull()) {
        qCWarning(dcHardware()) << name() << "resource busy. There is already a discovery running.";
        reply->setError(BluetoothDiscoveryReply::BluetoothDiscoveryReplyErrorBusy);
        reply->setFinished();
        return reply.data();
    }

    qCDebug(dcHardware) << name() << "start discovering";
    m_currentReply = reply;
    m_discoveredDevices.clear();

    // Start discovery on all adapters
    qCDebug(dcHardware()) << name() << "Start bluetooth discovery";
    foreach (QBluetoothDeviceDiscoveryAgent *discoveryAgent, m_bluetoothDiscoveryAgents) {
        discoveryAgent->start();
    }

    // TODO: limit interval to prevent resource blocking from a plugin
    m_timer->start(interval);

    return reply.data();
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

    connect(m_timer, &QTimer::timeout, this, &BluetoothLowEnergyManager::discoveryTimeout);

    qCDebug(dcHardware()) << "-->" << name() << "created successfully.";
    setAvailable(true);
}

void BluetoothLowEnergyManager::discoveryTimeout()
{
    // Stop discovery on all adapters
    qCDebug(dcHardware()) << name() << "Stop bluetooth discovery";
    foreach (QBluetoothDeviceDiscoveryAgent *discoveryAgent, m_bluetoothDiscoveryAgents) {
        discoveryAgent->stop();
    }

    qCDebug(dcHardware()) << name() << "Discovery finished. Found" << m_discoveredDevices.count() << "bluetooth devices.";

    if (m_currentReply.isNull()) {
        qCWarning(dcHardware()) << name() << "Reply does not exist any more. Please don't delete the reply before it has finished.";
        m_currentReply.clear();
        return;
    }

    m_currentReply->setError(BluetoothDiscoveryReply::BluetoothDiscoveryReplyErrorNoError);
    m_currentReply->setDiscoveredDevices(m_discoveredDevices);
    m_currentReply->setFinished();

    m_currentReply.clear();
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
