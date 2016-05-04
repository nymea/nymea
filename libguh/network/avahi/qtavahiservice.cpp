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

#include "qtavahiservice.h"
#include "qtavahiservice_p.h"

QtAvahiService::QtAvahiService(QObject *parent) :
    QObject(parent),
    d_ptr(new QtAvahiServicePrivate)
{
    d_ptr->client = new QtAvahiClient(this);
    d_ptr->client->start();
}

QtAvahiService::~QtAvahiService()
{
    if (d_ptr->group)
        avahi_entry_group_free(d_ptr->group);

    delete d_ptr;
}

quint16 QtAvahiService::port() const
{
    return d_ptr->port;
}

QString QtAvahiService::name() const
{
    return d_ptr->name;
}

QString QtAvahiService::serviceType() const
{
    return d_ptr->type;
}

bool QtAvahiService::registerService(QString name, quint16 port, QString type)
{
    // check if the client is running
    if (!d_ptr->client->client || AVAHI_CLIENT_S_RUNNING != avahi_client_get_state(d_ptr->client->client))
        return false;

    d_ptr->name = name;
    d_ptr->port = port;
    d_ptr->type = type;

    // if the group is not set yet, create it
    if (!d_ptr->group)
        d_ptr->group = avahi_entry_group_new(d_ptr->client->client, QtAvahiServicePrivate::callback, this);

    // if the group is empty
    if (avahi_entry_group_is_empty(d_ptr->group)) {
        // add the service
        d_ptr->error = avahi_entry_group_add_service(d_ptr->group,
                                                     AVAHI_IF_UNSPEC,
                                                     AVAHI_PROTO_UNSPEC,
                                                     (AvahiPublishFlags) 0,
                                                     d_ptr->name.toLatin1().data(),
                                                     d_ptr->type.toLatin1().data(),
                                                     0,
                                                     0,
                                                     (uint16_t)d_ptr->port,
                                                     NULL);

        // verify if the group has to be comitted
        if (!d_ptr->error)
            d_ptr->error = avahi_entry_group_commit(d_ptr->group);

        // if the group could not be commited, return false
        if (d_ptr->error)
            return false;

    }

    return true;
}

void QtAvahiService::resetService()
{
    avahi_entry_group_reset(d_ptr->group);
}

bool QtAvahiService::isValid() const
{
    return (d_ptr->group && !d_ptr->error);
}

QString QtAvahiService::errorString() const
{
    if (!d_ptr->client->client)
        return "Invalid client.";

    return avahi_strerror(avahi_client_errno(d_ptr->client->client));
}

