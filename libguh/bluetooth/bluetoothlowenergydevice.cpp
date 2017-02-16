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

BluetoothLowEnergyDevice::BluetoothLowEnergyDevice(const QBluetoothDeviceInfo &deviceInfo, const QLowEnergyController::RemoteAddressType &addressType, QObject *parent) :
    QObject(parent), m_deviceInfo(deviceInfo)
{
    m_controller = new QLowEnergyController(address(), this);
    m_controller->setRemoteAddressType(addressType);

    connect(m_controller, &QLowEnergyController::connected, this, &BluetoothLowEnergyDevice::connected);
    connect(m_controller, &QLowEnergyController::disconnected, this, &BluetoothLowEnergyDevice::disconnected);
    connect(m_controller, SIGNAL(error(QLowEnergyController::Error)), this, SLOT(deviceError(QLowEnergyController::Error)));
    connect(m_controller, SIGNAL(discoveryFinished()), this, SIGNAL(servicesDiscoveryFinished()));
}

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

void BluetoothLowEnergyDevice::connectDevice()
{
    m_controller->connectToDevice();
}

void BluetoothLowEnergyDevice::reconnectDevice()
{
    if (!isConnected())
        m_controller->connectToDevice();
}

void BluetoothLowEnergyDevice::disconnectDevice()
{
    m_controller->disconnectFromDevice();
}

bool BluetoothLowEnergyDevice::isConnected() const
{
    return m_connected;
}

void BluetoothLowEnergyDevice::connected()
{
    m_connected = true;
    qCDebug(dcHardware) << "Connected to Bluetooth LE device:" << name() << address().toString();
    emit connectionStatusChanged();
    qCDebug(dcHardware) << "Discover Bluetooth LE services...";
    m_controller->discoverServices();
}

void BluetoothLowEnergyDevice::disconnected()
{
    m_connected = false;
    qCWarning(dcHardware) << "Disconnected from Bluetooth LE device:" << name() << address().toString();
    emit connectionStatusChanged();
}

void BluetoothLowEnergyDevice::deviceError(const QLowEnergyController::Error &error)
{
    if (isConnected())
        qCWarning(dcHardware)  << "Bluetooth LE device:" << name() << address().toString() << ": " << error << m_controller->errorString();
}
