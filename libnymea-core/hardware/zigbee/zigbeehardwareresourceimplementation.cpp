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

ZigbeeNode *ZigbeeHardwareResourceImplementation::getNode(const QUuid &networkUuid, const ZigbeeAddress &extendedAddress)
{
    ZigbeeNetwork *network = m_zigbeeManager->zigbeeNetworks().value(networkUuid);
    if (!network) {
        qCWarning(dcZigbeeResource()) << "Network" << networkUuid << "not found.";
        return nullptr;
    }
    return network->getZigbeeNode(extendedAddress);
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

void ZigbeeHardwareResourceImplementation::onZigbeeAvailableChanged(bool available)
{
    if (available) {
        qCDebug(dcZigbeeResource()) << "Zigbee is now available";
    } else {
        qCWarning(dcZigbeeResource()) << "Zigbee is not available any more";
    }

    emit availableChanged(available);
}

void ZigbeeHardwareResourceImplementation::onZigbeeNodeAdded(const QUuid &networkUuid, ZigbeeNode *node)
{
    qCDebug(dcZigbeeResource()) << node << "joined the network" << m_zigbeeManager->zigbeeNetworks().value(networkUuid);
    ZigbeeHandler *handler = nullptr;
    foreach (ZigbeeHandler *tmp, m_handlers) {
        if (tmp->handleNode(node, networkUuid)) {
            handler = tmp;
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
}

}
