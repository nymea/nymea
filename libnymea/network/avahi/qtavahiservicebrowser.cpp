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
    \class QtAvahiServiceBrowser
    \brief Allows to browse avahi services in the local network.

    \ingroup hardware
    \inmodule libnymea

    The QtAvahiServiceBrowser allows to discover the avahi network and get services.


    \chapter Example

    In order to search for available avahi services in the current network you use this hardware resource like this:

    \tt devicepluginexample.h

    \code
    #include "network/avahi/avahiserviceentry.h"

    class DevicePluginExample : public DevicePlugin
    {
    ...

    public:
        void init() override;

    private slots:
        void onServiceEntryAdded(const AvahiServiceEntry &serviceEntry);
        void onServiceEntryRemoved(const AvahiServiceEntry &serviceEntry);

    ...

    };
    \endcode

    \tt devicepluginexample.cpp

    \code

    void DevicePluginExample::init() {
        connect(hardwareManager()->avahiBrowser(), &QtAvahiServiceBrowser::serviceEntryAdded, this, &DevicePluginExample::onServiceEntryAdded);
        connect(hardwareManager()->avahiBrowser(), &QtAvahiServiceBrowser::serviceEntryRemoved, this, &DevicePluginExample::onServiceEntryRemoved);
    }

    void DevicePluginExample::onServiceEntryAdded(const AvahiServiceEntry &serviceEntry) {
        qCDebug(dcExample()) << "New service added to network:" << serviceEntry;

        ...
    }

    void DevicePluginExample::onServiceEntryRemoved(const AvahiServiceEntry &serviceEntry) {
        qCDebug(dcExample()) << "Service removed from network:" << serviceEntry;

        ...
    }

    \endcode

    \sa AvahiServiceEntry

*/

/*! \fn QtAvahiServiceBrowser::~QtAvahiServiceBrowser();
    Destroys this QtAvahiServiceBrowser;
*/

/*! \fn QList<AvahiServiceEntry> QtAvahiServiceBrowser::serviceEntries() const;
    Returns the list of available service entries in the network of this browser.
*/

// Signals
/*! \fn void QtAvahiServiceBrowser::serviceEntryAdded(const AvahiServiceEntry &entry);
    This signal will be emitted when a new \a entry was added to the current entry list.
*/

/*! \fn void QtAvahiServiceBrowser::serviceEntryRemoved(const AvahiServiceEntry &entry);
    This signal will be emitted when a new \a entry was removed from the current entry list.
*/

#include "qtavahiservicebrowser.h"

/*! Constructs a new \l{QtAvahiServiceBrowser} with the given \a parent. */
QtAvahiServiceBrowser::QtAvahiServiceBrowser(QObject *parent) :
    HardwareResource("Avahi service browser", parent)
{

}


