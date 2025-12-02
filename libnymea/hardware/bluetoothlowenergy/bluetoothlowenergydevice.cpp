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

/*!
    \class BluetoothLowEnergyDevice
    \brief The class represent a physical Bluetooth LE device.

    \ingroup hardware
    \inmodule libnymea


    \sa BluetoothLowEnergyManager
*/

/*! \fn BluetoothLowEnergyDevice::~BluetoothLowEnergyDevice();
    The virtual destructor of the BluetoothLowEnergyDevice.
*/

/*! \fn QString BluetoothLowEnergyDevice::name() const;
    Returns the advertise name of this BluetoothLowEnergyDevice.
*/

/*! \fn QBluetoothAddress BluetoothLowEnergyDevice::address() const;
    Returns the bluetooth adderss of this BluetoothLowEnergyDevice.
*/

/*! \fn void BluetoothLowEnergyDevice::connectDevice();
    Start connecting to this BluetoothLowEnergyDevice.

    \sa stateChanged, disconnectDevice
*/

/*! \fn void BluetoothLowEnergyDevice::disconnectDevice();
    Start disconnecting from this BluetoothLowEnergyDevice.

    \sa stateChanged, connectDevice
*/

/*! \fn bool BluetoothLowEnergyDevice::autoConnecting() const;
    Returns true, if this BluetoothLowEnergyDevice is reconnecting by it self when disconnected.

    \sa autoConnectingChanged
*/


/*! \fn void BluetoothLowEnergyDevice::setAutoConnecting(const bool &autoConnecting);
    Sets the reconnecting behaviour of this BluetoothLowEnergyDevice. If the \a autoConnecting is true,
    the BluetoothLowEnergyDevice will reconnect by it self on disconnected.

    \sa autoConnectingChanged
*/

/*! \fn bool BluetoothLowEnergyDevice::connected() const;
    Returns true, if this BluetoothLowEnergyDevice is currently connected.

    \sa connectedChanged
*/

/*! \fn bool BluetoothLowEnergyDevice::discovered() const;
    Returns true, if all services of this BluetoothLowEnergyDevice have been discovered.

    \sa serviceUuids
*/

/*! \fn QList<QBluetoothUuid> BluetoothLowEnergyDevice::serviceUuids() const;
    Returns the list of service uuids from this BluetoothLowEnergyDevice. The list contains only data,
    if the device has been discovered.

    \sa discovered
*/

/*! \fn QLowEnergyController *BluetoothLowEnergyDevice::controller() const;
    Returns the \l{http://doc.qt.io/qt-5/qlowenergycontroller.html}{QLowEnergyController} object of this BluetoothLowEnergyDevice
    in order to provide the full Qt Bluetooth LE functionality.
*/



/*! \fn void BluetoothLowEnergyDevice::connectedChanged(const bool &connected);
    This signal will be emitted whenever the \a connected state of this BluetoothLowEnergyDevice changed.

    \sa connected, connectDevice, disconnectDevice
*/

/*! \fn void BluetoothLowEnergyDevice::autoConnectingChanged(const bool &autoConnecting);
    This signal will be emitted whenever the \a autoConnecting state of this BluetoothLowEnergyDevice changed.

    \sa autoConnecting, setAutoConnecting
*/

/*! \fn void BluetoothLowEnergyDevice::stateChanged(const QLowEnergyController::ControllerState &state);
    This signal will be emitted whenever the \a state of this BluetoothLowEnergyDevice changed.
*/

/*! \fn void BluetoothLowEnergyDevice::errorOccurred(const QLowEnergyController::Error &error);
    This signal will be emitted whenever an \a error occurred.
*/

/*! \fn void BluetoothLowEnergyDevice::servicesDiscoveryFinished();
    This signal will be emitted whenever the service discovery for this BluetoothLowEnergyDevice is finished.

    \sa discovered, serviceUuids
*/

#include "bluetoothlowenergydevice.h"

/*! Constructs a new BluetoothLowEnergyDevice with the given \a parent. */
BluetoothLowEnergyDevice::BluetoothLowEnergyDevice(QObject *parent) :
    QObject(parent)
{

}
