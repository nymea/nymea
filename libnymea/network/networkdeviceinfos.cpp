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

#include "networkdeviceinfos.h"

#include <algorithm>

NetworkDeviceInfos::NetworkDeviceInfos() :
    QVector<NetworkDeviceInfo>()
{

}

NetworkDeviceInfos::NetworkDeviceInfos(const QVector<NetworkDeviceInfo> &other) :
    QVector<NetworkDeviceInfo>(other)
{

}


int NetworkDeviceInfos::indexFromHostAddress(const QHostAddress &address)
{
    for (int i = 0; i < this->size(); i++) {
        if (at(i).address().toIPv4Address() == address.toIPv4Address()) {
            return i;
        }
    }

    return -1;
}

int NetworkDeviceInfos::indexFromHostName(const QString &hostName)
{
    for (int i = 0; i < this->size(); i++) {
        if (at(i).hostName() == hostName) {
            return i;
        }
    }

    return -1;
}

QList<int> NetworkDeviceInfos::indexFromMacAddress(const QString &macAddress)
{
    return indexFromMacAddress(MacAddress(macAddress));
}

QList<int> NetworkDeviceInfos::indexFromMacAddress(const MacAddress &macAddress)
{
    QList<int> indices;
    for (int i = 0; i < size(); i++) {
        if (at(i).macAddressInfos().hasMacAddress(macAddress)) {
            indices << i;
        }
    }

    return indices;
}

bool NetworkDeviceInfos::hasHostAddress(const QHostAddress &address)
{
    return indexFromHostAddress(address) >= 0;
}

bool NetworkDeviceInfos::hasMacAddress(const QString &macAddress)
{
    return !indexFromMacAddress(macAddress).isEmpty();
}

bool NetworkDeviceInfos::hasMacAddress(const MacAddress &macAddress)
{
    return !indexFromMacAddress(macAddress).isEmpty();
}

NetworkDeviceInfo NetworkDeviceInfos::get(const QHostAddress &address) const
{
    foreach (const NetworkDeviceInfo &networkDeviceInfo, *this) {
        if (networkDeviceInfo.address() == address) {
            return networkDeviceInfo;
        }
    }

    return NetworkDeviceInfo();
}

void NetworkDeviceInfos::removeHostAddress(const QHostAddress &address)
{
    for (int i = 0; i < size(); i++) {
        if (at(i).address() == address) {
            remove(i);
        }
    }
}

void NetworkDeviceInfos::sortNetworkDevices()
{
    std::sort(this->begin(), this->end(), [](const NetworkDeviceInfo& a, const NetworkDeviceInfo& b) {
        return a.address().toIPv4Address() < b.address().toIPv4Address();
    });
}

NetworkDeviceInfos &NetworkDeviceInfos::operator <<(const NetworkDeviceInfo &networkDeviceInfo)
{
    this->append(networkDeviceInfo);
    return *this;
}
