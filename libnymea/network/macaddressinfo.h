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

#ifndef MACADDRESSINFO_H
#define MACADDRESSINFO_H

#include "macaddress.h"

class MacAddressInfo
{
public:
    explicit MacAddressInfo();
    explicit MacAddressInfo(const MacAddress &macAddress);
    explicit MacAddressInfo(const MacAddress &macAddress, const QString &vendorName);

    MacAddress macAddress() const;

    QString vendorName() const;
    void setVendorName(const QString &vendorName);

    bool isValid() const;
    bool isComplete() const;

    bool operator==(const MacAddressInfo &other) const;
    bool operator!=(const MacAddressInfo &other) const;

private:
    MacAddress m_macAddress;
    QString m_vendorName;

    bool m_vendorNameSet = false;
};

QDebug operator<<(QDebug debug, const MacAddressInfo &addressInfo);

#endif // MACADDRESSINFO_H
