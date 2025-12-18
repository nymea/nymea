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

#ifndef NETWORKDEVICEMONITOR_H
#define NETWORKDEVICEMONITOR_H

#include <QDateTime>
#include <QObject>

#include "libnymea.h"
#include "networkdeviceinfo.h"

class LIBNYMEA_EXPORT NetworkDeviceMonitor : public QObject
{
    Q_OBJECT

public:
    explicit NetworkDeviceMonitor(QObject *parent = nullptr);
    virtual ~NetworkDeviceMonitor() = default;

    // Monitor parameters defining the monitor mode
    virtual MacAddress macAddress() const = 0;
    virtual QString hostName() const = 0;
    virtual QHostAddress address() const = 0;

    virtual NetworkDeviceInfo::MonitorMode monitorMode() const = 0;

    // Actual network device information
    virtual NetworkDeviceInfo networkDeviceInfo() const = 0;

    virtual bool reachable() const = 0;
    virtual QDateTime lastSeen() const = 0;

    virtual uint pingRetries() const = 0;
    virtual void setPingRetries(uint pingRetries) = 0;

signals:
    void reachableChanged(bool reachable);
    void lastSeenChanged(const QDateTime &lastSeen);
    void networkDeviceInfoChanged(const NetworkDeviceInfo &networkDeviceInfo);
    void pingRetriesChanged(uint pingRetries);
};

QDebug operator<<(QDebug debug, NetworkDeviceMonitor *networkDeviceMonitor);

#endif // NETWORKDEVICEMONITOR_H
