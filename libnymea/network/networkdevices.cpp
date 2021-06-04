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

#include "networkdevices.h"

NetworkDevices::NetworkDevices() :
    QList<NetworkDevice>()
{

}

NetworkDevices::NetworkDevices(const QList<NetworkDevice> &other) :
    QList<NetworkDevice>(other)
{

}

NetworkDevices NetworkDevices::operator<<(const NetworkDevice &networkDevice)
{
    this->append(networkDevice);
    return *this;
}

int NetworkDevices::indexFromHostAddress(const QHostAddress &address)
{
    for (int i = 0; i < this->size(); i++) {
        if (at(i).address().toIPv4Address() == address.toIPv4Address()) {
            return i;
        }
    }

    return -1;
}

int NetworkDevices::indexFromMacAddress(const QString &macAddress)
{
    for (int i = 0; i < size(); i++) {
        if (at(i).macAddress().toLower() == macAddress.toLower()) {
            return i;
        }
    }

    return -1;
}

bool NetworkDevices::hasHostAddress(const QHostAddress &address)
{
    return indexFromHostAddress(address) >= 0;
}

bool NetworkDevices::hasMacAddress(const QString &macAddress)
{
    return indexFromMacAddress(macAddress) >= 0;
}

NetworkDevice NetworkDevices::get(const QHostAddress &address)
{
    foreach (const NetworkDevice &networkDevice, *this) {
        if (networkDevice.address() == address) {
            return networkDevice;
        }
    }

    return NetworkDevice();
}

NetworkDevice NetworkDevices::get(const QString &macAddress)
{
    foreach (const NetworkDevice &networkDevice, *this) {
        if (networkDevice.macAddress() == macAddress) {
            return networkDevice;
        }
    }

    return NetworkDevice();
}
