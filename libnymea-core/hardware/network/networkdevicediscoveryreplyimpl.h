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

#ifndef NETWORKDEVICEDISCOVERYREPLYIMPL_H
#define NETWORKDEVICEDISCOVERYREPLYIMPL_H

#include <QHash>
#include <QObject>

#include "network/networkdeviceinfo.h"
#include "network/networkdevicediscoveryreply.h"

namespace nymeaserver {

class NetworkDeviceDiscoveryReplyImpl : public NetworkDeviceDiscoveryReply
{
    Q_OBJECT

public:
    explicit NetworkDeviceDiscoveryReplyImpl(QObject *parent = nullptr);
    ~NetworkDeviceDiscoveryReplyImpl() override = default;

    NetworkDeviceInfos networkDeviceInfos() const override;

    bool isFinished() const override;
    void setFinished(bool finished);

    // Add or update the network device info and verify if completed
    void processPingResponse(const QHostAddress &address, const QString &hostName);
    void processArpResponse(const QNetworkInterface &interface, const QHostAddress &address, const MacAddress &macAddress);
    void processMacManufacturer(const MacAddress &macAddress, const QString &manufacturer);

    void processDiscoveryFinished();

    QHash<QHostAddress, NetworkDeviceInfo> currentCache() const;

public slots:
    void addCompleteNetworkDeviceInfo(const NetworkDeviceInfo &networkDeviceInfo);

private:
    QHash<QHostAddress, NetworkDeviceInfo> m_networkDeviceCache;
    qint64 m_startTimestamp;
    bool m_isFinished = false;

    NetworkDeviceInfos m_networkDeviceInfos;

    void evaluateMonitorMode();
};

}

#endif // NETWORKDEVICEDISCOVERYREPLYIMPL_H
