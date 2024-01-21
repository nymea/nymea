/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2020, nymea GmbH
* Contact: contact@nymea.io
*
* This file is part of nymea.
* This project including source code and documentation is protected by
* copyright law, and remains the property of nymea GmbH. All rights, including
* reproduction, publication, editing and translation, are reserved. The use of
* this project is subject to the terms of a license agreement to be concluded
* with nymea GmbH in accordance with the terms of use of nymea GmbH, available
* under https://nymea.io/license
*
* GNU Lesser General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU Lesser General Public License as published by the Free
* Software Foundation; version 3. This project is distributed in the hope that
* it will be useful, but WITHOUT ANY WARRANTY; without even the implied
* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*!
    \class ZeroConfServiceEntry
    \brief Holds information about an avahi service entry.

    \ingroup types
    \inmodule libnymea

    You can find an example \l{QtAvahiServiceBrowser}{here}.

*/

#include "zeroconfserviceentry.h"

/*! Constructs an empty invalid \l{ZeroConfServiceEntry}*/
ZeroConfServiceEntry::ZeroConfServiceEntry() :
    m_port(0),
    m_protocol(QAbstractSocket::UnknownNetworkLayerProtocol)
{

}

/*! Constructs a new \l{ZeroConfServiceEntry} with the given \a name, \a serviceType, \a hostAddress, \a domain, \a hostName, \a port, \a protocol, \a txt and flags.*/
ZeroConfServiceEntry::ZeroConfServiceEntry(QString name, QString serviceType, QHostAddress hostAddress, QString domain, QString hostName, quint16 port, QAbstractSocket::NetworkLayerProtocol protocol, QStringList txt, bool cached, bool isWideArea, bool isMulticast, bool isLocal, bool isOurOwn):
    m_name(name),
    m_serviceType(serviceType),
    m_hostAddress(hostAddress),
    m_domain(domain),
    m_hostName(hostName),
    m_port(port),
    m_protocol(protocol),
    m_txt(txt),
    m_isCached(cached),
    m_isWideArea(isWideArea),
    m_isMulticast(isMulticast),
    m_isLocal(isLocal),
    m_isOurOwn(isOurOwn)
{

}

/*! Returns the name of this \l{ZeroConfServiceEntry}.*/
QString ZeroConfServiceEntry::name() const
{
    return m_name;
}

/*! Returns the service type of this \l{ZeroConfServiceEntry}.*/
QString ZeroConfServiceEntry::serviceType() const
{
    return m_serviceType;
}

/*! Returns the host address of this \l{ZeroConfServiceEntry}.*/
QHostAddress ZeroConfServiceEntry::hostAddress() const
{
    return m_hostAddress;
}

/*! Returns the domain of this \l{ZeroConfServiceEntry}.*/
QString ZeroConfServiceEntry::domain() const
{
    return m_domain;
}

/*! Returns the host name of this \l{ZeroConfServiceEntry}.*/
QString ZeroConfServiceEntry::hostName() const
{
    return m_hostName;
}

/*! Returns the port of this \l{ZeroConfServiceEntry}.*/
quint16 ZeroConfServiceEntry::port() const
{
    return m_port;
}

/*! Returns the network protocol of this \l{ZeroConfServiceEntry}.*/
QAbstractSocket::NetworkLayerProtocol ZeroConfServiceEntry::protocol() const
{
    return m_protocol;
}

/*! Returns the txt string list of this \l{ZeroConfServiceEntry}.*/
QStringList ZeroConfServiceEntry::txt() const
{
    return m_txt;
}

/*! Returns the txt entry of the given \a key of this \l{ZeroConfServiceEntry}.*/
QString ZeroConfServiceEntry::txt(const QString &key) const
{
    foreach (const QString &txtEntry, m_txt) {
        QStringList parts = txtEntry.split('=');
        if (parts.length() == 2 && parts.first() == key) {
            return parts.last();
        }
    }
    return QString();
}

/*! Returns true if this \l{ZeroConfServiceEntry} is valid.*/
bool ZeroConfServiceEntry::isValid() const
{
    return !m_hostAddress.isNull() && !m_hostName.isEmpty() && m_port != 0 && m_protocol != QAbstractSocket::UnknownNetworkLayerProtocol;
}

/*! Returns true if this \l{ZeroConfServiceEntry} is cached.*/
bool ZeroConfServiceEntry::isChached() const
{
    return m_isCached;
}

/*! Returns true if this \l{ZeroConfServiceEntry} was found in the wide area.*/
bool ZeroConfServiceEntry::isWideArea() const
{
    return m_isWideArea;
}

/*! Returns true if this \l{ZeroConfServiceEntry} is a multicast service.*/
bool ZeroConfServiceEntry::isMulticast() const
{
    return m_isMulticast;
}

/*! Returns true if this \l{ZeroConfServiceEntry} was found local.*/
bool ZeroConfServiceEntry::isLocal() const
{
    return m_isLocal;
}

/*! Returns true if this \l{ZeroConfServiceEntry} is our own service.*/
bool ZeroConfServiceEntry::isOurOwn() const
{
    return m_isOurOwn;
}

/*! Returns true if this \l{ZeroConfServiceEntry} is equal to \a other; otherwise returns false.*/
bool ZeroConfServiceEntry::operator ==(const ZeroConfServiceEntry &other) const
{
    return other.name() == m_name &&
            other.serviceType() == m_serviceType &&
            other.hostAddress() == m_hostAddress &&
            other.domain() == m_domain &&
            other.hostName() == m_hostName &&
            other.port() == m_port &&
            other.protocol() == m_protocol &&
            other.isChached() == m_isCached &&
            other.isWideArea() == m_isWideArea &&
            other.isMulticast() == m_isMulticast &&
            other.isLocal() == m_isLocal &&
            other.isOurOwn() == m_isOurOwn &&
            other.txt() == m_txt;
}

/*! Returns true if this \l{ZeroConfServiceEntry} is not equal to \a other; otherwise returns false.*/
bool ZeroConfServiceEntry::operator !=(const ZeroConfServiceEntry &other) const
{
    return !operator==(other);
}

/*! Writes the given \a entry to the specified \a dbg.*/
QDebug operator <<(QDebug dbg, const ZeroConfServiceEntry &entry)
{
    dbg.nospace() << "ZeroConfServiceEntry(";
    dbg << entry.name() << ")" << Qt::endl;
    dbg << "    location: " << entry.hostAddress().toString() << ":" << entry.port() << Qt::endl;
    dbg << "    hostname: " << entry.hostName() << Qt::endl;
    dbg << "      domain: " << entry.domain() << Qt::endl;
    dbg << "service type: " << entry.serviceType() << Qt::endl;
    dbg << "    protocol: " << entry.protocol() << Qt::endl;
    dbg << "         txt: " << entry.txt() << Qt::endl;
    return dbg;
}
