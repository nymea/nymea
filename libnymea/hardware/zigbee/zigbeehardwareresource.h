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

#ifndef ZIGBEEHARDWARERESOURCE_H
#define ZIGBEEHARDWARERESOURCE_H

#include <QObject>

#include "hardwareresource.h"

#include <zigbeeaddress.h>
#include <zigbeenetwork.h>

class ZigbeeHandler;
class ZigbeeNode;

class ZigbeeHardwareResource : public HardwareResource
{
    Q_OBJECT
public:
    enum HandlerType {
        HandlerTypeBranding,
        HandlerTypeVendor,
        HandlerTypeCatchAll
    };
    Q_ENUM(HandlerType)
    explicit ZigbeeHardwareResource(QObject *parent = nullptr);
    virtual ~ZigbeeHardwareResource() = default;

    virtual void registerHandler(ZigbeeHandler *handler, HandlerType type = HandlerTypeVendor) = 0;
    virtual ZigbeeNode* claimNode(ZigbeeHandler *hanlder, const QUuid &networkUuid, const ZigbeeAddress &extendedAddress) = 0;
    virtual void removeNodeFromNetwork(const QUuid &networkUuid, ZigbeeNode *node) = 0;

    virtual ZigbeeNetwork::State networkState(const QUuid &networkUuid) = 0;
    virtual ZigbeeAddress coordinatorAddress(const QUuid &networkUuid) = 0;

signals:
    void networkStateChanged(const QUuid &networkUuid, ZigbeeNetwork::State state);

};

#endif // ZIGBEEHARDWARERESOURCE_H
