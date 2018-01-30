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

#ifndef QTAVAHISERVICEBROWSERIMPLEMENTATION_H
#define QTAVAHISERVICEBROWSERIMPLEMENTATION_H

#include <QObject>
#include <avahi-client/lookup.h>

#include "qtavahiclient.h"

#include "network/avahi/avahiserviceentry.h"
#include "network/avahi/qtavahiservicebrowser.h"


namespace nymeaserver {

class QtAvahiServiceBrowserImplementationPrivate;

class QtAvahiServiceBrowserImplementation : public QtAvahiServiceBrowser
{
    Q_OBJECT

    friend class HardwareManagerImplementation;

public:
    explicit QtAvahiServiceBrowserImplementation(QObject *parent = nullptr);
    ~QtAvahiServiceBrowserImplementation();

    QList<AvahiServiceEntry> serviceEntries() const override;

    bool available() const override;
    bool enabled() const override;

private slots:
    void onClientStateChanged(const QtAvahiClient::QtAvahiClientState &state);

protected:
    void setEnabled(bool enabled) override;

private:
    bool m_available = false;
    bool m_enabled = false;

    QtAvahiServiceBrowserImplementationPrivate *d_ptr;

    QList<AvahiServiceEntry> m_serviceEntries;
    QStringList m_serviceTypes;

    void createServiceBrowser(const char* serviceType);

    Q_DECLARE_PRIVATE(QtAvahiServiceBrowserImplementation)
};

}

#endif // QTAVAHISERVICEBROWSERIMPLEMENTATION_H
