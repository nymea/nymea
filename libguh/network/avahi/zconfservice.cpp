/*
 *  This file is part of qtzeroconf. (c) 2012 Johannes Hilden
 *  https://github.com/johanneshilden/qtzeroconf
 *
 *  Modified: (C) 2016 Simon Stürz <stuerz.simon@gmail.com>
 *
 *  qtzeroconf is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU Lesser General Public License as
 *  published by the Free Software Foundation; either version 2.1 of the
 *  License, or (at your option) any later version.
 *
 *  qtzeroconf is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 *  or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General
 *  Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with qtzeroconf; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 */

#include <QDebug>
#include <QStringBuilder>
#include <avahi-client/publish.h>
#include <avahi-common/error.h>
#include <avahi-common/alternative.h>
#include "zconfserviceclient.h"
#include "zconfservice.h"

class ZConfServicePrivate
{
public:
    ZConfServicePrivate()
        : client(0), group(0), error(0)
    {
    }

    static void callback(AvahiEntryGroup *group, AvahiEntryGroupState state, void *userdata)
    {
        Q_UNUSED(group);
        ZConfService *serviceGroup = static_cast<ZConfService *>(userdata);
        if (serviceGroup) {
            switch (state)
            {
            case AVAHI_ENTRY_GROUP_ESTABLISHED:
                emit serviceGroup->entryGroupEstablished();
                qDebug() << ("Service '" % serviceGroup->d_ptr->name % "' successfully establised.");
                break;
            case AVAHI_ENTRY_GROUP_COLLISION:
                emit serviceGroup->entryGroupNameCollision();
                break;
            case AVAHI_ENTRY_GROUP_FAILURE:
                emit serviceGroup->entryGroupFailure();
                qDebug() << ("Entry group failure: " % serviceGroup->errorString());
                break;
            case AVAHI_ENTRY_GROUP_UNCOMMITED:
                qDebug() << "AVAHI_ENTRY_GROUP_UNCOMMITED";
                break;
            case AVAHI_ENTRY_GROUP_REGISTERING:
                qDebug() << "AVAHI_ENTRY_GROUP_REGISTERING";
            } // end switch
        }
    }

    ZConfServiceClient *client;
    AvahiEntryGroup    *group;
    QString             name;
    in_port_t           port;
    QString             type;
    int                 error;
};

/*!
    \class ZConfService

    \ingroup hardware
    \inmodule libguh

    \brief This class provides Avahi Zeroconf service registration. It can be
    used by server applications to announce a service on the local area network.

    Typical use involves creating an instance of ZConfService and calling
    registerService() with a service name and port number.
*/


/*! \fn void ZConfService::entryGroupEstablished();
    This signal will be emited when the service entry could be established successfully.
*/

/*! \fn void ZConfService::entryGroupNameCollision();
    This signal will be emited when the service entry name collided with an other service.
*/

/*! \fn void ZConfService::entryGroupFailure();
    This signal will be emited when the service entry could not be established successfully.
*/

/*! Constructs a \l{ZConfService} with the given \a parent.*/
ZConfService::ZConfService(QObject *parent)
    : QObject(parent),
      d_ptr(new ZConfServicePrivate)
{
    d_ptr->client = new ZConfServiceClient(this);
    d_ptr->client->run();
}

/*! Destroys the object and releases all resources associated with it. */
ZConfService::~ZConfService()
{
    if (d_ptr->group)
        avahi_entry_group_free(d_ptr->group);
    delete d_ptr;
}

/*!  Returns true if the service group was added and commited without error. */
bool ZConfService::isValid() const
{
    return (d_ptr->group && !d_ptr->error);
}

/*! Returns a human readable error string with details of the last error that occured. */
QString ZConfService::errorString() const
{
    if (!d_ptr->client->client)
        return "No client!";
    return avahi_strerror(avahi_client_errno(d_ptr->client->client));
}

/*!
    Registers a Zeroconf service with the given \a name and \a port on the LAN. If no service \a type is specified,
    "_http._tcp" is assumed.
 */
void ZConfService::registerService(QString name, in_port_t port, QString type)
{
    if (!d_ptr->client->client || AVAHI_CLIENT_S_RUNNING
            != avahi_client_get_state(d_ptr->client->client)) {
        qDebug() << "ZConfService error: Client is not running.";
        return;
    }

    d_ptr->name = name;
    d_ptr->port = port;
    d_ptr->type = type;

    if (!d_ptr->group) {
        d_ptr->group = avahi_entry_group_new(d_ptr->client->client,
                                             ZConfServicePrivate::callback,
                                             this);
    }

    if (avahi_entry_group_is_empty(d_ptr->group)) {
        d_ptr->error = avahi_entry_group_add_service(d_ptr->group,
                                                     AVAHI_IF_UNSPEC,
                                                     AVAHI_PROTO_UNSPEC,
                                                     (AvahiPublishFlags) 0,
                                                     d_ptr->name.toLatin1().data(),
                                                     d_ptr->type.toLatin1().data(),
                                                     0,
                                                     0,
                                                     d_ptr->port,
                                                     NULL);
        if (!d_ptr->error) {
            d_ptr->error = avahi_entry_group_commit(d_ptr->group);
        }
        if (d_ptr->error)
            qDebug() << ("Error creating service: " % errorString());
    }
}

/*!
    Deregisters the service associated with this object. You can reuse the same
    ZConfService object at any time to register another service on the network.
 */
void ZConfService::resetService()
{
    avahi_entry_group_reset(d_ptr->group);
}
