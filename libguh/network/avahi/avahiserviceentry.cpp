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

#include "avahiserviceentry.h"

AvahiServiceEntry::AvahiServiceEntry() :
    m_port(0),
    m_protocol(QAbstractSocket::UnknownNetworkLayerProtocol)
{

}

AvahiServiceEntry::AvahiServiceEntry(QString name, QString serviceType, QHostAddress hostAddress, QString domain, QString hostName, quint16 port, QAbstractSocket::NetworkLayerProtocol protocol, QStringList txt, AvahiLookupResultFlags flags) :
    m_name(name),
    m_serviceType(serviceType),
    m_hostAddress(hostAddress),
    m_domain(domain),
    m_hostName(hostName),
    m_port(port),
    m_protocol(protocol),
    m_txt(txt),
    m_flags(flags)
{

}

QString AvahiServiceEntry::name() const
{
    return m_name;
}

QString AvahiServiceEntry::serviceType() const
{
    return m_serviceType;
}

QHostAddress AvahiServiceEntry::hostAddress() const
{
    return m_hostAddress;
}

QString AvahiServiceEntry::domain() const
{
    return m_domain;
}

QString AvahiServiceEntry::hostName() const
{
    return m_hostName;
}

quint16 AvahiServiceEntry::port() const
{
    return m_port;
}

QAbstractSocket::NetworkLayerProtocol AvahiServiceEntry::protocol() const
{
    return m_protocol;
}

AvahiLookupResultFlags AvahiServiceEntry::flags() const
{
    return m_flags;
}

QStringList AvahiServiceEntry::txt() const
{
    return m_txt;
}

bool AvahiServiceEntry::isValid() const
{
    return !m_hostAddress.isNull() && !m_hostName.isEmpty() && m_port != 0 && m_protocol != QAbstractSocket::UnknownNetworkLayerProtocol;
}

bool AvahiServiceEntry::isChached() const
{
    return m_flags & AVAHI_LOOKUP_RESULT_CACHED;
}

bool AvahiServiceEntry::isWideArea() const
{
    return m_flags & AVAHI_LOOKUP_RESULT_WIDE_AREA;
}

bool AvahiServiceEntry::isMulticast() const
{
    return m_flags & AVAHI_LOOKUP_RESULT_MULTICAST;
}

bool AvahiServiceEntry::isLocal() const
{
    return m_flags & AVAHI_LOOKUP_RESULT_LOCAL;
}

bool AvahiServiceEntry::isOurOwn() const
{
    return m_flags & AVAHI_LOOKUP_RESULT_OUR_OWN;
}

bool AvahiServiceEntry::operator ==(const AvahiServiceEntry &other) const
{
    return other.name() == m_name &&
            other.serviceType() == m_serviceType &&
            other.hostAddress() == m_hostAddress &&
            other.domain() == m_domain &&
            other.hostName() == m_hostName &&
            other.port() == m_port &&
            other.protocol() == m_protocol &&
            other.flags() == m_flags &&
            other.txt() == m_txt;
}

bool AvahiServiceEntry::operator !=(const AvahiServiceEntry &other) const
{
    return !operator==(other);
}


QDebug operator <<(QDebug dbg, const AvahiServiceEntry &entry)
{
    dbg.nospace() << "AvahiServiceEntry(";
    dbg << entry.name() << ")" << endl;
    dbg << "    location: " << entry.hostAddress().toString() << ":" << entry.port() << endl;
    dbg << "    hostname: " << entry.hostName() << endl;
    dbg << "      domain: " << entry.domain() << endl;
    dbg << "service type: " << entry.serviceType() << endl;
    dbg << "    protocol: " << entry.protocol() << endl;
    dbg << "         txt: " << entry.txt() << endl;
    return dbg;
}
