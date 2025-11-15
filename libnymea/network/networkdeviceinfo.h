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

#ifndef NETWORKDEVICEINFO_H
#define NETWORKDEVICEINFO_H

#include <QDebug>
#include <QObject>
#include <QDateTime>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QMetaType>

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

Q_DECLARE_METATYPE(NetworkDeviceInfo)

#endif // NETWORKDEVICEINFO_H
