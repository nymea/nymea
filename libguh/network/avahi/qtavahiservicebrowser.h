/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2016 Simon Stürz <simon.stuerz@guh.io>                   *
 *                                                                         *
 *  This file is part of guh.                                              *
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

#ifndef QTAVAHISERVICEBROWSER_H
#define QTAVAHISERVICEBROWSER_H

#include <QObject>
#include <avahi-client/lookup.h>

#include "libguh.h"
#include "hardwareresource.h"
#include "qtavahiclient.h"
#include "avahiserviceentry.h"

class QtAvahiServiceBrowserPrivate;

class LIBGUH_EXPORT QtAvahiServiceBrowser : public HardwareResource
{
    Q_OBJECT

    friend class HardwareManager;

public:
    QList<AvahiServiceEntry> serviceEntries() const;

signals:
    void serviceEntryAdded(const AvahiServiceEntry &entry);
    void serviceEntryRemoved(const AvahiServiceEntry &entry);

private slots:
    void onClientStateChanged(const QtAvahiClient::QtAvahiClientState &state);

protected:
    virtual void setEnabled(bool enabled) override;

private:
    bool m_available = false;
    bool m_enabled = false;

    explicit QtAvahiServiceBrowser(QObject *parent = nullptr);
    ~QtAvahiServiceBrowser();

    QtAvahiServiceBrowserPrivate *d_ptr;

    QList<AvahiServiceEntry> m_serviceEntries;
    QStringList m_serviceTypes;

    void createServiceBrowser(const char* serviceType);

    Q_DECLARE_PRIVATE(QtAvahiServiceBrowser)
};

#endif // QTAVAHISERVICEBROWSER_H
