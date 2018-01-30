/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2016 Simon Stürz <simon.stuerz@guh.io>                   *
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
    \class QtAvahiServiceBrowserImplementation
    \brief Allows to browse avahi services in the local network.

    \ingroup hardware
    \inmodule libnymea
*/

/*! \fn void QtAvahiServiceBrowserImplementation::serviceEntryAdded(const AvahiServiceEntry &entry);
    This signal will be emitted when a new \a entry was added to the current entry list.
*/

/*! \fn void QtAvahiServiceBrowserImplementation::serviceEntryRemoved(const AvahiServiceEntry &entry);
    This signal will be emitted when a new \a entry was removed from the current entry list.
*/

#include "qtavahiservicebrowserimplementation.h"
#include "qtavahiservicebrowserimplementation_p.h"
#include "loggingcategories.h"

#include <avahi-common/error.h>

namespace nymeaserver {

/*! Constructs a new \l{QtAvahiServiceBrowserImplementation} with the given \a parent. */
QtAvahiServiceBrowserImplementation::QtAvahiServiceBrowserImplementation(QObject *parent) :
    QtAvahiServiceBrowser(parent),
    d_ptr(new QtAvahiServiceBrowserImplementationPrivate(new QtAvahiClient))
{
    // TODO: check available here
    m_available = true;

    connect(d_ptr->client, &QtAvahiClient::clientStateChanged, this, &QtAvahiServiceBrowserImplementation::onClientStateChanged);

    qCDebug(dcAvahi()) << "-->" << name() << "created successfully.";
}

/*! Destructs this \l{QtAvahiServiceBrowserImplementation}. */
QtAvahiServiceBrowserImplementation::~QtAvahiServiceBrowserImplementation()
{
    // Delete each service browser
    foreach (const QString &serviceType, d_ptr->serviceBrowserTable.keys()) {
        AvahiServiceBrowser *browser = d_ptr->serviceBrowserTable.take(serviceType);
        if (browser) {
            avahi_service_browser_free(browser);
        }
    }

    // Delete the service type browser
    if (d_ptr->serviceTypeBrowser)
        avahi_service_type_browser_free(d_ptr->serviceTypeBrowser);

    delete d_ptr;
}

/*! Returns the current \l{AvahiServiceEntry} list of this \l{QtAvahiServiceBrowserImplementation}. */
QList<AvahiServiceEntry> QtAvahiServiceBrowserImplementation::serviceEntries() const
{
    return m_serviceEntries;
}

bool QtAvahiServiceBrowserImplementation::available() const
{
    return m_available;
}

bool QtAvahiServiceBrowserImplementation::enabled() const
{
    return m_enabled;
}

void QtAvahiServiceBrowserImplementation::onClientStateChanged(const QtAvahiClient::QtAvahiClientState &state)
{
    if (state == QtAvahiClient::QtAvahiClientStateRunning) {
        qCDebug(dcAvahi()) << "Service browser client connected.";
        // Return if we already have a service type browser
        if (d_ptr->serviceTypeBrowser)
            return;

        d_ptr->serviceTypeBrowser = avahi_service_type_browser_new(d_ptr->client->m_client, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, 0, (AvahiLookupFlags) 0, QtAvahiServiceBrowserImplementationPrivate::callbackServiceTypeBrowser, this);
    } else if (state == QtAvahiClient::QtAvahiClientStateFailure) {
        qCWarning(dcAvahi()) << name() << "client failure:" << d_ptr->client->errorString();
    }
}

void QtAvahiServiceBrowserImplementation::setEnabled(bool enabled)
{
    if (m_enabled == enabled) {
        qCDebug(dcAvahi()) << "Avahi Service Browser already" << (enabled ? "enabled" : "disabled") << "... Not changing state.";
        return;
    }
    if (enabled) {
        d_ptr->client->start();
        qCDebug(dcAvahi()) << "Avahi Service Browser enabled";
    } else {
        d_ptr->client->stop();
        qCDebug(dcAvahi()) << "Avahi Service Browser disabled";
    }
}

void QtAvahiServiceBrowserImplementation::createServiceBrowser(const char *serviceType)
{
    // create a new service browser for the given serviceType
    AvahiServiceBrowser *browser = avahi_service_browser_new(d_ptr->client->m_client, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, serviceType, NULL, (AvahiLookupFlags) 0,  QtAvahiServiceBrowserImplementationPrivate::callbackServiceBrowser, this);
    d_ptr->serviceBrowserTable.insert(serviceType, browser);
}

}
