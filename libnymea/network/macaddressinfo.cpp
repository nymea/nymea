// SPDX-License-Identifier: LGPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2025, chargebyte austria GmbH
*
* This file is part of nymea.
*
* nymea is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* as published by the Free Software Foundation, either version 3
* of the License, or (at your option) any later version.
*
* nymea is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with nymea. If not, see <https://www.gnu.org/licenses/>.
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "macaddressinfo.h"

#include <QDebug>

MacAddressInfo::MacAddressInfo()
{

}

MacAddressInfo::MacAddressInfo(const MacAddress &macAddress)
    : m_macAddress{macAddress}
{

}

MacAddressInfo::MacAddressInfo(const MacAddress &macAddress, const QString &vendorName)
    : m_macAddress{macAddress},
    m_vendorName{vendorName},
    m_vendorNameSet{true}
{

}

MacAddress MacAddressInfo::macAddress() const
{
    return m_macAddress;
}

QString MacAddressInfo::vendorName() const
{
    return m_vendorName;
}

void MacAddressInfo::setVendorName(const QString &vendorName)
{
    m_vendorName = vendorName;
    m_vendorNameSet = true;
}

bool MacAddressInfo::isValid() const
{
    return !m_macAddress.isNull();
}

bool MacAddressInfo::isComplete() const
{
    return isValid() && m_vendorNameSet;
}

bool MacAddressInfo::operator==(const MacAddressInfo &other) const
{
    return m_macAddress == other.macAddress() &&
           m_vendorName == other.vendorName() &&
           isComplete() == other.isComplete();
}

bool MacAddressInfo::operator!=(const MacAddressInfo &other) const
{
    return !operator==(other);
}

QDebug operator<<(QDebug debug, const MacAddressInfo &addressInfo)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << addressInfo.macAddress().toString() << " (";
    if (addressInfo.vendorName().isEmpty()) {
        debug.nospace() << "unknown";
    } else {
        debug.nospace() << addressInfo.vendorName();
    }
    debug.nospace() << ")";
    return debug;
}
