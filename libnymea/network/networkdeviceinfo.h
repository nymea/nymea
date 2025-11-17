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

#ifndef NETWORKDEVICEINFO_H
#define NETWORKDEVICEINFO_H

#include <QDebug>
#include <QObject>
#include <QDateTime>
#include <QHostAddress>
#include <QNetworkInterface>

#include "libnymea.h"
#include "macaddressinfos.h"

class LIBNYMEA_EXPORT NetworkDeviceInfo
{
    Q_GADGET
public:

    enum MonitorMode {
        MonitorModeMac      = 0x01, // Unique MAC address within the network
        MonitorModeHostName = 0x02, // DNS hostname available, but no MAC address or not unique MAC available
        MonitorModeIp       = 0x03  // Only the IP can be used to monitor, simple ping on reachable
    };
    Q_ENUM(MonitorMode)

    explicit NetworkDeviceInfo();
    explicit NetworkDeviceInfo(const QString &macAddress);
    explicit NetworkDeviceInfo(const QHostAddress &address);

    QHostAddress address() const;
    void setAddress(const QHostAddress &address);

    QString hostName() const;
    void setHostName(const QString &hostName);

    MacAddressInfos macAddressInfos() const;
    void addMacAddress(const MacAddress &macAddress);
    void addMacAddress(const MacAddress &macAddress, const QString &vendorName);

    QNetworkInterface networkInterface() const;
    void setNetworkInterface(const QNetworkInterface &networkInterface);

    MonitorMode monitorMode() const;
    void setMonitorMode(MonitorMode monitorMode);

    bool isValid() const;
    bool isComplete() const;

    void forceComplete();

    QString incompleteProperties() const;

    // Helper methods for the networkdevice interface
    // They fill in automatically the correct parameters for the
    // right monitor
    QString thingParamValueMacAddress() const;
    QString thingParamValueHostName() const;
    QString thingParamValueAddress() const;

    bool operator==(const NetworkDeviceInfo &other) const;
    bool operator!=(const NetworkDeviceInfo &other) const;

private:
    QHostAddress m_address;
    MacAddressInfos m_macAddressInfos;
    QString m_hostName;
    QNetworkInterface m_networkInterface;
    MonitorMode m_monitorMode = MonitorModeMac;

    bool m_addressSet = false;
    bool m_hostNameSet = false;
    bool m_networkInterfaceSet = false;
    bool m_forceComplete = false;
};

QDebug operator<<(QDebug debug, const NetworkDeviceInfo &networkDeviceInfo);

#endif // NETWORKDEVICEINFO_H
