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

/*!
    \class AvahiServiceEntry
    \brief Holds information about an avahi service entry.

    \ingroup types
    \inmodule libguh

*/

#include "avahiserviceentry.h"


/*! Constructs an empty invalid \l{AvahiServiceEntry}*/
AvahiServiceEntry::AvahiServiceEntry() :
    m_port(0),
    m_protocol(QAbstractSocket::UnknownNetworkLayerProtocol)
{

}

/*! Constructs a new \l{AvahiServiceEntry} with the given \a name, \a hostAddress, \a domain, \a hostName, \a port, \a protocol, \a txt and \a flags.*/
AvahiServiceEntry::AvahiServiceEntry(QString name, QHostAddress hostAddress, QString domain, QString hostName, quint16 port, QAbstractSocket::NetworkLayerProtocol protocol, QStringList txt, AvahiLookupResultFlags flags) :
    m_name(name),
    m_hostAddress(hostAddress),
    m_domain(domain),
    m_hostName(hostName),
    m_port(port),
    m_protocol(protocol),
    m_txt(txt),
    m_flags(flags)
{

}

/*! Returns the name of this \l{AvahiServiceEntry}.*/
QString AvahiServiceEntry::name() const
{
    return m_name;
}


/*! Returns the host address of this \l{AvahiServiceEntry}.*/
QHostAddress AvahiServiceEntry::hostAddress() const
{
    return m_hostAddress;
}

/*! Returns the domain of this \l{AvahiServiceEntry}.*/
QString AvahiServiceEntry::domain() const
{
    return m_domain;
}

/*! Returns the host name of this \l{AvahiServiceEntry}.*/
QString AvahiServiceEntry::hostName() const
{
    return m_hostName;
}

/*! Returns the port of this \l{AvahiServiceEntry}.*/
quint16 AvahiServiceEntry::port() const
{
    return m_port;
}

/*! Returns the protocol of this \l{AvahiServiceEntry}.*/
QAbstractSocket::NetworkLayerProtocol AvahiServiceEntry::protocol() const
{
    return m_protocol;
}

/*! Returns the txt string list of this \l{AvahiServiceEntry}.*/
QStringList AvahiServiceEntry::txt() const
{
    return m_txt;
}

/*! Returns true if this \l{AvahiServiceEntry} is valid.*/
bool AvahiServiceEntry::isValid() const
{
    return !m_hostAddress.isNull() && !m_hostName.isEmpty() && m_port != 0 && m_protocol != QAbstractSocket::UnknownNetworkLayerProtocol;
}

/*! Returns true if this \l{AvahiServiceEntry} was loaded from the cache.*/
bool AvahiServiceEntry::isChached() const
{
    return m_flags & AVAHI_LOOKUP_RESULT_CACHED;
}

/*! Returns true if this \l{AvahiServiceEntry} was found in the wide area.*/
bool AvahiServiceEntry::isWideArea() const
{
    return m_flags & AVAHI_LOOKUP_RESULT_WIDE_AREA;
}

/*! Returns true if this \l{AvahiServiceEntry} is a multicast service.*/
bool AvahiServiceEntry::isMulticast() const
{
    return m_flags & AVAHI_LOOKUP_RESULT_MULTICAST;
}

/*! Returns true if this \l{AvahiServiceEntry} was found local.*/
bool AvahiServiceEntry::isLocal() const
{
    return m_flags & AVAHI_LOOKUP_RESULT_LOCAL;
}

/*! Returns true if this \l{AvahiServiceEntry} is our own service.*/
bool AvahiServiceEntry::isOurOwn() const
{
    return m_flags & AVAHI_LOOKUP_RESULT_OUR_OWN;
}

