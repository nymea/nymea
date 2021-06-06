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

#include "networkdevice.h"

NetworkDevice::NetworkDevice()
{

}

NetworkDevice::NetworkDevice(const QString &macAddress):
    m_macAddress(macAddress)
{

}

QString NetworkDevice::macAddress() const
{
    return m_macAddress;
}

void NetworkDevice::setMacAddress(const QString &macAddress)
{
    m_macAddress = macAddress;
}

QString NetworkDevice::macAddressManufacturer() const
{
    return m_macAddressManufacturer;
}

void NetworkDevice::setMacAddressManufacturer(const QString &macAddressManufacturer)
{
    m_macAddressManufacturer = macAddressManufacturer;
}

QHostAddress NetworkDevice::address() const
{
    return m_address;
}

void NetworkDevice::setAddress(const QHostAddress &address)
{
    m_address = address;
}

QString NetworkDevice::hostName() const
{
    return m_hostName;
}

void NetworkDevice::setHostName(const QString &hostName)
{
    m_hostName = hostName;
}

QNetworkInterface NetworkDevice::networkInterface() const
{
    return m_networkInterface;
}

void NetworkDevice::setNetworkInterface(const QNetworkInterface &networkInterface)
{
    m_networkInterface = networkInterface;
}

bool NetworkDevice::isValid() const
{
    return (!m_address.isNull() || !m_macAddress.isEmpty()) && m_networkInterface.isValid();
}

QDebug operator<<(QDebug dbg, const NetworkDevice &networkDevice)
{
    dbg.nospace() << "NetworkDevice(";
    dbg.nospace() << networkDevice.address().toString() << ", ";
    if (!networkDevice.hostName().isEmpty())
        dbg.nospace() << ", " << networkDevice.hostName();

    dbg.nospace() << networkDevice.macAddress();
    if (!networkDevice.macAddressManufacturer().isEmpty())
        dbg.nospace() << "(" << networkDevice.macAddressManufacturer() << ")";

    if (networkDevice.networkInterface().isValid())
        dbg.nospace() << ", " << networkDevice.networkInterface().name();

    dbg.nospace() << ")";
    return dbg.space();
}

