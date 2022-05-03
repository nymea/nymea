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

#include <QLoggingCategory>

#ifdef WITH_QTSERIALBUS
#include <QtSerialBus/QModbusReply>
#include <QtSerialBus/QModbusDataUnit>
#endif

Q_DECLARE_LOGGING_CATEGORY(dcModbusRtu)

namespace nymeaserver {

ModbusRtuMasterImpl::ModbusRtuMasterImpl(const QUuid &modbusUuid, const QString &serialPort, qint32 baudrate, QSerialPort::Parity parity, QSerialPort::DataBits dataBits, QSerialPort::StopBits stopBits, int numberOfRetries, int timeout, QObject *parent) :
    ModbusRtuMaster(parent),
    m_modbusUuid(modbusUuid),
    m_serialPort(serialPort),
    m_baudrate(baudrate),
    m_parity(parity),
    m_dataBits(dataBits),
    m_stopBits(stopBits),
    m_numberOfRetries(numberOfRetries),
    m_timeout(timeout)
{
#ifdef WITH_QTSERIALBUS
    m_modbus = new QModbusRtuSerialMaster(this);
    m_modbus->setConnectionParameter(QModbusDevice::SerialPortNameParameter, m_serialPort);
    m_modbus->setConnectionParameter(QModbusDevice::SerialBaudRateParameter, m_baudrate);
    m_modbus->setConnectionParameter(QModbusDevice::SerialDataBitsParameter, m_dataBits);
    m_modbus->setConnectionParameter(QModbusDevice::SerialStopBitsParameter, m_stopBits);
    m_modbus->setConnectionParameter(QModbusDevice::SerialParityParameter, m_parity);
    m_modbus->setNumberOfRetries(m_numberOfRetries);
    m_modbus->setTimeout(m_timeout);

    connect(m_modbus, &QModbusTcpClient::stateChanged, this, [=](QModbusDevice::State state){
        qCDebug(dcModbusRtu()) << "Connection state changed" << m_modbusUuid.toString() << m_serialPort << state;
        if (state == QModbusDevice::ConnectedState) {
            if (m_connected != true) {
                m_connected = true;
                emit connectedChanged(m_connected);
            }
        } else {
            if (m_connected != false) {
                m_connected = false;
                emit connectedChanged(m_connected);
            }
        }
    });

    connect(m_modbus, &QModbusRtuSerialMaster::errorOccurred, this, [=](QModbusDevice::Error error){
        qCWarning(dcModbusRtu()) << "Error occurred for modbus RTU master" << m_modbusUuid.toString() << m_serialPort << error << m_modbus->errorString();
        if (error != QModbusDevice::NoError) {
            disconnectDevice();
        }
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

void ModbusRtuMasterImpl::setSerialPort(const QString &serialPort)
{
    if (m_serialPort == serialPort)
        return;

    m_serialPort = serialPort;
    emit serialPortChanged(m_serialPort);
}

qint32 ModbusRtuMasterImpl::baudrate() const
{
    return m_baudrate;
}

void ModbusRtuMasterImpl::setBaudrate(qint32 baudrate)
{
    if (m_baudrate == baudrate)
        return;

    m_baudrate = baudrate;
    emit baudrateChanged(m_baudrate);
}

QSerialPort::Parity ModbusRtuMasterImpl::parity() const
{
    return m_parity;
}

void ModbusRtuMasterImpl::setParity(QSerialPort::Parity parity)
{
    if (m_parity == parity)
        return;

    m_parity = parity;
    emit parityChanged(m_parity);
}

QSerialPort::DataBits ModbusRtuMasterImpl::dataBits() const
{
    return m_dataBits;
}

void ModbusRtuMasterImpl::setDataBits(QSerialPort::DataBits dataBits)
{
    if (m_dataBits == dataBits)
        return;

    m_dataBits = dataBits;
    emit dataBitsChanged(m_dataBits);
}

QSerialPort::StopBits ModbusRtuMasterImpl::stopBits()
{
    return m_stopBits;
}

void ModbusRtuMasterImpl::setStopBits(QSerialPort::StopBits stopBits)
{
    if (m_stopBits == stopBits)
        return;

    m_stopBits = stopBits;
    emit stopBitsChanged(m_stopBits);
}

bool ModbusRtuMasterImpl::connected() const
{
    return m_connected;
}

void ModbusRtuMasterImpl::requestReconnect()
{
    disconnectDevice();
    connectDevice();
}

bool ModbusRtuMasterImpl::connectDevice()
{
#ifdef WITH_QTSERIALBUS
    m_modbus->setConnectionParameter(QModbusDevice::SerialPortNameParameter, m_serialPort);
    m_modbus->setConnectionParameter(QModbusDevice::SerialBaudRateParameter, m_baudrate);
    m_modbus->setConnectionParameter(QModbusDevice::SerialDataBitsParameter, m_dataBits);
    m_modbus->setConnectionParameter(QModbusDevice::SerialStopBitsParameter, m_stopBits);
    m_modbus->setConnectionParameter(QModbusDevice::SerialParityParameter, m_parity);
    m_modbus->setNumberOfRetries(m_numberOfRetries);
    m_modbus->setTimeout(m_timeout);
    return m_modbus->connectDevice();
#else
    qCWarning(dcModbusRtu()) << "Modbus is not available on this platform.";
    return false;
#endif
}

void ModbusRtuMasterImpl::disconnectDevice()
{
#ifdef WITH_QTSERIALBUS
    m_modbus->disconnectDevice();
#else
    qCWarning(dcModbusRtu()) << "Modbus is not available on this platform.";
#endif
}

int ModbusRtuMasterImpl::numberOfRetries() const
{
    return m_numberOfRetries;
}

void ModbusRtuMasterImpl::setNumberOfRetries(int numberOfRetries)
{
    if (m_numberOfRetries == numberOfRetries)
        return;

    m_numberOfRetries = numberOfRetries;
    emit numberOfRetriesChanged(m_numberOfRetries);
#ifdef WITH_QTSERIALBUS
    m_modbus->setNumberOfRetries(m_numberOfRetries);
#endif
}

int ModbusRtuMasterImpl::timeout() const
{
    return m_timeout;
}

void ModbusRtuMasterImpl::setTimeout(int timeout)
{
    if (m_timeout == timeout)
        return;

    m_timeout = timeout;
    emit timeoutChanged(m_timeout);
#ifdef WITH_QTSERIALBUS
    m_modbus->setTimeout(m_timeout);
#endif
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

    // Cleaning up modbusReply when our reply finishes, regardless if that happened because modbusReply finished or reply timeouted
    connect(reply, &ModbusRtuReply::finished, modbusReply, &QModbusReply::deleteLater);

    connect(modbusReply, &QModbusReply::finished, reply, [=](){
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

    return qobject_cast<ModbusRtuReply *>(reply);
#else
    Q_UNUSED(slaveAddress)
    Q_UNUSED(registerAddress)
    Q_UNUSED(size)
    qCWarning(dcModbusRtu()) << "Modbus is not available on this platform.";

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

    // Cleaning up modbusReply when our reply finishes, regardless if that happened because modbusReply finished or reply timeouted
    connect(reply, &ModbusRtuReply::finished, modbusReply, &QModbusReply::deleteLater);

    connect(modbusReply, &QModbusReply::finished, reply, [=](){
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

    return qobject_cast<ModbusRtuReply *>(reply);
#else
    Q_UNUSED(slaveAddress)
    Q_UNUSED(registerAddress)
    Q_UNUSED(size)
    qCWarning(dcModbusRtu()) << "Modbus is not available on this platform.";

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

    // Cleaning up modbusReply when our reply finishes, regardless if that happened because modbusReply finished or reply timeouted
    connect(reply, &ModbusRtuReply::finished, modbusReply, &QModbusReply::deleteLater);

    connect(modbusReply, &QModbusReply::finished, reply, [=](){
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

    return qobject_cast<ModbusRtuReply *>(reply);
#else
    Q_UNUSED(slaveAddress)
    Q_UNUSED(registerAddress)
    Q_UNUSED(size)
    qCWarning(dcModbusRtu()) << "Modbus is not available on this platform.";

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

    // Cleaning up modbusReply when our reply finishes, regardless if that happened because modbusReply finished or reply timeouted
    connect(reply, &ModbusRtuReply::finished, modbusReply, &QModbusReply::deleteLater);

    connect(modbusReply, &QModbusReply::finished, reply, [=](){
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

    return qobject_cast<ModbusRtuReply *>(reply);
#else
    Q_UNUSED(slaveAddress)
    Q_UNUSED(registerAddress)
    Q_UNUSED(size)
    qCWarning(dcModbusRtu()) << "Modbus is not available on this platform.";

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

    // Cleaning up modbusReply when our reply finishes, regardless if that happened because modbusReply finished or reply timeouted
    connect(reply, &ModbusRtuReply::finished, modbusReply, &QModbusReply::deleteLater);

    connect(modbusReply, &QModbusReply::finished, reply, [=](){
        // Fill common reply data
        reply->setFinished(true);
        reply->setError(static_cast<ModbusRtuReply::Error>(modbusReply->error()));
        reply->setErrorString(modbusReply->errorString());

        // Check if the reply finished with an error
        if (modbusReply->error() != QModbusDevice::NoError) {
            qCWarning(dcModbusRtu()) << "Write coil request finished with error" << modbusReply->error() << modbusReply->errorString();
            emit reply->errorOccurred(reply->error());
            emit reply->finished();
            return;
        }

        // Parse the data unit and set reply result
        const QModbusDataUnit unit = modbusReply->result();
        reply->setResult(unit.values());
        emit reply->finished();
    });

    return qobject_cast<ModbusRtuReply *>(reply);
#else
    Q_UNUSED(slaveAddress)
    Q_UNUSED(registerAddress)
    Q_UNUSED(values)
    qCWarning(dcModbusRtu()) << "Modbus is not available on this platform.";

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

    // Cleaning up modbusReply when our reply finishes, regardless if that happened because modbusReply finished or reply timeouted
    connect(reply, &ModbusRtuReply::finished, modbusReply, &QModbusReply::deleteLater);

    connect(modbusReply, &QModbusReply::finished, reply, [=](){
        // Fill common reply data
        reply->setFinished(true);
        reply->setError(static_cast<ModbusRtuReply::Error>(modbusReply->error()));
        reply->setErrorString(modbusReply->errorString());

        // Check if the reply finished with an error
        if (modbusReply->error() != QModbusDevice::NoError) {
            qCWarning(dcModbusRtu()) << "Write holding register request finished with error" << modbusReply->error() << modbusReply->errorString();
            emit reply->errorOccurred(reply->error());
            emit reply->finished();
            return;
        }

        // Parse the data unit and set reply result
        const QModbusDataUnit unit = modbusReply->result();
        reply->setResult(unit.values());
        emit reply->finished();
    });

    return qobject_cast<ModbusRtuReply *>(reply);
#else
    Q_UNUSED(slaveAddress)
    Q_UNUSED(registerAddress)
    Q_UNUSED(values)
    qCWarning(dcModbusRtu()) << "Modbus is not available on this platform.";

    return nullptr;
#endif
}

}
