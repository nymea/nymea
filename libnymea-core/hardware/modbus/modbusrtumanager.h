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

#ifndef MODBUSRTUMANAGER_H
#define MODBUSRTUMANAGER_H

#include <QHash>
#include <QObject>
#include <QTimer>
#include <QUuid>

#include "hardware/modbus/modbusrtumaster.h"
#include "hardware/serialport/serialportmonitor.h"

namespace nymeaserver {

class ModbusRtuMasterImpl;

class ModbusRtuManager : public QObject
{
    Q_OBJECT
public:
    enum ModbusRtuError {
        ModbusRtuErrorNoError,
        ModbusRtuErrorNotAvailable,
        ModbusRtuErrorUuidNotFound,
        ModbusRtuErrorHardwareNotFound,
        ModbusRtuErrorResourceBusy,
        ModbusRtuErrorNotSupported,
        ModbusRtuErrorInvalidTimeoutValue,
        ModbusRtuErrorConnectionFailed
    };
    Q_ENUM(ModbusRtuError)

    explicit ModbusRtuManager(SerialPortMonitor *serialPortMonitor, QObject *parent = nullptr);
    ~ModbusRtuManager() = default;

    bool supported() const;

    QList<ModbusRtuMaster *> modbusRtuMasters() const;
    bool hasModbusRtuMaster(const QUuid &modbusUuid) const;
    ModbusRtuMaster *getModbusRtuMaster(const QUuid &modbusUuid);

    QPair<ModbusRtuError, QUuid> addNewModbusRtuMaster(
        const QString &serialPort, qint32 baudrate, QSerialPort::Parity parity, QSerialPort::DataBits dataBits, QSerialPort::StopBits stopBits, int numberOfRetries, int timeout);
    ModbusRtuError reconfigureModbusRtuMaster(const QUuid &modbusUuid,
                                              const QString &serialPort,
                                              qint32 baudrate,
                                              QSerialPort::Parity parity,
                                              QSerialPort::DataBits dataBits,
                                              QSerialPort::StopBits stopBits,
                                              int numberOfRetries,
                                              int timeout);
    ModbusRtuError removeModbusRtuMaster(const QUuid &modbusUuid);

    // Returns the platform specific list of available serial ports for modbus RTU
    SerialPorts serialPorts() const;
    bool serialPortAvailable(const QString &systemLocation) const;

signals:
    void serialPortAdded(const SerialPort &serialPort);
    void serialPortRemoved(const SerialPort &serialPort);

    void modbusRtuMasterAdded(ModbusRtuMaster *modbusRtuMaster);
    void modbusRtuMasterRemoved(ModbusRtuMaster *modbusRtuMaster);
    void modbusRtuMasterChanged(ModbusRtuMaster *modbusRtuMaster);

private slots:
    void onSerialPortAdded(const SerialPort &serialPort);
    void onSerialPortRemoved(const SerialPort &serialPort);

private:
    typedef struct ModbusRtuPlatformConfiguration
    {
        QString name;
        QString description;
        QString serialPort;
        bool usable;
    } ModbusRtuPlatformConfiguration;

    QList<ModbusRtuPlatformConfiguration> m_platformConfigurations;

    QHash<QUuid, ModbusRtuMaster *> m_modbusRtuMasters;
    SerialPortMonitor *m_serialPortMonitor = nullptr;
    QHash<QString, SerialPort> m_serialPorts;

    void loadPlatformConfiguration();

    void loadRtuMasters();
    void saveModbusRtuMaster(ModbusRtuMaster *modbusRtuMaster);

    void addModbusRtuMasterInternally(ModbusRtuMasterImpl *modbusRtuMaster);
};

} // namespace nymeaserver

#endif // MODBUSRTUMANAGER_H
