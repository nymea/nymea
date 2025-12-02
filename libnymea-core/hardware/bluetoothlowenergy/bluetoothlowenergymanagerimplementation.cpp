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

#include "bluetoothlowenergymanagerimplementation.h"
#include "loggingcategories.h"
#include "nymeabluetoothagent.h"
#include "bluetoothpairingjobimplementation.h"

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusReply>
#include <QDBusInterface>

namespace nymeaserver {

BluetoothLowEnergyManagerImplementation::BluetoothLowEnergyManagerImplementation(PluginTimer *reconnectTimer, QObject *parent) :
    BluetoothLowEnergyManager(parent),
    m_reconnectTimer(reconnectTimer)
{
    // QBluetooth hangs for the D-Bus timeout if BlueZ is not available. In order to avoid that, let's first check
    // ourselves if bluez is registered on D-Bus.
    QDBusReply<QStringList> reply = QDBusConnection::systemBus().interface()->registeredServiceNames();
    if (!reply.isValid()) {
        qCWarning(dcBluetooth()) << "Unable to query D-Bus for bluez:" << reply.error().message();
        return;
    }
    const QStringList services = reply.value();
    if (!services.contains("org.bluez")) {
        qCWarning(dcBluetooth()) << "BlueZ not found on D-Bus. Skipping Bluetooth initialisation.";
        return;
    }

    m_agent = new NymeaBluetoothAgent(this);

    foreach (const QBluetoothHostInfo &hostInfo, QBluetoothLocalDevice::allDevices()) {
        qCDebug(dcBluetooth()) << "Enabling bluetooth adapter:" << hostInfo.name() << hostInfo.address().toString();
        QBluetoothLocalDevice localDevice(hostInfo.address());
        localDevice.powerOn();
        localDevice.setHostMode(QBluetoothLocalDevice::HostDiscoverable);
    }

    qCDebug(dcHardware()) << "-->" << name() << "created successfully.";
    m_available = true;
}

BluetoothDiscoveryReply *BluetoothLowEnergyManagerImplementation::discoverDevices(int timeout)
{
    BluetoothDiscoveryReplyImplementation *reply = new BluetoothDiscoveryReplyImplementation(this);
    if (!available()) {
        qCWarning(dcBluetooth()) << "is not avilable.";
        reply->setError(BluetoothDiscoveryReplyImplementation::BluetoothDiscoveryReplyErrorNotAvailable);
        reply->setFinished();
        return reply;
    }
    if (!enabled()) {
        qCWarning(dcBluetooth()) << "is not enabled.";
        reply->setError(BluetoothDiscoveryReplyImplementation::BluetoothDiscoveryReplyErrorNotEnabled);
        reply->setFinished();
        return reply;
    }

    if (QBluetoothLocalDevice::allDevices().isEmpty()) {
        qCWarning(dcBluetooth()) << "No Bluetooth adapter available.";
        reply->setError(BluetoothDiscoveryReplyImplementation::BluetoothDiscoveryReplyErrorNotAvailable);
        reply->setFinished();
        return reply;
    }

    // Create a scanner for each adapter
    foreach (const QBluetoothHostInfo &hostInfo, QBluetoothLocalDevice::allDevices()) {
        qCDebug(dcBluetooth()) << "Starting discovery on adapter:" << hostInfo.name() << hostInfo.address().toString();
        QBluetoothLocalDevice localDevice(hostInfo.address());
        localDevice.powerOn();
        localDevice.setHostMode(QBluetoothLocalDevice::HostDiscoverable);
        QBluetoothDeviceDiscoveryAgent *discoveryAgent = new QBluetoothDeviceDiscoveryAgent(hostInfo.address(), reply);
        discoveryAgent->setLowEnergyDiscoveryTimeout(qMax(5000, qMin(30000, timeout)));
        connect(discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered, reply, [=](const QBluetoothDeviceInfo &info){
            // Note: only show low energy devices
            if (info.coreConfigurations() & QBluetoothDeviceInfo::LowEnergyCoreConfiguration) {
                qCDebug(dcBluetooth()) << "device discovered" << info.name() << info.address().toString();
                reply->addDiscoveredDevice(info, hostInfo);
            }
        });
        connect(discoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished, reply, [=](){
            qCDebug(dcBluetooth()) << "Discovery finished";
            reply->setFinished();
        });

        discoveryAgent->start();
    }

    return reply;
}

BluetoothPairingJob *BluetoothLowEnergyManagerImplementation::pairDevice(const QBluetoothAddress &device, const QBluetoothAddress &adapter)
{
    qCDebug(dcBluetooth()) << "pairDevice";
    BluetoothPairingJobImplementation *job = new BluetoothPairingJobImplementation(m_agent, device, this);

    QBluetoothLocalDevice *localDevice = nullptr;
    if (adapter.isNull()) {
        localDevice = new QBluetoothLocalDevice(this);
    } else {
        localDevice = new QBluetoothLocalDevice(adapter, this);
    }

    if (!localDevice->isValid()) {
        qCWarning(dcBluetooth()) << "Local device" << adapter.toString() << "is not valid";
        job->finish(false);
        return job;
    }
    localDevice->requestPairing(device, QBluetoothLocalDevice::AuthorizedPaired);
#if QT_VERSION >= QT_VERSION_CHECK(6,2,0)
    connect(localDevice, &QBluetoothLocalDevice::errorOccurred, job, [=](QBluetoothLocalDevice::Error error){
        qCDebug(dcBluetooth()) << "Pairing error" << error;
        job->finish(false);
        localDevice->deleteLater();
    });
#else
    connect(localDevice, &QBluetoothLocalDevice::error, job, [=](QBluetoothLocalDevice::Error error){
        qCDebug(dcBluetooth()) << "Pairing error" << error;
        job->finish(false);
        localDevice->deleteLater();
    });
#endif
    connect(localDevice, &QBluetoothLocalDevice::pairingFinished, job, [=](const QBluetoothAddress &address, QBluetoothLocalDevice::Pairing pairing){
        qCDebug(dcBluetooth()) << "Pairing finished" << address.toString() << pairing;
        job->finish(true);
        localDevice->deleteLater();
    });
    return job;
}

void BluetoothLowEnergyManagerImplementation::unpairDevice(const QBluetoothAddress &device, const QBluetoothAddress &adapter)
{
    QBluetoothLocalDevice *localDevice = nullptr;
    if (adapter.isNull()) {
        localDevice = new QBluetoothLocalDevice(this);
    } else {
        localDevice = new QBluetoothLocalDevice(adapter, this);
    }

    if (!localDevice->isValid()) {
        qCWarning(dcBluetooth()) << "Local device" << adapter.toString() << "is not valid";
        return;
    }
    localDevice->requestPairing(device, QBluetoothLocalDevice::Unpaired);
}

BluetoothLowEnergyDevice *BluetoothLowEnergyManagerImplementation::registerDevice(const QBluetoothDeviceInfo &deviceInfo, const QLowEnergyController::RemoteAddressType &addressType)
{
    QPointer<BluetoothLowEnergyDeviceImplementation> bluetoothDevice = new BluetoothLowEnergyDeviceImplementation(deviceInfo, addressType, this);
    qCDebug(dcBluetooth()) << "Register device" << bluetoothDevice->name() << bluetoothDevice->address().toString();
    m_devices.append(bluetoothDevice);
    return bluetoothDevice.data();
}

void BluetoothLowEnergyManagerImplementation::unregisterDevice(BluetoothLowEnergyDevice *bluetoothDevice)
{
    QPointer<BluetoothLowEnergyDevice> devicePointer(bluetoothDevice);
    if (devicePointer.isNull()) {
        qCWarning(dcBluetooth()) << "Cannot unregister bluetooth device. Looks like the device is already unregistered.";
        return;
    }

    qCDebug(dcBluetooth()) << "Unregister device" << bluetoothDevice->name() << bluetoothDevice->address().toString();

    foreach (QPointer<BluetoothLowEnergyDeviceImplementation> dPointer, m_devices) {
        if (devicePointer.data() == dPointer.data()) {
            m_devices.removeAll(dPointer);
            dPointer->deleteLater();
        }
    }
}

bool BluetoothLowEnergyManagerImplementation::available() const
{
    return m_available;
}

bool BluetoothLowEnergyManagerImplementation::enabled() const
{
    return m_enabled;
}

void BluetoothLowEnergyManagerImplementation::setEnabled(bool enabled)
{
    qCDebug(dcBluetooth()) << "Set" << (enabled ? "enabled" : "disabled");
    if (m_enabled && enabled) {
        qCDebug(dcBluetooth()) << "Already enabled.";
        return;
    } else if (!m_enabled && !enabled) {
        qCDebug(dcBluetooth()) << "Already disabled.";
        return;
    }

    foreach (QBluetoothHostInfo info, QBluetoothLocalDevice::allDevices()) {
        QBluetoothLocalDevice localDevice(info.address());
        if (enabled) {
            localDevice.setHostMode(QBluetoothLocalDevice::HostConnectable);
        } else {
            localDevice.setHostMode(QBluetoothLocalDevice::HostPoweredOff);
        }
    }

    m_enabled = enabled;
    emit enabledChanged(m_enabled);
}

void BluetoothLowEnergyManagerImplementation::onReconnectTimeout()
{
    // Reconnect device if enabled and disconnected
    foreach (BluetoothLowEnergyDevice *device, m_devices) {
        if (device->autoConnecting() && !device->connected()) {
            device->connectDevice();
        }
    }
}

}
