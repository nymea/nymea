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

#ifndef NETWORKDEVICEMONITORIMPL_H
#define NETWORKDEVICEMONITORIMPL_H

#include <QObject>
#include <QDateTime>

#include "network/pingreply.h"
#include "network/networkdevicemonitor.h"

namespace nymeaserver {

class NetworkDeviceMonitorImpl : public NetworkDeviceMonitor
{
    Q_OBJECT

public:
    explicit NetworkDeviceMonitorImpl(const MacAddress &macAddress, const QString &hostName, const QHostAddress &address, QObject *parent = nullptr);
    ~NetworkDeviceMonitorImpl() override;

    // Properties from the thing
    MacAddress macAddress() const override;
    QString hostName() const override;
    QHostAddress address() const override;

    NetworkDeviceInfo::MonitorMode monitorMode() const override;
    void setMonitorMode(NetworkDeviceInfo::MonitorMode monitorMode);

    NetworkDeviceInfo networkDeviceInfo() const override;
    void setNetworkDeviceInfo(const NetworkDeviceInfo &networkDeviceInfo);

    bool reachable() const override;
    void setReachable(bool reachable);

    QDateTime lastSeen() const override;
    void setLastSeen(const QDateTime &lastSeen);

    uint pingRetries() const override;
    void setPingRetries(uint pingRetries) override;

    PingReply *currentPingReply() const;
    void setCurrentPingReply(PingReply *reply);

    QDateTime lastConnectionAttempt() const;
    void setLastConnectionAttempt(const QDateTime &lastConnectionAttempt);

    bool isMyNetworkDeviceInfo(const NetworkDeviceInfo &networkDeviceInfo) const;

    bool operator==(NetworkDeviceMonitorImpl *other) const;
    bool operator!=(NetworkDeviceMonitorImpl *other) const;

private:
    MacAddress m_macAddress;
    QString m_hostName;
    QHostAddress m_address;

    NetworkDeviceInfo::MonitorMode m_monitorMode = NetworkDeviceInfo::MonitorModeMac;

    NetworkDeviceInfo m_networkDeviceInfo;

    bool m_reachable = false;
    QDateTime m_lastSeen;
    QDateTime m_lastConnectionAttempt;
    uint m_pingRetries = 5;

    PingReply *m_currentPingReply = nullptr;
};

}

#endif // NETWORKDEVICEMONITORIMPL_H
