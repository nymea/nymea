/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015-2018 Simon Stürz <simon.stuerz@guh.io>              *
 *                                                                         *
 *  This file is part of nymea.                                            *
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

/*!
  \class UpnpDiscovery
  \brief Allows to detect UPnP devices in the network.

  \ingroup hardware
  \inmodule libnymea

  This resource allows plugins to discover UPnP devices in the network and receive notification messages. The resource
  will bind a UDP socket to the multicast 239.255.255.250 on port 1900.

  The communication was implementet using following documentation: \l{http://upnp.org/specs/arch/UPnP-arch-DeviceArchitecture-v1.1.pdf}

  \sa UpnpDevice, UpnpDeviceDescriptor
*/

/*!
 \fn UpnpDiscovery::upnpNotify(const QByteArray &notifyMessage)
 This signal will be emitted when a UPnP NOTIFY message \a notifyMessage will be recognized.
 \sa DevicePlugin::upnpNotifyReceived()
 */

#include "upnpdiscovery.h"

/*! Construct the hardware resource UpnpDiscovery with the given \a parent. */
UpnpDiscovery::UpnpDiscovery(QObject *parent) :
    HardwareResource("UPnP discovery", parent)
{

}
