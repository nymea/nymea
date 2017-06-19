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

/*!
    \class QtAvahiService
    \brief Allows to publish an avahi service to the network.

    \inmodule libguh
*/

/*! \enum QtAvahiService::QtAvahiServiceState

    This enum type specifies the state of a \l{QtAvahiService}.

    \value QtAvahiServiceStateUncomitted
        The group has not yet been commited, the user must still call avahi_entry_group_commit().
    \value QtAvahiServiceStateRegistering
        The entries of the group are currently being registered.
    \value QtAvahiServiceStateEstablished
        The entries have successfully been established.
    \value QtAvahiServiceStateCollision
        A name collision for one of the entries in the group has been detected, the entries have been withdrawn.
    \value QtAvahiServiceStateFailure
        Some kind of failure happened, the entries have been withdrawn.

*/


/*! \fn void QtAvahiService::serviceStateChanged(const QtAvahiServiceState &state);
    This signal will be emitted when the \a state of this \l{QtAvahiService} has changed.
*/

#include "qtavahiservice.h"
#include "qtavahiservice_p.h"

/*! Constructs a new \l{QtAvahiService} with the given \a parent. */
QtAvahiService::QtAvahiService(QObject *parent) :
    QObject(parent),
    d_ptr(new QtAvahiServicePrivate)
{
    d_ptr->client = new QtAvahiClient(this);
    d_ptr->client->start();
}

/*! Destructs this \l{QtAvahiService}. */
QtAvahiService::~QtAvahiService()
{
    if (d_ptr->group)
        avahi_entry_group_free(d_ptr->group);

    delete d_ptr;
}

/*! Returns the port of this \l{QtAvahiService}. */
quint16 QtAvahiService::port() const
{
    return d_ptr->port;
}

/*! Returns the name of this \l{QtAvahiService}. */
QString QtAvahiService::name() const
{
    return d_ptr->name;
}

/*! Returns the service type of this \l{QtAvahiService}. */
QString QtAvahiService::serviceType() const
{
    return d_ptr->type;
}

/*! Register a new \l{QtAvahiService} with the given \a name and \a port. The service type can be specified with the \a serviceType string. The \a txt records inform about additional information. Returns true if the service could be registered. */
bool QtAvahiService::registerService(const QString &name, const quint16 &port, const QString &serviceType, const QHash<QString, QString> &txt)
{
    // check if the client is running
    if (!d_ptr->client->client || AVAHI_CLIENT_S_RUNNING != avahi_client_get_state(d_ptr->client->client))
        return false;

    d_ptr->name = name;
    d_ptr->port = port;
    d_ptr->type = serviceType;

    // if the group is not set yet, create it
    if (!d_ptr->group)
        d_ptr->group = avahi_entry_group_new(d_ptr->client->client, QtAvahiServicePrivate::callback, this);

    // if the group is empty
    if (avahi_entry_group_is_empty(d_ptr->group)) {
        // add the service
        d_ptr->error = avahi_entry_group_add_service_strlst(d_ptr->group,
                                                            AVAHI_IF_UNSPEC,
                                                            AVAHI_PROTO_UNSPEC,
                                                            (AvahiPublishFlags) 0,
                                                            d_ptr->name.toLatin1().data(),
                                                            d_ptr->type.toLatin1().data(),
                                                            0,
                                                            0,
                                                            (uint16_t)d_ptr->port,
                                                            QtAvahiServicePrivate::createTxtList(txt));

        // verify if the group has to be comitted
        if (!d_ptr->error)
            d_ptr->error = avahi_entry_group_commit(d_ptr->group);

        // if the group could not be commited, return false
        if (d_ptr->error)
            return false;

    }

    return true;
}

/*! Remove this service from the local network. This \l{QtAvahiService} can be reused to register a new avahi service. */
void QtAvahiService::resetService()
{
    if (!d_ptr->group)
        return;

    avahi_entry_group_reset(d_ptr->group);
}

/*!  Returns true if the service group was added and commited to the network without errors. */
bool QtAvahiService::isValid() const
{
    return (d_ptr->group && !d_ptr->error);
}

/*! Returns the error string of this \l{QtAvahiService}. */
QString QtAvahiService::errorString() const
{
    if (!d_ptr->client->client)
        return "Invalid client.";

    return avahi_strerror(avahi_client_errno(d_ptr->client->client));
}

