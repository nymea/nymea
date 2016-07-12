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

#ifndef QTAVAHISERVICEPRIVATE_P
#define QTAVAHISERVICEPRIVATE_P

#include <QObject>
#include <QString>

#include "qtavahiservice.h"
#include "qtavahiclient.h"

#include "libguh.h"

#include <avahi-client/publish.h>
#include <avahi-common/error.h>
#include <avahi-common/alternative.h>

class LIBGUH_EXPORT QtAvahiServicePrivate
{
public:
    QtAvahiServicePrivate();

    static void callback(AvahiEntryGroup *group, AvahiEntryGroupState state, void *userdata);

    QtAvahiClient *client;
    AvahiEntryGroup *group;
    QString name;
    quint16 port;
    QString type;
    int error;

    static AvahiStringList *createTxtList(const QHash<QString, QString> &txt);

};

#endif // QTAVAHISERVICEPRIVATE_P

