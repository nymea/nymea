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


