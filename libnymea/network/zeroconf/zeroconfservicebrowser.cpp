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
    \class ZeroConfServiceBrowser
    \brief Allows to browse avahi services in the local network.

    \ingroup hardware
    \inmodule libnymea

    The ZeroConfServiceBrowser allows to discover the avahi network and get services.


    \chapter Example

    In order to search for available avahi services in the current network you use this hardware resource like this:

    \tt devicepluginexample.h

    \code
    #include "network/avahi/ZeroConfServiceEntry.h"

    class DevicePluginExample : public DevicePlugin
    {
    ...

    public:
        void init() override;

    private slots:
        void onServiceEntryAdded(const ZeroConfServiceEntry &serviceEntry);
        void onServiceEntryRemoved(const ZeroConfServiceEntry &serviceEntry);

    ...

    };
    \endcode

    \tt devicepluginexample.cpp

    \code

    void DevicePluginExample::init() {
        connect(hardwareManager()->avahiBrowser(), &ZeroConfServiceBrowser::serviceEntryAdded, this, &DevicePluginExample::onServiceEntryAdded);
        connect(hardwareManager()->avahiBrowser(), &ZeroConfServiceBrowser::serviceEntryRemoved, this, &DevicePluginExample::onServiceEntryRemoved);
    }

    void DevicePluginExample::onServiceEntryAdded(const ZeroConfServiceEntry &serviceEntry) {
        qCDebug(dcExample()) << "New service added to network:" << serviceEntry;

        ...
    }

    void DevicePluginExample::onServiceEntryRemoved(const ZeroConfServiceEntry &serviceEntry) {
        qCDebug(dcExample()) << "Service removed from network:" << serviceEntry;

        ...
    }

    \endcode

    \sa ZeroConfServiceEntry

*/

/*! \fn ZeroConfServiceBrowser::~ZeroConfServiceBrowser();
    Destroys this ZeroConfServiceBrowser;
*/

/*! \fn QList<ZeroConfServiceEntry> ZeroConfServiceBrowser::serviceEntries() const;
    Returns the list of available service entries in the network of this browser.
*/

// Signals
/*! \fn void ZeroConfServiceBrowser::serviceEntryAdded(const ZeroConfServiceEntry &entry);
    This signal will be emitted when a new \a entry was added to the current entry list.
*/

/*! \fn void ZeroConfServiceBrowser::serviceEntryRemoved(const ZeroConfServiceEntry &entry);
    This signal will be emitted when a new \a entry was removed from the current entry list.
*/

#include "zeroconfservicebrowser.h"

/*! Constructs a new \l{ZeroConfServiceBrowser} with the given \a parent. */
ZeroConfServiceBrowser::ZeroConfServiceBrowser(const QString &serviceType, QObject *parent):
    QObject(parent)
{
    Q_UNUSED(serviceType)
}

QList<ZeroConfServiceEntry> ZeroConfServiceBrowser::serviceEntries() const
{
    return {};
}


