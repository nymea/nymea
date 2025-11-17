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

#include "macaddressinfos.h"

MacAddressInfos::MacAddressInfos()
{

}

MacAddressInfos::MacAddressInfos(const QVector<MacAddressInfo> &other)
    : QVector<MacAddressInfo>(other)
{

}

int MacAddressInfos::indexFromMacAddress(const QString &macAddress)
{
    return indexFromMacAddress(MacAddress(macAddress));
}

int MacAddressInfos::indexFromMacAddress(const MacAddress &macAddress)
{
    for (int i = 0; i < size(); i++) {
        if (at(i).macAddress() == macAddress) {
            return i;
        }
    }

    return -1;
}

bool MacAddressInfos::hasMacAddress(const QString &macAddress)
{
    return indexFromMacAddress(macAddress) >= 0;
}

bool MacAddressInfos::hasMacAddress(const MacAddress &macAddress)
{
    return indexFromMacAddress(macAddress) >= 0;
}

MacAddressInfo MacAddressInfos::get(const QString &macAddress) const
{
    return get(MacAddress(macAddress));
}

MacAddressInfo MacAddressInfos::get(const MacAddress &macAddress) const
{
    foreach (const MacAddressInfo &info, *this) {
        if (info.macAddress() == macAddress) {
            return info;
        }
    }

    return MacAddressInfo();
}

void MacAddressInfos::removeMacAddress(const QString &macAddress)
{
    removeMacAddress(MacAddress(macAddress));
}

void MacAddressInfos::removeMacAddress(const MacAddress &macAddress)
{
    for (int i = 0; i < size(); i++) {
        if (MacAddress(at(i).macAddress()) == macAddress) {
            remove(i);
        }
    }
}

void MacAddressInfos::sortInfos()
{
    std::sort(this->begin(), this->end(), [](const MacAddressInfo& a, const MacAddressInfo& b) {
        return a.macAddress().toByteArray() < b.macAddress().toByteArray();
    });
}

bool MacAddressInfos::isComplete() const
{
    bool complete = true;
    foreach (const MacAddressInfo &info, *this) {
        if (!info.isComplete()) {
            complete = false;
            break;
        }
    }

    return complete;
}
