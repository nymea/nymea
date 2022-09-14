/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2020, nymea GmbH
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

#ifndef ZIGBEEHANDLER_H
#define ZIGBEEHANDLER_H

#include <QObject>

#include "jsonrpc/jsonhandler.h"

#include <zigbeenetworkmanager.h>

namespace nymeaserver {

class ZigbeeManager;

class ZigbeeHandler : public JsonHandler
{
    Q_OBJECT
public:
    enum ZigbeeNodeRelationship {
        ZigbeeNodeRelationshipParent,
        ZigbeeNodeRelationshipChild,
        ZigbeeNodeRelationshipSibling,
        ZigbeeNodeRelationshipNone,
        ZigbeeNodeRelationshipPreviousChild
    };
    Q_ENUM(ZigbeeNodeRelationship)

    enum ZigbeeNodeRouteStatus {
        ZigbeeNodeRouteStatusActive,
        ZigbeeNodeRouteStatusDiscoveryUnderway,
        ZigbeeNodeRouteStatusDiscoveryFailed,
        ZigbeeNodeRouteStatusInactive,
        ZigbeeNodeRouteStatusValidationUnderway
    };
    Q_ENUM(ZigbeeNodeRouteStatus)

    enum ZigbeeClusterDirection {
        ZigbeeClusterDirectionServer,
        ZigbeeClusterDirectionClient
    };
    Q_ENUM(ZigbeeClusterDirection)


    explicit ZigbeeHandler(ZigbeeManager *zigbeeManager, QObject *parent = nullptr);

    QString name() const override;

    Q_INVOKABLE JsonReply *GetAvailableBackends(const QVariantMap &params);
    Q_INVOKABLE JsonReply *GetAdapters(const QVariantMap &params);

    Q_INVOKABLE JsonReply *GetNetworks(const QVariantMap &params);
    Q_INVOKABLE JsonReply *AddNetwork(const QVariantMap &params);
    Q_INVOKABLE JsonReply *RemoveNetwork(const QVariantMap &params);
    Q_INVOKABLE JsonReply *FactoryResetNetwork(const QVariantMap &params);
    Q_INVOKABLE JsonReply *SetPermitJoin(const QVariantMap &params);

    Q_INVOKABLE JsonReply *GetNodes(const QVariantMap &params);
    Q_INVOKABLE JsonReply *RemoveNode(const QVariantMap &params);

    Q_INVOKABLE JsonReply *RefreshNeighborTables(const QVariantMap &params);

    Q_INVOKABLE JsonReply *RefreshBindings(const QVariantMap &params);
    Q_INVOKABLE JsonReply *CreateBinding(const QVariantMap &params);
    Q_INVOKABLE JsonReply *RemoveBinding(const QVariantMap &params);

    QVariantMap packNetwork(ZigbeeNetwork *network);
    QVariantMap packNode(ZigbeeNode *node);

private:
    ZigbeeManager *m_zigbeeManager = nullptr;

private slots:
    void onNodeJoined(const QUuid &networkUuid, ZigbeeNode *node);
    void onNodeAdded(const QUuid &networkUuid, ZigbeeNode *node);
    void onNodeChanged(const QUuid &networkUuid, ZigbeeNode *node);
    void onNodeRemoved(const QUuid &networkUuid, ZigbeeNode *node);

signals:
    void AdapterAdded(const QVariantMap &params);
    void AdapterRemoved(const QVariantMap &params);

    void NetworkAdded(const QVariantMap &params);
    void NetworkRemoved(const QVariantMap &params);
    void NetworkChanged(const QVariantMap &params);

    void NodeAdded(const QVariantMap &params);
    void NodeRemoved(const QVariantMap &params);
    void NodeChanged(const QVariantMap &params);
};

}

#endif // ZIGBEEHANDLER_H
