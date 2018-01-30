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

#ifndef QTAVAHISERVICEBROWSER_H
#define QTAVAHISERVICEBROWSER_H

#include <QObject>
#include <avahi-client/lookup.h>

#include "libnymea.h"
#include "hardwareresource.h"
#include "avahiserviceentry.h"

class LIBNYMEA_EXPORT QtAvahiServiceBrowser : public HardwareResource
{
    Q_OBJECT

public:
    explicit QtAvahiServiceBrowser(QObject *parent = nullptr);
    virtual ~QtAvahiServiceBrowser() = default;

    virtual QList<AvahiServiceEntry> serviceEntries() const = 0;

signals:
    void serviceEntryAdded(const AvahiServiceEntry &entry);
    void serviceEntryRemoved(const AvahiServiceEntry &entry);

};

#endif // QTAVAHISERVICEBROWSER_H
