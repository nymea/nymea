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

#ifndef AVAHISERVICEENTRY_H
#define AVAHISERVICEENTRY_H

#include <QObject>
#include <QString>
#include <QHostAddress>
#include <QAbstractSocket>
#include <avahi-client/publish.h>

#include "libguh.h"

class LIBGUH_EXPORT AvahiServiceEntry
{
public:
    AvahiServiceEntry();
    AvahiServiceEntry(QString name, QString serviceType, QHostAddress hostAddress, QString domain, QString hostName, quint16 port, QAbstractSocket::NetworkLayerProtocol protocol, QStringList txt, AvahiLookupResultFlags flags);

    QString name() const;
    QString serviceType() const;
    QHostAddress hostAddress() const;
    QString domain() const;
    QString hostName() const;
    quint16 port() const;
    QAbstractSocket::NetworkLayerProtocol protocol() const;
    AvahiLookupResultFlags flags() const;
    QStringList txt() const;

    bool isValid() const;

    bool isChached() const;
    bool isWideArea() const;
    bool isMulticast() const;
    bool isLocal() const;
    bool isOurOwn() const;

    bool operator ==(const AvahiServiceEntry &other) const;
    bool operator !=(const AvahiServiceEntry &other) const;

private:
    QString m_name;
    QString m_serviceType;
    QHostAddress m_hostAddress;
    QString m_domain;
    QString m_hostName;
    quint16 m_port;
    QAbstractSocket::NetworkLayerProtocol m_protocol;
    QStringList m_txt;
    AvahiLookupResultFlags m_flags;

};

QDebug operator <<(QDebug dbg, const AvahiServiceEntry &entry);
Q_DECLARE_METATYPE(AvahiServiceEntry)

#endif // AVAHISERVICEENTRY_H
