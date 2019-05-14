/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2016-2018 Simon Stürz <simon.stuerz@guh.io>              *
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
ZeroConfServiceBrowser::ZeroConfServiceBrowser(QObject *parent) :
    HardwareResource("ZeroConf service browser", parent)
{

}

bool ZeroConfServiceBrowser::available() const
{
    return false;
}

bool ZeroConfServiceBrowser::enabled() const
{
    return false;
}

void ZeroConfServiceBrowser::setEnabled(bool enabled)
{
    Q_UNUSED(enabled)
}

QList<ZeroConfServiceEntry> ZeroConfServiceBrowser::serviceEntries() const
{
    return {};
}


