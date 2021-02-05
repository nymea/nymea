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
    m_modbusUuid(modbusUuid),
    m_serialPort(serialPort),
    m_baudrate(baudrate),
    m_parity(parity),
    m_dataBits(dataBits),
    m_stopBits(stopBits)
{
#ifdef WITH_QTSERIALBUS
    m_modbus = new QModbusRtuSerialMaster(this);
    m_modbus->setConnectionParameter(QModbusDevice::SerialPortNameParameter, m_serialPort);
    m_modbus->setConnectionParameter(QModbusDevice::SerialBaudRateParameter, m_baudrate);
    m_modbus->setConnectionParameter(QModbusDevice::SerialDataBitsParameter, m_dataBits);
    m_modbus->setConnectionParameter(QModbusDevice::SerialStopBitsParameter, m_stopBits);
    m_modbus->setConnectionParameter(QModbusDevice::SerialParityParameter, m_parity);

    connect(m_modbus, &QModbusTcpClient::stateChanged, this, [=](QModbusDevice::State state){
        qCDebug(dcModbusRtu()) << "Connection state changed" << m_modbusUuid.toString() << m_serialPort << state;
        if (state == QModbusDevice::ConnectedState) {
            m_connected = true;
            emit connectedChanged(m_connected);
        } else {
            m_connected = false;
            emit connectedChanged(m_connected);
        }
    });

    connect(m_modbus, &QModbusRtuSerialMaster::errorOccurred, this, [=](QModbusDevice::Error error){
        qCDebug(dcModbusRtu()) << "Error occured for modbus RTU master" << m_modbusUuid.toString() << m_serialPort << error << m_modbus->errorString();
        // TODO: check if disconnected...
    });
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

qint32 ModbusRtuMasterImpl::baudrate() const
{
    return m_baudrate;
}

QSerialPort::Parity ModbusRtuMasterImpl::parity() const
{
    return m_parity;
}

QSerialPort::DataBits ModbusRtuMasterImpl::dataBits() const
{
    return m_dataBits;
}

QSerialPort::StopBits ModbusRtuMasterImpl::stopBits()
{
    return m_stopBits;
}

bool ModbusRtuMasterImpl::connected() const
{
    return m_connected;
}

ModbusRtuReply *ModbusRtuMasterImpl::readCoil(int slaveAddress, int registerAddress, quint16 size)
{
#ifdef WITH_QTSERIALBUS
    // Create the reply for the plugin
    ModbusRtuReplyImpl *reply = new ModbusRtuReplyImpl(slaveAddress, registerAddress, this);
    connect(reply, &ModbusRtuReplyImpl::finished, reply, &ModbusRtuReplyImpl::deleteLater);

    // Create the actual modbus lib reply
    QModbusDataUnit request = QModbusDataUnit(QModbusDataUnit::RegisterType::Coils, registerAddress, size);
    QModbusReply *modbusReply = m_modbus->sendReadRequest(request, slaveAddress);

    connect(modbusReply, &QModbusReply::finished, modbusReply, [=](){
        modbusReply->deleteLater();

        // Fill common reply data
        reply->setFinished(true);
        reply->setError(static_cast<ModbusRtuReply::Error>(modbusReply->error()));
        reply->setErrorString(modbusReply->errorString());

        // Check if the reply finished with an error
        if (modbusReply->error() != QModbusDevice::NoError) {
            qCWarning(dcModbusRtu()) << "Read coil request finished with error" << modbusReply->error() << modbusReply->errorString();
            emit reply->errorOccurred(reply->error());
            emit reply->finished();
            return;
        }

        // Parse the data unit and set reply result
        const QModbusDataUnit unit = modbusReply->result();
        reply->setResult(unit.values());
        emit reply->finished();
    });

    connect(modbusReply, &QModbusReply::errorOccurred, modbusReply, [=](QModbusDevice::Error error){
        qCWarning(dcModbusRtu()) << "Read coil request finished with error" << error << modbusReply->errorString();
        reply->setFinished(true);
        reply->setError(static_cast<ModbusRtuReply::Error>(modbusReply->error()));
        reply->setErrorString(modbusReply->errorString());
        emit reply->errorOccurred(reply->error());
        emit reply->finished();
    });

    return qobject_cast<ModbusRtuReply *>(reply);
#else
    Q_UNUSED(slaveAddress)
    Q_UNUSED(registerAddress)
    Q_UNUSED(size)

    return nullptr;
#endif
}

ModbusRtuReply *ModbusRtuMasterImpl::readDiscreteInput(int slaveAddress, int registerAddress, quint16 size)
{
#ifdef WITH_QTSERIALBUS
    // Create the reply for the plugin
    ModbusRtuReplyImpl *reply = new ModbusRtuReplyImpl(slaveAddress, registerAddress, this);
    connect(reply, &ModbusRtuReplyImpl::finished, reply, &ModbusRtuReplyImpl::deleteLater);

    // Create the actual modbus lib reply
    QModbusDataUnit request = QModbusDataUnit(QModbusDataUnit::RegisterType::DiscreteInputs, registerAddress, size);
    QModbusReply *modbusReply = m_modbus->sendReadRequest(request, slaveAddress);

    connect(modbusReply, &QModbusReply::finished, modbusReply, [=](){
        modbusReply->deleteLater();

        // Fill common reply data
        reply->setFinished(true);
        reply->setError(static_cast<ModbusRtuReply::Error>(modbusReply->error()));
        reply->setErrorString(modbusReply->errorString());

        // Check if the reply finished with an error
        if (modbusReply->error() != QModbusDevice::NoError) {
            qCWarning(dcModbusRtu()) << "Read descrete inputs request finished with error" << modbusReply->error() << modbusReply->errorString();
            emit reply->errorOccurred(reply->error());
            emit reply->finished();
            return;
        }

        // Parse the data unit and set reply result
        const QModbusDataUnit unit = modbusReply->result();
        reply->setResult(unit.values());
        emit reply->finished();
    });

    connect(modbusReply, &QModbusReply::errorOccurred, modbusReply, [=](QModbusDevice::Error error){
        qCWarning(dcModbusRtu()) << "Read descrete inputs request finished with error" << error << modbusReply->errorString();
        reply->setFinished(true);
        reply->setError(static_cast<ModbusRtuReply::Error>(modbusReply->error()));
        reply->setErrorString(modbusReply->errorString());
        emit reply->errorOccurred(reply->error());
        emit reply->finished();
    });

    return qobject_cast<ModbusRtuReply *>(reply);
#else
    Q_UNUSED(slaveAddress)
    Q_UNUSED(registerAddress)
    Q_UNUSED(size)

    return nullptr;
#endif
}

ModbusRtuReply *ModbusRtuMasterImpl::readInputRegister(int slaveAddress, int registerAddress, quint16 size)
{
#ifdef WITH_QTSERIALBUS
    // Create the reply for the plugin
    ModbusRtuReplyImpl *reply = new ModbusRtuReplyImpl(slaveAddress, registerAddress, this);
    connect(reply, &ModbusRtuReplyImpl::finished, reply, &ModbusRtuReplyImpl::deleteLater);

    // Create the actual modbus lib reply
    QModbusDataUnit request = QModbusDataUnit(QModbusDataUnit::RegisterType::InputRegisters, registerAddress, size);
    QModbusReply *modbusReply = m_modbus->sendReadRequest(request, slaveAddress);

    connect(modbusReply, &QModbusReply::finished, modbusReply, [=](){
        modbusReply->deleteLater();

        // Fill common reply data
        reply->setFinished(true);
        reply->setError(static_cast<ModbusRtuReply::Error>(modbusReply->error()));
        reply->setErrorString(modbusReply->errorString());

        // Check if the reply finished with an error
        if (modbusReply->error() != QModbusDevice::NoError) {
            qCWarning(dcModbusRtu()) << "Read input registers request finished with error" << modbusReply->error() << modbusReply->errorString();
            emit reply->errorOccurred(reply->error());
            emit reply->finished();
            return;
        }

        // Parse the data unit and set reply result
        const QModbusDataUnit unit = modbusReply->result();
        reply->setResult(unit.values());
        emit reply->finished();
    });

    connect(modbusReply, &QModbusReply::errorOccurred, modbusReply, [=](QModbusDevice::Error error){
        qCWarning(dcModbusRtu()) << "Read input registers request finished with error" << error << modbusReply->errorString();
        reply->setFinished(true);
        reply->setError(static_cast<ModbusRtuReply::Error>(modbusReply->error()));
        reply->setErrorString(modbusReply->errorString());
        emit reply->errorOccurred(reply->error());
        emit reply->finished();
    });

    return qobject_cast<ModbusRtuReply *>(reply);
#else
    Q_UNUSED(slaveAddress)
    Q_UNUSED(registerAddress)
    Q_UNUSED(size)

    return nullptr;
#endif
}

ModbusRtuReply *ModbusRtuMasterImpl::readHoldingRegister(int slaveAddress, int registerAddress, quint16 size)
{
#ifdef WITH_QTSERIALBUS
    // Create the reply for the plugin
    ModbusRtuReplyImpl *reply = new ModbusRtuReplyImpl(slaveAddress, registerAddress, this);
    connect(reply, &ModbusRtuReplyImpl::finished, reply, &ModbusRtuReplyImpl::deleteLater);

    // Create the actual modbus lib reply
    QModbusDataUnit request = QModbusDataUnit(QModbusDataUnit::RegisterType::HoldingRegisters, registerAddress, size);
    QModbusReply *modbusReply = m_modbus->sendReadRequest(request, slaveAddress);

    connect(modbusReply, &QModbusReply::finished, modbusReply, [=](){
        modbusReply->deleteLater();

        // Fill common reply data
        reply->setFinished(true);
        reply->setError(static_cast<ModbusRtuReply::Error>(modbusReply->error()));
        reply->setErrorString(modbusReply->errorString());

        // Check if the reply finished with an error
        if (modbusReply->error() != QModbusDevice::NoError) {
            qCWarning(dcModbusRtu()) << "Read holding registers request finished with error" << modbusReply->error() << modbusReply->errorString();
            emit reply->errorOccurred(reply->error());
            emit reply->finished();
            return;
        }

        // Parse the data unit and set reply result
        const QModbusDataUnit unit = modbusReply->result();
        reply->setResult(unit.values());
        emit reply->finished();
    });

    connect(modbusReply, &QModbusReply::errorOccurred, modbusReply, [=](QModbusDevice::Error error){
        qCWarning(dcModbusRtu()) << "Read holding registers request finished with error" << error << modbusReply->errorString();
        reply->setFinished(true);
        reply->setError(static_cast<ModbusRtuReply::Error>(modbusReply->error()));
        reply->setErrorString(modbusReply->errorString());
        emit reply->errorOccurred(reply->error());
        emit reply->finished();
    });

    return qobject_cast<ModbusRtuReply *>(reply);
#else
    Q_UNUSED(slaveAddress)
    Q_UNUSED(registerAddress)
    Q_UNUSED(size)

    return nullptr;
#endif
}

ModbusRtuReply *ModbusRtuMasterImpl::writeCoils(int slaveAddress, int registerAddress, const QVector<quint16> &values)
{
#ifdef WITH_QTSERIALBUS
    // Create the reply for the plugin
    ModbusRtuReplyImpl *reply = new ModbusRtuReplyImpl(slaveAddress, registerAddress, this);
    connect(reply, &ModbusRtuReplyImpl::finished, reply, &ModbusRtuReplyImpl::deleteLater);

    // Create the actual modbus lib reply
    QModbusDataUnit request = QModbusDataUnit(QModbusDataUnit::RegisterType::Coils, registerAddress, values.length());
    request.setValues(values);

    QModbusReply *modbusReply = m_modbus->sendWriteRequest(request, slaveAddress);

    connect(modbusReply, &QModbusReply::finished, modbusReply, [=](){
        modbusReply->deleteLater();

        // Fill common reply data
        reply->setFinished(true);
        reply->setError(static_cast<ModbusRtuReply::Error>(modbusReply->error()));
        reply->setErrorString(modbusReply->errorString());

        // Check if the reply finished with an error
        if (modbusReply->error() != QModbusDevice::NoError) {
            qCWarning(dcModbusRtu()) << "Read coil request finished with error" << modbusReply->error() << modbusReply->errorString();
            emit reply->errorOccurred(reply->error());
            emit reply->finished();
            return;
        }

        // Parse the data unit and set reply result
        const QModbusDataUnit unit = modbusReply->result();
        reply->setResult(unit.values());
        emit reply->finished();
    });

    connect(modbusReply, &QModbusReply::errorOccurred, modbusReply, [=](QModbusDevice::Error error){
        qCWarning(dcModbusRtu()) << "Read coil request finished with error" << error << modbusReply->errorString();
        reply->setFinished(true);
        reply->setError(static_cast<ModbusRtuReply::Error>(modbusReply->error()));
        reply->setErrorString(modbusReply->errorString());
        emit reply->errorOccurred(reply->error());
        emit reply->finished();
    });

    return qobject_cast<ModbusRtuReply *>(reply);
#else
    Q_UNUSED(slaveAddress)
    Q_UNUSED(registerAddress)
    Q_UNUSED(values)

    return nullptr;
#endif
}


ModbusRtuReply *ModbusRtuMasterImpl::writeHoldingRegisters(int slaveAddress, int registerAddress, const QVector<quint16> &values)
{
#ifdef WITH_QTSERIALBUS
    // Create the reply for the plugin
    ModbusRtuReplyImpl *reply = new ModbusRtuReplyImpl(slaveAddress, registerAddress, this);
    connect(reply, &ModbusRtuReplyImpl::finished, reply, &ModbusRtuReplyImpl::deleteLater);

    // Create the actual modbus lib reply
    QModbusDataUnit request = QModbusDataUnit(QModbusDataUnit::RegisterType::HoldingRegisters, registerAddress, values.length());
    request.setValues(values);

    QModbusReply *modbusReply = m_modbus->sendWriteRequest(request, slaveAddress);

    connect(modbusReply, &QModbusReply::finished, modbusReply, [=](){
        modbusReply->deleteLater();

        // Fill common reply data
        reply->setFinished(true);
        reply->setError(static_cast<ModbusRtuReply::Error>(modbusReply->error()));
        reply->setErrorString(modbusReply->errorString());

        // Check if the reply finished with an error
        if (modbusReply->error() != QModbusDevice::NoError) {
            qCWarning(dcModbusRtu()) << "Read coil request finished with error" << modbusReply->error() << modbusReply->errorString();
            emit reply->errorOccurred(reply->error());
            emit reply->finished();
            return;
        }

        // Parse the data unit and set reply result
        const QModbusDataUnit unit = modbusReply->result();
        reply->setResult(unit.values());
        emit reply->finished();
    });

    connect(modbusReply, &QModbusReply::errorOccurred, modbusReply, [=](QModbusDevice::Error error){
        qCWarning(dcModbusRtu()) << "Read coil request finished with error" << error << modbusReply->errorString();
        reply->setFinished(true);
        reply->setError(static_cast<ModbusRtuReply::Error>(modbusReply->error()));
        reply->setErrorString(modbusReply->errorString());
        emit reply->errorOccurred(reply->error());
        emit reply->finished();
    });

    return qobject_cast<ModbusRtuReply *>(reply);
#else
    Q_UNUSED(slaveAddress)
    Q_UNUSED(registerAddress)
    Q_UNUSED(values)

    return nullptr;
#endif
}

}
