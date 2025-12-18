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

#ifndef ZIGBEEHARDWARERESOURCEIMPLEMENTATION_H
#define ZIGBEEHARDWARERESOURCEIMPLEMENTATION_H

#include <QObject>

#include "hardware/zigbee/zigbeehardwareresource.h"
#include "zigbee/zigbeemanager.h"

namespace nymeaserver {

class ZigbeeHardwareResourceImplementation : public ZigbeeHardwareResource
{
    Q_OBJECT

public:
    explicit ZigbeeHardwareResourceImplementation(ZigbeeManager *zigbeeManager, QObject *parent = nullptr);

    bool available() const override;
    bool enabled() const override;

    void registerHandler(ZigbeeHandler *handler, HandlerType type = HandlerTypeVendor) override;

    ZigbeeNode *claimNode(ZigbeeHandler *handler, const QUuid &networkUuid, const ZigbeeAddress &extendedAddress) override;
    void removeNodeFromNetwork(const QUuid &networkUuid, ZigbeeNode *node) override;

    ZigbeeNetwork::State networkState(const QUuid &networkUuid) override;
    ZigbeeAddress coordinatorAddress(const QUuid &networkUuid) override;

public slots:
    bool enable();
    bool disable();

    void thingsLoaded();

protected:
    void setEnabled(bool enabled) override;

private slots:
    void onZigbeeAvailableChanged(bool available);
    void onZigbeeNetworkChanged(ZigbeeNetwork *network);
    void onZigbeeNodeAdded(const QUuid &networkUuid, ZigbeeNode *node);
    void onZigbeeNodeRemoved(const QUuid &networkUuid, ZigbeeNode *node);

private:
    bool m_available = false;
    bool m_enabled = false;
    ZigbeeManager *m_zigbeeManager = nullptr;

    QMultiMap<ZigbeeHardwareResource::HandlerType, ZigbeeHandler *> m_handlers;

    bool m_thingsLoaded = false;
    QHash<ZigbeeNode *, ZigbeeHandler *> m_nodeHandlers;
};

} // namespace nymeaserver

#endif // ZIGBEEHARDWARERESOURCEIMPLEMENTATION_H
