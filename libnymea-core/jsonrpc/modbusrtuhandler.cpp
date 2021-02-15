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

#include "modbusrtuhandler.h"
#include "modbus/modbusrtumanager.h"
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
    returns.insert("modbusRtuMasters", QVariantList() << objectRef("ModbusRtuMaster"));
    registerMethod("GetModbusRtuMasters", description, params, returns);

    // ModbusRtuMasterAdded notification
    params.clear();
    description = "Emitted whenever a new modbus RTU master has been added to the system.";
    params.insert("modbusRtuMaster", objectRef("ModbusRtuMaster"));
    registerNotification("ModbusRtuMasterAdded", description, params);

    // ModbusRtuMasterRemoved notification
    params.clear();
    description = "Emitted whenever a new modbus RTU master has been removed from the system.";
    params.insert("modbusUuid", enumValueName(Uuid));
    registerNotification("ModbusRtuMasterRemoved", description, params);

    // ModbusRtuMasterChanged notification
    params.clear();
    description = "Emitted whenever a new modbus RTU master has been changed to the system.";
    params.insert("modbusRtuMaster", objectRef("ModbusRtuMaster"));
    registerNotification("ModbusRtuMasterChanged", description, params);

    // AddModbusRtuMaster
    params.clear(); returns.clear();
    description = "Add a new modbus RTU master with the given configuration.";
    params.insert("serialPort", enumValueName(String));
    params.insert("baudrate", enumValueName(Uint));
    params.insert("parity", enumRef<SerialPort::SerialPortParity>());
    params.insert("dataBits", enumRef<SerialPort::SerialPortDataBits>());
    params.insert("stopBits", enumRef<SerialPort::SerialPortStopBits>());
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
    returns.insert("modbusError", enumRef<ModbusRtuManager::ModbusRtuError>());
    registerMethod("ReconfigureModbusRtuMaster", description, params, returns);

    // Serial port monitor
    connect(modbusRtuManager->serialPortMonitor(), &SerialPortMonitor::serialPortAdded, this, [=](const SerialPort &serialPort){
        QVariantMap params;
        params.insert("serialPort", pack(serialPort));
        emit SerialPortAdded(params);
    });

    connect(modbusRtuManager->serialPortMonitor(), &SerialPortMonitor::serialPortRemoved, this, [=](const SerialPort &serialPort){
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
    foreach (const SerialPort &serialPort, m_modbusRtuManager->serialPortMonitor()->serialPorts()) {
        portList << pack(serialPort);
    }
    returnMap.insert("serialPorts", portList);
    return createReply(returnMap);
}

JsonReply *ModbusRtuHandler::GetModbusRtuMasters(const QVariantMap &params)
{
    Q_UNUSED(params)

    QVariantMap returnMap;
    QVariantList modbusList;
    foreach (ModbusRtuMaster *modbusMaster, m_modbusRtuManager->modbusRtuMasters()) {
        modbusList << packModbusRtuMaster(modbusMaster);
    }
    returnMap.insert("modbusRtuMasters", modbusList);
    return createReply(returnMap);
}

JsonReply *ModbusRtuHandler::AddModbusRtuMaster(const QVariantMap &params)
{
    QString serialPort = params.value("serialPort").toString();
    qint32 baudrate = params.value("baudrate").toUInt();
    QSerialPort::Parity parity = static_cast<QSerialPort::Parity>(enumNameToValue<SerialPort::SerialPortParity>(params.value("parity").toString()));
    QSerialPort::StopBits stopBits = static_cast<QSerialPort::StopBits>(enumNameToValue<SerialPort::SerialPortStopBits>(params.value("stopBits").toString()));
    QSerialPort::DataBits dataBits = static_cast<QSerialPort::DataBits>(enumNameToValue<SerialPort::SerialPortDataBits>(params.value("dataBits").toString()));

    QPair<ModbusRtuManager::ModbusRtuError, QUuid> result = m_modbusRtuManager->addNewModbusRtuMaster(serialPort, baudrate, parity, dataBits, stopBits);

    QVariantMap returnMap;
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

    ModbusRtuManager::ModbusRtuError result = m_modbusRtuManager->reconfigureModbusRtuMaster(modbusUuid, serialPort, baudrate, parity, dataBits, stopBits);
    QVariantMap returnMap;
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
    return modbusRtuMasterMap;
}

}
