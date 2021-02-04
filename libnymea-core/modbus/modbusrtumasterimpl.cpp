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

#include "modbusrtumasterimpl.h"
#include "modbusrtureplyimpl.h"

#ifdef WITH_QTSERIALBUS
#include <QtSerialBus/QModbusReply>
#include <QtSerialBus/QModbusDataUnit>
#endif

Q_DECLARE_LOGGING_CATEGORY(dcModbusRtu)

namespace nymeaserver {

ModbusRtuMasterImpl::ModbusRtuMasterImpl(const QUuid &modbusUuid, const QString &serialPort, qint32 baudrate, QSerialPort::Parity parity, QSerialPort::DataBits dataBits, QSerialPort::StopBits stopBits, QObject *parent) :
    ModbusRtuMaster(parent),
    m_modbusUuid(modbusUuid)
{
#ifdef WITH_QTSERIALBUS
    m_modbus = new QModbusRtuSerialMaster(this);
    m_modbus->setConnectionParameter(QModbusDevice::SerialPortNameParameter, serialPort);
    m_modbus->setConnectionParameter(QModbusDevice::SerialBaudRateParameter, baudrate);
    m_modbus->setConnectionParameter(QModbusDevice::SerialDataBitsParameter, dataBits);
    m_modbus->setConnectionParameter(QModbusDevice::SerialStopBitsParameter, stopBits);
    m_modbus->setConnectionParameter(QModbusDevice::SerialParityParameter, parity);

    connect(m_modbus, &QModbusTcpClient::stateChanged, this, [=](QModbusDevice::State state){
        qCDebug(dcModbusRtu()) << "Connection state changed" << m_modbusUuid.toString() << serialPort << state;
    });
    //connect(m_modbus, &QModbusRtuSerialMaster::errorOccurred, this, &ModbusRtuMaster::onModbusErrorOccurred);

//    m_reconnectTimer = new QTimer(this);
//    m_reconnectTimer->setSingleShot(true);
//    connect(m_reconnectTimer, &QTimer::timeout, this, &ModbusRTUMaster::onReconnectTimer);

#endif
}

QUuid ModbusRtuMasterImpl::modbusUuid() const
{
    return m_modbusUuid;
}

QString ModbusRtuMasterImpl::serialPort() const
{
    return m_serialPort;
}

bool ModbusRtuMasterImpl::connected() const
{
    return m_connected;
}

ModbusRtuReply *ModbusRtuMasterImpl::readCoil(uint slaveAddress, uint registerAddress, uint size)
{
#ifndef WITH_QTSERIALBUS
    Q_UNUSED(slaveAddress)
    Q_UNUSED(registerAddress)
    Q_UNUSED(size)
    return nullptr;
#else

    QModbusDataUnit request = QModbusDataUnit(QModbusDataUnit::RegisterType::Coils, registerAddress, size);
    QModbusReply *modbusReply = m_modbus->sendReadRequest(request, slaveAddress);

    // TODO: fill data and return reply
    ModbusRtuReplyImpl *reply = new ModbusRtuReplyImpl(slaveAddress, registerAddress, this);
    connect(modbusReply, &QModbusReply::finished, modbusReply, [=](){
        if (modbusReply->error() != QModbusDevice::NoError) {
            qCWarning(dcModbusRtu()) << "Read coil request finished with error" << modbusReply->error() << modbusReply->errorString();

        }
    });
    return qobject_cast<ModbusRtuReply *>(reply);
#endif
}

ModbusRtuReply *ModbusRtuMasterImpl::readDiscreteInput(uint slaveAddress, uint registerAddress, uint size)
{
#ifndef WITH_QTSERIALBUS
    Q_UNUSED(slaveAddress)
    Q_UNUSED(registerAddress)
    Q_UNUSED(size)
    return nullptr;
#else
    // TODO:
    Q_UNUSED(slaveAddress)
    Q_UNUSED(registerAddress)
    Q_UNUSED(size)
    return nullptr;
#endif
}

ModbusRtuReply *ModbusRtuMasterImpl::readInputRegister(uint slaveAddress, uint registerAddress, uint size)
{
#ifndef WITH_QTSERIALBUS
    Q_UNUSED(slaveAddress)
    Q_UNUSED(registerAddress)
    Q_UNUSED(size)
    return nullptr;
#else
    // TODO:
    Q_UNUSED(slaveAddress)
    Q_UNUSED(registerAddress)
    Q_UNUSED(size)
    return nullptr;
#endif
}

ModbusRtuReply *ModbusRtuMasterImpl::readHoldingRegister(uint slaveAddress, uint registerAddress, uint size)
{
#ifndef WITH_QTSERIALBUS
    Q_UNUSED(slaveAddress)
    Q_UNUSED(registerAddress)
    Q_UNUSED(size)
    return nullptr;
#else
    // TODO:
    Q_UNUSED(slaveAddress)
    Q_UNUSED(registerAddress)
    Q_UNUSED(size)
    return nullptr;
#endif
}

ModbusRtuReply *ModbusRtuMasterImpl::writeCoil(uint slaveAddress, uint registerAddress, bool status)
{
#ifndef WITH_QTSERIALBUS
    Q_UNUSED(slaveAddress)
    Q_UNUSED(registerAddress)
    Q_UNUSED(size)
    return nullptr;
#else
    // TODO:
    Q_UNUSED(slaveAddress)
    Q_UNUSED(registerAddress)
    Q_UNUSED(status)
    return nullptr;
#endif
}

ModbusRtuReply *ModbusRtuMasterImpl::writeCoils(uint slaveAddress, uint registerAddress, const QVector<quint16> &values)
{
#ifndef WITH_QTSERIALBUS
    Q_UNUSED(slaveAddress)
    Q_UNUSED(registerAddress)
    Q_UNUSED(values)
    return nullptr;
#else
    // TODO:
    Q_UNUSED(slaveAddress)
    Q_UNUSED(registerAddress)
    Q_UNUSED(values)
    return nullptr;
#endif
}

ModbusRtuReply *ModbusRtuMasterImpl::writeHoldingRegister(uint slaveAddress, uint registerAddress, quint16 value)
{
#ifndef WITH_QTSERIALBUS
    Q_UNUSED(slaveAddress)
    Q_UNUSED(registerAddress)
    Q_UNUSED(value)
    return nullptr;
#else
    // TODO:
    Q_UNUSED(slaveAddress)
    Q_UNUSED(registerAddress)
    Q_UNUSED(value)
    return nullptr;
#endif
}

ModbusRtuReply *ModbusRtuMasterImpl::writeHoldingRegisters(uint slaveAddress, uint registerAddress, const QVector<quint16> &values)
{
#ifndef WITH_QTSERIALBUS
    Q_UNUSED(slaveAddress)
    Q_UNUSED(registerAddress)
    Q_UNUSED(values)
    return nullptr;
#else
    // TODO:
    Q_UNUSED(slaveAddress)
    Q_UNUSED(registerAddress)
    Q_UNUSED(values)
    return nullptr;
#endif
}

}
