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
