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

#include "qtavahiservice_p.h"
#include "loggingcategories.h"

#include <QString>
#include <QHash>
#include <QStringList>

namespace nymeaserver {

QtAvahiServicePrivate::QtAvahiServicePrivate() :
    client(0),
    group(0),
    error(0)
{

}

void QtAvahiServicePrivate::callback(AvahiEntryGroup *group, AvahiEntryGroupState state, void *userdata)
{
    Q_UNUSED(group);

    QtAvahiService *service = static_cast<QtAvahiService *>(userdata);
    if (!service)
        return;

    if (service->state() == (QtAvahiService::QtAvahiServiceState)state)
        return;

    switch (state) {
    case AVAHI_ENTRY_GROUP_UNCOMMITED:
        emit service->serviceStateChanged(QtAvahiService::QtAvahiServiceStateUncommitted);
        break;
    case AVAHI_ENTRY_GROUP_REGISTERING:
        emit service->serviceStateChanged(QtAvahiService::QtAvahiServiceStateRegistering);
        break;
    case AVAHI_ENTRY_GROUP_ESTABLISHED:
        emit service->serviceStateChanged(QtAvahiService::QtAvahiServiceStateEstablished);
        break;
    case AVAHI_ENTRY_GROUP_COLLISION:
        emit service->serviceStateChanged(QtAvahiService::QtAvahiServiceStateCollision);
        break;
    case AVAHI_ENTRY_GROUP_FAILURE:
        emit service->serviceStateChanged(QtAvahiService::QtAvahiServiceStateFailure);
        break;
    }
}

AvahiStringList *QtAvahiServicePrivate::createTxtList(const QHash<QString, QString> &txt)
{
    AvahiStringList *list = nullptr;
    if (txt.isEmpty())
        return list;

    const QStringList keys = txt.keys();
    list = avahi_string_list_new((keys.first() + '=' + txt[keys.first()]).toLatin1().constData(), nullptr);
    for (const QString &key : keys.mid(1)) {
        list = avahi_string_list_add_pair(list, key.toLatin1().constData(), txt[key].toLatin1().constData());
    }

    return list;
}

}
