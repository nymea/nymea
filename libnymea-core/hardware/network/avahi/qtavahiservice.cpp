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

/*!
    \class QtAvahiService
    \brief Allows to publish an avahi service to the network.

    \inmodule libnymea
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
#include "loggingcategories.h"

namespace nymeaserver {

/*! Constructs a new \l{QtAvahiService} with the given \a parent. */
QtAvahiService::QtAvahiService(QObject *parent) :
    QObject(parent),
    d_ptr(new QtAvahiServicePrivate),
    m_state(QtAvahiServiceStateUncomitted)
{
    connect(this, &QtAvahiService::serviceStateChanged, this, &QtAvahiService::onStateChanged);

    d_ptr->client = new QtAvahiClient(this);
    d_ptr->client->start();
}

/*! Destructs this \l{QtAvahiService}. */
QtAvahiService::~QtAvahiService()
{
    if (d_ptr->group) {
        if (d_ptr->serviceList) {
            avahi_string_list_free(d_ptr->serviceList);
        }
        avahi_entry_group_free(d_ptr->group);
    }

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

QHash<QString, QString> QtAvahiService::txtRecords() const
{
    return d_ptr->txtRecords;
}

QtAvahiService::QtAvahiServiceState QtAvahiService::state() const
{
    return m_state;
}

/*! Register a new \l{QtAvahiService} with the given \a name and \a port. The service type can be specified with the \a serviceType string. The \a txtRecords records inform about additional information. Returns true if the service could be registered. */
bool QtAvahiService::registerService(const QString &name, const quint16 &port, const QString &serviceType, const QHash<QString, QString> &txtRecords)
{
    // Check if the client is running
    if (!d_ptr->client->m_client || AVAHI_CLIENT_S_RUNNING != avahi_client_get_state(d_ptr->client->m_client)) {
        qCWarning(dcAvahi()) << "Could not register service" << name << port << serviceType << ". The client is not available.";
        return false;
    }

    d_ptr->name = name;
    d_ptr->port = port;
    d_ptr->type = serviceType;
    d_ptr->txtRecords = txtRecords;

    // If the group is not set yet, create it
    if (!d_ptr->group)
        d_ptr->group = avahi_entry_group_new(d_ptr->client->m_client, QtAvahiServicePrivate::callback, this);

    // If the group is empty
    if (avahi_entry_group_is_empty(d_ptr->group)) {
        // Add the service
        d_ptr->serviceList = QtAvahiServicePrivate::createTxtList(txtRecords);
        d_ptr->error = avahi_entry_group_add_service_strlst(d_ptr->group,
                                                            AVAHI_IF_UNSPEC,
                                                            AVAHI_PROTO_UNSPEC,
                                                            (AvahiPublishFlags) 0,
                                                            d_ptr->name.toLatin1().data(),
                                                            d_ptr->type.toLatin1().data(),
                                                            0,
                                                            0,
                                                            (uint16_t)d_ptr->port,
                                                            d_ptr->serviceList);

        // Verify if the group has to be comitted
        if (d_ptr->error) {

            if (d_ptr->error == AVAHI_ERR_COLLISION) {
                if (!handlCollision()) {
                    qCWarning(dcAvahi()) << this << "error:" << avahi_strerror(d_ptr->error);
                    return false;
                }


            } else {
                qCWarning(dcAvahi()) << this << "error:" << avahi_strerror(d_ptr->error);
                return false;
            }
        }

        // Commit the service
        d_ptr->error = avahi_entry_group_commit(d_ptr->group);
        if (d_ptr->error) {
            qCWarning(dcAvahi()) << this << "error:" << avahi_strerror(d_ptr->error);
            return false;
        }
    } else {
        qCWarning(dcAvahi()) << "Service already registered. Please reset the service before reusing it.";
        return false;
    }

    return true;
}

/*! Remove this service from the local network. This \l{QtAvahiService} can be reused to register a new avahi service. */
void QtAvahiService::resetService()
{
    if (!d_ptr->group)
        return;

    if (d_ptr->serviceList) {
        avahi_string_list_free(d_ptr->serviceList);
        d_ptr->serviceList = nullptr;
    }
    avahi_entry_group_reset(d_ptr->group);
}

/*! Update the TXT record of this service. Returns true of the record could be updated. */
bool QtAvahiService::updateTxtRecord(const QHash<QString, QString> &txtRecords)
{
    if (!d_ptr->group)
        return false;

    // Add the service
    d_ptr->serviceList = QtAvahiServicePrivate::createTxtList(txtRecords);
    d_ptr->error = avahi_entry_group_update_service_txt_strlst(d_ptr->group,
                                                        AVAHI_IF_UNSPEC,
                                                        AVAHI_PROTO_UNSPEC,
                                                        (AvahiPublishFlags) 0,
                                                        d_ptr->name.toLatin1().data(),
                                                        d_ptr->type.toLatin1().data(),
                                                        0,
                                                        d_ptr->serviceList);

    // Verify if the group has to be comitted
    if (d_ptr->error) {
        qCWarning(dcAvahi()) << this << "error:" << avahi_strerror(d_ptr->error);
        return false;
    }

    qCDebug(dcAvahi()) << this << "updated TXT record.";

    return true;
}

/*!  Returns true if the service group was added and commited to the network without errors. */
bool QtAvahiService::isValid() const
{
    return (d_ptr->group && !d_ptr->error);
}

/*! Returns the error string of this \l{QtAvahiService}. */
QString QtAvahiService::errorString() const
{
    if (!d_ptr->client->m_client)
        return "Invalid client.";

    return avahi_strerror(avahi_client_errno(d_ptr->client->m_client));
}

bool QtAvahiService::handlCollision()
{
    char* alt = avahi_alternative_service_name(name().toStdString().data());
    QString alternativeServiceName = QLatin1String(alt);
    free(alt);
    qCDebug(dcAvahi()) << "Service name colision. Picking alternative service name" << alternativeServiceName;

    resetService();
    return registerService(alternativeServiceName, port(), serviceType(), txtRecords());
}

void QtAvahiService::onStateChanged(const QtAvahiServiceState &state)
{
    if (m_state == state)
        return;

    m_state = state;

    switch (m_state) {
    case QtAvahiServiceStateUncomitted:
        qCDebug(dcAvahi()) << this << "state changed: uncomitted";
        break;
    case QtAvahiServiceStateRegistering:
        qCDebug(dcAvahi()) << this << "state changed: registering...";
        break;
    case QtAvahiServiceStateEstablished:
        qCDebug(dcAvahi()) << this << "state changed: established";
        break;
    case QtAvahiServiceStateCollision:
        qCDebug(dcAvahi()) << this << "state changed: collision";
        handlCollision();
        break;
    case QtAvahiServiceStateFailure:
        qCWarning(dcAvahi()) << this << "failure: " << errorString();
        break;
    default:
        break;
    }

}

QDebug operator <<(QDebug dbg, QtAvahiService *service)
{
    dbg.nospace() << "AvahiService(";
    dbg << service->name() << ", " << service->serviceType() << ", " << service->port() << ") ";
    return dbg;
}

}
