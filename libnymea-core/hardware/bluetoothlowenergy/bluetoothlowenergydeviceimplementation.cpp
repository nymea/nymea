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

#include "bluetoothlowenergydeviceimplementation.h"
#include "loggingcategories.h"

namespace guhserver {

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

BluetoothLowEnergyDeviceImplementation::BluetoothLowEnergyDeviceImplementation(const QBluetoothDeviceInfo &deviceInfo, const QLowEnergyController::RemoteAddressType &addressType, QObject *parent) :
    BluetoothLowEnergyDevice(parent),
    m_deviceInfo(deviceInfo)
{
    m_controller = new QLowEnergyController(address(), this);
    m_controller->setRemoteAddressType(addressType);

    connect(m_controller, &QLowEnergyController::connected, this, &BluetoothLowEnergyDeviceImplementation::onConnected);
    connect(m_controller, &QLowEnergyController::disconnected, this, &BluetoothLowEnergyDeviceImplementation::onDisconnected);
    connect(m_controller, &QLowEnergyController::discoveryFinished, this, &BluetoothLowEnergyDeviceImplementation::onServiceDiscoveryFinished);
    connect(m_controller, &QLowEnergyController::stateChanged, this, &BluetoothLowEnergyDeviceImplementation::onStateChanged);
    connect(m_controller, SIGNAL(error(QLowEnergyController::Error)), this, SLOT(onDeviceError(QLowEnergyController::Error)));
}

void BluetoothLowEnergyDeviceImplementation::setConnected(const bool &connected)
{
    if (m_connected != connected) {
        m_connected = connected;
        qCDebug(dcBluetooth()) << "Device" << name() << address().toString() << (connected ? "connected" : "disconnected");
        emit connectedChanged(m_connected);
    }
}

void BluetoothLowEnergyDeviceImplementation::setEnabled(const bool &enabled)
{
    m_enabled = enabled;

    if (!m_enabled) {
        m_controller->disconnectFromDevice();
    } else {
        if (m_autoConnecting) {
            m_controller->connectToDevice();
        }
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
    if (connected())
        qCWarning(dcBluetooth())  << "Device error:" << name() << address().toString() << ": " << error << m_controller->errorString();

    emit errorOccured(error);
}

}
