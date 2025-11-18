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
  \class UpnpDiscoveryReply
  \brief Allows to handle UPnP discovery request in the network.

  \ingroup hardware
  \inmodule libnymea

  \sa UpnpDevice, UpnpDiscovery
*/

/*! \enum UpnpDiscoveryReply::UpnpDiscoveryReplyError

    \value UpnpDiscoveryReplyErrorNoError
        The reply finished successfully.
    \value UpnpDiscoveryReplyErrorNotAvailable
        The UpnpDiscovery HardwareResource is not available.
    \value UpnpDiscoveryReplyErrorNotEnabled
        The UpnpDiscovery HardwareResource is not enabled.
    \value UpnpDiscoveryReplyErrorResourceBusy
        The UpnpDiscovery HardwareResource is currently busy.
*/

// Public
/*! \fn UpnpDiscoveryReply::~UpnpDiscoveryReply();
    The virtual destructor of the UpnpDiscoveryReply.
*/

/*! \fn int UpnpDiscoveryReply::searchTarget() const;
    Returns the search target which was used for this UpnpDiscovery request.

    \sa UpnpDiscovery::discoverDevices()
*/

/*! \fn int UpnpDiscoveryReply::userAgent() const;
    Returns the user agent which was used for this UpnpDiscovery request.

    \sa UpnpDiscovery::discoverDevices()
*/

/*! \fn UpnpDiscoveryReplyError UpnpDiscoveryReply::error() const;
    Returns the current error of this UpnpDiscoveryReply.

    \sa UpnpDiscoveryReplyError
*/

/*! \fn bool UpnpDiscoveryReply::isFinished() const;
    Returns true if this UpnpDiscoveryReply is finished.

    \sa UpnpDiscoveryReplyError
*/

/*! \fn QList<UpnpDeviceDescriptor> UpnpDiscoveryReply::deviceDescriptors() const;
    Returns the list of found \l{UpnpDeviceDescriptor}{UpnpDeviceDescriptors}. This list will be empty if an error occurred.

    \sa finished()
*/

// Signals
/*! \fn void UpnpDiscoveryReply::finished();
    This signal will be emitted once the UpnpDiscoveryReply is finished.
*/

/*! \fn void UpnpDiscoveryReply::errorOccurred(const UpnpDiscoveryReplyError &error);
    This signal will be emitted once an UpnpDiscoveryReply \a error occurred.
*/

#include "upnpdiscoveryreply.h"

/*! Construct a new UpnpDiscoveryReply with the given \a parent. */
UpnpDiscoveryReply::UpnpDiscoveryReply(QObject *parent) :
    QObject(parent)
{

}
