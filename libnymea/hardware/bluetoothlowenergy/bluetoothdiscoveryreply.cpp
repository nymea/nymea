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
    \class BluetoothDiscoveryReply
    \brief The BluetoothDiscoveryReply class contains the result and interaction of a discovery request done with the BluetoothLowEnergyManager.

    \ingroup hardware
    \inmodule libnymea

    \sa BluetoothLowEnergyManager
*/

/*! \fn BluetoothDiscoveryReply::~BluetoothDiscoveryReply();
    The virtual destructor of the BluetoothDiscoveryReply.
*/

/*! \enum BluetoothDiscoveryReply::BluetoothDiscoveryReplyError
    This enum represents the possible errors of a BluetoothDiscoveryReply.

    \value BluetoothDiscoveryReplyErrorNoError
        No error occurred. Everything is fine.
    \value BluetoothDiscoveryReplyErrorNotAvailable
        The discovery could not be performed because there is no Bluetooth hardware available.
    \value BluetoothDiscoveryReplyErrorNotEnabled
        The discovery could not be performed because there is no Bluetooth hardware resource in nymea is disabled.
    \value BluetoothDiscoveryReplyErrorBusy
        The resource is currently busy.

*/

/*! \fn bool BluetoothDiscoveryReply::isFinished() const;
    Returns true if this discovery replay is finished.

    \sa finished, discoveredDevices
*/

/*! \fn bool BluetoothDiscoveryReply::finished();
    This signal will be emitted whenever the discovery for this BluetoothDiscoveryReply is finished.
    You can get the result of the discovery from discoveredDevices();

    \sa isFinished, discoveredDevices
*/

/*! \fn void BluetoothDiscoveryReply::errorOccurred(const BluetoothDiscoveryReplyError &error);
    This signal will be emitted whenever an \a error occurred.

    \sa error, BluetoothDiscoveryReplyError
*/

/*! \fn QList<QBluetoothDeviceInfo> BluetoothDiscoveryReply::discoveredDevices() const;
    Returns the list of discovered \l{https://doc.qt.io/qt-5/qbluetoothdeviceinfo.html}{QBluetoothDeviceInfo}.

    \sa isFinished, discoveredDevices
*/

/*! \fn BluetoothDiscoveryReplyError BluetoothDiscoveryReply::error() const;
    Returns the current error of this BluetoothDiscoveryReply.

    \sa BluetoothDiscoveryReplyError
*/

#include "bluetoothdiscoveryreply.h"

/*! Constructs a new BluetoothDiscoveryReply with the given \a parent. */
BluetoothDiscoveryReply::BluetoothDiscoveryReply(QObject *parent)
    : QObject(parent)
{}
