/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2021, nymea GmbH
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
