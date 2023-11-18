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

#include "bluetoothlowenergymanagerimplementation.h"
#include "loggingcategories.h"

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusReply>

namespace nymeaserver {

BluetoothLowEnergyManagerImplementation::BluetoothLowEnergyManagerImplementation(PluginTimer *reconnectTimer, QObject *parent) :
    BluetoothLowEnergyManager(parent),
    m_reconnectTimer(reconnectTimer)
{
    // QBluetooth hangs for the D-Bus timeout if BlueZ is not available. In order to avoid that, let's first check
    // ourselves if bluez is registered on D-Bus.
    QDBusReply<QStringList> reply = QDBusConnection::systemBus().interface()->registeredServiceNames();
    if (!reply.isValid()) {
        qWarning(dcBluetooth()) << "Unable to query D-Bus for bluez:" << reply.error().message();
        return;
    }
    const QStringList services = reply.value();
    if (!services.contains("org.bluez")) {
        qCWarning(dcBluetooth()) << "BlueZ not found on D-Bus. Skipping Bluetooth initialisation.";
        return;
    }

    foreach (const QBluetoothHostInfo &hostInfo, QBluetoothLocalDevice::allDevices()) {
        qCDebug(dcBluetooth()) << "Enalbing bluetooth adapter:" << hostInfo.name() << hostInfo.address().toString();
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
                reply->addDiscoveredDevice(info);
            }
        });
        connect(discoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished, reply, [=](){
            qCDebug(dcBluetooth()) << "Discovery finished";
            reply->setFinished();
        });
//        connect(discoveryAgent, SIGNAL(error(QBluetoothDeviceDiscoveryAgent::Error)), this, SLOT(onDiscoveryError(QBluetoothDeviceDiscoveryAgent::Error)));

        discoveryAgent->start();
    }

    return reply;
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
