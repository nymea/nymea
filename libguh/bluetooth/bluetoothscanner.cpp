/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  This file is part of guh.                                              *
 *                                                                         *
 *  Guh is free software: you can redistribute it and/or modify            *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  Guh is distributed in the hope that it will be useful,                 *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with guh. If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "bluetoothscanner.h"

BluetoothScanner::BluetoothScanner(QObject *parent) :
    QObject(parent)
{
    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    m_timer->setInterval(5000);
    connect(m_timer, &QTimer::timeout, this, &BluetoothScanner::discoveryTimeout);
}

bool BluetoothScanner::isAvailable()
{    
    //Using default Bluetooth adapter
    QBluetoothLocalDevice localDevice;

    // Check if Bluetooth is available on this device
    if (!localDevice.isValid()) {
        qWarning() << "ERROR: no bluetooth device found.";
        m_available = false;
        return false;
    }

    // Turn Bluetooth on
    localDevice.powerOn();

    // Make it visible to others
    localDevice.setHostMode(QBluetoothLocalDevice::HostDiscoverable);

    // Get connected devices
    QList<QBluetoothHostInfo> remotes = localDevice.allDevices();
    if (remotes.isEmpty()) {
        qWarning() << "ERROR: no bluetooth host info found.";
        m_available = false;
        return false;
    }

    QBluetoothHostInfo hostInfo = remotes.first();

    // Create a discovery agent and connect to its signals
    m_discoveryAgent = new QBluetoothDeviceDiscoveryAgent(hostInfo.address(), this);

    connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered, this, &BluetoothScanner::deviceDiscovered);
    connect(m_discoveryAgent, SIGNAL(error(QBluetoothDeviceDiscoveryAgent::Error)), this, SLOT(onError(QBluetoothDeviceDiscoveryAgent::Error)));

    qDebug() << "--> Bluetooth discovery created successfully.";
    m_available = true;
    return true;
}

bool BluetoothScanner::isRunning()
{
    return m_discoveryAgent->isActive();
}

bool BluetoothScanner::discover(const PluginId &pluginId)
{
    if (m_available && !m_discoveryAgent->isActive()) {
        m_pluginId = pluginId;
        m_deviceInfos.clear();
        m_discoveryAgent->start();
        m_timer->start();
        qDebug() << "Bluetooth discovery started...";
        return true;
    }
    return false;
}

void BluetoothScanner::deviceDiscovered(const QBluetoothDeviceInfo &device)
{
    // check if this is a LE device
    bool bluetoothLE = false;
    if (device.coreConfigurations() & QBluetoothDeviceInfo::LowEnergyCoreConfiguration) {
        bluetoothLE = true;
    }
    qDebug() << "Bluetooth device discovered:" << device.name() << device.address() << "LE:" << bluetoothLE;

    m_deviceInfos.append(device);
}

void BluetoothScanner::onError(QBluetoothDeviceDiscoveryAgent::Error error)
{
    Q_UNUSED(error)

    m_available = false;

    if (m_timer->isActive())
        m_timer->stop();

    if (isRunning())
        m_discoveryAgent->stop();

    qWarning() << "ERROR: Bluetooth discovery:" << m_discoveryAgent->errorString();
}

void BluetoothScanner::discoveryTimeout()
{
    qDebug() << "Bluetooth discovery finished.";
    m_discoveryAgent->stop();
    emit bluetoothDiscoveryFinished(m_pluginId, m_deviceInfos);
}
