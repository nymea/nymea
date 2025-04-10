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

#include "zigbeehandler.h"
#include "zigbee/zigbeemanager.h"
#include "zigbee/zigbeeadapters.h"
#include "loggingcategories.h"

#include <zigbeeuartadapter.h>

namespace nymeaserver {

ZigbeeHandler::ZigbeeHandler(ZigbeeManager *zigbeeManager, QObject *parent) :
    JsonHandler(parent),
    m_zigbeeManager(zigbeeManager)
{
    qRegisterMetaType<nymeaserver::ZigbeeAdapter>();
    registerEnum<ZigbeeManager::ZigbeeNetworkState>();
    registerEnum<ZigbeeManager::ZigbeeError>();
    registerEnum<ZigbeeManager::ZigbeeNodeType>();
    registerEnum<ZigbeeManager::ZigbeeNodeState>();
    registerObject<ZigbeeAdapter, ZigbeeAdapters>();
    registerEnum<ZigbeeNodeRelationship>();
    registerEnum<ZigbeeNodeRouteStatus>();
    registerEnum<ZigbeeClusterDirection>();

    // Network object describing a network instance
    QVariantMap zigbeeNetworkDescription;
    zigbeeNetworkDescription.insert("networkUuid", enumValueName(Uuid));
    zigbeeNetworkDescription.insert("enabled", enumValueName(Bool));
    zigbeeNetworkDescription.insert("serialPort", enumValueName(String));
    zigbeeNetworkDescription.insert("baudRate", enumValueName(Uint));
    zigbeeNetworkDescription.insert("macAddress", enumValueName(String));
    zigbeeNetworkDescription.insert("firmwareVersion", enumValueName(String));
    zigbeeNetworkDescription.insert("panId", enumValueName(Uint));
    zigbeeNetworkDescription.insert("channel", enumValueName(Uint));
    zigbeeNetworkDescription.insert("channelMask", enumValueName(Uint));
    zigbeeNetworkDescription.insert("permitJoiningEnabled", enumValueName(Bool));
    zigbeeNetworkDescription.insert("permitJoiningDuration", enumValueName(Uint));
    zigbeeNetworkDescription.insert("permitJoiningRemaining", enumValueName(Uint));
    zigbeeNetworkDescription.insert("backend", enumValueName(String));
    zigbeeNetworkDescription.insert("networkState", enumRef<ZigbeeManager::ZigbeeNetworkState>());
    registerObject("ZigbeeNetwork", zigbeeNetworkDescription);

    QVariantMap zigbeeBindingTableRecordDescription;
    zigbeeBindingTableRecordDescription.insert("sourceAddress", enumValueName(String));
    zigbeeBindingTableRecordDescription.insert("sourceEndpointId", enumValueName(Uint));
    zigbeeBindingTableRecordDescription.insert("clusterId", enumValueName(Uint));
    zigbeeBindingTableRecordDescription.insert("o:destinationGroupAddress", enumValueName(Uint));
    zigbeeBindingTableRecordDescription.insert("o:destinationAddress", enumValueName(String));
    zigbeeBindingTableRecordDescription.insert("o:destinationEndpointId", enumValueName(Uint));
    registerObject("ZigbeeBindingTableRecord", zigbeeBindingTableRecordDescription);

    QVariantMap zigbeeNeighborTableRecordDescription;
    zigbeeNeighborTableRecordDescription.insert("networkAddress", enumValueName(Uint));
    zigbeeNeighborTableRecordDescription.insert("relationship", enumRef<ZigbeeNodeRelationship>());
    zigbeeNeighborTableRecordDescription.insert("lqi", enumValueName(Uint));
    zigbeeNeighborTableRecordDescription.insert("depth", enumValueName(Uint));
    zigbeeNeighborTableRecordDescription.insert("permitJoining", enumValueName(Bool));
    registerObject("ZigbeeNeighborTableRecord", zigbeeNeighborTableRecordDescription);

    QVariantMap zigbeeRoutingTableRecordDescription;
    zigbeeRoutingTableRecordDescription.insert("destinationAddress", enumValueName(Uint));
    zigbeeRoutingTableRecordDescription.insert("nextHopAddress", enumValueName(Uint));
    zigbeeRoutingTableRecordDescription.insert("status", enumRef<ZigbeeNodeRouteStatus>());
    zigbeeRoutingTableRecordDescription.insert("manyToOne", enumValueName(Bool));
    zigbeeRoutingTableRecordDescription.insert("memoryConstrained", enumValueName(Bool));
    registerObject("ZigbeeRoutingTableRecord", zigbeeRoutingTableRecordDescription);


    QVariantMap zigbeeClusterDescription;
    zigbeeClusterDescription.insert("clusterId", enumValueName(Uint));
    zigbeeClusterDescription.insert("direction", enumRef<ZigbeeClusterDirection>());
    registerObject("ZigbeeCluster", zigbeeClusterDescription);

    QVariantMap zigbeeNodeEndpointDescription;
    zigbeeNodeEndpointDescription.insert("endpointId", enumValueName(Uint));
    zigbeeNodeEndpointDescription.insert("inputClusters", QVariantList() << objectRef("ZigbeeCluster"));
    zigbeeNodeEndpointDescription.insert("outputClusters", QVariantList() << objectRef("ZigbeeCluster"));
    registerObject("ZigbeeNodeEndpoint", zigbeeNodeEndpointDescription);


    // Zigbee node description
    QVariantMap zigbeeNodeDescription;
    zigbeeNodeDescription.insert("networkUuid", enumValueName(Uuid));
    zigbeeNodeDescription.insert("ieeeAddress", enumValueName(String));
    zigbeeNodeDescription.insert("networkAddress", enumValueName(Uint));
    zigbeeNodeDescription.insert("type", enumRef<ZigbeeManager::ZigbeeNodeType>());
    zigbeeNodeDescription.insert("state", enumRef<ZigbeeManager::ZigbeeNodeState>());
    zigbeeNodeDescription.insert("manufacturer", enumValueName(String));
    zigbeeNodeDescription.insert("model", enumValueName(String));
    zigbeeNodeDescription.insert("version", enumValueName(String));
    zigbeeNodeDescription.insert("receiverOnWhileIdle", enumValueName(Bool));
    zigbeeNodeDescription.insert("reachable", enumValueName(Bool));
    zigbeeNodeDescription.insert("lqi", enumValueName(Uint));
    zigbeeNodeDescription.insert("lastSeen", enumValueName(Uint));
    zigbeeNodeDescription.insert("endpoints", QVariantList() << objectRef("ZigbeeNodeEndpoint"));
    zigbeeNodeDescription.insert("neighborTableRecords", QVariantList() << objectRef("ZigbeeNeighborTableRecord"));
    zigbeeNodeDescription.insert("routingTableRecords", QVariantList() << objectRef("ZigbeeRoutingTableRecord"));
    zigbeeNodeDescription.insert("bindingTableRecords", QVariantList() << objectRef("ZigbeeBindingTableRecord"));
    registerObject("ZigbeeNode", zigbeeNodeDescription);

    QVariantMap params, returns;
    QString description;

    // GetAvailableBackends
    params.clear(); returns.clear();
    description = "Get the list of available ZigBee backends.";
    returns.insert("backends", QVariantList() << enumValueName(String));
    registerMethod("GetAvailableBackends", description, params, returns);

    // GetAdapters
    params.clear(); returns.clear();
    description = "Get the list of available ZigBee adapters and serial ports in order to set up the ZigBee network "
                  "on the desired interface. The \'serialPort\' property can be used as unique identifier for an adapter. "
                  "If an adapter hardware has been recognized as a well known ZigBee adapter, "
                  "the \'hardwareRecognized\' property will be true and the \'baudRate\' and \'backend\' "
                  "configurations can be used as they where given, otherwise the user might set the backend "
                  "and baud rate manually. The available backends can be fetched using the GetAvailableBackends method.";
    returns.insert("adapters", objectRef<ZigbeeAdapters>());
    registerMethod("GetAdapters", description, params, returns);

    // AdapterAdded notification
    params.clear();
    description = "Emitted whenever a new ZigBee adapter or serial port has been detected in the system.";
    params.insert("adapter", objectRef<ZigbeeAdapter>());
    registerNotification("AdapterAdded", description, params);

    // AdapterRemoved notification
    params.clear();
    description = "Emitted whenever a ZigBee adapter or serial port has been removed from the system (i.e. unplugged).";
    params.insert("adapter", objectRef<ZigbeeAdapter>());
    registerNotification("AdapterRemoved", description, params);

    // GetNetworks
    params.clear(); returns.clear();
    description = "Returns the list of configured ZigBee networks in the system.";
    returns.insert("zigbeeNetworks", QVariantList() << objectRef("ZigbeeNetwork"));
    registerMethod("GetNetworks", description, params, returns);

    // AddNetwork
    params.clear(); returns.clear();
    description = "Create a new ZigBee network for the given \'serialPort\', \'baudRate\' and \'backend\'. "
                  "The serial ports can be fetched from the available adapters. See \'GetAdapters\' for more information. "
                  "The available backends can be fetched using the \'GetAvailableBackends\' method.";
    params.insert("serialPort", enumValueName(String));
    params.insert("baudRate", enumValueName(Uint));
    params.insert("backend", enumValueName(String));
    params.insert("o:channelMask", enumValueName(Uint));
    returns.insert("zigbeeError", enumRef<ZigbeeManager::ZigbeeError>());
    returns.insert("o:networkUuid", enumValueName(Uuid));
    registerMethod("AddNetwork", description, params, returns);

    // RemoveNetwork
    params.clear(); returns.clear();
    description = "Remove the ZigBee network with the given network uuid.";
    params.insert("networkUuid", enumValueName(Uuid));
    returns.insert("zigbeeError", enumRef<ZigbeeManager::ZigbeeError>());
    registerMethod("RemoveNetwork", description, params, returns);

    // NetworkAdded notification
    params.clear();
    description = "Emitted whenever a new ZigBee network has been added.";
    params.insert("zigbeeNetwork", objectRef("ZigbeeNetwork"));
    registerNotification("NetworkAdded", description, params);

    // NetworkRemoved notification
    params.clear();
    description = "Emitted whenever a new ZigBee network has been removed.";
    params.insert("networkUuid", enumValueName(Uuid));
    registerNotification("NetworkRemoved", description, params);

    // NetworkChanged notification
    params.clear();
    description = "Emitted whenever a new ZigBee network has changed.";
    params.insert("zigbeeNetwork", objectRef("ZigbeeNetwork"));
    registerNotification("NetworkChanged", description, params);

    // FactoryResetNetwork
    params.clear(); returns.clear();
    description = "Factory reset the network with the given \'networkUuid\'. The network does not have "
                  "to be online for this procedure, and all associated nodes and things will be removed permanently.";
    params.insert("networkUuid", enumValueName(Uuid));
    returns.insert("zigbeeError", enumRef<ZigbeeManager::ZigbeeError>());
    registerMethod("FactoryResetNetwork", description, params, returns);

    // SetPermitJoin
    params.clear(); returns.clear();
    description = "Allow or deny nodes to join the network with the given \'networkUuid\' for a specific \'duration\' in seconds. "
                  "The duration value has to be between 0 and 255 seconds. The \'permitJoinDuration\' property of ZigBee network "
                  "object indicates how long permit has been enabled and the \'permitJoiningRemaining\' indicates the rest of the time. "
                  "Those values can be used to show a countdown or progressbar. This method can be recalled for resetting the timeout. "
                  "If the duration is set to 0 seconds, joining will be disabled immediatly for the entire network. "
                  "The \'shortAddress\' is optional and defaults to the broadcast address 0xfffc for all routers in the network. "
                  "If the short address matches the address of a router node in the network, only that specific router will "
                  "be able to allow new nodes to join the network. A new node will join to the router with the best link quality index (LQI).";
    params.insert("networkUuid", enumValueName(Uuid));
    params.insert("duration", enumValueName(Uint));
    params.insert("o:shortAddress", enumValueName(Uint));
    returns.insert("zigbeeError", enumRef<ZigbeeManager::ZigbeeError>());
    registerMethod("SetPermitJoin", description, params, returns);

    params.clear(); returns.clear();
    description = "Refresh the neighbor table for all nodes. Note that calling this may cause a lot of traffic in the ZigBee network.";
    params.insert("networkUuid", enumValueName(Uuid));
    returns.insert("zigbeeError", enumRef<ZigbeeManager::ZigbeeError>());
    registerMethod("RefreshNeighborTables", description, params, returns);

    // GetNodes
    params.clear(); returns.clear();
    description = "Returns the list of ZigBee nodes from the network the given \'networkUuid\' in the system.";
    params.insert("networkUuid", enumValueName(Uuid));
    returns.insert("zigbeeError", enumRef<ZigbeeManager::ZigbeeError>());
    returns.insert("o:zigbeeNodes", QVariantList() << objectRef("ZigbeeNode"));
    registerMethod("GetNodes", description, params, returns);

    // RefreshBindings
    params.clear(); returns.clear();
    description = "Refresh the binding table for the given node.";
    params.insert("networkUuid", enumValueName(Uuid));
    params.insert("ieeeAddress", enumValueName(String));
    returns.insert("zigbeeError", enumRef<ZigbeeManager::ZigbeeError>());
    registerMethod("RefreshBindings", description, params, returns);

    // CreateBinding
    params.clear(); returns.clear();
    description = "Create a binding. Use destinationAddress and destinationEndpointId to create a node to node binding, or use destinationGroupAddress to create a group binding.";
    params.insert("networkUuid", enumValueName(Uuid));
    params.insert("sourceAddress", enumValueName(String));
    params.insert("sourceEndpointId", enumValueName(Uint));
    params.insert("clusterId", enumValueName(Uint));
    params.insert("o:destinationAddress", enumValueName(String));
    params.insert("o:destinationEndpointId", enumValueName(Uint));
    params.insert("o:destinationGroupAddress", enumValueName(Uint));
    returns.insert("zigbeeError", enumRef<ZigbeeManager::ZigbeeError>());
    registerMethod("CreateBinding", description, params, returns);


    // RemoveBinding
    params.clear(); returns.clear();
    description = "Remove a binding.";
    params.insert("networkUuid", enumValueName(Uuid));
    params.insert("sourceAddress", enumValueName(String));
    params.insert("sourceEndpointId", enumValueName(Uint));
    params.insert("clusterId", enumValueName(Uint));
    params.insert("o:destinationAddress", enumValueName(String));
    params.insert("o:destinationEndpointId", enumValueName(Uint));
    params.insert("o:destinationGroupAddress", enumValueName(Uint));
    returns.insert("zigbeeError", enumRef<ZigbeeManager::ZigbeeError>());
    registerMethod("RemoveBinding", description, params, returns);

    // RemoveNode
    params.clear(); returns.clear();
    description = "Remove a ZigBee node with the given \'ieeeAddress\' from the network with the given \'networkUuid\'. "
                  "If there is a thing configured for this node, also the thing will be removed from the system. "
                  "The coordinator node cannot be removed.";
    params.insert("networkUuid", enumValueName(Uuid));
    params.insert("ieeeAddress", enumValueName(String));
    returns.insert("zigbeeError", enumRef<ZigbeeManager::ZigbeeError>());
    registerMethod("RemoveNode", description, params, returns);

    // NodeAdded
    params.clear();
    description = "Emitted whenever a new ZigBee node has joined the network with the given \'networkUuid\'.";
    params.insert("networkUuid", enumValueName(Uuid));
    params.insert("zigbeeNode", objectRef("ZigbeeNode"));
    registerNotification("NodeAdded", description, params);

    // NodeRemoved
    params.clear();
    description = "Emitted whenever a ZigBee node has removed from the network with the given \'networkUuid\'.";
    params.insert("networkUuid", enumValueName(Uuid));
    params.insert("zigbeeNode", objectRef("ZigbeeNode"));
    registerNotification("NodeRemoved", description, params);

    // NodeChanged
    params.clear();
    description = "Emitted whenever a ZigBee node has changed.";
    params.insert("networkUuid", enumValueName(Uuid));
    params.insert("zigbeeNode", objectRef("ZigbeeNode"));
    registerNotification("NodeChanged", description, params);

    connect(m_zigbeeManager, &ZigbeeManager::availableAdapterAdded, this, [this](const ZigbeeAdapter &adapter){
        QVariantMap params;
        params.insert("adapter", pack(adapter));
        emit AdapterAdded(params);
    });

    connect(m_zigbeeManager, &ZigbeeManager::availableAdapterRemoved, this, [this](const ZigbeeAdapter &adapter){
        QVariantMap params;
        params.insert("adapter", pack(adapter));
        emit AdapterRemoved(params);
    });

    connect(m_zigbeeManager, &ZigbeeManager::zigbeeNetworkAdded, this, [this](ZigbeeNetwork *network){
        QVariantMap params;
        params.insert("zigbeeNetwork", packNetwork(network));
        emit NetworkAdded(params);
    });

    connect(m_zigbeeManager, &ZigbeeManager::zigbeeNetworkChanged, this, [this](ZigbeeNetwork *network){
        QVariantMap params;
        params.insert("zigbeeNetwork", packNetwork(network));
        emit NetworkChanged(params);
    });

    connect(m_zigbeeManager, &ZigbeeManager::zigbeeNetworkRemoved, this, [this](const QUuid &networkUuid){
        QVariantMap params;
        params.insert("networkUuid", networkUuid);
        emit NetworkRemoved(params);
    });

    connect(m_zigbeeManager, &ZigbeeManager::nodeJoined, this, &ZigbeeHandler::onNodeJoined);
    connect(m_zigbeeManager, &ZigbeeManager::nodeAdded, this, &ZigbeeHandler::onNodeAdded);
    connect(m_zigbeeManager, &ZigbeeManager::nodeChanged, this, &ZigbeeHandler::onNodeChanged);
    connect(m_zigbeeManager, &ZigbeeManager::nodeRemoved, this, &ZigbeeHandler::onNodeRemoved);

}

QString ZigbeeHandler::name() const
{
    return "Zigbee";
}

JsonReply *ZigbeeHandler::GetAvailableBackends(const QVariantMap &params)
{
    Q_UNUSED(params)

    QVariantMap returnMap;
    QVariantList backendsList;
    foreach (const QString &backendName, ZigbeeAdapter::backendNames().values()) {
        backendsList << backendName;
    }
    returnMap.insert("backends", backendsList);
    return createReply(returnMap);
}

JsonReply *ZigbeeHandler::GetAdapters(const QVariantMap &params)
{
    Q_UNUSED(params)

    QVariantMap returnMap;
    QVariantList adapterList;
    foreach (const ZigbeeAdapter &adapter, m_zigbeeManager->availableAdapters()) {
        adapterList << pack(adapter);
    }
    returnMap.insert("adapters", adapterList);
    return createReply(returnMap);
}

JsonReply *ZigbeeHandler::AddNetwork(const QVariantMap &params)
{
    QVariantMap returnMap;

    QString serialPort = params.value("serialPort").toString();
    uint baudRate = params.value("baudRate").toUInt();
    QString backendString = params.value("backend").toString();
    if (!ZigbeeAdapter::backendNames().values().contains(backendString)) {
        returnMap.insert("zigbeeError", enumValueName<ZigbeeManager::ZigbeeError>(ZigbeeManager::ZigbeeErrorUnknownBackend));
        return createReply(returnMap);
    }

    ZigbeeChannelMask channelMask = ZigbeeChannelMask::ChannelConfigurationAllChannels;
    if (params.contains("channelMask")) {
        channelMask = params.value("channelMask").toUInt() & ZigbeeChannelMask::ChannelConfigurationAllChannels;
        if (channelMask == ZigbeeChannelMask::ChannelConfigurationNoChannel) {
            returnMap.insert("zigbeeError", enumValueName<ZigbeeManager::ZigbeeError>(ZigbeeManager::ZigbeeErrorInvalidChannel));
            return createReply(returnMap);
        }
    }

    QPair<ZigbeeManager::ZigbeeError, QUuid> result = m_zigbeeManager->createZigbeeNetwork(serialPort, baudRate, ZigbeeAdapter::backendNames().key(backendString), channelMask);
    if (result.first == ZigbeeManager::ZigbeeErrorNoError) {
        returnMap.insert("networkUuid", result.second);
    }
    returnMap.insert("zigbeeError", enumValueName<ZigbeeManager::ZigbeeError>(result.first));
    return createReply(returnMap);
}

JsonReply *ZigbeeHandler::RemoveNetwork(const QVariantMap &params)
{
    QUuid networkUuid = params.value("networkUuid").toUuid();
    ZigbeeManager::ZigbeeError error = m_zigbeeManager->removeZigbeeNetwork(networkUuid);
    QVariantMap returnMap;
    returnMap.insert("zigbeeError", enumValueName<ZigbeeManager::ZigbeeError>(error));
    return createReply(returnMap);
}

JsonReply *ZigbeeHandler::FactoryResetNetwork(const QVariantMap &params)
{
    QUuid networkUuid = params.value("networkUuid").toUuid();
    ZigbeeManager::ZigbeeError error = m_zigbeeManager->factoryResetNetwork(networkUuid);
    QVariantMap returnMap;
    returnMap.insert("zigbeeError", enumValueName<ZigbeeManager::ZigbeeError>(error));
    return createReply(returnMap);
}

JsonReply *ZigbeeHandler::SetPermitJoin(const QVariantMap &params)
{
    QUuid networkUuid = params.value("networkUuid").toUuid();
    uint duration = params.value("duration").toUInt();
    quint16 shortAddress = static_cast<quint16>(Zigbee::BroadcastAddressAllRouters);
    if (params.contains("shortAddress")) {
        shortAddress = static_cast<quint16>(params.value("shortAddress").toUInt());
    }
    ZigbeeManager::ZigbeeError error = m_zigbeeManager->setZigbeeNetworkPermitJoin(networkUuid, shortAddress, duration);
    QVariantMap returnMap;
    returnMap.insert("zigbeeError", enumValueName<ZigbeeManager::ZigbeeError>(error));
    return createReply(returnMap);
}

JsonReply *ZigbeeHandler::GetNodes(const QVariantMap &params)
{
    QVariantMap returnMap;
    QUuid networkUuid = params.value("networkUuid").toUuid();
    ZigbeeNetwork *network = m_zigbeeManager->zigbeeNetworks().value(networkUuid);
    if (!network) {
        returnMap.insert("zigbeeError", enumValueName<ZigbeeManager::ZigbeeError>(ZigbeeManager::ZigbeeErrorNetworkUuidNotFound));
        return createReply(returnMap);
    }

    QVariantList nodeList;
    foreach (ZigbeeNode *node, network->nodes()) {
        nodeList << packNode(node);
    }

    returnMap.insert("zigbeeError", enumValueName<ZigbeeManager::ZigbeeError>(ZigbeeManager::ZigbeeErrorNoError));
    returnMap.insert("zigbeeNodes", nodeList);
    return createReply(returnMap);
}

JsonReply *ZigbeeHandler::RemoveNode(const QVariantMap &params)
{
    QVariantMap returnMap;
    QUuid networkUuid = params.value("networkUuid").toUuid();
    ZigbeeAddress nodeAddress(params.value("ieeeAddress").toString());
    ZigbeeNetwork *network = m_zigbeeManager->zigbeeNetworks().value(networkUuid);
    if (!network) {
        returnMap.insert("zigbeeError", enumValueName<ZigbeeManager::ZigbeeError>(ZigbeeManager::ZigbeeErrorNetworkUuidNotFound));
        return createReply(returnMap);
    }

    if (!network->hasNode(nodeAddress)) {
        returnMap.insert("zigbeeError", enumValueName<ZigbeeManager::ZigbeeError>(ZigbeeManager::ZigbeeErrorNodeNotFound));
        return createReply(returnMap);
    }

    ZigbeeNode *node = network->getZigbeeNode(nodeAddress);
    if (node->shortAddress() == 0x0000) {
        returnMap.insert("zigbeeError", enumValueName<ZigbeeManager::ZigbeeError>(ZigbeeManager::ZigbeeErrorForbidden));
        return createReply(returnMap);
    }

    network->removeZigbeeNode(nodeAddress);
    returnMap.insert("zigbeeError", enumValueName<ZigbeeManager::ZigbeeError>(ZigbeeManager::ZigbeeErrorNoError));
    return createReply(returnMap);
}

JsonReply *ZigbeeHandler::RefreshNeighborTables(const QVariantMap &params)
{
    ZigbeeManager::ZigbeeError error = m_zigbeeManager->refreshNeighborTables(params.value("networkUuid").toUuid());
    return createReply({{"zigbeeError", enumValueName(error)}});
}

JsonReply *ZigbeeHandler::RefreshBindings(const QVariantMap &params)
{
    QUuid networkUuid = params.value("networkUuid").toUuid();
    QString ieeeAddress = params.value("ieeeAddress").toString();
    ZigbeeNetwork *network = m_zigbeeManager->zigbeeNetworks().value(networkUuid);
    if (!network) {
        return createReply({{"zigbeeError", enumValueName(ZigbeeManager::ZigbeeErrorNetworkUuidNotFound)}});
    }
    ZigbeeNode *node = m_zigbeeManager->zigbeeNetworks().value(networkUuid)->getZigbeeNode(ZigbeeAddress(ieeeAddress));
    if (!node) {
        return createReply({{"zigbeeError", enumValueName(ZigbeeManager::ZigbeeErrorNodeNotFound)}});
    }
    JsonReply *jsonReply = createAsyncReply("RefreshBindings");
    ZigbeeReply *reply = node->readBindingTableEntries();
    connect(reply, &ZigbeeReply::finished, jsonReply, [reply, jsonReply](){
        jsonReply->setData({{"zigbeeError", enumValueName(reply->error())}});
        jsonReply->finished();
    });
    return jsonReply;
}

JsonReply *ZigbeeHandler::CreateBinding(const QVariantMap &params)
{
    QUuid networkUuid = params.value("networkUuid").toUuid();
    ZigbeeNetwork *network = m_zigbeeManager->zigbeeNetworks().value(networkUuid);
    if (!network) {
        return createReply({{"zigbeeError", enumValueName(ZigbeeManager::ZigbeeErrorNetworkUuidNotFound)}});
    }
    QString sourceAddress = params.value("sourceAddress").toString();
    ZigbeeNode *node = network->getZigbeeNode(ZigbeeAddress(sourceAddress));
    if (!node) {
        qCWarning(dcJsonRpc()) << "No Zigbee node for sourceAddress" << sourceAddress;
        return createReply({{"zigbeeError", enumValueName(ZigbeeManager::ZigbeeErrorNodeNotFound)}});
    }

    quint8 sourceEndpointId = params.value("sourceEndpointId").toUInt();
    quint16 clusterId = params.value("clusterId").toUInt();
    if (params.contains("destinationAddress") && params.contains("destinationEndpointId")) {
        QString destinationAddress = params.value("destinationAddress").toString();
        quint8 destinationEndpointId = params.value("destinationEndpointId").toUInt();
        ZigbeeReply *reply = node->addBinding(sourceEndpointId, clusterId, ZigbeeAddress(destinationAddress), destinationEndpointId);
        JsonReply *jsonReply = createAsyncReply("CreateBinding");
        connect(reply, &ZigbeeReply::finished, jsonReply, [reply, jsonReply](){
            ZigbeeManager::ZigbeeError error = ZigbeeManager::ZigbeeErrorNoError;
            switch (reply->error()) {
            case ZigbeeReply::ErrorNoError:
                break;
            case ZigbeeReply::ErrorTimeout:
                error = ZigbeeManager::ZigbeeErrorTimeoutError;
                break;
            default:
                error = ZigbeeManager::ZigbeeErrorNetworkError;
                break;
            }
            jsonReply->setData({{"zigbeeError", enumValueName(static_cast<ZigbeeManager::ZigbeeError>(error))}});
            emit jsonReply->finished();
        });
        return jsonReply;

    } else if (params.contains("destinationGroupAddress")) {
        quint16 destinationGroupAddress = params.value("destinationGroupAddress").toUInt();
        ZigbeeReply *reply = node->addBinding(sourceEndpointId, clusterId, destinationGroupAddress);
        JsonReply *jsonReply = createAsyncReply("CreateBinding");
        connect(reply, &ZigbeeReply::finished, jsonReply, [reply, jsonReply](){
            ZigbeeManager::ZigbeeError error = ZigbeeManager::ZigbeeErrorNoError;
            switch (reply->error()) {
            case ZigbeeReply::ErrorNoError:
                break;
            case ZigbeeReply::ErrorTimeout:
                error = ZigbeeManager::ZigbeeErrorTimeoutError;
                break;
            default:
                error = ZigbeeManager::ZigbeeErrorNetworkError;
                break;
            }
            jsonReply->setData({{"zigbeeError", enumValueName(static_cast<ZigbeeManager::ZigbeeError>(error))}});
            emit jsonReply->finished();
        });
        return jsonReply;
    }
    return createReply({{"zigbeeError", enumValueName(ZigbeeManager::ZigbeeErrorNodeNotFound)}});
}

JsonReply *ZigbeeHandler::RemoveBinding(const QVariantMap &params)
{
    QUuid networkUuid = params.value("networkUuid").toUuid();
    ZigbeeNetwork *network = m_zigbeeManager->zigbeeNetworks().value(networkUuid);
    if (!network) {
        return createReply({{"zigbeeError", enumValueName(ZigbeeManager::ZigbeeErrorNetworkUuidNotFound)}});
    }
    ZigbeeAddress sourceAddress = ZigbeeAddress(params.value("sourceAddress").toString());
    ZigbeeNode *node = network->getZigbeeNode(sourceAddress);
    if (!node) {
        qCWarning(dcJsonRpc()) << "No Zigbee node for sourceAddress" << sourceAddress;
        return createReply({{"zigbeeError", enumValueName(ZigbeeManager::ZigbeeErrorNodeNotFound)}});
    }

    quint8 sourceEndpointId = params.value("sourceEndpointId").toUInt();
    quint16 clusterId = params.value("clusterId").toUInt();
    quint16 destinationGroupAddress = params.value("destinationGroupAddress").toUInt();
    ZigbeeAddress destinationAddress = ZigbeeAddress(params.value("destinationAddress").toString());
    quint8 destinationEndpointId = params.value("destinationEndpointId").toUInt();
    bool isGroup = params.contains("destinationGroupAddress");

    foreach (const ZigbeeDeviceProfile::BindingTableListRecord &binding, node->bindingTableRecords()) {
        bool found = false;
        if (isGroup) {
            if (binding.sourceAddress == sourceAddress
                    && binding.sourceEndpoint == sourceEndpointId
                    && binding.clusterId == clusterId
                    && binding.destinationShortAddress == destinationGroupAddress) {
                found = true;
            }
        } else {
            if (binding.sourceAddress == sourceAddress
                    && binding.sourceEndpoint == sourceEndpointId
                    && binding.clusterId == clusterId
                    && binding.destinationIeeeAddress == destinationAddress
                    && binding.destinationEndpoint == destinationEndpointId) {
                found = true;
            }
        }

        if (found) {
            ZigbeeReply *reply = node->removeBinding(binding);
            JsonReply *jsonReply = createAsyncReply("RemoveBinding");
            connect(reply, &ZigbeeReply::finished, jsonReply, [reply, jsonReply](){
                ZigbeeManager::ZigbeeError error = ZigbeeManager::ZigbeeErrorNoError;
                switch (reply->error()) {
                case ZigbeeReply::ErrorNoError:
                    break;
                case ZigbeeReply::ErrorTimeout:
                    error = ZigbeeManager::ZigbeeErrorTimeoutError;
                    break;
                default:
                    error = ZigbeeManager::ZigbeeErrorNetworkError;
                    break;
                }
                jsonReply->setData({{"zigbeeError", enumValueName(static_cast<ZigbeeManager::ZigbeeError>(error))}});
                emit jsonReply->finished();
            });
            return jsonReply;
        }
    }
    return createReply({{"zigbeeError", enumValueName(ZigbeeManager::ZigbeeErrorNodeNotFound)}});
}

JsonReply *ZigbeeHandler::GetNetworks(const QVariantMap &params)
{
    Q_UNUSED(params)

    QVariantMap returnMap;
    QVariantList networkList;
    foreach (ZigbeeNetwork *network, m_zigbeeManager->zigbeeNetworks().values()) {
        networkList.append(packNetwork(network));
    }
    returnMap.insert("zigbeeNetworks", networkList);
    return createReply(returnMap);
}

QVariantMap ZigbeeHandler::packNetwork(ZigbeeNetwork *network)
{
    QVariantMap networkMap;
    networkMap.insert("networkUuid", network->networkUuid());
    networkMap.insert("enabled", true); // FIXME: set actual value once supported
    networkMap.insert("serialPort", network->serialPortName());
    networkMap.insert("baudRate", network->serialBaudrate());
    networkMap.insert("macAddress", network->macAddress().toString());
    networkMap.insert("firmwareVersion", network->firmwareVersion());
    networkMap.insert("panId", network->panId());
    networkMap.insert("channel", network->channel());
    networkMap.insert("channelMask", network->channelMask().toUInt32());
    networkMap.insert("permitJoiningEnabled", network->permitJoiningEnabled());
    networkMap.insert("permitJoiningDuration", network->permitJoiningDuration());
    networkMap.insert("permitJoiningRemaining", network->permitJoiningRemaining());

    switch (network->backendType()) {
    case Zigbee::ZigbeeBackendTypeDeconz:
        networkMap.insert("backend", ZigbeeAdapter::backendNames().value(ZigbeeAdapter::ZigbeeBackendTypeDeconz));
        break;
    case Zigbee::ZigbeeBackendTypeNxp:
        networkMap.insert("backend", ZigbeeAdapter::backendNames().value(ZigbeeAdapter::ZigbeeBackendTypeNxp));
        break;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    case Zigbee::ZigbeeBackendTypeTi:
        networkMap.insert("backend", ZigbeeAdapter::backendNames().value(ZigbeeAdapter::ZigbeeBackendTypeTi));
        break;
#endif
    }

    switch (network->state()) {
    case ZigbeeNetwork::StateOffline:
    case ZigbeeNetwork::StateStopping:
        networkMap.insert("networkState", enumValueName<ZigbeeManager::ZigbeeNetworkState>(ZigbeeManager::ZigbeeNetworkStateOffline));
        break;
    case ZigbeeNetwork::StateStarting:
        networkMap.insert("networkState", enumValueName<ZigbeeManager::ZigbeeNetworkState>(ZigbeeManager::ZigbeeNetworkStateStarting));
        break;
    case ZigbeeNetwork::StateRunning:
        networkMap.insert("networkState", enumValueName<ZigbeeManager::ZigbeeNetworkState>(ZigbeeManager::ZigbeeNetworkStateOnline));
        break;
    case ZigbeeNetwork::StateUpdating:
        networkMap.insert("networkState", enumValueName<ZigbeeManager::ZigbeeNetworkState>(ZigbeeManager::ZigbeeNetworkStateUpdating));
        break;
    case ZigbeeNetwork::StateUninitialized:
        networkMap.insert("networkState", enumValueName<ZigbeeManager::ZigbeeNetworkState>(ZigbeeManager::ZigbeeNetworkStateError));
        break;
    }

    return networkMap;
}

QVariantMap ZigbeeHandler::packNode(ZigbeeNode *node)
{
    QVariantMap nodeMap;
    nodeMap.insert("networkUuid", node->networkUuid());
    nodeMap.insert("ieeeAddress", node->extendedAddress().toString());
    nodeMap.insert("networkAddress", node->shortAddress());
    switch (node->nodeDescriptor().nodeType) {
    case ZigbeeDeviceProfile::NodeTypeCoordinator:
        nodeMap.insert("type", enumValueName<ZigbeeManager::ZigbeeNodeType>(ZigbeeManager::ZigbeeNodeTypeCoordinator));
        break;
    case ZigbeeDeviceProfile::NodeTypeRouter:
        nodeMap.insert("type", enumValueName<ZigbeeManager::ZigbeeNodeType>(ZigbeeManager::ZigbeeNodeTypeRouter));
        break;
    default:
        nodeMap.insert("type", enumValueName<ZigbeeManager::ZigbeeNodeType>(ZigbeeManager::ZigbeeNodeTypeEndDevice));
        break;
    }

    switch (node->state()) {
    case ZigbeeNode::StateUninitialized:
        nodeMap.insert("state", enumValueName<ZigbeeManager::ZigbeeNodeState>(ZigbeeManager::ZigbeeNodeStateUninitialized));
        break;
    case ZigbeeNode::StateInitializing:
        nodeMap.insert("state", enumValueName<ZigbeeManager::ZigbeeNodeState>(ZigbeeManager::ZigbeeNodeStateInitializing));
        break;
    case ZigbeeNode::StateInitialized:
        nodeMap.insert("state", enumValueName<ZigbeeManager::ZigbeeNodeState>(ZigbeeManager::ZigbeeNodeStateInitialized));
        break;
    }

    nodeMap.insert("manufacturer", node->manufacturerName());
    nodeMap.insert("model", node->modelName());
    nodeMap.insert("version", node->version());
    nodeMap.insert("receiverOnWhileIdle", node->macCapabilities().receiverOnWhenIdle);
    nodeMap.insert("reachable", node->reachable());
    nodeMap.insert("lqi", node->lqi());
    nodeMap.insert("lastSeen", node->lastSeen().toMSecsSinceEpoch() / 1000);
    QVariantList neighborTableRecords;
    foreach (const ZigbeeDeviceProfile::NeighborTableListRecord &record, node->neighborTableRecords()) {
        QVariantMap recordMap;
        recordMap.insert("networkAddress", record.shortAddress);
        recordMap.insert("depth", record.depth);
        recordMap.insert("lqi", record.lqi);
        recordMap.insert("relationship", enumValueName(static_cast<ZigbeeHandler::ZigbeeNodeRelationship>(record.relationship)));
        recordMap.insert("permitJoining", record.permitJoining);
        neighborTableRecords.append(recordMap);
    }
    nodeMap.insert("neighborTableRecords", neighborTableRecords);
    QVariantList routingTableRecords;
    foreach (const ZigbeeDeviceProfile::RoutingTableListRecord &record, node->routingTableRecords()) {
        QVariantMap recordMap;
        recordMap.insert("destinationAddress", record.destinationAddress);
        recordMap.insert("nextHopAddress", record.nextHopAddress);
        recordMap.insert("status", enumValueName(static_cast<ZigbeeHandler::ZigbeeNodeRouteStatus>(record.status)));
        recordMap.insert("memoryConstrained", record.memoryConstrained);
        recordMap.insert("manyToOne", record.manyToOne);
        routingTableRecords.append(recordMap);
    }
    nodeMap.insert("routingTableRecords", routingTableRecords);
    QVariantList bindingTableRecords;
    foreach (const ZigbeeDeviceProfile::BindingTableListRecord &record, node->bindingTableRecords()) {
        QVariantMap recordMap;
        recordMap.insert("sourceAddress", record.sourceAddress.toString());
        recordMap.insert("sourceEndpointId", record.sourceEndpoint);
        recordMap.insert("clusterId", record.clusterId);
        if (record.destinationAddressMode == Zigbee::DestinationAddressModeGroup) {
            recordMap.insert("destinationGroupAddress", record.destinationShortAddress);
        } else if (record.destinationAddressMode == Zigbee::DestinationAddressModeIeeeAddress) {
            recordMap.insert("destinationAddress", record.destinationIeeeAddress.toString());
            recordMap.insert("destinationEndpointId", record.destinationEndpoint);
        }
        bindingTableRecords.append(recordMap);
    }
    nodeMap.insert("bindingTableRecords", bindingTableRecords);
    QVariantList endpoints;
    foreach (ZigbeeNodeEndpoint *endpoint, node->endpoints()) {
        QVariantMap endpointMap;
        endpointMap.insert("endpointId", endpoint->endpointId());
        QVariantList inputClusters;
        foreach (ZigbeeCluster *cluster, endpoint->inputClusters()) {
            QVariantMap clusterMap;
            clusterMap.insert("clusterId", cluster->clusterId());
            clusterMap.insert("direction", enumValueName(static_cast<ZigbeeClusterDirection>(cluster->direction())));
            inputClusters.append(clusterMap);
        }
        endpointMap.insert("inputClusters", inputClusters);
        QVariantList outputClusters;
        foreach (ZigbeeCluster *cluster, endpoint->outputClusters()) {
            QVariantMap clusterMap;
            clusterMap.insert("clusterId", cluster->clusterId());
            clusterMap.insert("direction", enumValueName(static_cast<ZigbeeClusterDirection>(cluster->direction())));
            outputClusters.append(clusterMap);
        }
        endpointMap.insert("outputClusters", outputClusters);
        endpoints.append(endpointMap);
    }
    nodeMap.insert("endpoints", endpoints);
    return nodeMap;
}

void ZigbeeHandler::onNodeJoined(const QUuid &networkUuid, ZigbeeNode *node)
{
    QVariantMap params;
    params.insert("networkUuid", networkUuid);
    params.insert("zigbeeNode", packNode(node));
    emit NodeAdded(params);
}

void ZigbeeHandler::onNodeAdded(const QUuid &networkUuid, ZigbeeNode *node)
{
    // Note: we emit the node changed signal here, since the node has been added internally after initialization.
    onNodeChanged(networkUuid, node);
}

void ZigbeeHandler::onNodeChanged(const QUuid &networkUuid, ZigbeeNode *node)
{
    QVariantMap params;
    params.insert("networkUuid", networkUuid);
    params.insert("zigbeeNode", packNode(node));
    emit NodeChanged(params);
}

void ZigbeeHandler::onNodeRemoved(const QUuid &networkUuid, ZigbeeNode *node)
{
    QVariantMap params;
    params.insert("networkUuid", networkUuid);
    params.insert("zigbeeNode", packNode(node));
    emit NodeRemoved(params);
}

}
