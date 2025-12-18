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

#include "zwavehardwareresourceimplementation.h"

#include "zwave/zwavemanager.h"
#include "zwave/zwavenetwork.h"

#include "loggingcategories.h"
Q_DECLARE_LOGGING_CATEGORY(dcZWave)

namespace nymeaserver {

ZWaveHardwareResourceImplementation::ZWaveHardwareResourceImplementation(ZWaveManager *zwaveManager, QObject *parent)
    : ZWaveHardwareResource(parent)
    , m_zwaveManager(zwaveManager)
{
    connect(m_zwaveManager, &ZWaveManager::networkStateChanged, this, &ZWaveHardwareResourceImplementation::onNetworkStateChanged);
    connect(m_zwaveManager, &ZWaveManager::nodeInitialized, this, &ZWaveHardwareResourceImplementation::onNodeInitialized);
    connect(m_zwaveManager, &ZWaveManager::nodeRemoved, this, &ZWaveHardwareResourceImplementation::onNodeRemoved);
    //    connect(m_zigbeeManager, &ZigbeeManager::availableChanged, this, &ZigbeeHardwareResourceImplementation::onZigbeeAvailableChanged);
}

bool ZWaveHardwareResourceImplementation::available() const
{
    return m_zwaveManager->available();
}

bool ZWaveHardwareResourceImplementation::enabled() const
{
    return m_zwaveManager->enabled();
}

void ZWaveHardwareResourceImplementation::setEnabled(bool enabled)
{
    m_zwaveManager->setEnabled(enabled);
}

void ZWaveHardwareResourceImplementation::registerHandler(ZWaveHandler *handler, HandlerType type)
{
    qCDebug(dcZWave()) << "Registering new Z-Wave handler" << handler->name() << "with type" << type;
    m_handlers.insert(type, handler);
}

ZWaveNode *ZWaveHardwareResourceImplementation::claimNode(ZWaveHandler *handler, const QUuid &networkUuid, quint8 nodeId)
{
    if (!m_handlers.values().contains(handler)) {
        qCWarning(dcZWave()) << "Handler" << handler->name() << "is not registered. Not allowing node to be claimed.";
        return nullptr;
    }

    ZWaveNetwork *network = m_zwaveManager->network(networkUuid);
    if (!network) {
        qCWarning(dcZWave()) << "Network" << networkUuid << "not found.";
        return nullptr;
    }

    ZWaveNode *node = network->node(nodeId);
    if (!node) {
        qCWarning(dcZWave()) << "Node with ID" << nodeId << "not found in ZWave network" << networkUuid.toString();
        return nullptr;
    }

    if (m_nodeHandlers.contains(node) && m_nodeHandlers.value(node) != handler) {
        qCWarning(dcZWave()) << "Node with ID" << nodeId << "is already claimed by another handler (" << m_nodeHandlers.value(node)->name()
                             << "). Not allowing node to be reclaimed.";
        return nullptr;
    }

    m_nodeHandlers[node] = handler;
    return node;
}

void ZWaveHardwareResourceImplementation::thingsLoaded()
{
    m_thingsLoaded = true;
    qCDebug(dcZWave) << "Things loaded. Checking for unhandled nodes...";

    // We can assume here that all handled nodes have been claimed by plugins
    // In case we started up and loaded new Z-Wave plugins, let's try to get all previously joined nodes handled now...
    foreach (ZWaveNetwork *network, m_zwaveManager->networks()) {
        foreach (ZWaveNode *node, network->nodes()) {
            // Ignore the controller node
            if (node->nodeId() == network->controllerNodeId())
                continue;

            if (!m_nodeHandlers.contains(node)) {
                qCDebug(dcZWave()) << "Node" << node << "is not yet handled by any plugin. Trying to find a suitable plugin.";
                handleNewNode(node);
            }
        }
    }
}

void ZWaveHardwareResourceImplementation::onNetworkStateChanged(ZWaveNetwork *network)
{
    // If the network is now ready and things have been loaded already, check if there are
    // unclaimed nodes that might be handled now. This might happen if a node joins the network
    // but no appropriate plugin had been installed at the time. If additional plugins have
    // been installed now, such nodes might be handled by them now.
    if (network->networkState() == ZWaveNetwork::ZWaveNetworkStateOnline && m_thingsLoaded) {
        foreach (ZWaveNode *node, network->nodes()) {
            // Ignore the controller node
            if (node->nodeId() == network->controllerNodeId())
                continue;

            if (!m_nodeHandlers.contains(node)) {
                handleNewNode(node);
            }
        }
    }
}

void ZWaveHardwareResourceImplementation::onNodeInitialized(ZWaveNode *node)
{
    if (!m_thingsLoaded) {
        return;
    }
    handleNewNode(node);
}

void ZWaveHardwareResourceImplementation::onNodeRemoved(ZWaveNode *node)
{
    qCDebug(dcZWave()) << "Node removed from the network";
    ZWaveHandler *handler = m_nodeHandlers.value(node);
    if (handler) {
        handler->handleRemoveNode(node);
    }
}

void ZWaveHardwareResourceImplementation::handleNewNode(ZWaveNode *node)
{
    ZWaveNetwork *network = m_zwaveManager->network(node->networkUuid());
    if (node->nodeId() == network->controllerNodeId()) {
        // Not forwarding the controller node to plugins...
        return;
    }
    qCDebug(dcZWave()) << "Node" << node->nodeId() << "added to the network:" << node->networkUuid().toString();
    ZWaveHandler *handler = nullptr;
    foreach (ZWaveHandler *tmp, m_handlers) {
        if (tmp->handleNode(node)) {
            handler = tmp;
            m_nodeHandlers.insert(node, handler);
            qCDebug(dcZWave()) << "Node" << node->nodeId() << "taken by handler" << handler->name();
            break;
        }
    }
    if (!handler) {
        qCInfo(dcZWave()) << "No Z-Wave handler available to handle node" << node;
        return;
    }
}

} // namespace nymeaserver
