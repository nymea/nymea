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
        qCWarning(dcBluetooth()) << "is not avilable.";
        reply->setError(BluetoothDiscoveryReply::BluetoothDiscoveryReplyErrorNotAvailable);
        reply->setFinished();
        return reply.data();
    }

    if (!enabled()) {
        qCWarning(dcBluetooth()) << "is not enabled.";
        reply->setError(BluetoothDiscoveryReply::BluetoothDiscoveryReplyErrorNotEnabled);
        reply->setFinished();
        return reply.data();
    }

    if (!m_currentReply.isNull()) {
        qCWarning(dcBluetooth()) << "resource busy. There is already a discovery running.";
        reply->setError(BluetoothDiscoveryReply::BluetoothDiscoveryReplyErrorBusy);
        reply->setFinished();
        return reply.data();
    }

    m_currentReply = reply;
    m_discoveredDevices.clear();

    // Start discovery on all adapters
    qCDebug(dcBluetooth()) << "Start bluetooth discovery";
    foreach (QBluetoothDeviceDiscoveryAgent *discoveryAgent, m_bluetoothDiscoveryAgents) {
        discoveryAgent->start();
    }

    // Prevent blocking the hardware resource from plugins
    int finalInterval = interval;
    if (finalInterval > 30) {
        qCWarning(dcBluetooth()) << "Discovery interval out of range. Reset to 30 seconds.";
        finalInterval = 30;
    }

    if (finalInterval <= 0) {
        qCWarning(dcBluetooth()) << "Discovery interval out of range. Reset to 5 seconds.";
        finalInterval = 5;
    }

    m_timer->start(finalInterval);
    return reply.data();
}

BluetoothLowEnergyDevice *BluetoothLowEnergyManager::registerDevice(const QBluetoothDeviceInfo &deviceInfo, const QLowEnergyController::RemoteAddressType &addressType)
{
    QPointer<BluetoothLowEnergyDevice> device = new BluetoothLowEnergyDevice(deviceInfo, addressType, this);
    m_devices.append(device);
    return device.data();
}

void BluetoothLowEnergyManager::unregisterDevice(BluetoothLowEnergyDevice *bluetoothDevice)
{
    QPointer<BluetoothLowEnergyDevice> devicePointer(bluetoothDevice);
    if (devicePointer.isNull()) {
        qCWarning(dcBluetooth()) << "Cannot unregister bluetooth device. Looks like the device is already unregistered.";
        return;
    }

    foreach (QPointer<BluetoothLowEnergyDevice> dPointer, m_devices) {
        if (devicePointer.data() == dPointer.data()) {
            m_devices.removeAll(dPointer);
            dPointer->deleteLater();
        }
    }
}

BluetoothLowEnergyManager::BluetoothLowEnergyManager(QObject *parent) :
    HardwareResource(HardwareResource::TypeBluetoothLE, "Bluetooth LE manager", parent)
{
    // Check which bluetooth adapter are available
    QList<QBluetoothHostInfo> bluetoothAdapters = QBluetoothLocalDevice::allDevices();
    if (bluetoothAdapters.isEmpty()) {
        qCWarning(dcBluetooth()) << "No bluetooth adapter found. Resource not available.";
        setAvailable(false);
        return;
    }

    // Create a scanner for each adapter
    foreach (const QBluetoothHostInfo &hostInfo, bluetoothAdapters) {
        qCDebug(dcBluetooth()) << "Using adapter:" << hostInfo.name() << hostInfo.address().toString();
        QBluetoothLocalDevice localDevice(hostInfo.address());
        localDevice.powerOn();
        localDevice.setHostMode(QBluetoothLocalDevice::HostDiscoverable);
        QBluetoothDeviceDiscoveryAgent *discoveryAgent = new QBluetoothDeviceDiscoveryAgent(hostInfo.address(), this);
        connect(discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered, this, &BluetoothLowEnergyManager::onDeviceDiscovered);
        connect(discoveryAgent, SIGNAL(error(QBluetoothDeviceDiscoveryAgent::Error)), this, SLOT(onDiscoveryError(QBluetoothDeviceDiscoveryAgent::Error)));
        m_bluetoothDiscoveryAgents.append(discoveryAgent);
    }

    // Discovery timer, interval depends on discovery call
    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    connect(m_timer, &QTimer::timeout, this, &BluetoothLowEnergyManager::onDiscoveryTimeout);

    qCDebug(dcHardware()) << "-->" << name() << "created successfully.";
    setAvailable(true);
}

void BluetoothLowEnergyManager::onDiscoveryTimeout()
{
    // Stop discovery on all adapters
    qCDebug(dcBluetooth()) << "Stop bluetooth discovery";
    foreach (QBluetoothDeviceDiscoveryAgent *discoveryAgent, m_bluetoothDiscoveryAgents) {
        discoveryAgent->stop();
    }

    qCDebug(dcBluetooth()) << "Discovery finished. Found" << m_discoveredDevices.count() << "bluetooth devices.";

    if (m_currentReply.isNull()) {
        qCWarning(dcBluetooth()) << "Reply does not exist any more. Please don't delete the reply before it has finished.";
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
        qCDebug(dcBluetooth()) << "device discovered" << deviceInfo.name() <<deviceInfo.address().toString();
        m_discoveredDevices.append(deviceInfo);
    }
}

void BluetoothLowEnergyManager::onDiscoveryError(const QBluetoothDeviceDiscoveryAgent::Error &error)
{
    QBluetoothDeviceDiscoveryAgent *discoveryAgent = static_cast<QBluetoothDeviceDiscoveryAgent *>(sender());
    qCWarning(dcBluetooth()) << "Discovery error:" << error << discoveryAgent->errorString();
}

bool BluetoothLowEnergyManager::enable()
{
    qCDebug(dcBluetooth()) << "Hardware resource enabled.";
    setEnabled(true);

    foreach (QPointer<BluetoothLowEnergyDevice> bluetoothDevice, m_devices) {
        bluetoothDevice->setEnabled(true);
    }

    return true;
}

bool BluetoothLowEnergyManager::disable()
{
    qCDebug(dcBluetooth()) << "Hardware resource disabled.";
    setEnabled(false);

    foreach (QPointer<BluetoothLowEnergyDevice> bluetoothDevice, m_devices) {
        bluetoothDevice->setEnabled(false);
    }

    return true;
}
