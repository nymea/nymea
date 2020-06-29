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

#include <QDir>
#include <QFileInfo>

#include "nymea-zigbee/zigbeenetworkmanager.h"

namespace nymeaserver {

ZigbeeHardwareResourceImplementation::ZigbeeHardwareResourceImplementation(QObject *parent) :
    ZigbeeHardwareResource(parent)
{

}

bool ZigbeeHardwareResourceImplementation::available() const
{
    return m_available;
}

bool ZigbeeHardwareResourceImplementation::enabled() const
{
    return m_enabled;
}

void ZigbeeHardwareResourceImplementation::setZigbeeNetwork(ZigbeeNetwork *network)
{
    // Clean up
    if (m_zigbeeNetwork) {
        disconnect(m_zigbeeNetwork, &ZigbeeNetwork::stateChanged, this, &ZigbeeHardwareResourceImplementation::onZigbeeNetworkStateChanged);
    }

    // Set new network
    m_zigbeeNetwork = network;
    connect(m_zigbeeNetwork, &ZigbeeNetwork::stateChanged, this, &ZigbeeHardwareResourceImplementation::onZigbeeNetworkStateChanged);
}

void ZigbeeHardwareResourceImplementation::setEnabled(bool enabled)
{
    qCDebug(dcZigbeeHardwareResource()) << "Set" << (enabled ? "enabled" : "disabled");
    if (m_enabled && enabled) {
        qCDebug(dcZigbeeHardwareResource()) << "Already enabled.";
        return;
    } else if (!m_enabled && !enabled) {
        qCDebug(dcZigbeeHardwareResource()) << "Already disabled.";
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

void ZigbeeHardwareResourceImplementation::onZigbeeNetworkStateChanged(ZigbeeNetwork::State state)
{
    qCDebug(dcZigbeeHardwareResource()) << "Network state changed" << state;
}

bool ZigbeeHardwareResourceImplementation::enable()
{
    qCDebug(dcZigbeeHardwareResource()) << "Enable hardware resource";

    if (!m_zigbeeNetwork) {
        qCDebug(dcZigbeeHardwareResource()) << "There is no zigbee network configured as hardware resource";
    } else {
        // TODO: start network
    }

    return true;
}

bool ZigbeeHardwareResourceImplementation::disable()
{
    qCDebug(dcZigbeeHardwareResource()) << "Disable hardware resource";
    if (!m_zigbeeNetwork) {
        qCDebug(dcZigbeeHardwareResource()) << "There is no zigbee network configured as hardware resource";
    }

    // TODO: stop network
    return true;
}

}
