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

namespace nymeaserver {

NYMEA_LOGGING_CATEGORY(dcNetworkDeviceDiscovery, "NetworkDeviceDiscovery")

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

void NetworkDeviceDiscoveryReplyImpl::processPingResponse(const QHostAddress &address, const QString &hostName)
{
    foreach (const NetworkDeviceInfo &info, m_networkDeviceCache) {
        if (info.address() == address) {
            // Already found info, set host name and check if complete
            m_networkDeviceCache[info.macAddress()].setHostName(hostName);
            if (m_networkDeviceCache[info.macAddress()].isComplete() && m_networkDeviceCache[info.macAddress()].isValid()) {
                emit networkDeviceInfoAdded(m_networkDeviceCache[info.macAddress()]);
                m_networkDeviceInfos.append(m_networkDeviceCache.take(info.macAddress()));
            }

            emit hostAddressDiscovered(address);
            return;
        }
    }

    // Not added yet
    NetworkDeviceInfo info;
    info.setAddress(address);
    info.setHostName(hostName);
    m_pingCache.insert(address, info);
    emit hostAddressDiscovered(address);
}

void NetworkDeviceDiscoveryReplyImpl::processArpResponse(const QNetworkInterface &interface, const QHostAddress &address, const QString &macAddress)
{
    if (m_pingCache.contains(address)) {
        NetworkDeviceInfo info = m_pingCache.take(address);
        info.setAddress(address);
        info.setNetworkInterface(interface);
        info.setMacAddress(macAddress);
        m_networkDeviceCache[macAddress] = info;
    } else {
        if (m_networkDeviceCache.contains(macAddress)) {
            m_networkDeviceCache[macAddress].setNetworkInterface(interface);
            m_networkDeviceCache[macAddress].setAddress(address);
        } else {
            NetworkDeviceInfo info = m_pingCache.take(address);
            info.setNetworkInterface(interface);
            info.setMacAddress(macAddress);
            m_networkDeviceCache[macAddress] = info;
        }
    }

    if (m_networkDeviceCache[macAddress].isComplete() && m_networkDeviceCache[macAddress].isValid()) {
        m_networkDeviceInfos.append(m_networkDeviceCache.take(macAddress));
        emit networkDeviceInfoAdded(m_networkDeviceCache[macAddress]);
    }
}

void NetworkDeviceDiscoveryReplyImpl::processMacManufacturer(const QString &macAddress, const QString &manufacturer)
{
    if (m_networkDeviceCache.contains(macAddress)) {
        m_networkDeviceCache[macAddress].setMacAddressManufacturer(manufacturer);
    } else {
        NetworkDeviceInfo info(macAddress);
        info.setMacAddressManufacturer(manufacturer);
        m_networkDeviceCache[macAddress] = info;
    }

    if (m_networkDeviceCache[macAddress].isComplete() && m_networkDeviceCache[macAddress].isValid()) {
        m_networkDeviceInfos.append(m_networkDeviceCache.take(macAddress));
        emit networkDeviceInfoAdded(m_networkDeviceCache[macAddress]);
    }
}

void NetworkDeviceDiscoveryReplyImpl::processDiscoveryFinished()
{
    qint64 durationMilliSeconds = QDateTime::currentMSecsSinceEpoch() - m_startTimestamp;
    qCDebug(dcNetworkDeviceDiscovery()) << "Discovery finished. Found" << networkDeviceInfos().count() << "network devices in" << QTime::fromMSecsSinceStartOfDay(durationMilliSeconds).toString("mm:ss.zzz");

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
    foreach (const NetworkDeviceInfo &info, m_networkDeviceCache) {
        qCDebug(dcNetworkDeviceDiscovery()) << "--> " << info << "Complete:" << info.isComplete() << "Valid:" << info.isValid();
    }

    emit finished();
}

}
