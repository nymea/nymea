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

#ifndef MODBUSRTUMANAGER_H
#define MODBUSRTUMANAGER_H

#include <QHash>
#include <QUuid>
#include <QObject>
#include <QTimer>

#include "hardware/modbus/modbusrtumaster.h"

namespace nymeaserver {

class SerialPortMonitor;
class ModbusRtuMasterImpl;

class ModbusRtuManager : public QObject
{
    Q_OBJECT
public:
    enum ModbusRtuError {
        ModbusRtuErrorNoError,
        ModbusRtuErrorUuidNotFound,
        ModbusRtuErrorHardwareNotFound,
        ModbusRtuErrorConnectionFailed
    };
    Q_ENUM(ModbusRtuError)

    explicit ModbusRtuManager(SerialPortMonitor *serialPortMonitor, QObject *parent = nullptr);
    ~ModbusRtuManager() = default;

    SerialPortMonitor *serialPortMonitor() const;

    QList<ModbusRtuMaster *> modbusRtuMasters() const;
    bool hasModbusRtuMaster(const QUuid &modbusUuid) const;
    ModbusRtuMaster *getModbusRtuMaster(const QUuid &modbusUuid);

    QPair<ModbusRtuError, QUuid> addNewModbusRtuMaster(const QString &serialPort, qint32 baudrate, QSerialPort::Parity parity, QSerialPort::DataBits dataBits, QSerialPort::StopBits stopBits);
    ModbusRtuError reconfigureModbusRtuMaster(const QUuid &modbusUuid, const QString &serialPort, qint32 baudrate, QSerialPort::Parity parity, QSerialPort::DataBits dataBits, QSerialPort::StopBits stopBits);
    ModbusRtuError removeModbusRtuMaster(const QUuid &modbusUuid);

signals:
    void modbusRtuMasterAdded(ModbusRtuMaster *modbusRtuMaster);
    void modbusRtuMasterRemoved(ModbusRtuMaster *modbusRtuMaster);
    void modbusRtuMasterChanged(ModbusRtuMaster *modbusRtuMaster);

private:
    QHash<QUuid, ModbusRtuMaster *> m_modbusRtuMasters;
    SerialPortMonitor *m_serialPortMonitor = nullptr;

    void loadRtuMasters();
    void saveModbusRtuMaster(ModbusRtuMaster *modbusRtuMaster);

    void addModbusRtuMasterInternally(ModbusRtuMasterImpl *modbusRtuMaster);

};

}

#endif // MODBUSRTUMANAGER_H
