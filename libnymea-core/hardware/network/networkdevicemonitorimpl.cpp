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

#include "networkdevicemonitorimpl.h"

namespace nymeaserver {

NetworkDeviceMonitorImpl::NetworkDeviceMonitorImpl(const MacAddress &macAddress, QObject *parent) :
    NetworkDeviceMonitor(parent),
    m_macAddress(macAddress)
{

}

NetworkDeviceMonitorImpl::~NetworkDeviceMonitorImpl()
{

}

MacAddress NetworkDeviceMonitorImpl::macAddress() const
{
    return m_macAddress;
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

    m_reachable = reachable;
    emit reachableChanged(m_reachable);
}

QDateTime NetworkDeviceMonitorImpl::lastSeen() const
{
    return m_lastSeen;
}

void NetworkDeviceMonitorImpl::setLastSeen(const QDateTime lastSeen)
{
    if (m_lastSeen == lastSeen)
        return;

    m_lastSeen = lastSeen;

    emit lastSeenChanged(m_lastSeen);
}

}
