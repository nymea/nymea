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

#include "modbusrtuhandler.h"
#include "hardware/modbus/modbusrtumanager.h"
#include "hardware/serialport/serialportmonitor.h"

namespace nymeaserver {

ModbusRtuHandler::ModbusRtuHandler(ModbusRtuManager *modbusRtuManager, QObject *parent) :
    JsonHandler(parent),
    m_modbusRtuManager(modbusRtuManager)
{
    qRegisterMetaType<nymeaserver::SerialPort>();
    registerEnum<ModbusRtuManager::ModbusRtuError>();
    registerEnum<SerialPort::SerialPortParity>();
    registerEnum<SerialPort::SerialPortStopBits>();
    registerEnum<SerialPort::SerialPortDataBits>();
    registerObject<SerialPort, SerialPorts>();

    QVariantMap modbusRtuMasterDescription;
    modbusRtuMasterDescription.insert("modbusUuid", enumValueName(Uuid));
    modbusRtuMasterDescription.insert("connected", enumValueName(Bool));
    modbusRtuMasterDescription.insert("serialPort", enumValueName(String));
    modbusRtuMasterDescription.insert("baudrate", enumValueName(Uint));
    modbusRtuMasterDescription.insert("parity", enumRef<SerialPort::SerialPortParity>());
    modbusRtuMasterDescription.insert("stopBits", enumRef<SerialPort::SerialPortStopBits>());
    modbusRtuMasterDescription.insert("dataBits", enumRef<SerialPort::SerialPortDataBits>());
    modbusRtuMasterDescription.insert("numberOfRetries", enumValueName(Uint));
    modbusRtuMasterDescription.insert("timeout", enumValueName(Uint));

    registerObject("ModbusRtuMaster", modbusRtuMasterDescription);

    QVariantMap params, returns;
    QString description;

    // GetSerialPorts
    params.clear(); returns.clear();
    description = "Get the list of available serial ports from the host system.";
    returns.insert("serialPorts", objectRef<SerialPorts>());
    registerMethod("GetSerialPorts", description, params, returns);

    // SerialPortAdded notification
    params.clear();
    description = "Emitted whenever a serial port has been added to the system.";
    params.insert("serialPort", objectRef<SerialPort>());
    registerNotification("SerialPortAdded", description, params);

    // SerialPortRemoved notification
    params.clear();
    description = "Emitted whenever a serial port has been removed from the system.";
    params.insert("serialPort", objectRef<SerialPort>());
    registerNotification("SerialPortRemoved", description, params);

    // GetModbusRtuMasters
    params.clear(); returns.clear();
    description = "Get the list of configured modbus RTU masters.";
    returns.insert("o:modbusRtuMasters", QVariantList() << objectRef("ModbusRtuMaster"));
    returns.insert("modbusError", enumRef<ModbusRtuManager::ModbusRtuError>());
    registerMethod("GetModbusRtuMasters", description, params, returns);

    // ModbusRtuMasterAdded notification
    params.clear();
    description = "Emitted whenever a new modbus RTU master has been added to the system.";
    params.insert("modbusRtuMaster", objectRef("ModbusRtuMaster"));
    registerNotification("ModbusRtuMasterAdded", description, params);

    // ModbusRtuMasterRemoved notification
    params.clear();
    description = "Emitted whenever a modbus RTU master has been removed from the system.";
    params.insert("modbusUuid", enumValueName(Uuid));
    registerNotification("ModbusRtuMasterRemoved", description, params);

    // ModbusRtuMasterChanged notification
    params.clear();
    description = "Emitted whenever a modbus RTU master has been changed in the system.";
    params.insert("modbusRtuMaster", objectRef("ModbusRtuMaster"));
    registerNotification("ModbusRtuMasterChanged", description, params);

    // AddModbusRtuMaster
    params.clear(); returns.clear();
    description = "Add a new modbus RTU master with the given configuration. The timeout value is in milli seconds and the minimum value is 10 ms.";
    params.insert("serialPort", enumValueName(String));
    params.insert("baudrate", enumValueName(Uint));
    params.insert("parity", enumRef<SerialPort::SerialPortParity>());
    params.insert("dataBits", enumRef<SerialPort::SerialPortDataBits>());
    params.insert("stopBits", enumRef<SerialPort::SerialPortStopBits>());
    params.insert("numberOfRetries", enumValueName(Uint));
    params.insert("timeout", enumValueName(Uint));
    returns.insert("o:modbusUuid", enumValueName(Uuid));
    returns.insert("modbusError", enumRef<ModbusRtuManager::ModbusRtuError>());
    registerMethod("AddModbusRtuMaster", description, params, returns);

    // RemoveModbusRtuMaster
    params.clear(); returns.clear();
    description = "Remove the modbus RTU master with the given modbus UUID.";
    params.insert("modbusUuid", enumValueName(Uuid));
    returns.insert("modbusError", enumRef<ModbusRtuManager::ModbusRtuError>());
    registerMethod("RemoveModbusRtuMaster", description, params, returns);

    // ReconfigureModbusRtuMaster
    params.clear(); returns.clear();
    description = "Reconfigure the modbus RTU master with the given UUID and configuration.";
    params.insert("modbusUuid", enumValueName(Uuid));
    params.insert("serialPort", enumValueName(String));
    params.insert("baudrate", enumValueName(Uint));
    params.insert("parity", enumRef<SerialPort::SerialPortParity>());
    params.insert("dataBits", enumRef<SerialPort::SerialPortDataBits>());
    params.insert("stopBits", enumRef<SerialPort::SerialPortStopBits>());
    params.insert("numberOfRetries", enumValueName(Uint));
    params.insert("timeout", enumValueName(Uint));
    returns.insert("modbusError", enumRef<ModbusRtuManager::ModbusRtuError>());
    registerMethod("ReconfigureModbusRtuMaster", description, params, returns);

    // Serial port monitor
    connect(modbusRtuManager, &ModbusRtuManager::serialPortAdded, this, [=](const SerialPort &serialPort){
        QVariantMap params;
        params.insert("serialPort", pack(serialPort));
        emit SerialPortAdded(params);
    });

    connect(modbusRtuManager, &ModbusRtuManager::serialPortRemoved, this, [=](const SerialPort &serialPort){
        QVariantMap params;
        params.insert("serialPort", pack(serialPort));
        emit SerialPortRemoved(params);
    });

    // Modbus manager
    connect(modbusRtuManager, &ModbusRtuManager::modbusRtuMasterAdded, this, [=](ModbusRtuMaster *modbusRtuMaster){
        QVariantMap params;
        params.insert("modbusRtuMaster", packModbusRtuMaster(modbusRtuMaster));
        emit ModbusRtuMasterAdded(params);
    });

    connect(modbusRtuManager, &ModbusRtuManager::modbusRtuMasterChanged, this, [=](ModbusRtuMaster *modbusRtuMaster){
        QVariantMap params;
        params.insert("modbusRtuMaster", packModbusRtuMaster(modbusRtuMaster));
        emit ModbusRtuMasterChanged(params);
    });

    connect(modbusRtuManager, &ModbusRtuManager::modbusRtuMasterRemoved, this, [=](ModbusRtuMaster *modbusRtuMaster){
        QVariantMap params;
        params.insert("modbusUuid", modbusRtuMaster->modbusUuid());
        emit ModbusRtuMasterRemoved(params);
    });
}

QString ModbusRtuHandler::name() const
{
    return "ModbusRtu";
}

JsonReply *ModbusRtuHandler::GetSerialPorts(const QVariantMap &params)
{
    Q_UNUSED(params)

    QVariantMap returnMap;
    QVariantList portList;
    foreach (const SerialPort &serialPort, m_modbusRtuManager->serialPorts()) {
        portList << pack(serialPort);
    }
    returnMap.insert("serialPorts", portList);
    return createReply(returnMap);
}

JsonReply *ModbusRtuHandler::GetModbusRtuMasters(const QVariantMap &params)
{
    Q_UNUSED(params)

    QVariantMap returnMap;

    if (m_modbusRtuManager->supported()) {
        QVariantList modbusList;
        foreach (ModbusRtuMaster *modbusMaster, m_modbusRtuManager->modbusRtuMasters()) {
            modbusList << packModbusRtuMaster(modbusMaster);
        }
        returnMap.insert("modbusRtuMasters", modbusList);
        returnMap.insert("modbusError", enumValueName<ModbusRtuManager::ModbusRtuError>(ModbusRtuManager::ModbusRtuErrorNoError));
    } else {
        returnMap.insert("modbusError", enumValueName<ModbusRtuManager::ModbusRtuError>(ModbusRtuManager::ModbusRtuErrorNotSupported));
    }


    return createReply(returnMap);
}

JsonReply *ModbusRtuHandler::AddModbusRtuMaster(const QVariantMap &params)
{
    QString serialPort = params.value("serialPort").toString();
    qint32 baudrate = params.value("baudrate").toUInt();
    QSerialPort::Parity parity = static_cast<QSerialPort::Parity>(enumNameToValue<SerialPort::SerialPortParity>(params.value("parity").toString()));
    QSerialPort::StopBits stopBits = static_cast<QSerialPort::StopBits>(enumNameToValue<SerialPort::SerialPortStopBits>(params.value("stopBits").toString()));
    QSerialPort::DataBits dataBits = static_cast<QSerialPort::DataBits>(enumNameToValue<SerialPort::SerialPortDataBits>(params.value("dataBits").toString()));
    uint numberOfRetries = params.value("numberOfRetries").toUInt();
    uint timeout = params.value("timeout").toUInt();

    QVariantMap returnMap;
    if (timeout < 10) {
        returnMap.insert("modbusError", enumValueName<ModbusRtuManager::ModbusRtuError>(ModbusRtuManager::ModbusRtuErrorInvalidTimeoutValue));
        return createReply(returnMap);
    }

    QPair<ModbusRtuManager::ModbusRtuError, QUuid> result = m_modbusRtuManager->addNewModbusRtuMaster(serialPort, baudrate, parity, dataBits, stopBits, numberOfRetries, timeout);
    returnMap.insert("modbusError", enumValueName<ModbusRtuManager::ModbusRtuError>(result.first));
    if (result.first == ModbusRtuManager::ModbusRtuErrorNoError) {
        returnMap.insert("modbusUuid", result.second);
    }
    return createReply(returnMap);
}

JsonReply *ModbusRtuHandler::RemoveModbusRtuMaster(const QVariantMap &params)
{
    QUuid modbusUuid = params.value("modbusUuid").toUuid();

    ModbusRtuManager::ModbusRtuError result = m_modbusRtuManager->removeModbusRtuMaster(modbusUuid);
    QVariantMap returnMap;
    returnMap.insert("modbusError", enumValueName<ModbusRtuManager::ModbusRtuError>(result));
    return createReply(returnMap);
}

JsonReply *ModbusRtuHandler::ReconfigureModbusRtuMaster(const QVariantMap &params)
{
    QUuid modbusUuid = params.value("modbusUuid").toUuid();
    QString serialPort = params.value("serialPort").toString();
    qint32 baudrate = params.value("baudrate").toUInt();
    QSerialPort::Parity parity = static_cast<QSerialPort::Parity>(enumNameToValue<SerialPort::SerialPortParity>(params.value("parity").toString()));
    QSerialPort::StopBits stopBits = static_cast<QSerialPort::StopBits>(enumNameToValue<SerialPort::SerialPortStopBits>(params.value("stopBits").toString()));
    QSerialPort::DataBits dataBits = static_cast<QSerialPort::DataBits>(enumNameToValue<SerialPort::SerialPortDataBits>(params.value("dataBits").toString()));
    uint numberOfRetries = params.value("numberOfRetries").toUInt();
    uint timeout = params.value("timeout").toUInt();

    QVariantMap returnMap;
    if (timeout < 10) {
        returnMap.insert("modbusError", enumValueName<ModbusRtuManager::ModbusRtuError>(ModbusRtuManager::ModbusRtuErrorInvalidTimeoutValue));
        return createReply(returnMap);
    }

    ModbusRtuManager::ModbusRtuError result = m_modbusRtuManager->reconfigureModbusRtuMaster(modbusUuid, serialPort, baudrate, parity, dataBits, stopBits, numberOfRetries, timeout);
    returnMap.insert("modbusError", enumValueName<ModbusRtuManager::ModbusRtuError>(result));
    return createReply(returnMap);
}


QVariantMap ModbusRtuHandler::packModbusRtuMaster(ModbusRtuMaster *modbusRtuMaster)
{
    QVariantMap modbusRtuMasterMap;
    modbusRtuMasterMap.insert("modbusUuid", modbusRtuMaster->modbusUuid());
    modbusRtuMasterMap.insert("connected", modbusRtuMaster->connected());
    modbusRtuMasterMap.insert("serialPort", modbusRtuMaster->serialPort());
    modbusRtuMasterMap.insert("baudrate", modbusRtuMaster->baudrate());
    modbusRtuMasterMap.insert("parity", enumValueName<SerialPort::SerialPortParity>(static_cast<SerialPort::SerialPortParity>(modbusRtuMaster->parity())));
    modbusRtuMasterMap.insert("stopBits", enumValueName<SerialPort::SerialPortStopBits>(static_cast<SerialPort::SerialPortStopBits>(modbusRtuMaster->stopBits())));
    modbusRtuMasterMap.insert("dataBits", enumValueName<SerialPort::SerialPortDataBits>(static_cast<SerialPort::SerialPortDataBits>(modbusRtuMaster->dataBits())));
    modbusRtuMasterMap.insert("numberOfRetries", modbusRtuMaster->numberOfRetries());
    modbusRtuMasterMap.insert("timeout", modbusRtuMaster->timeout());
    return modbusRtuMasterMap;
}

}
