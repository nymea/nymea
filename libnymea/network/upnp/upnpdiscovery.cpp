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
    \class UpnpDiscovery
    \brief Allows to discover UPnP devices in the network.

    \ingroup hardware
    \inmodule libnymea

    This resource allows plugins to discover UPnP devices in the network and receive notification messages. The resource
    will bind a UDP socket to the multicast 239.255.255.250 on port 1900.

    The communication was implementet using following documentation: \l{http://upnp.org/specs/arch/UPnP-arch-DeviceArchitecture-v1.1.pdf}

    \chapter Example

    In order to perform an UPnP discovery in your plugin, you can take a look at following example:

    \tt devicepluginexample.h

    \code
    class DevicePluginExample : public DevicePlugin
    {
    ...

    private:
        void discoverUpnpDevices();

    private slots:
        void onUpnpDiscoveryFinished();

    ...

    };

    \endcode

    \tt devicepluginexample.cpp

    \code
    #include "network/upnp/upnpdiscovery.h"
    #include "network/upnp/upnpdiscoveryreply.h"

    void DevicePluginExample::discoverUpnpDevices() {
        UpnpDiscoveryReply *reply = hardwareManager()->upnpDiscovery()->discoverDevices();
        connect(reply, &UpnpDiscoveryReply::finished, this, &DevicePluginExample::onUpnpDiscoveryFinished);
    }

    void DevicePluginExample::onUpnpDiscoveryFinished() {
        UpnpDiscoveryReply *reply = static_cast<UpnpDiscoveryReply *>(sender());

        if (reply->error() != UpnpDiscoveryReply::UpnpDiscoveryReplyErrorNoError) {
            qCWarning(dcExample()) << "UPnP discovery error" << reply->error();
        }

        // Note: you have to delete the reply using deleteLater()
        reply->deleteLater();

        foreach (const UpnpDeviceDescriptor &upnpDevice, reply->deviceDescriptors()) {
            qCDebug(dcExample()) << upnpDevice.friendlyName() << upnpDevice.hostAddress().toString();
        }

        ...

    }
    \endcode

    \sa UpnpDevice, UpnpDeviceDescriptor
*/

/*! \fn UpnpDiscovery::~UpnpDiscovery();
    Destroys this UpnpDiscovery.
*/

/*! \fn UpnpDiscoveryReply *UpnpDiscovery::discoverDevices(const QString &searchTarget = "ssdp:all", const QString &userAgent = QString(), const int &timeout = 5000);
    Start a UPnP discovery request for devices listening on the given \a searchTarget and \a userAgent. The discovery duration can be specified with \a timeout parameter.
*/

/*! \fn void UpnpDiscovery::sendToMulticast(const QByteArray &data);
    Sends \a data to the UpnP multicast group. This method can be used in order to send raw data to the group.
*/

/*! \fn UpnpDiscovery::upnpNotify(const QByteArray &notifyMessage)
    This signal will be emitted when a UPnP NOTIFY message \a notifyMessage will be recognized.
*/

#include "upnpdiscovery.h"

/*! Construct the hardware resource UpnpDiscovery with the given \a parent. */
UpnpDiscovery::UpnpDiscovery(QObject *parent) :
    HardwareResource("UPnP discovery", parent)
{

}
