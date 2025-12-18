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

#include "modbusrtumanager.h"
#include "loggingcategories.h"
#include "nymeasettings.h"

#include "hardware/serialport/serialportmonitor.h"
#include "modbusrtumasterimpl.h"
#include "qglobal.h"

#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonParseError>

NYMEA_LOGGING_CATEGORY(dcModbusRtu, "ModbusRtu")

namespace nymeaserver {

ModbusRtuManager::ModbusRtuManager(SerialPortMonitor *serialPortMonitor, QObject *parent)
    : QObject(parent)
    , m_serialPortMonitor(serialPortMonitor)
{
    if (!supported())
        return;

    // Load the platform config
    loadPlatformConfiguration();

    // Load uart configurations
    loadRtuMasters();

    // Initialize the list of serial ports already available in the system...
    foreach (const SerialPort &serialPort, m_serialPortMonitor->serialPorts()) {
        onSerialPortAdded(serialPort);
    }

    connect(m_serialPortMonitor, &SerialPortMonitor::serialPortAdded, this, &ModbusRtuManager::onSerialPortAdded);
    connect(m_serialPortMonitor, &SerialPortMonitor::serialPortRemoved, this, &ModbusRtuManager::onSerialPortRemoved);

    // Try to connect the modbus RTU masters
    foreach (ModbusRtuMaster *modbusMaster, m_modbusRtuMasters.values()) {
        ModbusRtuMasterImpl *modbusMasterImpl = qobject_cast<ModbusRtuMasterImpl *>(modbusMaster);
        if (!modbusMasterImpl->connectDevice()) {
            qCWarning(dcModbusRtu()) << "Failed to connect modbus RTU master. Could not connect to" << modbusMaster;
        }
    }
}

SerialPorts ModbusRtuManager::serialPorts() const
{
    return m_serialPorts.values();
}

bool ModbusRtuManager::serialPortAvailable(const QString &systemLocation) const
{
    return m_serialPorts.keys().contains(systemLocation);
}

void ModbusRtuManager::onSerialPortAdded(const SerialPort &serialPort)
{
    if (m_serialPorts.contains(serialPort.systemLocation()))
        return;

    // Depending on the platform configuration we have to filter out those serial ports which are not suitable for modbus RTU communication.
    foreach (const ModbusRtuPlatformConfiguration &platformConfig, m_platformConfigurations) {
        if (platformConfig.serialPort == serialPort.systemLocation()) {
            if (platformConfig.usable) {
                SerialPort internalPort = serialPort;
                internalPort.setCustomDescription(platformConfig.description);
                m_serialPorts.insert(internalPort.systemLocation(), internalPort);
                emit serialPortAdded(internalPort);
            } else {
                qCDebug(dcModbusRtu()) << "Serial port" << serialPort.systemLocation() << "added but not usable for modbus RTU according to the platorm configuration.";
                return;
            }
        }
    }

    if (!m_serialPorts.contains(serialPort.systemLocation())) {
        m_serialPorts.insert(serialPort.systemLocation(), serialPort);
        emit serialPortAdded(serialPort);
    }

    qCDebug(dcModbusRtu()) << "Serial port" << serialPort.systemLocation() << "added. Evaluate modbus RTU masters...";

    // Check if we have to reconnect any modbus RTU masters
    foreach (ModbusRtuMaster *modbusMaster, m_modbusRtuMasters.values()) {
        ModbusRtuMasterImpl *modbusMasterImpl = qobject_cast<ModbusRtuMasterImpl *>(modbusMaster);

        // Try only to reconnect if the added serial port matches a disconnected modbus RTU master
        if (!modbusMasterImpl->connected() && modbusMasterImpl->serialPort() == serialPort.systemLocation()) {
            if (!modbusMasterImpl->connectDevice()) {
                qCDebug(dcModbusRtu()) << "Reconnect" << modbusMaster << "failed.";
            } else {
                qCDebug(dcModbusRtu()) << "Reconnected" << modbusMaster << "successfully.";
            }
        }
    }
}

void ModbusRtuManager::onSerialPortRemoved(const SerialPort &serialPort)
{
    if (m_serialPorts.contains(serialPort.systemLocation())) {
        qCDebug(dcModbusRtu()) << "Serial port removed" << serialPort.systemLocation();
        m_serialPorts.remove(serialPort.systemLocation());
        emit serialPortRemoved(serialPort);
    }
}

bool ModbusRtuManager::supported() const
{
#ifdef WITH_QTSERIALBUS
    return true;
#else
    return false;
#endif
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

QPair<ModbusRtuManager::ModbusRtuError, QUuid> ModbusRtuManager::addNewModbusRtuMaster(
    const QString &serialPort, qint32 baudrate, QSerialPort::Parity parity, QSerialPort::DataBits dataBits, QSerialPort::StopBits stopBits, int numberOfRetries, int timeout)
{
    if (!supported()) {
        qCWarning(dcModbusRtu()) << "Cannot add new modbus RTU master because serialbus is not suppoerted on this platform.";
        return QPair<ModbusRtuManager::ModbusRtuError, QUuid>(ModbusRtuErrorNotSupported, QUuid());
    }

    // Check if the serial port exists
    if (!m_serialPortMonitor->serialPortAvailable(serialPort)) {
        qCWarning(dcModbusRtu()) << "Cannot add new modbus RTU master because the serial port" << serialPort << "is not available any more";
        return QPair<ModbusRtuManager::ModbusRtuError, QUuid>(ModbusRtuErrorHardwareNotFound, QUuid());
    }

    QUuid modbusUuid = QUuid::createUuid();
    ModbusRtuMasterImpl *modbusMaster = new ModbusRtuMasterImpl(modbusUuid, serialPort, baudrate, parity, dataBits, stopBits, numberOfRetries, timeout, this);
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

ModbusRtuManager::ModbusRtuError ModbusRtuManager::reconfigureModbusRtuMaster(const QUuid &modbusUuid,
                                                                              const QString &serialPort,
                                                                              qint32 baudrate,
                                                                              QSerialPort::Parity parity,
                                                                              QSerialPort::DataBits dataBits,
                                                                              QSerialPort::StopBits stopBits,
                                                                              int numberOfRetries,
                                                                              int timeout)
{
    if (!supported()) {
        qCWarning(dcModbusRtu()) << "Cannot reconfigure modbus RTU master because serialbus is not suppoerted on this platform.";
        return ModbusRtuErrorNotSupported;
    }

    if (!m_modbusRtuMasters.contains(modbusUuid)) {
        qCWarning(dcModbusRtu()) << "Could not reconfigure modbus RTU master because no resource could be found with uuid" << modbusUuid.toString();
        return ModbusRtuErrorUuidNotFound;
    }

    // Take the modbus masters
    ModbusRtuMasterImpl *modbusMaster = qobject_cast<ModbusRtuMasterImpl *>(m_modbusRtuMasters.value(modbusUuid));

    // Disconnect
    modbusMaster->disconnectDevice();

    // Reconfigure
    modbusMaster->setSerialPort(serialPort);
    modbusMaster->setBaudrate(baudrate);
    modbusMaster->setParity(parity);
    modbusMaster->setDataBits(dataBits);
    modbusMaster->setStopBits(stopBits);
    modbusMaster->setNumberOfRetries(numberOfRetries);
    modbusMaster->setTimeout(timeout);

    // Connect again
    if (!modbusMaster->connectDevice()) {
        qCWarning(dcModbusRtu()) << "Failed to connect to" << m_modbusRtuMasters.value(modbusUuid);
        // FIXME: check if we should reload the old configuration
        emit modbusRtuMasterChanged(m_modbusRtuMasters.value(modbusUuid));
        return ModbusRtuErrorConnectionFailed;
    }

    emit modbusRtuMasterChanged(m_modbusRtuMasters.value(modbusUuid));

    qCDebug(dcModbusRtu()) << "Reconfigured successfully" << m_modbusRtuMasters.value(modbusUuid);
    saveModbusRtuMaster(modbusMaster);
    return ModbusRtuErrorNoError;
}

ModbusRtuManager::ModbusRtuError ModbusRtuManager::removeModbusRtuMaster(const QUuid &modbusUuid)
{
    if (!supported()) {
        qCWarning(dcModbusRtu()) << "Cannot remove modbus RTU master because serialbus is not suppoerted on this platform.";
        return ModbusRtuErrorNotSupported;
    }

    if (!m_modbusRtuMasters.contains(modbusUuid)) {
        qCWarning(dcModbusRtu()) << "Could not remove modbus RTU master because no resource could be found with uuid" << modbusUuid.toString();
        return ModbusRtuErrorUuidNotFound;
    }

    ModbusRtuMasterImpl *modbusMaster = qobject_cast<ModbusRtuMasterImpl *>(m_modbusRtuMasters.take(modbusUuid));
    qCDebug(dcModbusRtu()) << "Removing modbus RTU master" << qobject_cast<ModbusRtuMaster *>(modbusMaster);
    modbusMaster->disconnectDevice();
    modbusMaster->deleteLater();

    // Remove from settings
    NymeaSettings settings(NymeaSettings::SettingsRoleModbusRtu);
    settings.beginGroup("ModbusRtuMasters");
    settings.beginGroup(modbusUuid.toString());
    settings.remove("");
    settings.endGroup(); // modbusUuid
    settings.endGroup(); // ModbusRtuMasters

    emit modbusRtuMasterRemoved(modbusMaster);

    return ModbusRtuErrorNoError;
}

void ModbusRtuManager::loadPlatformConfiguration()
{
    QFileInfo platformConfigurationFileInfo(NymeaSettings::settingsPath() + QDir::separator() + "modbus-rtu-platform.conf");

    if (platformConfigurationFileInfo.exists()) {
        qCDebug(dcModbusRtu()) << "Loading modbus RTU platform configuration from" << platformConfigurationFileInfo.absoluteFilePath();
        QFile platformConfigurationFile(platformConfigurationFileInfo.absoluteFilePath());
        if (!platformConfigurationFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qCWarning(dcModbusRtu()) << "Failed to open modbus RTU platform configuration file" << platformConfigurationFileInfo.absoluteFilePath() << ":"
                                     << platformConfigurationFile.errorString();
            return;
        }

        QByteArray data = platformConfigurationFile.readAll();
        platformConfigurationFile.close();

        QJsonParseError jsonError;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &jsonError);
        if (jsonError.error != QJsonParseError::NoError) {
            qCWarning(dcModbusRtu()) << "Failed to parse JSON data from modbus RTU platform configuration file" << platformConfigurationFileInfo.absoluteFilePath() << ":"
                                     << jsonError.errorString();
            return;
        }

        // Make sure we have a clean start
        m_platformConfigurations.clear();

        QVariantMap platformConfigMap = jsonDoc.toVariant().toMap();
        foreach (const QVariant &configVariant, platformConfigMap.value("interfaces").toList()) {
            QVariantMap configMap = configVariant.toMap();
            ModbusRtuPlatformConfiguration config;
            config.name = configMap.value("name").toString();
            config.description = configMap.value("description").toString();
            config.serialPort = configMap.value("serialPort").toString();
            config.usable = configMap.value("usable").toBool();
            m_platformConfigurations.append(config);
        }

        if (m_platformConfigurations.isEmpty()) {
            qCDebug(dcModbusRtu()) << "No platform configurations available";
        } else {
            qCDebug(dcModbusRtu()) << "Loaded successfully" << m_platformConfigurations.count() << "platform configurations";
            foreach (const ModbusRtuPlatformConfiguration &config, m_platformConfigurations) {
                qCDebug(dcModbusRtu()).nospace() << "- Platform configuration: " << config.description << " (" << config.name << ") " << "Serial port: " << config.serialPort
                                                 << " usable: " << config.usable;
            }
        }
    }
}

void ModbusRtuManager::loadRtuMasters()
{
    if (!supported())
        return;

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
        int numberOfRetries = settings.value("numberOfRetries").toInt();
        int timeout = settings.value("timeout").toInt();
        settings.endGroup(); // uuid

        addModbusRtuMasterInternally(new ModbusRtuMasterImpl(QUuid(uuidString), serialPort, baudrate, parity, dataBits, stopBits, numberOfRetries, timeout, this));
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
    settings.setValue("numberOfRetries", modbusRtuMaster->numberOfRetries());
    settings.setValue("timeout", modbusRtuMaster->timeout());
    settings.endGroup(); // uuid
    settings.endGroup(); // ModbusRtuMasters
}

void ModbusRtuManager::addModbusRtuMasterInternally(ModbusRtuMasterImpl *modbusRtuMaster)
{
    ModbusRtuMaster *modbusMaster = qobject_cast<ModbusRtuMaster *>(modbusRtuMaster);
    qCDebug(dcModbusRtu()) << "Adding" << modbusMaster;
    m_modbusRtuMasters.insert(modbusMaster->modbusUuid(), modbusMaster);

    connect(modbusMaster, &ModbusRtuMaster::connectedChanged, this, [=](bool connected) {
        qCDebug(dcModbusRtu()) << modbusMaster << (connected ? "connected" : "disconnected");
        emit modbusRtuMasterChanged(modbusMaster);
    });

    emit modbusRtuMasterAdded(modbusMaster);
}

} // namespace nymeaserver
