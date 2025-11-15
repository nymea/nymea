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

#include "networkdevicemonitorimpl.h"
#include "loggingcategories.h"

Q_DECLARE_LOGGING_CATEGORY(dcNetworkDeviceDiscovery)

namespace nymeaserver {

NetworkDeviceMonitorImpl::NetworkDeviceMonitorImpl(const MacAddress &macAddress, const QString &hostName, const QHostAddress &address, QObject *parent) :
    NetworkDeviceMonitor{parent},
    m_macAddress{macAddress},
    m_hostName{hostName},
    m_address{address}
{

}

NetworkDeviceMonitorImpl::~NetworkDeviceMonitorImpl()
{
    if (m_currentPingReply) {
        m_currentPingReply->abort();
    }
}

MacAddress NetworkDeviceMonitorImpl::macAddress() const
{
    return m_macAddress;
}

QString NetworkDeviceMonitorImpl::hostName() const
{
    return m_hostName;
}

QHostAddress NetworkDeviceMonitorImpl::address() const
{
    return m_address;
}

NetworkDeviceInfo::MonitorMode NetworkDeviceMonitorImpl::monitorMode() const
{
    return m_monitorMode;
}

void NetworkDeviceMonitorImpl::setMonitorMode(NetworkDeviceInfo::MonitorMode monitorMode)
{
    if (m_monitorMode == monitorMode)
        return;

    m_monitorMode = monitorMode;

    if (m_networkDeviceInfo.monitorMode() != monitorMode) {
        m_networkDeviceInfo.setMonitorMode(monitorMode);
        emit networkDeviceInfoChanged(m_networkDeviceInfo);
    }
}

NetworkDeviceInfo NetworkDeviceMonitorImpl::networkDeviceInfo() const
{
    return m_networkDeviceInfo;
}

void NetworkDeviceMonitorImpl::setNetworkDeviceInfo(const NetworkDeviceInfo &networkDeviceInfo)
{
    if (m_networkDeviceInfo == networkDeviceInfo)
        return;

    m_networkDeviceInfo = networkDeviceInfo;
    emit networkDeviceInfoChanged(m_networkDeviceInfo);
}

bool NetworkDeviceMonitorImpl::reachable() const
{
    return m_reachable;
}

void NetworkDeviceMonitorImpl::setReachable(bool reachable)
{
    if (m_reachable == reachable)
        return;

    qCDebug(dcNetworkDeviceDiscovery()) << this << (reachable ? "is now reachable" : "is not reachable any more.");
    m_reachable = reachable;
    emit reachableChanged(m_reachable);
}

QDateTime NetworkDeviceMonitorImpl::lastSeen() const
{
    return m_lastSeen;
}

void NetworkDeviceMonitorImpl::setLastSeen(const QDateTime &lastSeen)
{
    if (m_lastSeen == lastSeen)
        return;

    m_lastSeen = lastSeen;
    emit lastSeenChanged(m_lastSeen);
}

uint NetworkDeviceMonitorImpl::pingRetries() const
{
    return m_pingRetries;
}

void NetworkDeviceMonitorImpl::setPingRetries(uint pingRetries)
{
    if (m_pingRetries == pingRetries)
        return;

    m_pingRetries = pingRetries;
    emit pingRetriesChanged(m_pingRetries);
}

PingReply *NetworkDeviceMonitorImpl::currentPingReply() const
{
    return m_currentPingReply;
}

void NetworkDeviceMonitorImpl::setCurrentPingReply(PingReply *reply)
{
    m_currentPingReply = reply;
}

QDateTime NetworkDeviceMonitorImpl::lastConnectionAttempt() const
{
    return m_lastConnectionAttempt;
}

void NetworkDeviceMonitorImpl::setLastConnectionAttempt(const QDateTime &lastConnectionAttempt)
{
    m_lastConnectionAttempt = lastConnectionAttempt;
}

bool NetworkDeviceMonitorImpl::isMyNetworkDeviceInfo(const NetworkDeviceInfo &networkDeviceInfo) const
{
    bool myNetworkDevice = false;
    switch (m_monitorMode) {
    case NetworkDeviceInfo::MonitorModeMac:
        if (!m_macAddress.isNull() && networkDeviceInfo.macAddressInfos().hasMacAddress(m_macAddress))
            myNetworkDevice = true;

        break;
    case NetworkDeviceInfo::MonitorModeHostName:
        if (!m_hostName.isEmpty() && networkDeviceInfo.hostName().compare(m_hostName, Qt::CaseInsensitive) == 0)
            myNetworkDevice = true;

        break;
    case NetworkDeviceInfo::MonitorModeIp:
        if (!m_address.isNull() && networkDeviceInfo.address() == m_address)
            myNetworkDevice = true;

        break;
    }
    return myNetworkDevice;
}

bool NetworkDeviceMonitorImpl::operator==(const NetworkDeviceMonitorImpl &other) const
{
    return m_macAddress == other.macAddress()
            && m_hostName == other.hostName()
            && m_address == other.address();
}

bool NetworkDeviceMonitorImpl::operator!=(const NetworkDeviceMonitorImpl &other) const
{
    return !operator==(other);
}

}
