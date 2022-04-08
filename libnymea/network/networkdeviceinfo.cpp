/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2022, nymea GmbH
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

#include "networkdeviceinfo.h"
#include "macaddress.h"

NetworkDeviceInfo::NetworkDeviceInfo()
{

}

NetworkDeviceInfo::NetworkDeviceInfo(const QString &macAddress):
    m_macAddress(macAddress)
{
    m_macAddressSet = true;
}

NetworkDeviceInfo::NetworkDeviceInfo(const QHostAddress &address):
    m_address(address)
{
    m_addressSet = true;
}

QString NetworkDeviceInfo::macAddress() const
{
    return m_macAddress;
}

void NetworkDeviceInfo::setMacAddress(const QString &macAddress)
{
    m_macAddress = macAddress;
    m_macAddressSet = true;
}

QString NetworkDeviceInfo::macAddressManufacturer() const
{
    return m_macAddressManufacturer;
}

void NetworkDeviceInfo::setMacAddressManufacturer(const QString &macAddressManufacturer)
{
    m_macAddressManufacturer = macAddressManufacturer;
    m_macAddressManufacturerSet = true;
}

QHostAddress NetworkDeviceInfo::address() const
{
    return m_address;
}

void NetworkDeviceInfo::setAddress(const QHostAddress &address)
{
    m_address = address;
    m_addressSet = true;
}

QString NetworkDeviceInfo::hostName() const
{
    return m_hostName;
}

void NetworkDeviceInfo::setHostName(const QString &hostName)
{
    m_hostName = hostName;
    m_hostNameSet = true;
}

QNetworkInterface NetworkDeviceInfo::networkInterface() const
{
    return m_networkInterface;
}

void NetworkDeviceInfo::setNetworkInterface(const QNetworkInterface &networkInterface)
{
    m_networkInterface = networkInterface;
    m_networkInterfaceSet = true;
}

bool NetworkDeviceInfo::isValid() const
{
    return (!m_address.isNull() || !MacAddress(m_macAddress).isNull()) && m_networkInterface.isValid();
}

bool NetworkDeviceInfo::isComplete() const
{
    return m_macAddressSet && m_macAddressManufacturerSet && m_addressSet && m_hostNameSet && m_networkInterfaceSet;
}

QString NetworkDeviceInfo::incompleteProperties() const
{
    QStringList list;
    if (!m_macAddressSet) list.append("MAC not set");
    if (!m_macAddressManufacturerSet) list.append("MAC vendor not set");
    if (!m_hostNameSet) list.append("hostname not set");
    if (!m_networkInterfaceSet) list.append("nework interface not set");
    return list.join(", ");
}

bool NetworkDeviceInfo::operator==(const NetworkDeviceInfo &other) const
{
    return MacAddress(m_macAddress) == MacAddress(other.macAddress()) &&
            m_address == other.address() &&
            m_hostName == other.hostName() &&
            m_macAddressManufacturer == other.macAddressManufacturer() &&
            m_networkInterface.name() == other.networkInterface().name() &&
            isComplete() == other.isComplete();
}

bool NetworkDeviceInfo::operator!=(const NetworkDeviceInfo &other) const
{
    return !operator==(other);
}

QDebug operator<<(QDebug dbg, const NetworkDeviceInfo &networkDeviceInfo)
{
    dbg.nospace() << "NetworkDeviceInfo(" << networkDeviceInfo.address().toString();

    if (!networkDeviceInfo.macAddress().isEmpty())
        dbg.nospace() << ", " << MacAddress(networkDeviceInfo.macAddress()).toString();

    if (!networkDeviceInfo.macAddressManufacturer().isEmpty())
        dbg.nospace() << " (" << networkDeviceInfo.macAddressManufacturer() << ") ";

    if (!networkDeviceInfo.hostName().isEmpty())
        dbg.nospace() << ", hostname: " << networkDeviceInfo.hostName();

    if (networkDeviceInfo.networkInterface().isValid())
        dbg.nospace() << ", " << networkDeviceInfo.networkInterface().name();

    dbg.nospace() << ")";
    return dbg.space();
}
