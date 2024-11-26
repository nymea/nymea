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

#include "bluetoothlowenergydeviceimplementation.h"
#include "loggingcategories.h"

namespace nymeaserver {

BluetoothLowEnergyDeviceImplementation::BluetoothLowEnergyDeviceImplementation(const QBluetoothDeviceInfo &deviceInfo, const QLowEnergyController::RemoteAddressType &addressType, QObject *parent) :
    BluetoothLowEnergyDevice(parent),
    m_deviceInfo(deviceInfo)
{
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
    m_controller = QLowEnergyController::createCentral(deviceInfo, this);
#else
    m_controller = new QLowEnergyController(address(), this);
#endif
    m_controller->setRemoteAddressType(addressType);

    connect(m_controller, &QLowEnergyController::connected, this, &BluetoothLowEnergyDeviceImplementation::onConnected);
    connect(m_controller, &QLowEnergyController::disconnected, this, &BluetoothLowEnergyDeviceImplementation::onDisconnected);
    connect(m_controller, &QLowEnergyController::discoveryFinished, this, &BluetoothLowEnergyDeviceImplementation::onServiceDiscoveryFinished);
    connect(m_controller, &QLowEnergyController::stateChanged, this, &BluetoothLowEnergyDeviceImplementation::onStateChanged);
    connect(m_controller, SIGNAL(error(QLowEnergyController::Error)), this, SLOT(onDeviceError(QLowEnergyController::Error)));
}

QString BluetoothLowEnergyDeviceImplementation::name() const
{
    return m_deviceInfo.name();
}

QBluetoothAddress BluetoothLowEnergyDeviceImplementation::address() const
{
    return m_deviceInfo.address();
}

QLowEnergyController *BluetoothLowEnergyDeviceImplementation::controller() const
{
    return m_controller;
}


void BluetoothLowEnergyDeviceImplementation::setConnected(bool connected)
{
    if (m_connected != connected) {
        m_connected = connected;
        qCDebug(dcBluetooth()) << "Device" << name() << address().toString() << (connected ? "connected" : "disconnected");
        emit connectedChanged(m_connected);
    }
}

void BluetoothLowEnergyDeviceImplementation::onConnected()
{
    setConnected(true);

    if (m_controller->state() != QLowEnergyController::DiscoveredState) {
        qCDebug(dcBluetooth()) << "Discover services on" << name() << address().toString();
        m_controller->discoverServices();
    }
}

void BluetoothLowEnergyDeviceImplementation::onDisconnected()
{
    qCWarning(dcBluetooth()) << "Device disconnected" << name() << address().toString();
    setConnected(false);
}

void BluetoothLowEnergyDeviceImplementation::onServiceDiscoveryFinished()
{
    qCDebug(dcBluetooth()) << "Service discovery finished for" << name() << address().toString();
    foreach (const QBluetoothUuid &serviceUuid, m_controller->services()) {
        qCDebug(dcBluetooth()) << "--> Service" << serviceUuid.toString();
    }
    emit servicesDiscoveryFinished();
}

void BluetoothLowEnergyDeviceImplementation::onStateChanged(const QLowEnergyController::ControllerState &state)
{
    qCDebug(dcBluetooth()) << "State changed for" << name() << address().toString() << state;
    emit stateChanged(state);
}

void BluetoothLowEnergyDeviceImplementation::connectDevice()
{
    if (!m_enabled)
        return;

    // Only connect if not connected
    if (m_controller->state() != QLowEnergyController::UnconnectedState)
        return;

    m_controller->connectToDevice();
}

void BluetoothLowEnergyDeviceImplementation::disconnectDevice()
{
    m_controller->disconnectFromDevice();
}

bool BluetoothLowEnergyDeviceImplementation::autoConnecting() const
{
    return m_autoConnecting;
}

void BluetoothLowEnergyDeviceImplementation::setAutoConnecting(const bool &autoConnecting)
{
    if (m_autoConnecting != autoConnecting) {
        m_autoConnecting = autoConnecting;
        emit autoConnectingChanged(m_autoConnecting);
    }
}

bool BluetoothLowEnergyDeviceImplementation::connected() const
{
    return m_connected;
}

bool BluetoothLowEnergyDeviceImplementation::discovered() const
{
    return m_discovered;
}

QList<QBluetoothUuid> BluetoothLowEnergyDeviceImplementation::serviceUuids() const
{
    return m_controller->services();
}

void BluetoothLowEnergyDeviceImplementation::onDeviceError(const QLowEnergyController::Error &error)
{
    qCWarning(dcBluetooth())  << "Device error:" << name() << address().toString() << ": " << error << m_controller->errorString();

    emit errorOccurred(error);
}

}
