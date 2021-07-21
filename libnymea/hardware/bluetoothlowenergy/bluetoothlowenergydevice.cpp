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
* GNU Lesser General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU Lesser General Public License as published by the Free
* Software Foundation; version 3. This project is distributed in the hope that
* it will be useful, but WITHOUT ANY WARRANTY; without even the implied
* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
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

#ifdef WITH_BLUETOOTH

/*! Constructs a new BluetoothLowEnergyDevice with the given \a parent. */
BluetoothLowEnergyDevice::BluetoothLowEnergyDevice(QObject *parent) :
    QObject(parent)
{

}

#endif // WITH_BLUETOOTH
