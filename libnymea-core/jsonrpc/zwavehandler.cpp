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

#include "zwavehandler.h"

namespace nymeaserver {

ZWaveHandler::ZWaveHandler(ZWaveManager *zwaveManager, QObject *parent)
    : JsonHandler(parent)
    , m_zwaveManager(zwaveManager)
{
    registerEnum<ZWave::ZWaveError>();
    registerObject<SerialPort, SerialPorts>();
    registerEnum<ZWaveNetwork::ZWaveNetworkState>();
    registerEnum<ZWaveNode::ZWaveNodeType>();
    registerEnum<ZWaveNode::ZWaveNodeRole>();
    registerEnum<ZWaveNode::ZWaveDeviceType>();

    QVariantMap networkDescription;
    networkDescription.insert("networkUuid", enumValueName(Uuid));
    networkDescription.insert("serialPort", enumValueName(String));
    networkDescription.insert("networkState", enumRef<ZWaveNetwork::ZWaveNetworkState>());
    networkDescription.insert("homeId", enumValueName(Uint));
    networkDescription.insert("isZWavePlus", enumValueName(Bool));
    networkDescription.insert("isStaticUpdateController", enumValueName(Bool));
    networkDescription.insert("isPrimaryController", enumValueName(Bool));
    networkDescription.insert("isBridgeController", enumValueName(Bool));
    networkDescription.insert("waitingForNodeAddition", enumValueName(Bool));
    networkDescription.insert("waitingForNodeRemoval", enumValueName(Bool));
    registerObject("ZWaveNetwork", networkDescription);

    QVariantMap nodeDescription;
    nodeDescription.insert("nodeId", enumValueName(Uint));
    nodeDescription.insert("networkUuid", enumValueName(Uuid));
    nodeDescription.insert("initialized", enumValueName(Bool));
    nodeDescription.insert("reachable", enumValueName(Bool));
    nodeDescription.insert("failed", enumValueName(Bool));
    nodeDescription.insert("sleeping", enumValueName(Bool));
    nodeDescription.insert("linkQuality", enumValueName(Uint));
    nodeDescription.insert("securityMode", enumValueName(Uint));
    nodeDescription.insert("nodeType", enumRef<ZWaveNode::ZWaveNodeType>());
    nodeDescription.insert("role", enumRef<ZWaveNode::ZWaveNodeRole>());
    nodeDescription.insert("deviceType", enumRef<ZWaveNode::ZWaveDeviceType>());
    nodeDescription.insert("productType", enumValueName(Uint));
    nodeDescription.insert("productId", enumValueName(Uint));
    nodeDescription.insert("productName", enumValueName(String));
    nodeDescription.insert("manufacturerId", enumValueName(Uint));
    nodeDescription.insert("manufacturerName", enumValueName(String));
    nodeDescription.insert("version", enumValueName(String));
    nodeDescription.insert("isZWavePlusDevice", enumValueName(Bool));
    nodeDescription.insert("isSecurityDevice", enumValueName(Bool));
    nodeDescription.insert("isBeamingDevice", enumValueName(Bool));

    registerObject("ZWaveNode", nodeDescription);

    QVariantMap params, returns;
    QString description;

    params.clear();
    returns.clear();
    description = "Query if the Z-Wave subsystem is available at all.";
    returns.insert("available", enumValueName(Bool));
    registerMethod("IsZWaveAvailable", description, params, returns);

    params.clear();
    returns.clear();
    description = "Get the list of available serial ports from the host system.";
    returns.insert("serialPorts", objectRef<SerialPorts>());
    registerMethod("GetSerialPorts", description, params, returns);

    params.clear();
    returns.clear();
    description = "Get all the Z-Wave networks in the system.";
    returns.insert("networks", QVariantList() << objectRef("ZWaveNetwork"));
    registerMethod("GetNetworks", description, params, returns);

    params.clear();
    returns.clear();
    description = "Add a new Z-Wave network with the given serial port.";
    params.insert("serialPort", enumValueName(String));
    returns.insert("o:networkUuid", enumValueName(Uuid));
    returns.insert("zwaveError", enumRef<ZWave::ZWaveError>());
    registerMethod("AddNetwork", description, params, returns);

    params.clear();
    returns.clear();
    description = "Remove the given Z-Wave network from the system.";
    params.insert("networkUuid", enumValueName(Uuid));
    returns.insert("zwaveError", enumRef<ZWave::ZWaveError>());
    registerMethod("RemoveNetwork", description, params, returns);

    params.clear();
    returns.clear();
    description = "Start the node inclusion procedure for the given Z-Wave network.";
    params.insert("networkUuid", enumValueName(Uuid));
    returns.insert("zwaveError", enumRef<ZWave::ZWaveError>());
    registerMethod("AddNode", description, params, returns);

    params.clear();
    returns.clear();
    description = "Start the node removal procedure for the given Z-Wave network.";
    params.insert("networkUuid", enumValueName(Uuid));
    returns.insert("zwaveError", enumRef<ZWave::ZWaveError>());
    registerMethod("RemoveNode", description, params, returns);

    params.clear();
    returns.clear();
    description = "Cancel any running node inclusion or removal procedure for the given Z-Wave network.";
    params.insert("networkUuid", enumValueName(Uuid));
    returns.insert("zwaveError", enumRef<ZWave::ZWaveError>());
    registerMethod("CancelPendingOperation", description, params, returns);

    params.clear();
    returns.clear();
    description = "Remove the given failed node from the given Z-Wave network. This will not work if node is not marked as failed.";
    params.insert("networkUuid", enumValueName(Uuid));
    params.insert("nodeId", enumValueName(Uint));
    returns.insert("zwaveError", enumRef<ZWave::ZWaveError>());
    registerMethod("RemoveFailedNode", description, params, returns);

    params.clear();
    returns.clear();
    description = "Factory reset the controller for the given Z-Wave network.";
    params.insert("networkUuid", enumValueName(Uuid));
    returns.insert("zwaveError", enumRef<ZWave::ZWaveError>());
    registerMethod("FactoryResetNetwork", description, params, returns);

    params.clear();
    returns.clear();
    description = "Get the list of nodes in a network";
    params.insert("networkUuid", enumValueName(Uuid));
    returns.insert("zwaveError", enumRef<ZWave::ZWaveError>());
    returns.insert("o:nodes", QVariantList() << objectRef("ZWaveNode"));
    registerMethod("GetNodes", description, params, returns);

    // Notifications
    params.clear();
    description = "Emitted whenever a new Z-Wave network has been added to the system.";
    params.insert("network", objectRef("ZWaveNetwork"));
    registerNotification("NetworkAdded", description, params);

    params.clear();
    description = "Emitted whenever a Z-Wave network has been removed from the system.";
    params.insert("networkUuid", enumValueName(Uuid));
    registerNotification("NetworkRemoved", description, params);

    params.clear();
    description = "Emitted whenever a Z-Wave network changes.";
    params.insert("network", objectRef("ZWaveNetwork"));
    registerNotification("NetworkChanged", description, params);

    params.clear();
    description = "Emitted whenever a Z-Wave node is added.";
    params.insert("networkUuid", enumValueName(Uuid));
    params.insert("node", objectRef("ZWaveNode"));
    registerNotification("NodeAdded", description, params);

    params.clear();
    description = "Emitted whenever a Z-Wave node has changed.";
    params.insert("networkUuid", enumValueName(Uuid));
    params.insert("node", objectRef("ZWaveNode"));
    registerNotification("NodeChanged", description, params);

    params.clear();
    description = "Emitted whenever a Z-Wave node is removed.";
    params.insert("networkUuid", enumValueName(Uuid));
    params.insert("nodeId", enumValueName(Uint));
    registerNotification("NodeRemoved", description, params);

    connect(m_zwaveManager, &ZWaveManager::networkAdded, this, &ZWaveHandler::onNetworkAdded);
    connect(m_zwaveManager, &ZWaveManager::networkChanged, this, &ZWaveHandler::onNetworkChanged);
    connect(m_zwaveManager, &ZWaveManager::networkStateChanged, this, &ZWaveHandler::onNetworkChanged);
    connect(m_zwaveManager, &ZWaveManager::networkRemoved, this, &ZWaveHandler::onNetworkRemoved);
    connect(m_zwaveManager, &ZWaveManager::nodeAdded, this, &ZWaveHandler::onNodeAdded);
    connect(m_zwaveManager, &ZWaveManager::nodeChanged, this, &ZWaveHandler::onNodeChanged);
    connect(m_zwaveManager, &ZWaveManager::nodeRemoved, this, &ZWaveHandler::onNodeRemoved);
}

QString ZWaveHandler::name() const
{
    return "ZWave";
}

JsonReply *ZWaveHandler::IsZWaveAvailable(const QVariantMap &params)
{
    Q_UNUSED(params)
    return createReply({{"available", m_zwaveManager->available()}});
}

JsonReply *ZWaveHandler::GetSerialPorts(const QVariantMap &params)
{
    Q_UNUSED(params)

    QVariantList portList;
    foreach (const SerialPort &serialPort, m_zwaveManager->serialPorts()) {
        portList << pack(serialPort);
    }
    return createReply({{"serialPorts", portList}});
}

JsonReply *ZWaveHandler::GetNetworks(const QVariantMap &params)
{
    Q_UNUSED(params)

    QVariantList networkList;
    foreach (ZWaveNetwork *network, m_zwaveManager->networks()) {
        networkList.append(packNetwork(network));
    }
    return createReply({{"networks", networkList}});
}

JsonReply *ZWaveHandler::AddNetwork(const QVariantMap &params)
{
    QPair<ZWave::ZWaveError, QUuid> status = m_zwaveManager->createNetwork(params.value("serialPort").toString());

    QVariantMap returns;
    returns.insert("zwaveError", enumValueName(status.first));
    if (status.first == ZWave::ZWaveErrorNoError) {
        returns.insert("networkUuid", status.second);
    }
    return createReply(returns);
}

JsonReply *ZWaveHandler::RemoveNetwork(const QVariantMap &params)
{
    QUuid networkUuid = params.value("networkUuid").toUuid();
    ZWave::ZWaveError status = m_zwaveManager->removeNetwork(networkUuid);
    return createReply({{"zwaveError", enumValueName(status)}});
}

JsonReply *ZWaveHandler::FactoryResetNetwork(const QVariantMap &params)
{
    QUuid networkUuid = params.value("networkUuid").toUuid();
    ZWave::ZWaveError status = m_zwaveManager->factoryResetNetwork(networkUuid);
    return createReply({{"zwaveError", enumValueName(status)}});
}

JsonReply *ZWaveHandler::AddNode(const QVariantMap &params)
{
    ZWaveReply *zwaveReply = m_zwaveManager->addNode(params.value("networkUuid").toUuid());
    JsonReply *jsonReply = createAsyncReply("AddNode");
    connect(zwaveReply, &ZWaveReply::finished, jsonReply, [jsonReply](ZWave::ZWaveError status) {
        jsonReply->setData({{"zwaveError", enumValueName(status)}});
        jsonReply->finished();
    });
    return jsonReply;
}

JsonReply *ZWaveHandler::GetNodes(const QVariantMap &params)
{
    QUuid networkUuid = params.value("networkUuid").toUuid();
    ZWaveNetwork *network = m_zwaveManager->network(networkUuid);
    if (!network) {
        return createReply({{"zwaveError", enumValueName(ZWave::ZWaveErrorNetworkUuidNotFound)}});
    }
    ZWaveNodes nodes = m_zwaveManager->network(networkUuid)->nodes();
    QVariantList nodeList;
    foreach (ZWaveNode *node, nodes) {
        nodeList.append(packNode(node));
    }
    return createReply({{"zwaveError", enumValueName(ZWave::ZWaveErrorNoError)}, {"nodes", nodeList}});
}

JsonReply *ZWaveHandler::RemoveNode(const QVariantMap &params)
{
    QUuid networkUuid = params.value("networkUuid").toUuid();

    JsonReply *jsonReply = createAsyncReply("RemoveNode");
    ZWaveReply *zwaveReply = m_zwaveManager->removeNode(networkUuid);
    connect(zwaveReply, &ZWaveReply::finished, jsonReply, [jsonReply](ZWave::ZWaveError status) {
        jsonReply->setData({{"zwaveError", enumValueName(status)}});
        jsonReply->finished();
    });
    return jsonReply;
}

JsonReply *ZWaveHandler::RemoveFailedNode(const QVariantMap &params)
{
    QUuid networkUuid = params.value("networkUuid").toUuid();
    quint8 nodeId = params.value("nodeId").toUInt();

    m_zwaveManager->removeFailedNode(networkUuid, nodeId);
    return createReply({{"zwaveError", enumValueName(ZWave::ZWaveErrorNoError)}});
}

JsonReply *ZWaveHandler::CancelPendingOperation(const QVariantMap &params)
{
    QUuid networkUuid = params.value("networkUuid").toUuid();
    m_zwaveManager->cancelPendingOperation(networkUuid);
    return createReply({{"zwaveError", enumValueName(ZWave::ZWaveErrorNoError)}});
}

void ZWaveHandler::onNetworkAdded(ZWaveNetwork *network)
{
    emit NetworkAdded({{"network", packNetwork(network)}});
}

void ZWaveHandler::onNetworkChanged(ZWaveNetwork *network)
{
    emit NetworkChanged({{"network", packNetwork(network)}});
}

void ZWaveHandler::onNetworkRemoved(const QUuid &networkUuid)
{
    emit NetworkRemoved({{"networkUuid", networkUuid.toString()}});
}

void ZWaveHandler::onNodeAdded(ZWaveNode *node)
{
    emit NodeAdded({{"networkUuid", node->networkUuid()}, {"node", packNode(node)}});
}

void ZWaveHandler::onNodeChanged(ZWaveNode *node)
{
    emit NodeChanged({{"networkUuid", node->networkUuid()}, {"node", packNode(node)}});
}

void ZWaveHandler::onNodeRemoved(ZWaveNode *node)
{
    emit NodeRemoved({{"networkUuid", node->networkUuid()}, {"nodeId", node->nodeId()}});
}

QVariantMap ZWaveHandler::packNetwork(ZWaveNetwork *network)
{
    return {{"networkUuid", network->networkUuid()},
            {"serialPort", network->serialPort()},
            {"networkState", enumValueName(network->networkState())},
            {"homeId", network->homeId()},
            {"isZWavePlus", network->isZWavePlus()},
            {"isPrimaryController", network->isPrimaryController()},
            {"isStaticUpdateController", network->isStaticUpdateController()},
            {"isBridgeController", network->isBridgeController()},
            {"waitingForNodeAddition", network->waitingForNodeAddition()},
            {"waitingForNodeRemoval", network->waitingForNodeRemoval()}};
}

QVariantMap ZWaveHandler::packNode(ZWaveNode *node)
{
    return {{"nodeId", node->nodeId()},
            {"networkUuid", node->networkUuid()},
            {"initialized", node->initialized()},
            {"reachable", node->reachable()},
            {"failed", node->failed()},
            {"sleeping", node->sleeping()},
            {"linkQuality", node->linkQuality()},
            {"securityMode", node->securityMode()},
            {"nodeType", enumValueName(node->nodeType())},
            {"role", enumValueName(node->role())},
            {"deviceType", enumValueName(node->deviceType())},
            {"productType", node->productType()},
            {"productId", node->productId()},
            {"productName", node->productName()},
            {"manufacturerId", node->manufacturerId()},
            {"manufacturerName", node->manufacturerName()},
            {"version", node->version()},
            {"isZWavePlusDevice", node->isZWavePlusDevice()},
            {"isSecurityDevice", node->isSecurityDevice()},
            {"isBeamingDevice", node->isBeamingDevice()}};
}

} // namespace nymeaserver
