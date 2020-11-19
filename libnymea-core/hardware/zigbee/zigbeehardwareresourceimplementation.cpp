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
* GNU General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU General Public License as published by the Free Software
* Foundation, GNU version 3. This project is distributed in the hope that it
* will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
* of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
* Public License for more details.
*
* You should have received a copy of the GNU General Public License along with
* this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "zigbeehardwareresourceimplementation.h"
#include "loggingcategories.h"
#include "nymeasettings.h"
#include "hardware/zigbee/zigbeehandler.h"

#include <QDir>
#include <QFileInfo>

#include <zigbeenetworkmanager.h>

NYMEA_LOGGING_CATEGORY(dcZigbeeResource, "ZigbeeResource")

namespace nymeaserver {

ZigbeeHardwareResourceImplementation::ZigbeeHardwareResourceImplementation(ZigbeeManager *zigbeeManager, QObject *parent) :
    ZigbeeHardwareResource(parent),
    m_zigbeeManager(zigbeeManager)
{
    connect(m_zigbeeManager, &ZigbeeManager::zigbeeNetworkChanged, this, &ZigbeeHardwareResourceImplementation::onZigbeeNetworkChanged);
    connect(m_zigbeeManager, &ZigbeeManager::nodeAdded, this, &ZigbeeHardwareResourceImplementation::onZigbeeNodeAdded);
    connect(m_zigbeeManager, &ZigbeeManager::nodeRemoved, this, &ZigbeeHardwareResourceImplementation::onZigbeeNodeRemoved);
    connect(m_zigbeeManager, &ZigbeeManager::availableChanged, this, &ZigbeeHardwareResourceImplementation::onZigbeeAvailableChanged);
}

bool ZigbeeHardwareResourceImplementation::available() const
{
    return m_zigbeeManager->available();
}

bool ZigbeeHardwareResourceImplementation::enabled() const
{
    return m_enabled;
}


void ZigbeeHardwareResourceImplementation::registerHandler(ZigbeeHandler *handler, HandlerType type)
{
    qCDebug(dcZigbeeResource()) << "Registering new zigbee handler" << handler->name() << "with type" << type;
    m_handlers.insert(type, handler);
}

ZigbeeNode *ZigbeeHardwareResourceImplementation::claimNode(ZigbeeHandler *handler, const QUuid &networkUuid, const ZigbeeAddress &extendedAddress)
{
    if (!m_handlers.values().contains(handler)) {
        qCWarning(dcZigbeeResource()) << "Handler" << handler->name() << "is not registered. Not allowing node to be claimed.";
        return nullptr;
    }

    ZigbeeNetwork *network = m_zigbeeManager->zigbeeNetworks().value(networkUuid);
    if (!network) {
        qCWarning(dcZigbeeResource()) << "Network" << networkUuid << "not found.";
        return nullptr;
    }

    ZigbeeNode *node = network->getZigbeeNode(extendedAddress);
    if (!node) {
        qCWarning(dcZigbeeResource()) << "Node with address" << extendedAddress << "not found in Zigbee network" << networkUuid.toString();
        return nullptr;
    }

    if (m_nodeHandlers.contains(node) && m_nodeHandlers.value(node) != handler) {
        qCWarning(dcZigbeeResource()) << "Node with address" << extendedAddress << "is already claimed by another handler (" << m_nodeHandlers.value(node)->name() << "). Not allowing node to be reclaimed.";
        return nullptr;
    }

    m_nodeHandlers[node] = handler;
    return node;
}

void ZigbeeHardwareResourceImplementation::removeNodeFromNetwork(const QUuid &networkUuid, ZigbeeNode *node)
{
    ZigbeeNetwork *network = m_zigbeeManager->zigbeeNetworks().value(networkUuid);
    if (!network) {
        qCWarning(dcZigbeeResource()) << "Can not remove note from network" << networkUuid << "because there is no network with this uuid.";
        return;
    }

    network->removeZigbeeNode(node->extendedAddress());
}

ZigbeeNetwork::State ZigbeeHardwareResourceImplementation::networkState(const QUuid &networkUuid)
{
    ZigbeeNetwork *network = m_zigbeeManager->zigbeeNetworks().value(networkUuid);
    if (!network) {
        qCWarning(dcZigbeeResource()) << "Network" << networkUuid << "not found.";
        return ZigbeeNetwork::StateUninitialized;
    }
    return network->state();
}

void ZigbeeHardwareResourceImplementation::setEnabled(bool enabled)
{
    qCDebug(dcZigbeeResource()) << "Set" << (enabled ? "enabled" : "disabled");
    if (m_enabled && enabled) {
        qCDebug(dcZigbeeResource()) << "Already enabled.";
        return;
    } else if (!m_enabled && !enabled) {
        qCDebug(dcZigbeeResource()) << "Already disabled.";
        return;
    }

    bool success = false;
    if (enabled) {
        success = enable();
    } else {
        success = disable();
    }

    if (success) {
        m_enabled = enabled;
        emit enabledChanged(m_enabled);
    }
}

bool ZigbeeHardwareResourceImplementation::enable()
{
    qCDebug(dcZigbeeResource()) << "Enable hardware resource. Not implemented yet.";

    // TODO: enable all networks in the zigbee manager

    return true;
}

bool ZigbeeHardwareResourceImplementation::disable()
{
    qCDebug(dcZigbeeResource()) << "Disable hardware resource. Not implemented yet.";

    // TODO: disable all networks in the zigbee manager

    return true;
}

void ZigbeeHardwareResourceImplementation::thingsLoaded()
{
    m_thingsLoaded = true;

    // We can assume here that all handled nodes have been claimed by plugins
    // In case we started up and loaded new zigbee plugins, let's try to get all previously joined nodes handled now...
    foreach (ZigbeeNetwork *network, m_zigbeeManager->zigbeeNetworks()) {
        if (network->state() == ZigbeeNetwork::StateRunning) {
            foreach (ZigbeeNode *node, network->nodes()) {
                // Ignore the coordinator node
                if (node->shortAddress() == 0x0000)
                    continue;

                if (!m_nodeHandlers.contains(node)) {
                    qCDebug(dcZigbeeResource()) << "Node" << node << "is not yet handled by any plugin. Trying to find a suitable plugin.";
                    onZigbeeNodeAdded(network->networkUuid(), node);
                }
            }
        }
    }
}

void ZigbeeHardwareResourceImplementation::onZigbeeAvailableChanged(bool available)
{
    if (available) {
        qCDebug(dcZigbeeResource()) << "Zigbee is now available";
    } else {
        qCWarning(dcZigbeeResource()) << "Zigbee is not available any more";
    }

    emit availableChanged(available);
}

void ZigbeeHardwareResourceImplementation::onZigbeeNetworkChanged(ZigbeeNetwork *network)
{
    emit networkStateChanged(network->networkUuid(), network->state());

    // If the network is now ready and things have been loaded already, check if there are
    // unclaimed nodes that might be handled now. This might happen if a node joins the network
    // but no appropriate plugin had been installed at the time. If additional plugins have
    // been installed now, such nodes might be handled by them now.
    if (network->state() == ZigbeeNetwork::StateRunning && m_thingsLoaded) {
        foreach (ZigbeeNode *node, network->nodes()) {
            // Ignore the coordinator node
            if (node->shortAddress() == 0x0000)
                continue;

            if (!m_nodeHandlers.contains(node)) {
                onZigbeeNodeAdded(network->networkUuid(), node);
            }
        }
    }
}

void ZigbeeHardwareResourceImplementation::onZigbeeNodeAdded(const QUuid &networkUuid, ZigbeeNode *node)
{
    qCDebug(dcZigbeeResource()) << node << "joined the network" << m_zigbeeManager->zigbeeNetworks().value(networkUuid);
    ZigbeeHandler *handler = nullptr;
    foreach (ZigbeeHandler *tmp, m_handlers) {
        if (tmp->handleNode(node, networkUuid)) {
            handler = tmp;
            m_nodeHandlers.insert(node, handler);
            qCDebug(dcZigbeeResource()) << "Node" << node << "taken by handler" << handler->name();
            break;
        }
    }
    if (!handler) {
        qCWarning(dcZigbeeResource()) << "No zigbee handler available to handle node" << node;
        return;
    }
}

void ZigbeeHardwareResourceImplementation::onZigbeeNodeRemoved(const QUuid &networkUuid, ZigbeeNode *node)
{
    qCDebug(dcZigbeeResource()) << node << "left the network" << m_zigbeeManager->zigbeeNetworks().value(networkUuid);

    ZigbeeHandler *handler = m_nodeHandlers.value(node);
    if (handler) {
        handler->handleRemoveNode(node, networkUuid);
    }
}

}
