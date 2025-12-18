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

#ifndef NETWORKDEVICEINFOS_H
#define NETWORKDEVICEINFOS_H

#include <QObject>

#include "libnymea.h"
#include "macaddress.h"
#include "networkdeviceinfo.h"

class LIBNYMEA_EXPORT NetworkDeviceInfos : public QVector<NetworkDeviceInfo>
{
public:
    explicit NetworkDeviceInfos();
    NetworkDeviceInfos(const QVector<NetworkDeviceInfo> &other);

    int indexFromHostAddress(const QHostAddress &address);
    int indexFromHostName(const QString &hostName);
    QList<int> indexFromMacAddress(const QString &macAddress);
    QList<int> indexFromMacAddress(const MacAddress &macAddress);

    bool hasHostAddress(const QHostAddress &address);
    bool hasMacAddress(const QString &macAddress);
    bool hasMacAddress(const MacAddress &macAddress);

    NetworkDeviceInfo get(const QHostAddress &address) const;
    void removeHostAddress(const QHostAddress &address);

    void sortNetworkDevices();

    NetworkDeviceInfos &operator<<(const NetworkDeviceInfo &networkDeviceInfo);
};

#endif // NETWORKDEVICEINFOS_H
