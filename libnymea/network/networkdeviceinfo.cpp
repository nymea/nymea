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

#include "networkdeviceinfo.h"
#include "macaddress.h"

NetworkDeviceInfo::NetworkDeviceInfo()
{

}

NetworkDeviceInfo::NetworkDeviceInfo(const QString &macAddress)
{
    addMacAddress(MacAddress(macAddress));
}

NetworkDeviceInfo::NetworkDeviceInfo(const QHostAddress &address):
    m_address{address},
    m_addressSet{true}
{

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

MacAddressInfos NetworkDeviceInfo::macAddressInfos() const
{
    return m_macAddressInfos;
}

void NetworkDeviceInfo::addMacAddress(const MacAddress &macAddress)
{
    if (m_macAddressInfos.hasMacAddress(macAddress))
        return;

    m_macAddressInfos.append(MacAddressInfo(macAddress));
    // Note: we have to sort them in order to compare MacAddressInfos
    m_macAddressInfos.sortInfos();
}

void NetworkDeviceInfo::addMacAddress(const MacAddress &macAddress, const QString &vendorName)
{
    int index = m_macAddressInfos.indexFromMacAddress(macAddress);
    if (index >= 0) {
        m_macAddressInfos[index].setVendorName(vendorName);
    } else {
        m_macAddressInfos.append(MacAddressInfo(macAddress, vendorName));
        // Note: we have to sort them in order to compare MacAddressInfos
        m_macAddressInfos.sortInfos();
    }
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

NetworkDeviceInfo::MonitorMode NetworkDeviceInfo::monitorMode() const
{
    return m_monitorMode;
}

void NetworkDeviceInfo::setMonitorMode(MonitorMode monitorMode)
{
    m_monitorMode = monitorMode;
}

bool NetworkDeviceInfo::isValid() const
{
    return (!m_address.isNull() || m_macAddressInfos.isEmpty()) && m_networkInterface.isValid();
}

bool NetworkDeviceInfo::isComplete() const
{
    if (m_forceComplete)
        return true;

    return !m_macAddressInfos.isEmpty() && m_macAddressInfos.isComplete() && m_addressSet && m_hostNameSet && m_networkInterfaceSet;
}

void NetworkDeviceInfo::forceComplete()
{
    m_forceComplete = true;
}

QString NetworkDeviceInfo::incompleteProperties() const
{
    QStringList list;
    if (m_macAddressInfos.isEmpty())
        list.append("MAC address not set");

    if (!m_macAddressInfos.isEmpty() && !m_macAddressInfos.isComplete())
        list.append("MAC infos incomplete");

    if (!m_hostNameSet)
        list.append("hostname not set");

    if (!m_networkInterfaceSet)
        list.append("nework interface not set");

    return list.join(", ");
}

bool NetworkDeviceInfo::operator==(const NetworkDeviceInfo &other) const
{
    return m_address == other.address() &&
           m_macAddressInfos == other.macAddressInfos() &&
           m_hostName == other.hostName() &&
           m_networkInterface.name() == other.networkInterface().name() &&
           m_monitorMode == other.monitorMode() &&
           isComplete() == other.isComplete();
}

bool NetworkDeviceInfo::operator!=(const NetworkDeviceInfo &other) const
{
    return !operator==(other);
}

QDebug operator<<(QDebug dbg, const NetworkDeviceInfo &networkDeviceInfo)
{
    QDebugStateSaver saver(dbg);
    dbg.nospace().noquote() << "NetworkDeviceInfo(" << networkDeviceInfo.address().toString();

    dbg.nospace().noquote() << ", Monitor mode: ";
    switch (networkDeviceInfo.monitorMode()) {
    case NetworkDeviceInfo::MonitorModeMac:
        dbg.nospace().noquote() << "MAC";
        break;
    case NetworkDeviceInfo::MonitorModeHostname:
        dbg.nospace().noquote() << "hostname";
        break;
    case NetworkDeviceInfo::MonitorModeIp:
        dbg.nospace().noquote() << "IP";
        break;
    }

    foreach (const MacAddressInfo &macInfo, networkDeviceInfo.macAddressInfos())
        dbg.nospace().noquote() << ", " << macInfo;

    if (!networkDeviceInfo.hostName().isEmpty())
        dbg.nospace().noquote() << ", hostname: " << networkDeviceInfo.hostName();

    if (networkDeviceInfo.networkInterface().isValid())
        dbg.nospace().noquote() << ", " << networkDeviceInfo.networkInterface().name();

    dbg.nospace().noquote() << ")";
    return dbg;
}
