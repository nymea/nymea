/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2024, nymea GmbH
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

#ifndef MODBUSRTUMANAGER_H
#define MODBUSRTUMANAGER_H

#include <QHash>
#include <QUuid>
#include <QObject>
#include <QTimer>

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

    QPair<ModbusRtuError, QUuid> addNewModbusRtuMaster(const QString &serialPort, qint32 baudrate, QSerialPort::Parity parity, QSerialPort::DataBits dataBits, QSerialPort::StopBits stopBits, int numberOfRetries, int timeout);
    ModbusRtuError reconfigureModbusRtuMaster(const QUuid &modbusUuid, const QString &serialPort, qint32 baudrate, QSerialPort::Parity parity, QSerialPort::DataBits dataBits, QSerialPort::StopBits stopBits, int numberOfRetries, int timeout);
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
    typedef struct ModbusRtuPlatformConfiguration {
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

}

#endif // MODBUSRTUMANAGER_H
