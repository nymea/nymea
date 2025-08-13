/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2021, nymea GmbH
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

#include "modbusrtuhardwareresourceimplementation.h"
#include "loggingcategories.h"
#include "hardware/modbus/modbusrtumanager.h"

NYMEA_LOGGING_CATEGORY(dcModbusRtuResource, "ModbusRtuResource")

namespace nymeaserver {

ModbusRtuHardwareResourceImplementation::ModbusRtuHardwareResourceImplementation(ModbusRtuManager *modbusRtuManager, QObject *parent) :
    ModbusRtuHardwareResource(parent),
    m_modbusRtuManager(modbusRtuManager)
{
    connect(m_modbusRtuManager, &ModbusRtuManager::modbusRtuMasterAdded, this, [=](ModbusRtuMaster *modbusRtuMaster){
        emit modbusRtuMasterAdded(modbusRtuMaster->modbusUuid());
    });

    connect(m_modbusRtuManager, &ModbusRtuManager::modbusRtuMasterRemoved, this, [=](ModbusRtuMaster *modbusRtuMaster){
        emit modbusRtuMasterRemoved(modbusRtuMaster->modbusUuid());
    });

    connect(m_modbusRtuManager, &ModbusRtuManager::modbusRtuMasterChanged, this, [=](ModbusRtuMaster *modbusRtuMaster){
        emit modbusRtuMasterChanged(modbusRtuMaster->modbusUuid());
    });
}

QList<ModbusRtuMaster *> ModbusRtuHardwareResourceImplementation::modbusRtuMasters() const
{
    return m_modbusRtuManager->modbusRtuMasters();
}

bool ModbusRtuHardwareResourceImplementation::hasModbusRtuMaster(const QUuid &modbusUuid) const
{
    return m_modbusRtuManager->hasModbusRtuMaster(modbusUuid);
}

ModbusRtuMaster *ModbusRtuHardwareResourceImplementation::getModbusRtuMaster(const QUuid &modbusUuid) const
{
    return m_modbusRtuManager->getModbusRtuMaster(modbusUuid);
}

bool ModbusRtuHardwareResourceImplementation::available() const
{
    return m_modbusRtuManager->supported();
}

bool ModbusRtuHardwareResourceImplementation::enabled() const
{
    return m_enabled;
}

bool ModbusRtuHardwareResourceImplementation::enable()
{
    qCWarning(dcModbusRtuResource()) << "Enable hardware resource. Not implemented yet.";

    // TODO: enable all modbus clients

    return true;
}

bool ModbusRtuHardwareResourceImplementation::disable()
{
    qCWarning(dcModbusRtuResource()) << "Disable hardware resource. Not implemented yet.";

    // TODO: disable all modbus clients

    return true;
}

void ModbusRtuHardwareResourceImplementation::setEnabled(bool enabled)
{
    qCDebug(dcModbusRtuResource()) << "Set" << (enabled ? "enabled" : "disabled");
    if (m_enabled && enabled) {
        qCDebug(dcModbusRtuResource()) << "Already enabled.";
        return;
    } else if (!m_enabled && !enabled) {
        qCDebug(dcModbusRtuResource()) << "Already disabled.";
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

}
