/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2024, nymea GmbH
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
