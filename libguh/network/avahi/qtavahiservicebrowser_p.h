/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2016 Simon Stürz <simon.stuerz@guh.guru>                 *
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

#ifndef QTAVAHISERVICEBROWSERPRIVATE_H
#define QTAVAHISERVICEBROWSERPRIVATE_H

#include <QStringList>
#include <QAbstractSocket>
#include <avahi-client/lookup.h>

#include "qtavahiclient.h"
#include "qtavahiservice.h"
#include "libguh.h"

class LIBGUH_EXPORT QtAvahiServiceBrowserPrivate
{
public:
    QtAvahiServiceBrowserPrivate(QtAvahiClient *client);

    // Callback members
    static void callbackServiceTypeBrowser(AvahiServiceTypeBrowser *browser,
                                           AvahiIfIndex interface,
                                           AvahiProtocol protocol,
                                           AvahiBrowserEvent event,
                                           const char *type,
                                           const char *domain,
                                           AvahiLookupResultFlags flags,
                                           void *userdata);

    static void callbackServiceBrowser(AvahiServiceBrowser *browser,
                                       AvahiIfIndex interface,
                                       AvahiProtocol protocol,
                                       AvahiBrowserEvent event,
                                       const char *name,
                                       const char *type,
                                       const char *domain,
                                       AvahiLookupResultFlags flags,
                                       void *userdata);

    static void callbackServiceResolver(AvahiServiceResolver *resolver, AvahiIfIndex interface,
                        AvahiProtocol protocol,
                        AvahiResolverEvent event,
                        const char *name,
                        const char *type,
                        const char *domain,
                        const char *host_name,
                        const AvahiAddress *address,
                        uint16_t port,
                        AvahiStringList *txt,
                        AvahiLookupResultFlags flags,
                        void *userdata);

    // Convert members
    static QStringList convertTxtList(AvahiStringList *txt);
    static QAbstractSocket::NetworkLayerProtocol convertProtocol(const AvahiProtocol &protocol);

    QtAvahiClient *client;
    AvahiServiceTypeBrowser *serviceTypeBrowser;
    QHash<QString, AvahiServiceBrowser *> serviceBrowserTable;
};

#endif // QTAVAHISERVICEBROWSERPRIVATE_H
