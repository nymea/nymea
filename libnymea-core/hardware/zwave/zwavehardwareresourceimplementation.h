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

#ifndef ZWAVEHARDWARERESOURCEIMPLEMENTATION_H
#define ZWAVEHARDWARERESOURCEIMPLEMENTATION_H

#include <QObject>
#include <QHash>
#include <QMultiMap>

#include "hardware/zwave/zwavehardwareresource.h"

class ZWaveNetwork;

namespace nymeaserver
{

class ZWaveManager;

class ZWaveHardwareResourceImplementation : public ZWaveHardwareResource
{
    Q_OBJECT
public:
    ZWaveHardwareResourceImplementation(ZWaveManager *zwaveManager, QObject *parent = nullptr);

    bool available() const override;
    bool enabled() const override;
    void setEnabled(bool enabled) override;

    void registerHandler(ZWaveHandler *handler, HandlerType type = HandlerTypeVendor) override;
    ZWaveNode *claimNode(ZWaveHandler *handler, const QUuid &networkUuid, quint8 nodeId) override;

signals:

public slots:
    void thingsLoaded();

private slots:
    void onNetworkStateChanged(ZWaveNetwork *network);
    void onNodeInitialized(ZWaveNode *node);
    void onNodeRemoved(ZWaveNode *node);

    void handleNewNode(ZWaveNode *node);

private:
    bool m_available = false;
    bool m_enabled = false;
    ZWaveManager *m_zwaveManager = nullptr;

    QMultiMap<ZWaveHardwareResource::HandlerType, ZWaveHandler*> m_handlers;

    bool m_thingsLoaded = false;
    QHash<ZWaveNode*, ZWaveHandler*> m_nodeHandlers;

};

}

#endif // ZWAVEHARDWARERESOURCEIMPLEMENTATION_H
