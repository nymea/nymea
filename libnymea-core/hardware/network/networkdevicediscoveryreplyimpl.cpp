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

#include "networkdevicediscoveryreplyimpl.h"
#include "loggingcategories.h"
#include "network/networkutils.h"

#include <QDateTime>

Q_DECLARE_LOGGING_CATEGORY(dcNetworkDeviceDiscovery)

namespace nymeaserver {

NetworkDeviceDiscoveryReplyImpl::NetworkDeviceDiscoveryReplyImpl(QObject *parent)
    : NetworkDeviceDiscoveryReply(parent)
{
    m_startTimestamp = QDateTime::currentMSecsSinceEpoch();
}

NetworkDeviceInfos NetworkDeviceDiscoveryReplyImpl::networkDeviceInfos() const
{
    return m_networkDeviceInfos;
}

bool NetworkDeviceDiscoveryReplyImpl::isFinished() const
{
    return m_isFinished;
}

void NetworkDeviceDiscoveryReplyImpl::setFinished(bool finished)
{
    m_isFinished = finished;
}

void NetworkDeviceDiscoveryReplyImpl::processPingResponse(const QHostAddress &address, const QString &hostName)
{
    if (m_networkDeviceCache.contains(address)) {
        m_networkDeviceCache[address].setHostName(hostName);
    } else {
        NetworkDeviceInfo info;
        info.setAddress(address);
        info.setHostName(hostName);
        m_networkDeviceCache.insert(address, info);

        // First time seeing this host address
        emit hostAddressDiscovered(address);
    }
}

void NetworkDeviceDiscoveryReplyImpl::processArpResponse(const QNetworkInterface &interface, const QHostAddress &address, const MacAddress &macAddress)
{
    if (m_networkDeviceCache.contains(address)) {
        m_networkDeviceCache[address].addMacAddress(macAddress);
        m_networkDeviceCache[address].setNetworkInterface(interface);
    } else {
        NetworkDeviceInfo info(macAddress.toString());
        info.setAddress(address);
        info.setNetworkInterface(interface);
        m_networkDeviceCache[address] = info;

        // First time seeing this host address
        emit hostAddressDiscovered(address);
    }
}

void NetworkDeviceDiscoveryReplyImpl::processMacManufacturer(const MacAddress &macAddress, const QString &manufacturer)
{
    if (macAddress.isNull())
        return;

    foreach (const NetworkDeviceInfo &info, m_networkDeviceCache) {
        if (info.macAddressInfos().hasMacAddress(macAddress)) {
            m_networkDeviceCache[info.address()].addMacAddress(macAddress, manufacturer);
        }
    }
}

void NetworkDeviceDiscoveryReplyImpl::processDiscoveryFinished()
{
    // Add the discovery cache to the final result
    foreach (const QHostAddress &address, m_networkDeviceCache.keys()) {
        if (m_networkDeviceCache.value(address).macAddressInfos().isEmpty() && !m_networkDeviceCache.value(address).networkInterface().isValid()) {
            // Set the network interface for the virtual hosts like VPN where we are not receiving any ARP information
            m_networkDeviceCache[address].setNetworkInterface(NetworkUtils::getInterfaceForHostaddress(address));
        }

        NetworkDeviceInfo info = m_networkDeviceCache.value(address);
        qCDebug(dcNetworkDeviceDiscovery()) << "--> " << info << "Valid:" << info.isValid() << "Complete:" << info.isComplete() << info.incompleteProperties();

        qCDebug(dcNetworkDeviceDiscovery()) << "Adding incomplete" << info << "to the final result:" << info.incompleteProperties();
        m_networkDeviceCache[address].forceComplete();
        m_networkDeviceInfos.append(m_networkDeviceCache.take(address));
    }

    // Evaluate overall monitor mode...
    evaluateMonitorMode();

    // Done, lets sort the result and inform
    m_networkDeviceInfos.sortNetworkDevices();

    qint64 durationMilliSeconds = QDateTime::currentMSecsSinceEpoch() - m_startTimestamp;
    qCInfo(dcNetworkDeviceDiscovery()) << "Discovery finished. Found" << networkDeviceInfos().count() << "network devices in"
                                       << QTime::fromMSecsSinceStartOfDay(durationMilliSeconds).toString("mm:ss.zzz");

    // Process what's left and add it to result list
    foreach (const NetworkDeviceInfo &info, m_networkDeviceInfos) {
        qCDebug(dcNetworkDeviceDiscovery()) << "--> " << info;
    }

    m_isFinished = true;
    emit finished();
}

QHash<QHostAddress, NetworkDeviceInfo> NetworkDeviceDiscoveryReplyImpl::currentCache() const
{
    return m_networkDeviceCache;
}

void NetworkDeviceDiscoveryReplyImpl::addCompleteNetworkDeviceInfo(const NetworkDeviceInfo &networkDeviceInfo)
{
    if (!m_networkDeviceInfos.hasHostAddress(networkDeviceInfo.address()))
        m_networkDeviceInfos.append(networkDeviceInfo);
}

void NetworkDeviceDiscoveryReplyImpl::evaluateMonitorMode()
{
    for (int i = 0; i < m_networkDeviceInfos.size(); i++) {
        const NetworkDeviceInfo info = m_networkDeviceInfos.at(i);
        qCDebug(dcNetworkDeviceDiscovery()) << "MonitorMode: Evaluating host" << info.address().toString();

        NetworkDeviceInfo::MonitorMode mode = NetworkDeviceInfo::MonitorModeMac;

        if (info.macAddressInfos().isEmpty()) {
            // No MAC address found, no ARP for this host, probably a VPN client
            if (info.hostName().isEmpty()) {
                qCDebug(dcNetworkDeviceDiscovery()) << "MonitorMode: --> No MAC address and no host name, using MonitorModeIp";
                mode = NetworkDeviceInfo::MonitorModeIp;
            } else {
                qCDebug(dcNetworkDeviceDiscovery()) << "MonitorMode: --> No MAC address, but we have a host name, suing MonitorModeHostName";
                mode = NetworkDeviceInfo::MonitorModeHostName;
            }

        } else if (info.macAddressInfos().size() == 1) {
            // Single mac address for this host..
            MacAddress macAddress = info.macAddressInfos().constFirst().macAddress();
            bool macAddressIsUnique = true;

            // Check if this mac is unique
            foreach (const NetworkDeviceInfo &networkDeviceInfo, m_networkDeviceInfos) {
                // Skip our self...
                if (networkDeviceInfo.address() == info.address())
                    continue;

                if (networkDeviceInfo.macAddressInfos().hasMacAddress(macAddress)) {
                    macAddressIsUnique = false;
                    break;
                }
            }

            if (!macAddressIsUnique) {
                if (info.hostName().isEmpty()) {
                    qCDebug(dcNetworkDeviceDiscovery()) << "MonitorMode: --> the MAC address of" << info.address().toString()
                                                        << "is not unique in this network and no host name available, usgin MonitorModeIp";
                    mode = NetworkDeviceInfo::MonitorModeIp;
                } else {
                    qCDebug(dcNetworkDeviceDiscovery()) << "MonitorMode: --> the MAC address of" << info.address().toString()
                                                        << "is not unique in this network but we have a host name, usgin MonitorModeHostName";
                    mode = NetworkDeviceInfo::MonitorModeHostName;
                }
            } else {
                qCDebug(dcNetworkDeviceDiscovery()) << "MonitorMode: --> the MAC address of" << info.address().toString() << "is unique in this network, usgin MonitorModeMac";
                mode = NetworkDeviceInfo::MonitorModeMac;
            }

        } else if (info.macAddressInfos().size() > 1) {
            // Multiple MAC addresses
            if (info.hostName().isEmpty()) {
                qCDebug(dcNetworkDeviceDiscovery()) << "MonitorMode: --> multiple MAC addresses and no host name, usgin MonitorModeIp";
                mode = NetworkDeviceInfo::MonitorModeIp;
            } else {
                qCDebug(dcNetworkDeviceDiscovery()) << "MonitorMode: --> multiple MAC addresses, but we have a host name, usgin MonitorModeHostName";
                mode = NetworkDeviceInfo::MonitorModeHostName;
            }
        }

        m_networkDeviceInfos[i].setMonitorMode(mode);
        qCDebug(dcNetworkDeviceDiscovery()) << "MonitorMode: --> Final" << m_networkDeviceInfos.at(i);
    }
}

} // namespace nymeaserver
