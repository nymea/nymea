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
    // Load available serial ports
    updateSerialPorts();

    // Load uart configurations
    loadRtuMasters();

    // Enable autoconnect for each modbus rtu master
    m_timer = new QTimer(this);
    m_timer->setInterval(5000);
    m_timer->setSingleShot(false);
    connect(m_timer, &QTimer::timeout, this, [=](){
        // Update serial port list
        updateSerialPorts();

        // Check if we have to reconnect a device
        foreach (ModbusRtuMaster *modbusMaster, m_modbusRtuMasters.values()) {
            ModbusRtuMasterImpl *modbusMasterImpl = qobject_cast<ModbusRtuMasterImpl *>(modbusMaster);

            if (!modbusMasterImpl->connected()) {
                if (!modbusMasterImpl->connectDevice()) {
                    qCDebug(dcModbusRtu()) << "Reconnect" << modbusMaster << "failed.";
                } else {
                    qCDebug(dcModbusRtu()) << "Reconnected" << modbusMaster << "sucessfully.";
                }
            }
        }
    });

    m_timer->start();
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

QPair<ModbusRtuManager::Error, QUuid>  ModbusRtuManager::addNewModbusRtuMaster(const QString &serialPort, qint32 baudrate, QSerialPort::Parity parity, QSerialPort::DataBits dataBits, QSerialPort::StopBits stopBits)
{
    // Check if the serial port exists

    // Check if the serial port is not occupied

    QUuid modbusUuid = QUuid::createUuid();
    ModbusRtuMasterImpl *modbusMaster = new ModbusRtuMasterImpl(modbusUuid, serialPort, baudrate, parity, dataBits, stopBits, this);
    ModbusRtuMaster *modbus = qobject_cast<ModbusRtuMaster *>(modbusMaster);
    qCDebug(dcModbusRtu()) << "Adding new" << qobject_cast<ModbusRtuMaster *>(modbusMaster) << parity << dataBits << stopBits;

    // Connect the modbus master bus;
    m_modbusRtuMasters.insert(modbusUuid, modbus);
    emit modbusRtuMasterAdded(modbus);
    saveModbusRtuMaster(modbus);

    return QPair<Error, QUuid>(ErrorNoError, modbusUuid);
}

ModbusRtuManager::Error ModbusRtuManager::reconfigureRtuMaster(const QUuid &modbusUuid, const QString &serialPort, qint32 baudrate, QSerialPort::Parity parity, QSerialPort::DataBits dataBits, QSerialPort::StopBits stopBits)
{
    if (!m_modbusRtuMasters.contains(modbusUuid)) {
        qCWarning(dcModbusRtu()) << "Could not reconfigure modbus RTU master because no resource could be found with uuid" << modbusUuid.toString();
        return ErrorNotFound;
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
        return ErrorConnectionFailed;
    }

    emit modbusRtuMasterChanged(m_modbusRtuMasters.value(modbusUuid));

    qCDebug(dcModbusRtu()) << "Reconfigured successfully" << m_modbusRtuMasters.value(modbusUuid);
    return ErrorNoError;
}


ModbusRtuManager::Error ModbusRtuManager::removeModbusRtuMaster(const QUuid &modbusUuid)
{
    ModbusRtuMasterImpl *modbusMaster = qobject_cast<ModbusRtuMasterImpl *>(m_modbusRtuMasters.value(modbusUuid));
    if (!modbusMaster) {
        qCWarning(dcModbusRtu()) << "Could not remove modbus RTU master because no resource could be found with uuid" << modbusUuid.toString();
        return ErrorNotFound;
    }

    qCDebug(dcModbusRtu()) << "Removing modbus RTU master" << qobject_cast<ModbusRtuMaster *>(modbusMaster);
    emit modbusRtuMasterRemoved(modbusMaster);

    modbusMaster->deleteLater();
    return ErrorNoError;
}

void ModbusRtuManager::updateSerialPorts()
{
    // Check if serial port added or removed

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
        emit modbusRtuMasterAdded(modbusRtuMaster);
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
