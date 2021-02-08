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

#include "modbusrtumanager.h"
#include "nymeasettings.h"
#include "loggingcategories.h"

#include "modbusrtumasterimpl.h"

NYMEA_LOGGING_CATEGORY(dcModbusRtu, "ModbusRtu")

namespace nymeaserver {

ModbusRtuManager::ModbusRtuManager(QObject *parent) : QObject(parent)
{
    // Load uart configurations
    loadRtuMasters();

    // Connect signals

    // Enable autoconnect for each modbus rtu master

}

QList<ModbusRtuMaster *> ModbusRtuManager::modbusRtuMasters() const
{
    return m_modbusRtuMasters.values();
}

bool ModbusRtuManager::hasModbusRtuMaster(const QUuid &modbusUuid) const
{
    return m_modbusRtuMasters.value(modbusUuid) != nullptr;
}

ModbusRtuMaster *ModbusRtuManager::getModbusRtuMaster(const QUuid &modbusUuid)
{
    if (hasModbusRtuMaster(modbusUuid)) {
        return m_modbusRtuMasters.value(modbusUuid);
    }

    return nullptr;
}

void ModbusRtuManager::loadRtuMasters()
{
    NymeaSettings settings(NymeaSettings::SettingsRoleModbusRtu);
    qCDebug(dcModbusRtu()) << "Loading modbus RTU resources from" << settings.fileName();

    settings.beginGroup("ModbusRtuMasters");
    foreach (const QString &uuidString, settings.childGroups()) {
        settings.beginGroup(uuidString);
        QString serialPort = settings.value("serialPort").toString();
        quint32 baudrate = settings.value("baudrate").toUInt();
        QSerialPort::Parity parity = static_cast<QSerialPort::Parity>(settings.value("parity").toInt());
        QSerialPort::DataBits dataBits = static_cast<QSerialPort::DataBits>(settings.value("dataBits").toInt());
        QSerialPort::StopBits stopBits = static_cast<QSerialPort::StopBits>(settings.value("stopBits").toInt());
        settings.endGroup(); // uuid

        ModbusRtuMasterImpl *modbus = new ModbusRtuMasterImpl(QUuid(uuidString), serialPort, baudrate, parity, dataBits, stopBits, this);
        ModbusRtuMaster *modbusRtuMaster = qobject_cast<ModbusRtuMaster *>(modbus);
        qCDebug(dcModbusRtu()) << "Loaded" << modbusRtuMaster;
        m_modbusRtuMasters.insert(modbusRtuMaster->modbusUuid(), modbusRtuMaster);
        emit modbusRtuMasterAdded(modbusRtuMaster->modbusUuid());
    }

    settings.endGroup(); // ModbusRtuMasters
}

void ModbusRtuManager::saveModbusRtuMaster(ModbusRtuMaster *modbusRtuMaster)
{
    NymeaSettings settings(NymeaSettings::SettingsRoleModbusRtu);
    settings.beginGroup("ModbusRtuMasters");
    settings.beginGroup(modbusRtuMaster->modbusUuid().toString());
    settings.setValue("serialPort", modbusRtuMaster->serialPort());
    settings.setValue("baudrate", modbusRtuMaster->baudrate());
    settings.setValue("parity", static_cast<int>(modbusRtuMaster->parity()));
    settings.setValue("dataBits", static_cast<int>(modbusRtuMaster->dataBits()));
    settings.setValue("stopBits", static_cast<int>(modbusRtuMaster->stopBits()));
    settings.endGroup(); // uuid
    settings.endGroup(); // ModbusRtuMasters
}

}
