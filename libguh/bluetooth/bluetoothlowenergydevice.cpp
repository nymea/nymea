/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
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

#include "bluetoothlowenergydevice.h"
#include "loggingcategories.h"

QString BluetoothLowEnergyDevice::name() const
{
    return m_deviceInfo.name();
}

QBluetoothAddress BluetoothLowEnergyDevice::address() const
{
    return m_deviceInfo.address();
}

QLowEnergyController *BluetoothLowEnergyDevice::controller() const
{
    return m_controller;
}

BluetoothLowEnergyDevice::BluetoothLowEnergyDevice(const QBluetoothDeviceInfo &deviceInfo, const QLowEnergyController::RemoteAddressType &addressType, QObject *parent) :
    QObject(parent), m_deviceInfo(deviceInfo)
{
    m_controller = new QLowEnergyController(address(), this);
    m_controller->setRemoteAddressType(addressType);

    connect(m_controller, &QLowEnergyController::connected, this, &BluetoothLowEnergyDevice::onConnected);
    connect(m_controller, &QLowEnergyController::disconnected, this, &BluetoothLowEnergyDevice::onDisconnected);
    connect(m_controller, &QLowEnergyController::discoveryFinished, this, &BluetoothLowEnergyDevice::onServiceDiscoveryFinished);
    connect(m_controller, &QLowEnergyController::stateChanged, this, &BluetoothLowEnergyDevice::onStateChanged);
    connect(m_controller, SIGNAL(error(QLowEnergyController::Error)), this, SLOT(onDeviceError(QLowEnergyController::Error)));
}

void BluetoothLowEnergyDevice::setConnected(const bool &connected)
{
    if (m_connected != connected) {
        m_connected = connected;
        qCDebug(dcBluetooth()) << "Device" << name() << address().toString() << (connected ? "connected" : "disconnected");
        emit connectedChanged(m_connected);
    }
}

void BluetoothLowEnergyDevice::setEnabled(const bool &enabled)
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

void BluetoothLowEnergyDevice::onConnected()
{
    setConnected(true);

    if (m_controller->state() != QLowEnergyController::DiscoveredState) {
        qCDebug(dcBluetooth()) << "Discover services on" << name() << address().toString();
        m_controller->discoverServices();
    }
}

void BluetoothLowEnergyDevice::onDisconnected()
{
    qCWarning(dcBluetooth()) << "Device disconnected" << name() << address().toString();
    setConnected(false);
}

void BluetoothLowEnergyDevice::onServiceDiscoveryFinished()
{
    qCDebug(dcBluetooth()) << "Service discovery finished for" << name() << address().toString();
    foreach (const QBluetoothUuid &serviceUuid, m_controller->services()) {
        qCDebug(dcBluetooth()) << "--> Service" << serviceUuid.toString();
    }
    emit servicesDiscoveryFinished();
}

void BluetoothLowEnergyDevice::onStateChanged(const QLowEnergyController::ControllerState &state)
{
    qCDebug(dcBluetooth()) << "State changed for" << name() << address().toString() << state;
    emit stateChanged(state);
}

void BluetoothLowEnergyDevice::connectDevice()
{
    if (!m_enabled)
        return;

    // Only connect if not connected
    if (m_controller->state() != QLowEnergyController::UnconnectedState)
        return;

    m_controller->connectToDevice();
}

void BluetoothLowEnergyDevice::disconnectDevice()
{
    m_controller->disconnectFromDevice();
}

bool BluetoothLowEnergyDevice::autoConnecting() const
{
    return m_autoConnecting;
}

void BluetoothLowEnergyDevice::setAutoConnecting(const bool &autoConnecting)
{
    if (m_autoConnecting != autoConnecting) {
        m_autoConnecting = autoConnecting;
        emit autoConnectingChanged(m_autoConnecting);
    }
}

bool BluetoothLowEnergyDevice::connected() const
{
    return m_connected;
}

bool BluetoothLowEnergyDevice::discovered() const
{
    return m_discovered;
}

QList<QBluetoothUuid> BluetoothLowEnergyDevice::serviceUuids() const
{
    return m_controller->services();
}

void BluetoothLowEnergyDevice::onDeviceError(const QLowEnergyController::Error &error)
{
    if (connected())
        qCWarning(dcBluetooth())  << "Device error:" << name() << address().toString() << ": " << error << m_controller->errorString();

    emit errorOccured(error);
}
