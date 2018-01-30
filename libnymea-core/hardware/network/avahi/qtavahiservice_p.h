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

#ifndef QTAVAHISERVICEPRIVATE_P
#define QTAVAHISERVICEPRIVATE_P

#include <QObject>
#include <QString>

#include "qtavahiservice.h"
#include "qtavahiclient.h"

#include <avahi-client/publish.h>
#include <avahi-common/error.h>
#include <avahi-common/alternative.h>

namespace nymeaserver {

class QtAvahiServicePrivate
{
public:
    QtAvahiServicePrivate();

    static void callback(AvahiEntryGroup *group, AvahiEntryGroupState state, void *userdata);

    QtAvahiClient *client;
    AvahiEntryGroup *group;
    AvahiStringList *serviceList = nullptr;
    QString name;
    quint16 port;
    QString type;
    QHash<QString, QString> txtRecords;
    int error;

    static AvahiStringList *createTxtList(const QHash<QString, QString> &txt);

};

}

#endif // QTAVAHISERVICEPRIVATE_P

