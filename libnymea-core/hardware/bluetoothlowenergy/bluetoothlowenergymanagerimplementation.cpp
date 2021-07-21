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

#ifdef WITH_BLUETOOTH

#include "bluetoothlowenergymanagerimplementation.h"
#include "loggingcategories.h"

namespace nymeaserver {

BluetoothLowEnergyManagerImplementation::BluetoothLowEnergyManagerImplementation(PluginTimer *reconnectTimer, QObject *parent) :
    BluetoothLowEnergyManager(parent),
    m_reconnectTimer(reconnectTimer)
{
    // Check which bluetooth adapter are available
    QList<QBluetoothHostInfo> bluetoothAdapters = QBluetoothLocalDevice::allDevices();
    if (bluetoothAdapters.isEmpty()) {
        qCWarning(dcBluetooth()) << "No bluetooth adapter found. Resource not available.";
        m_available = false;
        return;
    }

    // Create a scanner for each adapter
    foreach (const QBluetoothHostInfo &hostInfo, bluetoothAdapters) {
        qCDebug(dcBluetooth()) << "Using adapter:" << hostInfo.name() << hostInfo.address().toString();
        QBluetoothLocalDevice localDevice(hostInfo.address());
        localDevice.powerOn();
        localDevice.setHostMode(QBluetoothLocalDevice::HostDiscoverable);
        QBluetoothDeviceDiscoveryAgent *discoveryAgent = new QBluetoothDeviceDiscoveryAgent(hostInfo.address(), this);
        connect(discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered, this, &BluetoothLowEnergyManagerImplementation::onDeviceDiscovered);
        connect(discoveryAgent, SIGNAL(error(QBluetoothDeviceDiscoveryAgent::Error)), this, SLOT(onDiscoveryError(QBluetoothDeviceDiscoveryAgent::Error)));
        m_bluetoothDiscoveryAgents.append(discoveryAgent);
    }

    // Discovery timer, interval depends on discovery call
    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    connect(m_timer, &QTimer::timeout, this, &BluetoothLowEnergyManagerImplementation::onDiscoveryTimeout);

    // Reconnect timer
    connect(m_reconnectTimer, &PluginTimer::timeout, this, &BluetoothLowEnergyManagerImplementation::onReconnectTimeout);

    qCDebug(dcHardware()) << "-->" << name() << "created successfully.";
    m_available = true;
}

BluetoothDiscoveryReply *BluetoothLowEnergyManagerImplementation::discoverDevices(int interval)
{
    // Create the reply for this discovery request
    QPointer<BluetoothDiscoveryReplyImplementation> reply = new BluetoothDiscoveryReplyImplementation(this);
    if (!available()) {
        qCWarning(dcBluetooth()) << "is not avilable.";
        reply->setError(BluetoothDiscoveryReplyImplementation::BluetoothDiscoveryReplyErrorNotAvailable);
        reply->setFinished();
        return reply.data();
    }

    if (!enabled()) {
        qCWarning(dcBluetooth()) << "is not enabled.";
        reply->setError(BluetoothDiscoveryReplyImplementation::BluetoothDiscoveryReplyErrorNotEnabled);
        reply->setFinished();
        return reply.data();
    }

    if (!m_currentReply.isNull()) {
        qCWarning(dcBluetooth()) << "resource busy. There is already a discovery running.";
        reply->setError(BluetoothDiscoveryReplyImplementation::BluetoothDiscoveryReplyErrorBusy);
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
    if (finalInterval > 30000) {
        qCWarning(dcBluetooth()) << "Discovery interval out of range. Reset to 30 seconds.";
        finalInterval = 30000;
    }

    if (finalInterval <= 0) {
        qCWarning(dcBluetooth()) << "Discovery interval out of range. Reset to 5 seconds.";
        finalInterval = 5000;
    }

    m_timer->start(finalInterval);
    return reply.data();
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

    bool success = false;
    if (enabled) {
        success = enable();
    } else {
        success = disable();
    }

    if (success) {
        m_enabled = enabled;
        emit enabledChanged(m_enabled);
    }
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

void BluetoothLowEnergyManagerImplementation::onDiscoveryTimeout()
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

void BluetoothLowEnergyManagerImplementation::onDeviceDiscovered(const QBluetoothDeviceInfo &deviceInfo)
{
    // Add the device to the list if not already added
    bool alreadyAdded = false;
    foreach (const QBluetoothDeviceInfo &device, m_discoveredDevices) {
        if (device.address() == deviceInfo.address()) {
            alreadyAdded = true;
        }
    }

    // Note: only show low energy devices
    if (!alreadyAdded && deviceInfo.coreConfigurations() & QBluetoothDeviceInfo::LowEnergyCoreConfiguration) {
        qCDebug(dcBluetooth()) << "device discovered" << deviceInfo.name() << deviceInfo.address().toString();
        m_discoveredDevices.append(deviceInfo);
    }
}

void BluetoothLowEnergyManagerImplementation::onDiscoveryError(const QBluetoothDeviceDiscoveryAgent::Error &error)
{
    QBluetoothDeviceDiscoveryAgent *discoveryAgent = static_cast<QBluetoothDeviceDiscoveryAgent *>(sender());
    qCWarning(dcBluetooth()) << "Discovery error:" << error << discoveryAgent->errorString();
}

bool BluetoothLowEnergyManagerImplementation::enable()
{
    if (!available()) {
        qCWarning(dcBluetooth()) << "Bluetooth hardware not available. Cannot enable Hardware resource";
        return false;
    }
    foreach (QPointer<BluetoothLowEnergyDeviceImplementation> bluetoothDevice, m_devices) {
        bluetoothDevice->setEnabled(true);
    }
    qCDebug(dcBluetooth()) << "Hardware resource enabled.";
    return true;
}

bool BluetoothLowEnergyManagerImplementation::disable()
{
    foreach (QPointer<BluetoothLowEnergyDeviceImplementation> bluetoothDevice, m_devices) {
        bluetoothDevice->setEnabled(false);
    }
    qCDebug(dcBluetooth()) << "Hardware resource disabled.";
    return true;
}

}

#endif // WITH_BLUETOOTH
