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

#include "networkdevicediscoveryreplyimpl.h"
#include "network/networkutils.h"
#include "loggingcategories.h"

#include <QDateTime>

Q_DECLARE_LOGGING_CATEGORY(dcNetworkDeviceDiscovery)

namespace nymeaserver {

NetworkDeviceDiscoveryReplyImpl::NetworkDeviceDiscoveryReplyImpl(QObject *parent) :
    NetworkDeviceDiscoveryReply(parent)
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
        // Update existing hostname...
        m_networkDeviceCache[address].setHostName(hostName);
        evaluateMonitorMode(address);
    } else {
        // Adding new host...
        NetworkDeviceInfo info;
        info.setAddress(address);
        info.setHostName(hostName);
        m_networkDeviceCache.insert(address, info);
        evaluateMonitorMode(address);

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

    evaluateMonitorMode(address);
}

void NetworkDeviceDiscoveryReplyImpl::processMacManufacturer(const MacAddress &macAddress, const QString &manufacturer)
{
    if (macAddress.isNull())
        return;

    foreach (const NetworkDeviceInfo &info, m_networkDeviceCache) {
        if (info.macAddressInfos().hasMacAddress(macAddress)) {
            m_networkDeviceCache[info.address()].addMacAddress(macAddress, manufacturer);
            evaluateMonitorMode(info.address());
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
        qCDebug(dcNetworkDeviceDiscovery()) << "--> " << info
                                            << "Valid:" << info.isValid()
                                            << "Complete:" << info.isComplete()
                                            << info.incompleteProperties();

        qCDebug(dcNetworkDeviceDiscovery()) << "Adding incomplete" << info << "to the final result:" << info.incompleteProperties();
        m_networkDeviceCache[address].forceComplete();

        evaluateMonitorMode(address);
        m_networkDeviceInfos.append(m_networkDeviceCache.value(address));
    }

    // Done, lets sort the result and inform
    m_networkDeviceInfos.sortNetworkDevices();

    qint64 durationMilliSeconds = QDateTime::currentMSecsSinceEpoch() - m_startTimestamp;
    qCInfo(dcNetworkDeviceDiscovery()) << "Discovery finished. Found" << networkDeviceInfos().count() << "network devices in" << QTime::fromMSecsSinceStartOfDay(durationMilliSeconds).toString("mm:ss.zzz");

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

void NetworkDeviceDiscoveryReplyImpl::evaluateMonitorMode(const QHostAddress &address)
{
    qCDebug(dcNetworkDeviceDiscovery()) << "MonitorMode: Evaluating monitor mode for host" << address.toString();

    if (m_networkDeviceCache.value(address).macAddressInfos().isEmpty()) {
        // Not discovered yet, or this is a virtual host like VPN
        if (m_networkDeviceCache.value(address).hostName().isEmpty()) {
            m_networkDeviceCache[address].setMonitorMode(NetworkDeviceInfo::MonitorModeIp);
            qCDebug(dcNetworkDeviceDiscovery()) << "MonitorMode: No MAC address and no hostname, using IP only";
        } else {
            qCDebug(dcNetworkDeviceDiscovery()) << "MonitorMode: No MAC address, but we have a hostname.";
            m_networkDeviceCache[address].setMonitorMode(NetworkDeviceInfo::MonitorModeHostname);
        }

    } else {
        // We have at least one mac address, check if there are other network devices with this MAC or if we have multiple MAC addresses
        if (m_networkDeviceCache.value(address).macAddressInfos().count() == 1) {

            bool uniqueMac = true;
            // Check if this mac is unique
            foreach (const NetworkDeviceInfo &info, m_networkDeviceCache) {
                if (info.address() == address)
                    continue;

                if (info.macAddressInfos().hasMacAddress(m_networkDeviceCache.value(address).macAddressInfos().first().macAddress())) {
                    uniqueMac = false;
                    break;
                }
            }

            if (!uniqueMac) {
                if (m_networkDeviceCache.value(address).hostName().isEmpty()) {
                    m_networkDeviceCache[address].setMonitorMode(NetworkDeviceInfo::MonitorModeIp);
                    qCDebug(dcNetworkDeviceDiscovery()) << "MonitorMode: The MAC address of" << address.toString() << "is not unique in this network and no hostname available, using IP only";
                } else {
                    qCDebug(dcNetworkDeviceDiscovery()) << "MonitorMode: The MAC address of" << address.toString() << "is not unique in this network but we have a hostname";
                    m_networkDeviceCache[address].setMonitorMode(NetworkDeviceInfo::MonitorModeHostname);
                }
            }
        } else {
            // Multiple MAC addresses
            if (m_networkDeviceCache.value(address).hostName().isEmpty()) {
                m_networkDeviceCache[address].setMonitorMode(NetworkDeviceInfo::MonitorModeIp);
                qCDebug(dcNetworkDeviceDiscovery()) << "MonitorMode: Multiple MAC addresses and no hostname, using IP only";
            } else {
                qCDebug(dcNetworkDeviceDiscovery()) << "MonitorMode: Multiple MAC addresses, but we have a hostname.";
                m_networkDeviceCache[address].setMonitorMode(NetworkDeviceInfo::MonitorModeHostname);
            }
        }
    }
}

}
