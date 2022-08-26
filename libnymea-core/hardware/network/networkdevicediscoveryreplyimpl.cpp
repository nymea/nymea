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

NetworkDeviceInfos NetworkDeviceDiscoveryReplyImpl::virtualNetworkDeviceInfos() const
{
    return m_virtualNetworkDeviceInfos;
}

QString NetworkDeviceDiscoveryReplyImpl::macAddressFromHostAddress(const QHostAddress &address)
{
    foreach (const NetworkDeviceInfo &info, m_networkDeviceCache) {
        if (info.address() == address) {
            return info.macAddress();
        }
    }

    return QString();
}

bool NetworkDeviceDiscoveryReplyImpl::hasHostAddress(const QHostAddress &address)
{
    return ! macAddressFromHostAddress(address).isEmpty();
}

void NetworkDeviceDiscoveryReplyImpl::verifyComplete(const MacAddress &macAddress)
{
    if (!m_networkDeviceCache.contains(macAddress))
        return;

    if (m_networkDeviceCache[macAddress].isComplete() && m_networkDeviceCache[macAddress].isValid()) {
        if (m_networkDeviceInfos.hasMacAddress(macAddress)) {
            if (m_networkDeviceInfos.get(macAddress) != m_networkDeviceCache.value(macAddress)) {
                qCWarning(dcNetworkDeviceDiscovery()) << "Already complete network device info changed during discovery process! Please report a bug if you see this message.";
                qCWarning(dcNetworkDeviceDiscovery()) << m_networkDeviceInfos.get(macAddress);
                qCWarning(dcNetworkDeviceDiscovery()) << m_networkDeviceCache.value(macAddress);
            }
        } else {
            m_networkDeviceInfos.append(m_networkDeviceCache.value(macAddress));
            emit networkDeviceInfoAdded(m_networkDeviceCache[macAddress]);
        }
    }
}

void NetworkDeviceDiscoveryReplyImpl::processPingResponse(const QHostAddress &address, const QString &hostName)
{
    foreach (const NetworkDeviceInfo &info, m_networkDeviceCache) {
        if (info.address() == address) {
            // Already found info, set host name and check if complete
            MacAddress macAddress(info.macAddress());
            m_networkDeviceCache[macAddress].setHostName(hostName);
            verifyComplete(macAddress);
            return;
        }
    }

    // Unknown and we have no mac address yet, add it to the ping cache
    NetworkDeviceInfo info;
    info.setAddress(address);
    info.setHostName(hostName);
    m_pingCache.insert(address, info);
    // First time seeing this host address
    emit hostAddressDiscovered(address);
}

void NetworkDeviceDiscoveryReplyImpl::processArpResponse(const QNetworkInterface &interface, const QHostAddress &address, const MacAddress &macAddress)
{
    if (m_pingCache.contains(address)) {
        // We know this device from a ping response
        NetworkDeviceInfo info = m_pingCache.take(address);
        info.setAddress(address);
        info.setNetworkInterface(interface);
        info.setMacAddress(macAddress.toString());
        m_networkDeviceCache[macAddress] = info;
    } else {
        if (m_networkDeviceCache.contains(macAddress)) {
            m_networkDeviceCache[macAddress].setAddress(address);
            m_networkDeviceCache[macAddress].setNetworkInterface(interface);
        } else {
            NetworkDeviceInfo info(macAddress.toString());
            info.setAddress(address);
            info.setNetworkInterface(interface);
            m_networkDeviceCache[macAddress] = info;
            // First time seeing this host address
            emit hostAddressDiscovered(address);
        }
    }

    verifyComplete(macAddress);
}

void NetworkDeviceDiscoveryReplyImpl::processMacManufacturer(const MacAddress &macAddress, const QString &manufacturer)
{
    if (macAddress.isNull())
        return;

    if (m_networkDeviceCache.contains(macAddress)) {
        m_networkDeviceCache[macAddress].setMacAddressManufacturer(manufacturer);
    } else {
        NetworkDeviceInfo info(macAddress.toString());
        info.setMacAddressManufacturer(manufacturer);
        m_networkDeviceCache[macAddress] = info;
    }

    verifyComplete(macAddress);
}

void NetworkDeviceDiscoveryReplyImpl::processDiscoveryFinished()
{
    // Lets see if we have any incomplete infos but enougth data to be shown
    foreach (const MacAddress &macAddress, m_networkDeviceCache.keys()) {
        // If already in the result, ignore it
        if (m_networkDeviceInfos.hasMacAddress(macAddress))
            continue;

        NetworkDeviceInfo info = m_networkDeviceCache.value(macAddress);
        MacAddress infoMacAddress(info.macAddress());
        qCDebug(dcNetworkDeviceDiscovery()) << "--> " << info
                                            << "Valid:" << info.isValid()
                                            << "Complete:" << info.isComplete()
                                            << info.incompleteProperties();

        // We need at least a valid mac address and a valid ip address, the rest ist pure informative
        if (infoMacAddress == macAddress && !infoMacAddress.isNull() && !info.address().isNull()) {
            qCDebug(dcNetworkDeviceDiscovery()) << "Adding incomplete" << info << "to the final result:" << info.incompleteProperties();
            // Note: makeing it complete
            m_networkDeviceCache[macAddress].setAddress(info.address());
            m_networkDeviceCache[macAddress].setHostName(info.hostName());
            m_networkDeviceCache[macAddress].setMacAddress(info.macAddress());
            m_networkDeviceCache[macAddress].setMacAddressManufacturer(info.macAddressManufacturer());
            m_networkDeviceCache[macAddress].setNetworkInterface(info.networkInterface());
            verifyComplete(macAddress);
        }
    }

    // Done, lets sort the result and inform
    m_networkDeviceInfos.sortNetworkDevices();

    qint64 durationMilliSeconds = QDateTime::currentMSecsSinceEpoch() - m_startTimestamp;
    qCInfo(dcNetworkDeviceDiscovery()) << "Discovery finished. Found" << networkDeviceInfos().count() << "network devices in" << QTime::fromMSecsSinceStartOfDay(durationMilliSeconds).toString("mm:ss.zzz");

    // Process what's left and add it to result list
    foreach (const NetworkDeviceInfo &info, m_networkDeviceInfos) {
        qCDebug(dcNetworkDeviceDiscovery()) << "--> " << info;
    }

    // Create valid infos from the ping cache and offer them in the virtual infos
    foreach (const NetworkDeviceInfo &info, m_pingCache) {
        NetworkDeviceInfo finalInfo = info;
        finalInfo.setAddress(finalInfo.address());
        finalInfo.setHostName(finalInfo.hostName());
        finalInfo.setMacAddress(finalInfo.macAddress());
        finalInfo.setNetworkInterface(finalInfo.networkInterface());
        finalInfo.setMacAddressManufacturer(finalInfo.macAddressManufacturer());
        m_virtualNetworkDeviceInfos.append(info);
    }

    m_virtualNetworkDeviceInfos.sortNetworkDevices();

    qCDebug(dcNetworkDeviceDiscovery()) << "Virtual hosts (" << m_virtualNetworkDeviceInfos.count() << ")";
    foreach (const NetworkDeviceInfo &info, m_virtualNetworkDeviceInfos) {
        qCDebug(dcNetworkDeviceDiscovery()) << "--> " << info;
    }

    qCDebug(dcNetworkDeviceDiscovery()) << "Rest:";
    foreach (const MacAddress &macAddress, m_networkDeviceCache.keys()) {
        if (m_networkDeviceInfos.hasMacAddress(macAddress))
            continue;

        NetworkDeviceInfo info = m_networkDeviceCache.value(macAddress);
        qCDebug(dcNetworkDeviceDiscovery()) << "--> " << info << "Valid:" << info.isValid() << "Complete:" << info.isComplete() << info.incompleteProperties();
    }

    emit finished();
}

}
