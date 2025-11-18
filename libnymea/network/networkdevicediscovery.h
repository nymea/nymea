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

#ifndef NETWORKDEVICEDISCOVERY_H
#define NETWORKDEVICEDISCOVERY_H

#include <QTimer>
#include <QObject>
#include <QLoggingCategory>

#include "libnymea.h"
#include "hardwareresource.h"

#include "networkdevicemonitor.h"

#include "pingreply.h"
#include "macaddressdatabasereply.h"
#include "networkdevicediscoveryreply.h"

#include "integrations/thing.h"

class LIBNYMEA_EXPORT NetworkDeviceDiscovery : public HardwareResource
{
    Q_OBJECT
public:
    explicit NetworkDeviceDiscovery(QObject *parent = nullptr);
    virtual ~NetworkDeviceDiscovery() = default;

    virtual NetworkDeviceDiscoveryReply *discover() = 0;

    virtual bool running() const = 0;

    virtual NetworkDeviceMonitor *registerMonitor(Thing *thing) = 0;
    virtual void unregisterMonitor(NetworkDeviceMonitor *networkDeviceMonitor) = 0;

    virtual PingReply *ping(const QHostAddress &address, uint retries = 3) = 0;
    virtual PingReply *ping(const QString &hostName, uint retries = 3) = 0;

    virtual MacAddressDatabaseReply *lookupMacAddress(const QString &macAddress) = 0;
    virtual MacAddressDatabaseReply *lookupMacAddress(const MacAddress &macAddress) = 0;

    virtual bool sendArpRequest(const QHostAddress &address) = 0;

    virtual NetworkDeviceInfos cache() const = 0;

signals:
    void runningChanged(bool running);
    void cacheUpdated();

};

#endif // NETWORKDEVICEDISCOVERY_H
