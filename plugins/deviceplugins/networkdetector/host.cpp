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

#include "host.h"

Host::Host()
{

}

Host::Host(const QString &hostName, const QString &address, const bool &reachable):
    m_hostName(hostName),
    m_address(address),
    m_reachable(reachable)
{

}

QString Host::hostName() const
{
    return m_hostName;
}

QString Host::adderss() const
{
    return m_address;
}

bool Host::reachable() const
{
    return m_reachable;
}

bool Host::isValid() const
{
    return !m_hostName.isEmpty() && !m_address.isEmpty();
}

QDebug operator<<(QDebug dbg, const Host &host)
{
    dbg.nospace() << "Host(" << host.hostName() << ", " << host.adderss() << ", " << (host.reachable() ? "up" : "down") << ")";
    return dbg.space();
}
