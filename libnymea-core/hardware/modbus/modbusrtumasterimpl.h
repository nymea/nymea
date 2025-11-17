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

#ifndef MODBUSRTUMASTERIMPL_H
#define MODBUSRTUMASTERIMPL_H

#include <QObject>
#include <QSerialPort>

#ifdef WITH_QTSERIALBUS
#include <QtSerialBus/QtSerialBus>
#endif

#include "hardware/modbus/modbusrtumaster.h"

namespace nymeaserver {

class ModbusRtuMasterImpl : public ModbusRtuMaster
{
    Q_OBJECT
public:
    explicit ModbusRtuMasterImpl(const QUuid &modbusUuid, const QString &serialPort, qint32 baudrate, QSerialPort::Parity parity, QSerialPort::DataBits dataBits, QSerialPort::StopBits stopBits, int numberOfRetries, int timeout, QObject *parent = nullptr);
    ~ModbusRtuMasterImpl() override = default;

    QUuid modbusUuid() const override;

    QString serialPort() const override;
    void setSerialPort(const QString &serialPort);

    qint32 baudrate() const override;
    void setBaudrate(qint32 baudrate);

    QSerialPort::Parity parity() const override;
    void setParity(QSerialPort::Parity parity);

    QSerialPort::DataBits dataBits() const override;
    void setDataBits(QSerialPort::DataBits dataBits);

    QSerialPort::StopBits stopBits() override;
    void setStopBits(QSerialPort::StopBits stopBits);

    bool connected() const override;

    void requestReconnect() override;

    bool connectDevice();
    void disconnectDevice();

    int numberOfRetries() const override;
    void setNumberOfRetries(int numberOfRetries);

    int timeout() const override;
    void setTimeout(int timeout);

    // Requests
    ModbusRtuReply *readCoil(int slaveAddress, int registerAddress, quint16 size = 1) override;
    ModbusRtuReply *readDiscreteInput(int slaveAddress, int registerAddress, quint16 size = 1) override;
    ModbusRtuReply *readInputRegister(int slaveAddress, int registerAddress, quint16 size = 1) override;
    ModbusRtuReply *readHoldingRegister(int slaveAddress, int registerAddress, quint16 size = 1) override;

    ModbusRtuReply *writeCoils(int slaveAddress, int registerAddress, const QVector<quint16> &values) override;
    ModbusRtuReply *writeHoldingRegisters(int slaveAddress, int registerAddress, const QVector<quint16> &values) override;

private:
    QUuid m_modbusUuid;
    bool m_connected = false;

#ifdef WITH_QTSERIALBUS

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QModbusRtuSerialClient *m_modbus = nullptr;
#else
    QModbusRtuSerialMaster *m_modbus = nullptr;
#endif

#endif // WITH_QTSERIALBUS

    QString m_serialPort;
    qint32 m_baudrate;
    QSerialPort::Parity m_parity;
    QSerialPort::DataBits m_dataBits;
    QSerialPort::StopBits m_stopBits;
    int m_numberOfRetries = 3;
    int m_timeout = 100;
};

}

#endif // MODBUSRTUMASTERIMPL_H
