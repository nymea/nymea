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

#ifndef ZEROCONFSERVICEENTRY_H
#define ZEROCONFSERVICEENTRY_H

#include <QObject>
#include <QString>
#include <QHostAddress>
#include <QAbstractSocket>

#include "libnymea.h"

class LIBNYMEA_EXPORT ZeroConfServiceEntry
{
public:
    ZeroConfServiceEntry();
    ZeroConfServiceEntry(QString name, QString serviceType, QHostAddress hostAddress, QString domain, QString hostName, quint16 port, QAbstractSocket::NetworkLayerProtocol protocol, QStringList txt, bool cached, bool isWideArea, bool isMulticase, bool isLocal, bool isOurOwn);

    QString name() const;
    QString serviceType() const;
    QHostAddress hostAddress() const;
    QString domain() const;
    QString hostName() const;
    quint16 port() const;
    QAbstractSocket::NetworkLayerProtocol protocol() const;
    QStringList txt() const;

    bool isValid() const;

    bool isChached() const;
    bool isWideArea() const;
    bool isMulticast() const;
    bool isLocal() const;
    bool isOurOwn() const;

    bool operator ==(const ZeroConfServiceEntry &other) const;
    bool operator !=(const ZeroConfServiceEntry &other) const;

private:
    QString m_name;
    QString m_serviceType;
    QHostAddress m_hostAddress;
    QString m_domain;
    QString m_hostName;
    quint16 m_port;
    QAbstractSocket::NetworkLayerProtocol m_protocol;
    QStringList m_txt;

    bool m_isCached = false;
    bool m_isWideArea = false;
    bool m_isMulticast = false;
    bool m_isLocal = false;
    bool m_isOurOwn = false;
};

QDebug operator <<(QDebug dbg, const ZeroConfServiceEntry &entry);
Q_DECLARE_METATYPE(ZeroConfServiceEntry)

#endif // ZEROCONFSERVICEENTRY_H
