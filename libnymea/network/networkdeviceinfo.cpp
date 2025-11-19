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

#include "networkdeviceinfo.h"
#include "macaddress.h"

#include <QMetaType>

static const int networkDeviceInfoMetaTypeId = qRegisterMetaType<NetworkDeviceInfo>("NetworkDeviceInfo");

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
        list.append("host name not set");

    if (!m_networkInterfaceSet)
        list.append("nework interface not set");

    return list.join(", ");
}

QString NetworkDeviceInfo::thingParamValueMacAddress() const
{
    QString macString;
    switch (m_monitorMode) {
    case MonitorModeMac:
        macString = m_macAddressInfos.constFirst().macAddress().toString();
        break;
    default:
        // In any other case we don't want to store the mac address since we can not relai on it
        break;
    }
    return macString;
}

QString NetworkDeviceInfo::thingParamValueHostName() const
{
    QString hostNameString;
    switch (m_monitorMode) {
    case MonitorModeMac:
    case MonitorModeHostName:
        hostNameString = m_hostName;
        break;
    default:
        break;
    }
    return hostNameString;
}

QString NetworkDeviceInfo::thingParamValueAddress() const
{
    QString addressString;
    switch (m_monitorMode) {
    case MonitorModeIp:
        addressString = m_address.toString();
        break;
    default:
        // In any other case we don't want to store the IP address because we want to discover it
        break;
    }
    return addressString;
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
    case NetworkDeviceInfo::MonitorModeHostName:
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
