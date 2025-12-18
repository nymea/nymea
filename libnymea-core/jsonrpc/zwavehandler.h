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

#ifndef ZWAVEHANDLER_H
#define ZWAVEHANDLER_H

#include "jsonrpc/jsonhandler.h"
#include "zwave/zwavemanager.h"

#include <QObject>

namespace nymeaserver {

class ZWaveHandler : public JsonHandler
{
    Q_OBJECT
public:
    explicit ZWaveHandler(ZWaveManager *zwaveManager, QObject *parent = nullptr);

    QString name() const override;

    Q_INVOKABLE JsonReply *IsZWaveAvailable(const QVariantMap &params);
    Q_INVOKABLE JsonReply *GetSerialPorts(const QVariantMap &params);

    Q_INVOKABLE JsonReply *GetNetworks(const QVariantMap &params);
    Q_INVOKABLE JsonReply *AddNetwork(const QVariantMap &params);
    Q_INVOKABLE JsonReply *RemoveNetwork(const QVariantMap &params);
    Q_INVOKABLE JsonReply *FactoryResetNetwork(const QVariantMap &params);

    Q_INVOKABLE JsonReply *GetNodes(const QVariantMap &params);
    Q_INVOKABLE JsonReply *AddNode(const QVariantMap &params);
    Q_INVOKABLE JsonReply *RemoveNode(const QVariantMap &params);
    Q_INVOKABLE JsonReply *RemoveFailedNode(const QVariantMap &params);
    Q_INVOKABLE JsonReply *CancelPendingOperation(const QVariantMap &params);

signals:
    void NetworkAdded(const QVariantMap &params);
    void NetworkChanged(const QVariantMap &params);
    void NetworkRemoved(const QVariantMap &params);

    void NodeAdded(const QVariantMap &params);
    void NodeChanged(const QVariantMap &params);
    void NodeRemoved(const QVariantMap &params);

private slots:
    void onNetworkAdded(ZWaveNetwork *network);
    void onNetworkChanged(ZWaveNetwork *network);
    void onNetworkRemoved(const QUuid &networkUuid);

    void onNodeAdded(ZWaveNode *node);
    void onNodeChanged(ZWaveNode *node);
    void onNodeRemoved(ZWaveNode *node);

private:
    QVariantMap packNetwork(ZWaveNetwork *network);
    QVariantMap packNode(ZWaveNode *node);

private:
    ZWaveManager *m_zwaveManager = nullptr;
};

} // namespace nymeaserver

#endif // ZWAVEHANDLER_H
