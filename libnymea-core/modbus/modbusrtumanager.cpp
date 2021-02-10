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
#include "hardware/serialport/serialportmonitor.h"

NYMEA_LOGGING_CATEGORY(dcModbusRtu, "ModbusRtu")

namespace nymeaserver {

ModbusRtuManager::ModbusRtuManager(SerialPortMonitor *serialPortMonitor, QObject *parent) :
    QObject(parent),
    m_serialPortMonitor(serialPortMonitor)
{
    // Load uart configurations
    loadRtuMasters();

    connect(m_serialPortMonitor, &SerialPortMonitor::serialPortAdded, this, [=](const QSerialPortInfo &serialPortInfo){
        qCDebug(dcModbusRtu()) << "Serial port added. Verify modbus RTU masters...";

        // Check if we have to reconnect any modbus RTU masters
        foreach (ModbusRtuMaster *modbusMaster, m_modbusRtuMasters.values()) {
            ModbusRtuMasterImpl *modbusMasterImpl = qobject_cast<ModbusRtuMasterImpl *>(modbusMaster);

            // Try only to reconnect if the added serial port matches a disconnected modbus RTU master
            if (!modbusMasterImpl->connected() && modbusMasterImpl->serialPort() == serialPortInfo.systemLocation()) {
                if (!modbusMasterImpl->connectDevice()) {
                    qCDebug(dcModbusRtu()) << "Reconnect" << modbusMaster << "failed.";
                } else {
                    qCDebug(dcModbusRtu()) << "Reconnected" << modbusMaster << "sucessfully.";
                }
            }
        }
    });
}

SerialPortMonitor *ModbusRtuManager::serialPortMonitor() const
{
    return m_serialPortMonitor;
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

QPair<ModbusRtuManager::ModbusRtuError, QUuid>  ModbusRtuManager::addNewModbusRtuMaster(const QString &serialPort, qint32 baudrate, QSerialPort::Parity parity, QSerialPort::DataBits dataBits, QSerialPort::StopBits stopBits)
{
    // Check if the serial port exists
    if (!m_serialPortMonitor->serialPortAvailable(serialPort)) {
        qCWarning(dcModbusRtu()) << "Cannot add new modbus RTU master because the serial port" << serialPort << "is not available any more";
        return QPair<ModbusRtuManager::ModbusRtuError, QUuid>(ModbusRtuErrorHardwareNotFound, QUuid());
    }

    QUuid modbusUuid = QUuid::createUuid();
    ModbusRtuMasterImpl *modbusMaster = new ModbusRtuMasterImpl(modbusUuid, serialPort, baudrate, parity, dataBits, stopBits, this);
    ModbusRtuMaster *modbus = qobject_cast<ModbusRtuMaster *>(modbusMaster);
    qCDebug(dcModbusRtu()) << "Adding new" << modbus << parity << dataBits << stopBits;

    // Note: We could add the modbus master event if a connection is currently not possible...not sure yet
    if (!modbusMaster->connectDevice()) {
        qCWarning(dcModbusRtu()) << "Failed to add new modbus RTU master. Could not connect to" << modbus << parity << dataBits << stopBits;
        modbusMaster->deleteLater();
        return QPair<ModbusRtuError, QUuid>(ModbusRtuErrorConnectionFailed, QUuid());
    }

    addModbusRtuMasterInternally(modbusMaster);
    saveModbusRtuMaster(modbus);

    return QPair<ModbusRtuError, QUuid>(ModbusRtuErrorNoError, modbusUuid);
}

ModbusRtuManager::ModbusRtuError ModbusRtuManager::reconfigureModbusRtuMaster(const QUuid &modbusUuid, const QString &serialPort, qint32 baudrate, QSerialPort::Parity parity, QSerialPort::DataBits dataBits, QSerialPort::StopBits stopBits)
{
    if (!m_modbusRtuMasters.contains(modbusUuid)) {
        qCWarning(dcModbusRtu()) << "Could not reconfigure modbus RTU master because no resource could be found with uuid" << modbusUuid.toString();
        return ModbusRtuErrorUuidNotFound;
    }

    ModbusRtuMasterImpl *modbusMaster = qobject_cast<ModbusRtuMasterImpl *>(m_modbusRtuMasters.value(modbusUuid));

    // Disconnect
    modbusMaster->disconnectDevice();

    // Reconfigure
    modbusMaster->setSerialPort(serialPort);
    modbusMaster->setBaudrate(baudrate);
    modbusMaster->setParity(parity);
    modbusMaster->setDataBits(dataBits);
    modbusMaster->setStopBits(stopBits);

    // Connect again
    if (!modbusMaster->connectDevice()) {
        qCWarning(dcModbusRtu()) << "Failed to connect to" << m_modbusRtuMasters.value(modbusUuid);
        emit modbusRtuMasterChanged(m_modbusRtuMasters.value(modbusUuid));
        return ModbusRtuErrorConnectionFailed;
    }

    emit modbusRtuMasterChanged(m_modbusRtuMasters.value(modbusUuid));

    qCDebug(dcModbusRtu()) << "Reconfigured successfully" << m_modbusRtuMasters.value(modbusUuid);
    return ModbusRtuErrorNoError;
}

ModbusRtuManager::ModbusRtuError ModbusRtuManager::removeModbusRtuMaster(const QUuid &modbusUuid)
{
    if (!m_modbusRtuMasters.contains(modbusUuid)) {
        qCWarning(dcModbusRtu()) << "Could not remove modbus RTU master because no resource could be found with uuid" << modbusUuid.toString();
        return ModbusRtuErrorUuidNotFound;
    }

    ModbusRtuMasterImpl *modbusMaster = qobject_cast<ModbusRtuMasterImpl *>(m_modbusRtuMasters.take(modbusUuid));
    qCDebug(dcModbusRtu()) << "Removing modbus RTU master" << qobject_cast<ModbusRtuMaster *>(modbusMaster);
    modbusMaster->disconnectDevice();
    modbusMaster->deleteLater();

    emit modbusRtuMasterRemoved(modbusMaster);

    return ModbusRtuErrorNoError;
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

        addModbusRtuMasterInternally(new ModbusRtuMasterImpl(QUuid(uuidString), serialPort, baudrate, parity, dataBits, stopBits, this));
    }

    settings.endGroup(); // ModbusRtuMasters
}

void ModbusRtuManager::saveModbusRtuMaster(ModbusRtuMaster *modbusRtuMaster)
{
    NymeaSettings settings(NymeaSettings::SettingsRoleModbusRtu);
    qCDebug(dcModbusRtu()) << "Saving" << modbusRtuMaster << "to" << settings.fileName();
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

void ModbusRtuManager::addModbusRtuMasterInternally(ModbusRtuMasterImpl *modbusRtuMaster)
{
    ModbusRtuMaster *modbusMaster = qobject_cast<ModbusRtuMaster *>(modbusRtuMaster);
    qCDebug(dcModbusRtu()) << "Adding" << modbusMaster;
    m_modbusRtuMasters.insert(modbusMaster->modbusUuid(), modbusMaster);

    connect(modbusMaster, &ModbusRtuMaster::connectedChanged, this, [=](bool connected){
        qCDebug(dcModbusRtu()) << modbusMaster << (connected ? "connected" : "disconnected");
        emit modbusRtuMasterChanged(modbusMaster);
    });

    emit modbusRtuMasterAdded(modbusMaster);
}

}
