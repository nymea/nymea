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

#include "zwavemanager.h"
#include "hardware/zwave/zwavebackend.h"
#include "zwavemanagerreply.h"
#include "zwave/zwavenodeimplementation.h"

#include "nymeasettings.h"

#include <QSerialPortInfo>
#include <QCoreApplication>
#include <QDir>
#include <QPluginLoader>
#include <QStringList>
#include <QRegularExpression>

namespace nymeaserver
{

ZWaveManager::ZWaveManager(SerialPortMonitor *serialPortMonitor, QObject *parent):
    QObject(parent),
    m_serialPortMonitor(serialPortMonitor)
{
    qRegisterMetaType<ZWave::ZWaveError>();
    qRegisterMetaType<ZWaveValue::Genre>();
    qRegisterMetaType<ZWaveValue::CommandClass>();

#ifdef WITH_OPENZWAVE
    m_backend = new OpenZWaveBackend(this);
#endif

    if (!loadBackend()) {
        qCInfo(dcZWave()) << "No Z-Wave backend found. Z-Wave support will be disabled.";
        return;
    }

    loadZWaveNetworks();

    connect(m_backend, &ZWaveBackend::networkStarted, this, &ZWaveManager::onNetworkStarted);
    connect(m_backend, &ZWaveBackend::networkFailed, this, &ZWaveManager::onNetworkFailed);
    connect(m_backend, &ZWaveBackend::waitingForNodeAdditionChanged, this, &ZWaveManager::onWaitingForNodeAdditionChanged);
    connect(m_backend, &ZWaveBackend::waitingForNodeRemovalChanged, this, &ZWaveManager::onWaitingForNodeRemovalChanged);

    connect(m_backend, &ZWaveBackend::nodeAdded, this, &ZWaveManager::onNodeAdded);
    connect(m_backend, &ZWaveBackend::nodeInitialized, this, &ZWaveManager::onNodeInitialized);
    connect(m_backend, &ZWaveBackend::nodeRemoved, this, &ZWaveManager::onNodeRemoved);
    connect(m_backend, &ZWaveBackend::nodeDataChanged, this, &ZWaveManager::onNodeDataChanged);
    connect(m_backend, &ZWaveBackend::nodeReachableStatus, this, &ZWaveManager::onNodeReachableStatus);
    connect(m_backend, &ZWaveBackend::nodeFailedStatus, this, &ZWaveManager::onNodeFailedStatus);
    connect(m_backend, &ZWaveBackend::nodeSleepStatus, this, &ZWaveManager::onNodeSleepStatus);
    connect(m_backend, &ZWaveBackend::nodeLinkQualityStatus, this, &ZWaveManager::onNodeLinkQualityStatus);

    connect(m_backend, &ZWaveBackend::valueAdded, this, &ZWaveManager::onValueAdded);
    connect(m_backend, &ZWaveBackend::valueChanged, this, &ZWaveManager::onValueChanged);
    connect(m_backend, &ZWaveBackend::valueRemoved, this, &ZWaveManager::onValueRemoved);
}

ZWaveManager::~ZWaveManager()
{
}

bool ZWaveManager::available() const
{
    return m_backend != nullptr;
}

bool ZWaveManager::enabled() const
{
    return true; // TODO
}

void ZWaveManager::setEnabled(bool enabled)
{
    Q_UNUSED(enabled)
    // TODO
}

SerialPorts ZWaveManager::serialPorts() const
{
    SerialPorts serialPorts;

    // FIXME: There should be a mechanism in SerialPortMonitor so that resources can claim a port and it won't show
    // up any more in other resources.
    foreach (const SerialPort &serialPort, m_serialPortMonitor->serialPorts()) {
        bool used = false;
        foreach (ZWaveNetwork *network, m_networks) {
            if (network->serialPort() == serialPort.portName()) {
                used = true;
            }
        }
        if (!used) {
            serialPorts.append(serialPort);
        }
    }
    return serialPorts;
}

void ZWaveManager::loadZWaveNetworks()
{
    NymeaSettings settings(NymeaSettings::SettingsRoleZWave);
    qCDebug(dcZWave()) << "Loading ZWave networks from" << settings.fileName();
    settings.beginGroup("Networks");
        foreach (const QString &uuidString, settings.childGroups()) {
        settings.beginGroup(uuidString);
        QString serialPort = settings.value("serialPort").toString();
        quint32 homeId = settings.value("homeId").toULongLong();
        QString networkKey = settings.value("networkKey").toString();
        quint8 controllerNodeId = settings.value("controllerNodeId").toUInt();
        bool isZWavePlus = settings.value("isZWavePlus").toBool();
        bool isPrimaryController = settings.value("isPrimaryController").toBool();
        bool isStaticUpdateController = settings.value("isStaticUpdateController").toBool();
        settings.endGroup(); // uuid

        ZWaveNetwork *network = new ZWaveNetwork(QUuid(uuidString), serialPort, networkKey, this);
        network->setHomeId(homeId);
        network->setControllerNodeId(controllerNodeId);
        network->setIsZWavePlus(isZWavePlus);
        network->setIsPrimaryController(isPrimaryController);
        network->setIsStaticUpdateController(isStaticUpdateController);

        loadNetwork(network);
        qCInfo(dcZWave) << "Loaded network" << uuidString << "with" << network->nodes().count() << "nodes";
        foreach (ZWaveNode *node, network->nodes()) {
            qCDebug(dcZWave) << node;
        }
    }

    settings.endGroup(); // Networks
}

bool ZWaveManager::loadNetwork(ZWaveNetwork *network)
{
    bool success = m_backend->startNetwork(network->networkUuid(), network->serialPort(), network->networkKey());
    if (!success) {
        return false;
    }

    ZWaveDeviceDatabase *db = new ZWaveDeviceDatabase(NymeaSettings::settingsPath(), network->networkUuid());
    if (!db->initDB()) {
        qCCritical(dcZWave()) << "Unable to initialize ZWave device database";
        delete db;
        return false;
    }

    m_networks.insert(network->networkUuid(), network);
    m_dbs.insert(network->networkUuid(), db);

    foreach (ZWaveNode *n, db->createNodes(this)) {
        ZWaveNodeImplementation *node = qobject_cast<ZWaveNodeImplementation*>(n);
        node->setInitialized(true);
        connect(node, &ZWaveNodeImplementation::nodeChanged, this, [this, node](){emit nodeChanged(node);});
        network->addNode(node);
    }

    network->setNetworkState(ZWaveNetwork::ZWaveNetworkStateStarting);
    emit networkStateChanged(network);
    return true;
}

void ZWaveManager::storeNetwork(ZWaveNetwork *network)
{
    NymeaSettings settings(NymeaSettings::SettingsRoleZWave);
    settings.beginGroup("Networks");
    settings.beginGroup(network->networkUuid().toString());
    settings.setValue("serialPort", network->serialPort());
    settings.setValue("homeId", network->homeId());
    settings.setValue("networkKey", network->networkKey());
    settings.setValue("controllerNodeId", network->controllerNodeId());
    settings.setValue("isZWavePlus", network->isZWavePlus());
    settings.setValue("isPrimaryController", network->isPrimaryController());
    settings.setValue("isStaticUpdateController", network->isStaticUpdateController());
    settings.endGroup();
    settings.endGroup();
}

void ZWaveManager::onNetworkStarted(const QUuid &networkUuid)
{
    ZWaveNetwork *network = m_networks.value(networkUuid);
    if (!network) {
        qCWarning(dcZWave()) << "Received a network started signal for a network we don't know:" << networkUuid.toString();
        return;
    }

    network->setHomeId(m_backend->homeId(networkUuid));
    network->setControllerNodeId(m_backend->controllerNodeId(networkUuid));
    network->setIsPrimaryController(m_backend->isPrimaryController(networkUuid));
    network->setIsStaticUpdateController(m_backend->isStaticUpdateController(networkUuid));
    network->setIsBridgeController(m_backend->isBridgeController(networkUuid));
    qCDebug(dcZWave()) << "Network started" << network->networkUuid().toString();
    storeNetwork(network);
    emit networkChanged(network);

    network->setNetworkState(ZWaveNetwork::ZWaveNetworkStateOnline);
    emit networkStateChanged(network);

    foreach (ZWaveNode *n, network->nodes()) {
        ZWaveNodeImplementation *node = qobject_cast<ZWaveNodeImplementation*>(n);
        if (node->failed() != m_backend->isNodeFailed(networkUuid, node->nodeId())) {
            node->setFailed(m_backend->isNodeFailed(networkUuid, node->nodeId()));
            emit nodeChanged(node);
        }
        qCDebug(dcZWave) << "Node" << node->productName() << "is failed:" << node->failed() << "awake:" << m_backend->isNodeAwake(networkUuid, node->nodeId());
    }
}

void ZWaveManager::onNetworkFailed(const QUuid &networkUuid)
{
    ZWaveNetwork *network = m_networks.value(networkUuid);
    if (!network) {
        qCWarning(dcZWave()) << "Received a network failed signal for a network we don't know:" << networkUuid.toString();
        return;
    }
    qCWarning(dcZWave()).nospace() << "Failed to initialize adapter for network " << network->networkUuid().toString() << " at " << network->serialPort() << ". Retrying in 5 seconds...";
    network->setNetworkState(ZWaveNetwork::ZWaveNetworkStateError);
    emit networkStateChanged(network);

    // As long as the network exists, keep retrying...
    QTimer::singleShot(5000, network, [this, network]() {
        qCInfo(dcZWave()) << "Retrying to initialize adapter for network" << network->networkUuid().toString() << "at" << network->serialPort();
        network->setNetworkState(ZWaveNetwork::ZWaveNetworkStateStarting);
        emit networkStateChanged(network);
        if (!m_backend->startNetwork(network->networkUuid(), network->serialPort())) {
            onNetworkFailed(network->networkUuid());
        }
    });
}

void ZWaveManager::onWaitingForNodeAdditionChanged(const QUuid &networkUuid, bool waitingForNodeAddition)
{
    ZWaveNetwork *network = m_networks.value(networkUuid);
    if (!network) {
        qCWarning(dcZWave()) << "Received a network waiting for node addition changed signal for a network we don't know:" << networkUuid.toString();
        return;
    }
    network->setWaitingForNodeAddition(waitingForNodeAddition);
    emit networkChanged(network);
}

void ZWaveManager::onWaitingForNodeRemovalChanged(const QUuid &networkUuid, bool waitingForNodeRemoval)
{
    ZWaveNetwork *network = m_networks.value(networkUuid);
    if (!network) {
        qCWarning(dcZWave()) << "Received a network waiting for node addition changed signal for a network we don't know:" << networkUuid.toString();
        return;
    }
    network->setWaitingForNodeRemoval(waitingForNodeRemoval);
    emit networkChanged(network);
}

void ZWaveManager::onNodeAdded(const QUuid &networkUuid, quint8 nodeId)
{
    ZWaveNetwork *network = m_networks.value(networkUuid);
    if (!network) {
        qCWarning(dcZWave()) << "Received a node added signal for a network we don't know:" << networkUuid.toString();
        return;
    }
    ZWaveNodeImplementation *node = qobject_cast<ZWaveNodeImplementation*>(network->node(nodeId));
    if (node) {
        qCDebug(dcZWave()) << "Received a new node signal for a node we already know:" << nodeId;
        return;
    }

    qCInfo(dcZWave()) << "New node with ID" << nodeId << "joined the network" << network->networkUuid();

    node = new ZWaveNodeImplementation(this, network->networkUuid(), nodeId, this);
    connect(node, &ZWaveNodeImplementation::nodeChanged, this, [this, node](){emit nodeChanged(node);});
    network->addNode(node);

    emit nodeAdded(node);

}

void ZWaveManager::onNodeInitialized(const QUuid &networkUuid, quint8 nodeId)
{
    ZWaveNetwork *network = m_networks.value(networkUuid);
    if (!network) {
        qCWarning(dcZWave()) << "Received a node initialized signal for a network we don't know:" << networkUuid.toString();
        return;
    }

    ZWaveNodeImplementation *node = qobject_cast<ZWaveNodeImplementation*>(network->node(nodeId));
    if (!node) {
        qCWarning(dcZWave()) << "Received a node initialized signal for a node we don't know:" << nodeId;
        return;
    }
    node->setReachable(true);

    m_dbs.value(network->networkUuid())->storeNode(node);


    if (!node->initialized()) {
        node->setInitialized(true);
        emit nodeInitialized(node);
    }

    qCInfo(dcZWave()) << "Node initialized:" << node->nodeId();
    emit nodeChanged(node);
}

void ZWaveManager::onNodeDataChanged(const QUuid &networkUuid, quint8 nodeId)
{
    ZWaveNetwork *network = m_networks.value(networkUuid);
    if (!network) {
        qCWarning(dcZWave()) << "Received a node names changed signal for a network we don't know:" << networkUuid.toString();
        return;
    }

    ZWaveNodeImplementation *node = qobject_cast<ZWaveNodeImplementation*>(network->node(nodeId));
    if (!node) {
        qCWarning(dcZWave()) << "Received a node names changed signal for a node we don't know:" << nodeId;
        return;
    }

    qCDebug(dcZWave()) << "Node names changed for node" << nodeId << "in network" << network->networkUuid();
    node->blockSignals(true);
    node->setName(m_backend->nodeName(networkUuid, nodeId));
    node->setNodeType(m_backend->nodeType(networkUuid, nodeId));
    node->setRole(m_backend->nodeRole(networkUuid, nodeId));
    node->setDeviceType(m_backend->nodeDeviceType(networkUuid, nodeId));
    node->setManufacturerId(m_backend->nodeManufacturerId(networkUuid, nodeId));
    node->setManufacturerName(m_backend->nodeManufacturerName(networkUuid, nodeId));
    node->setProductId(m_backend->nodeProductId(networkUuid, nodeId));
    node->setProductName(m_backend->nodeProductName(networkUuid, nodeId));
    node->setProductType(m_backend->nodeProductType(networkUuid, nodeId));
    node->setVersion(m_backend->nodeVersion(networkUuid, nodeId));
    node->setIsZWavePlusDevice(m_backend->nodeIsZWavePlus(networkUuid, nodeId));
    node->setIsSecurityDevice(m_backend->nodeIsSecureDevice(networkUuid, nodeId));
    node->setSecurityMode(m_backend->nodeSecurityMode(networkUuid, nodeId));
    node->setIsBeamingDevice(m_backend->nodeIsBeamingDevice(networkUuid, nodeId));
    node->setPlusDeviceType(m_backend->nodePlusDeviceType(networkUuid, nodeId));
    node->blockSignals(false);
    emit nodeChanged(node);

    m_dbs.value(network->networkUuid())->storeNode(node);

    if (node->nodeId() == network->controllerNodeId()) {
        network->setIsZWavePlus(m_backend->nodeIsZWavePlus(networkUuid, network->controllerNodeId()));
        emit networkChanged(network);
    }
}

void ZWaveManager::onNodeReachableStatus(const QUuid &networkUuid, quint8 nodeId, bool reachable)
{
    ZWaveNetwork *network = m_networks.value(networkUuid);
    if (!network) {
        qCWarning(dcZWave()) << "Received a node reachable changed signal for a network we don't know:" << networkUuid.toString();
        return;
    }

    ZWaveNodeImplementation *node = qobject_cast<ZWaveNodeImplementation*>(network->node(nodeId));
    if (!node) {
        qCWarning(dcZWave()) << "Received a node reachable status signal for a node we don't know:" << nodeId;
        return;
    }
    qCInfo(dcZWave()) << "Node" << nodeId << "in network" << network->networkUuid().toString() << "is" << (reachable ? "reachable" : "not reachable");
    node->setReachable(reachable);
}

void ZWaveManager::onNodeFailedStatus(const QUuid &networkUuid, quint8 nodeId, bool failed)
{
    ZWaveNetwork *network = m_networks.value(networkUuid);
    if (!network) {
        qCWarning(dcZWave()) << "Received a node failed status signal for a network we don't know:" << networkUuid.toString();
        return;
    }

    ZWaveNodeImplementation *node = qobject_cast<ZWaveNodeImplementation*>(network->node(nodeId));
    if (!node) {
        qCWarning(dcZWave()) << "Received a node reachable changed signal for a node we don't know:" << nodeId;
        return;
    }
    qCInfo(dcZWave()) << "Node" << nodeId << "in network" << network->networkUuid().toString() << "is" << (failed ? "failed" : "ok");
    node->setFailed(failed);
}

void ZWaveManager::onNodeSleepStatus(const QUuid &networkUuid, quint8 nodeId, bool sleeping)
{
    ZWaveNetwork *network = m_networks.value(networkUuid);
    if (!network) {
        qCWarning(dcZWave()) << "Received a node sleep status signal for a network we don't know:" << networkUuid.toString();
        return;
    }

    ZWaveNodeImplementation *node = qobject_cast<ZWaveNodeImplementation*>(network->node(nodeId));
    if (!node) {
        qCWarning(dcZWave()) << "Received a node sleep signal for a node we don't know:" << nodeId;
        return;
    }
    qCInfo(dcZWave()) << "Node" << nodeId << "in network" << network->networkUuid().toString() << "is" << (sleeping ? "sleeping" : "awake");
    node->setSleeping(sleeping);
}

void ZWaveManager::onNodeLinkQualityStatus(const QUuid &networkUuid, quint8 nodeId, quint8 linkQuality)
{
    ZWaveNetwork *network = m_networks.value(networkUuid);
    if (!network) {
        qCWarning(dcZWave()) << "Received a node link qlility status signal for a network we don't know:" << networkUuid.toString();
        return;
    }

    ZWaveNodeImplementation *node = qobject_cast<ZWaveNodeImplementation*>(network->node(nodeId));
    if (!node) {
        qCWarning(dcZWave()) << "Received a node link quality signal for a node we don't know:" << nodeId;
        return;
    }
    qCInfo(dcZWave()) << "Link quality for node" << nodeId << "in network" << network->networkUuid().toString() << "is" << linkQuality;
    node->setLinkQuality(linkQuality);
}

void ZWaveManager::onNodeRemoved(const QUuid &networkUuid, quint8 nodeId)
{
    ZWaveNetwork *network = m_networks.value(networkUuid);
    if (!network) {
        qCWarning(dcZWave()) << "Received a node removed signal for a network we don't know:" << networkUuid.toString();
        return;
    }

    ZWaveNode *node = network->node(nodeId);
    if (!node) {
        qCWarning(dcZWave()) << "Received a node removed signal for a node we don't know:" << nodeId;
        return;
    }
    qCInfo(dcZWave()) << "Node" << nodeId << "removed from network" << network->networkUuid();
    network->removeNode(nodeId);
    m_dbs.value(network->networkUuid())->removeNode(nodeId);
    emit nodeRemoved(node);
}

void ZWaveManager::onValueAdded(const QUuid &networkUuid, quint8 nodeId, const ZWaveValue &value)
{
    ZWaveNetwork *network = m_networks.value(networkUuid);
    if (!network) {
        qCWarning(dcZWave) << "Received a value added signal for a network we don't know:" << networkUuid.toString();
        return;
    }

    ZWaveNodeImplementation *node = qobject_cast<ZWaveNodeImplementation*>(network->node(nodeId));
    if (!node) {
        qCWarning(dcZWave) << "Received a value added signal for a node we don't know:" << nodeId;
        return;
    }

    qCDebug(dcZWave()) << "Value added to node" << nodeId << value;
    node->updateValue(value);
    m_dbs.value(network->networkUuid())->storeValue(node, value.id());
}

void ZWaveManager::onValueChanged(const QUuid &networkUuid, quint8 nodeId, const ZWaveValue &value)
{
    ZWaveNetwork *network = m_networks.value(networkUuid);
    if (!network) {
        qCWarning(dcZWave) << "Received a value changed signal for a network we don't know:" << networkUuid.toString();
        return;
    }

    ZWaveNodeImplementation *node = qobject_cast<ZWaveNodeImplementation*>(network->node(nodeId));
    if (!node) {
        qCWarning(dcZWave) << "Received a value changed signal for a node we don't know:" << nodeId;
        return;
    }

    qCDebug(dcZWave) << "Value changed for node" << node->nodeId() << value;
    node->updateValue(value);
//    node->setReachable(true);
    m_dbs.value(network->networkUuid())->storeValue(node, value.id());
}

void ZWaveManager::onValueRemoved(const QUuid &networkUuid, quint8 nodeId, quint64 valueId)
{
    ZWaveNetwork *network = m_networks.value(networkUuid);
    if (!network) {
        qCWarning(dcZWave()) << "Received a value removed signal for a network we don't know:" << networkUuid.toString();
        return;
    }
    ZWaveNodeImplementation *node = qobject_cast<ZWaveNodeImplementation*>(network->node(nodeId));
    if (!node) {
        qCWarning(dcZWave()) << "Received a value removed signal for a node we don't know:" << nodeId;
        return;
    }
    node->removeValue(valueId);
    m_dbs.value(network->networkUuid())->removeValue(node, valueId);
}


ZWaveNetworks ZWaveManager::networks() const
{
    return m_networks.values();
}

ZWaveNetwork *ZWaveManager::network(const QUuid &networkUuid) const
{
    return m_networks.value(networkUuid);
}

QPair<ZWave::ZWaveError, QUuid> ZWaveManager::createNetwork(const QString &serialPort)
{
    if (!available()) {
        qCWarning(dcZWave()) << "Z-Wave is not available.";
        return QPair<ZWave::ZWaveError, QUuid>(ZWave::ZWaveErrorBackendError, QUuid());
    }

    QString networkKey = QUuid::createUuid().toString().remove(QRegularExpression("[{\\-}]*"));
    ZWaveNetwork *network = new ZWaveNetwork(QUuid::createUuid(), serialPort, networkKey, this);
    bool success = loadNetwork(network);
    if (!success) {
        delete network;
        return QPair<ZWave::ZWaveError, QUuid>(ZWave::ZWaveErrorInUse, QUuid());
    }
    emit networkAdded(network);

    storeNetwork(network);

    return QPair<ZWave::ZWaveError, QUuid>(ZWave::ZWaveErrorNoError, network->networkUuid());
}

ZWave::ZWaveError ZWaveManager::removeNetwork(const QUuid &networkUuid)
{
    ZWaveNetwork *network = m_networks.value(networkUuid);
    if (!network) {
        return ZWave::ZWaveErrorNetworkUuidNotFound;
    }
    bool status = m_backend->stopNetwork(networkUuid);
    if (!status) {
        return ZWave::ZWaveErrorBackendError;
    }

    foreach (ZWaveNode *node, network->nodes()) {
        network->removeNode(node->nodeId());
        emit nodeRemoved(node);
    }

    m_networks.remove(network->networkUuid());
    ZWaveDeviceDatabase *db = m_dbs.take(network->networkUuid());
    db->removeDB();
    delete db;
    network->deleteLater();
    NymeaSettings settings(NymeaSettings::SettingsRoleZWave);
    settings.beginGroup("Networks");
    settings.remove(network->networkUuid().toString());
    settings.endGroup();
    emit networkRemoved(network->networkUuid());

    return ZWave::ZWaveErrorNoError;
}

ZWave::ZWaveError ZWaveManager::factoryResetNetwork(const QUuid &networkUuid)
{
    if (!m_networks.contains(networkUuid)) {
        return ZWave::ZWaveErrorNetworkUuidNotFound;
    }
    qCInfo(dcZWave()) << "Resetting controller for network:" << networkUuid.toString();

    ZWaveNetwork *network = m_networks.value(networkUuid);

    foreach (ZWaveNode *node, network->nodes()) {
        network->removeNode(node->nodeId());
        emit nodeRemoved(node);
    }

    m_backend->factoryResetNetwork(networkUuid);
    ZWaveDeviceDatabase *db = m_dbs.value(network->networkUuid());
    db->clearDB();
    network->setHomeId(0);
    emit networkChanged(network);
    network->setNetworkState(ZWaveNetwork::ZWaveNetworkStateStarting);
    emit networkStateChanged(network);
    storeNetwork(network);

    qCInfo(dcZWave()) << "Controller reset succeeded";
    return ZWave::ZWaveErrorNoError;
}


ZWaveReply *ZWaveManager::addNode(const QUuid &networkUuid)
{
    ZWaveNetwork *network = m_networks.value(networkUuid);
    ZWaveManagerReply *reply = new ZWaveManagerReply(this);
    if (!network) {
        reply->finish(ZWave::ZWaveErrorNetworkUuidNotFound);
        return reply;
    }
    ZWaveReply *backendReply = m_backend->addNode(network->networkUuid(), true);
    connect(backendReply, &ZWaveReply::finished, reply, &ZWaveReply::finished);
    qCDebug(dcZWave) << "Adding node to network" << networkUuid.toString();
    return reply;
}

ZWaveReply* ZWaveManager::removeNode(const QUuid &networkUuid)
{
    ZWaveManagerReply *reply = new ZWaveManagerReply(this);
    ZWaveNetwork *network = m_networks.value(networkUuid);
    if (!network) {
        reply->finish(ZWave::ZWaveErrorNetworkUuidNotFound);
        return reply;
    }
    ZWaveReply *backendReply = m_backend->removeNode(networkUuid);
    connect(backendReply, &ZWaveReply::finished, reply, &ZWaveManagerReply::finish);
    return reply;
}

ZWaveReply* ZWaveManager::removeFailedNode(const QUuid &networkUuid, quint8 nodeId)
{
    ZWaveManagerReply *reply = new ZWaveManagerReply(this);
    ZWaveNetwork *network = m_networks.value(networkUuid);
    if (!network) {
        reply->finish(ZWave::ZWaveErrorNetworkUuidNotFound);
        return reply;
    }
    ZWaveNode *node = network->node(nodeId);
    if (!node) {
        reply->finish(ZWave::ZWaveErrorNodeIdNotFound);
        return reply;
    }

    ZWaveReply *backendReply = m_backend->removeFailedNode(networkUuid, nodeId);
    connect(backendReply, &ZWaveReply::finished, reply, &ZWaveManagerReply::finish);
    return reply;
}

ZWaveReply* ZWaveManager::cancelPendingOperation(const QUuid &networkUuid)
{
    ZWaveManagerReply *reply = new ZWaveManagerReply(this);
    ZWaveNetwork *network = m_networks.value(networkUuid);
    if (!network) {
        reply->finish(ZWave::ZWaveErrorNetworkUuidNotFound);
        return reply;
    }

    ZWaveReply *backendReply = m_backend->cancelPendingOperation(networkUuid);
    connect(backendReply, &ZWaveReply::finished, reply, &ZWaveManagerReply::finish);
    return reply;
}

void ZWaveManager::setValue(const QUuid &networkUuid, quint8 nodeId, const ZWaveValue &value)
{
    ZWaveNetwork *network = m_networks.value(networkUuid);
    if (!network) {
        qCWarning(dcZWave()) << "No network with UUID" << networkUuid;
        return;
    }

    qCDebug(dcZWave) << "Setting value" << value.id() << "on node" << nodeId;
    m_backend->setValue(network->networkUuid(), nodeId, value);

}

bool ZWaveManager::loadBackend()
{
    QStringList searchDirs;
    QByteArray envPath = qgetenv("NYMEA_ZWAVE_PLUGIN_PATH");
    if (!envPath.isEmpty()) {
        searchDirs << QString(envPath).split(':');
    }
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    foreach (QString libraryPath, QCoreApplication::libraryPaths()) {
        searchDirs << libraryPath.replace("qt5", "nymea").replace("plugins", "zwave");
    }
#else
    foreach (QString libraryPath, QCoreApplication::libraryPaths()) {
        searchDirs << libraryPath.replace("qt6", "nymea").replace("plugins", "zwave");
    }
#endif

    foreach (QString libraryPath, QCoreApplication::libraryPaths()) {
        searchDirs << libraryPath.replace("plugins", "nymea/zwave");
    }
    searchDirs << QCoreApplication::applicationDirPath() + "/../lib/nymea/zwave/";
    searchDirs << QCoreApplication::applicationDirPath() + "/../zwave/";
    searchDirs << QCoreApplication::applicationDirPath() + "/../../../zwave/";

    foreach (const QString &path, searchDirs) {
        QDir dir(path);
        qCDebug(dcZWave) << "Loading Z-Wave backend from:" << dir.absolutePath();
        foreach (const QString &entry, dir.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot)) {
            QFileInfo fi(path + "/" + entry);
            if (fi.isFile()) {
                if (entry.startsWith("libnymea_zwaveplugin") && entry.endsWith(".so")) {
                    QPluginLoader loader;
                    loader.setFileName(path + "/" + entry);
                    loader.setLoadHints(QLibrary::ResolveAllSymbolsHint);
                    if (!loader.load()) {
                        qCWarning(dcZWave) << loader.errorString();
                        continue;
                    }
                    m_backend = qobject_cast<ZWaveBackend*>(loader.instance());
                    if (!m_backend) {
                        qCWarning(dcZWave) << "Could not get plugin instance of" << loader.fileName();
                        loader.unload();
                        continue;
                    }
                    qCDebug(dcZWave()) << "Loaded Z-Wave backend:" << loader.fileName();
                    m_backend->setParent(this);
                    return true;
                }
            }
        }
    }
    return false;
}

}
