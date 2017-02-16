/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2016 Simon Stürz <simon.stuerz@guh.io>                   *
 *                                                                         *
 *  This file is part of guh.                                              *
 *                                                                         *
 *  Guh is free software: you can redistribute it and/or modify            *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  Guh is distributed in the hope that it will be useful,                 *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with guh. If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef QTAVAHISERVICEBROWSER_H
#define QTAVAHISERVICEBROWSER_H

#include <QObject>
#include <avahi-client/lookup.h>

#include "libguh.h"
#include "qtavahiclient.h"
#include "avahiserviceentry.h"

class QtAvahiServiceBrowserPrivate;

class LIBGUH_EXPORT QtAvahiServiceBrowser : public QObject
{
    Q_OBJECT
public:
    explicit QtAvahiServiceBrowser(QObject *parent = 0);
    ~QtAvahiServiceBrowser();

    void enable();

    QList<AvahiServiceEntry> serviceEntries() const;

signals:
    void serviceEntryAdded(const AvahiServiceEntry &entry);
    void serviceEntryRemoved(const AvahiServiceEntry &entry);

private slots:
    void onClientStateChanged(const QtAvahiClient::QtAvahiClientState &state);

private:
    QtAvahiServiceBrowserPrivate *d_ptr;

    QList<AvahiServiceEntry> m_serviceEntries;
    QStringList m_serviceTypes;

    void createServiceBrowser(const char* serviceType);

    Q_DECLARE_PRIVATE(QtAvahiServiceBrowser)
};

#endif // QTAVAHISERVICEBROWSER_H
