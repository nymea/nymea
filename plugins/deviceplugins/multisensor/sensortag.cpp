/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
 *  Copyright (C) 2016 nicc                                                *
 *                                                                         *
 *  This file is part of guh.                                              *
 *                                                                         *
 *  This library is free software; you can redistribute it and/or          *
 *  modify it under the terms of the GNU Lesser General Public             *
 *  License as published by the Free Software Foundation; either           *
 *  version 2.1 of the License, or (at your option) any later version.     *
 *                                                                         *
 *  This library is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
 *  Lesser General Public License for more details.                        *
 *                                                                         *
 *  You should have received a copy of the GNU Lesser General Public       *
 *  License along with this library; If not, see                           *
 *  <http://www.gnu.org/licenses/>.                                        *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifdef BLUETOOTH_LE

#include <QPointer>
#include <QLowEnergyService>
#include <QtMath>
#include "extern-plugininfo.h"
#include "sensortag.h"

SensorTag::SensorTag(const QBluetoothDeviceInfo &deviceInfo, const QLowEnergyController::RemoteAddressType &addressType, QObject *parent) :
    BluetoothLowEnergyDevice(deviceInfo, addressType, parent)
{
    connect(this, SIGNAL(connectionStatusChanged()), this,SLOT(onConnectionStatusChanged()));
    connect(this, SIGNAL(servicesDiscoveryFinished()), this, SLOT(setupServices()));
}

double SensorTag::calculateMeanValue(const QList<double> &list)
{
    double sum = 0;
    foreach (const double &value, list)
        sum += value;

    return sum / list.count();
}

void SensorTag::setupServices()
{
    foreach (auto id, m_services.keys()) {
        if (!controller()->services().contains(id)) {
            qCWarning(dcMultiSensor) << "Service not found for device" << name() << address().toString();
            return;
        }

        if (m_services.value(id)) {
            qCWarning(dcMultiSensor) << "Attention! bad implementation of service handling!!";
            return;
        }

        qCDebug(dcMultiSensor) << "Setup service";

        // service for temperature
        QSharedPointer<QLowEnergyService> service{controller()->createServiceObject(id, this)};

        if (service.isNull()) {
            qCWarning(dcMultiSensor) << "Could not create service for device" << name() << address().toString();
            return;
        }

        m_services.insert(id, service);

        connect(service.data(), SIGNAL(error(QLowEnergyService::ServiceError)), this, SLOT(onServiceError(QLowEnergyService::ServiceError)));
        connect(service.data(), &QLowEnergyService::stateChanged, this, &SensorTag::onServiceStateChanged);
        connect(service.data(), &QLowEnergyService::characteristicChanged, this, &SensorTag::onServiceCharacteristicChanged);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 5, 0))
        connect(service.data(), &QLowEnergyService::characteristicRead, this, &SensorTag::onServiceCharacteristicChanged);
#endif

        service->discoverDetails();
    }
}

void SensorTag::onConnectionStatusChanged()
{
    if (!isConnected()) {
        // delete the services, they need to be recreated and
        // rediscovered once the device will be reconnected
        foreach (QSharedPointer<QLowEnergyService> service, m_services)
            service->deleteLater();

        emit valueChanged(connectedStateTypeId, false);
    } else {
        emit valueChanged(connectedStateTypeId, true);
    }
}

void SensorTag::onServiceStateChanged(const QLowEnergyService::ServiceState &state)
{
    QPointer<QLowEnergyService> service = qobject_cast<QLowEnergyService *>(sender());

    switch (state) {
    case QLowEnergyService::DiscoveringServices:
        qCDebug(dcMultiSensor) << "Start discovering service" << service->serviceUuid();
        break;
    case QLowEnergyService::ServiceDiscovered:
        if (m_services.contains(service->serviceUuid())) {
            qCDebug(dcMultiSensor) << "... service discovered.";

            auto dataId = service->serviceUuid();
            dataId.data1 += 1;
            auto sensorCharacteristic = service->characteristic(dataId);

            if (!sensorCharacteristic.isValid()) {
                qCWarning(dcMultiSensor) << "Characteristic not found for device " << name() << address().toString();
                break;
            }

            const auto notificationDescriptor = sensorCharacteristic.descriptor(
                        QBluetoothUuid::ClientCharacteristicConfiguration);

            if (notificationDescriptor.isValid()) {
                service->writeDescriptor(notificationDescriptor, QByteArray::fromHex("0100"));
                qCDebug(dcMultiSensor) << "Measuring";
            }

            if (service->serviceUuid().data1 == 0xffe0)
                break;

            auto configId = service->serviceUuid();
            configId.data1 += 2;
            auto sensorConfig = service->characteristic(configId);

            if (!sensorConfig.isValid()) {
                qCWarning(dcMultiSensor) << "Characteristic not found for device " << name() << address().toString();
                break;
            }

            if (service->serviceUuid().data1 == 0xf000aa50) {
                service->writeCharacteristic(sensorConfig, QByteArray::fromHex("07"));
            } else {
                service->writeCharacteristic(sensorConfig, QByteArray::fromHex("01"));
            }

            if (service->serviceUuid().data1 == 0xf000aa40) {
                service->writeCharacteristic(sensorConfig, QByteArray::fromHex("02"));
                auto calibId  = service->serviceUuid();
                calibId.data1 += 3;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 5, 0))
                service->readCharacteristic(service->characteristic(calibId));
#endif
            }

            // TODO: initialize the states with the current value

        }
        break;
    default:
        break;
    }
}

inline quint16 buildUINT16(quint8 loByte, quint8 hiByte) {
    return ((quint16)(((loByte) & 0x00FF) + (((hiByte) & 0x00FF) << 8)));
}

void SensorTag::onServiceCharacteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &value)
{
    qCDebug(dcMultiSensor) << "Service characteristic changed" << characteristic.uuid().toString() << value.toHex();

    auto id = characteristic.uuid();
    id.data1 -= 1;
    switch (characteristic.uuid().data1) {
    case 0xf000aa01: {
        const quint16 *data = reinterpret_cast<const quint16 *>(value.constData());
        qint16 rawTamb = data[1];
        m_temperatureValues.append((double)rawTamb / 128);
        if (m_temperatureValues.count() % 60 == 0) {
            emit valueChanged(temperatureStateTypeId, calculateMeanValue(m_temperatureValues));
            m_temperatureValues.clear();
        }

        qint16 Vobj2 = (double)data[0];
        Vobj2 *= 0.00000015625;
        double Tdie2 = ((double)rawTamb / 128) + 273.15;
        const double S0 = 6.4E-14;            // Calibration factor
        const double a1 = 1.75E-3;
        const double a2 = -1.678E-5;
        const double b0 = -2.94E-5;
        const double b1 = -5.7E-7;
        const double b2 = 4.63E-9;
        const double c2 = 13.4;
        const double Tref = 298.15;
        double S = S0*(1+a1*(Tdie2 - Tref)+a2*qPow((Tdie2 - Tref),2));
        double Vos = b0 + b1*(Tdie2 - Tref) + b2*qPow((Tdie2 - Tref),2);
        double fObj = (Vobj2 - Vos) + c2*qPow((Vobj2 - Vos),2);
        double tObj = qPow(qPow(Tdie2,4) + (fObj/S),.25);
        m_irTemperatureValues.append(tObj - 273.15);
        if (m_irTemperatureValues.count() % 60 == 0) {
            emit valueChanged(IRtemperatureStateTypeId, calculateMeanValue(m_irTemperatureValues));
            m_irTemperatureValues.clear();
        }

        break;
    }
    case 0xf000aa11: {
        // TODO: evaluate movement

        //        const qint8 *data = reinterpret_cast<const qint8 *>(value.constData());
        //        emit valueChanged(accelerationXStateTypeId, (double)data[0] / 64);
        //        emit valueChanged(accelerationYStateTypeId, (double)data[1] / 64);
        //        emit valueChanged(accelerationZStateTypeId, (double)data[2] / 64);
        break;
    }
    case 0xf000aa21: {
        const quint16 *data = reinterpret_cast<const quint16 *>(value.constData());
        quint16 rawH = data[1];
        rawH &= ~0x0003;
        m_humidityValues.append(-6.0 + 125.0/65536 * (double)rawH);
        if (m_humidityValues.count() % 60 == 0) {
            emit valueChanged(humidityStateTypeId, calculateMeanValue(m_humidityValues));
            m_humidityValues.clear();
        }
        break;
    }
    case 0xf000aa31: {
        // TODO: evaluate movement
        //        const qint16 *data = reinterpret_cast<const qint16 *>(value.constData());
        //        emit valueChanged(magneticFieldXStateTypeId, (double)data[0] / 32.768);
        //        emit valueChanged(magneticFieldYStateTypeId, (double)data[1] / 32.768);
        //        emit valueChanged(magneticFieldZStateTypeId, (double)data[2] / 32.768);
        break;
    }
    case 0xf000aa41: {
        if (m_c.empty())
            break;
        const quint16 *data = reinterpret_cast<const quint16 *>(value.constData());
        quint16 Pr = data[1];
        qint16 Tr = data[0];
        // Sensitivity
        qint64 s = (qint64)m_c[2];
        qint64 val = (qint64)m_c[3] * Tr;
        s += (val >> 17);
        val = (qint64)m_c2[0] * Tr * Tr;
        s += (val >> 34);
        // Offset
        qint64 o = (qint64)m_c2[1] << 14;
        val = (qint64)m_c2[2] * Tr;
        o += (val >> 3);
        val = (qint64)m_c2[3] * Tr * Tr;
        o += (val >> 19);
        // Pressure (Pa)
        qint64 pres = ((qint64)(s * Pr) + o) >> 14;
        m_pressureValues.append((double)pres/100);
        if (m_pressureValues.count() % 60 == 0) {
            emit valueChanged(pressureStateTypeId, calculateMeanValue(m_pressureValues));
            m_pressureValues.clear();
        }

        break;
    }
    case 0xf000aa43: {
        const quint8 *data = reinterpret_cast<const quint8 *>(value.constData());
        m_c.resize(4);
        m_c2.resize(4);
        m_c[0] = buildUINT16(data[0],data[1]);
        m_c[1] = buildUINT16(data[2],data[3]);
        m_c[2] = buildUINT16(data[4],data[5]);
        m_c[3] = buildUINT16(data[6],data[7]);
        m_c2[0] = buildUINT16(data[8],data[9]);
        m_c2[1] = buildUINT16(data[10],data[11]);
        m_c2[2] = buildUINT16(data[12],data[13]);
        m_c2[3] = buildUINT16(data[14],data[15]);
        break;
    }
    case 0xf000aa51: {
        // TODO: evaluate movement
        //        const qint16 *data = reinterpret_cast<const qint16 *>(value.constData());
        //        emit valueChanged(rotationXStateTypeId, (double)data[0] / 131.072);
        //        emit valueChanged(rotationYStateTypeId, (double)data[1] / 131.072);
        //        emit valueChanged(rotationZStateTypeId, (double)data[2] / 131.072);
        break;
    }
    case 0xffe1: {
        const quint8 *data = reinterpret_cast<const quint8 *>(value.constData());
        if (*data & 1)
            emit event(leftKeyEventTypeId);
        if (*data & 2)
            emit event(rightKeyEventTypeId);
        break;
    }
    default:
        break;
    }
}

void SensorTag::onServiceError(const QLowEnergyService::ServiceError &error)
{
    QString errorString;
    switch (error) {
    case QLowEnergyService::NoError:
        errorString = "No error";
        break;
    case QLowEnergyService::OperationError:
        errorString = "Operation error";
        break;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 5, 0))
    case QLowEnergyService::CharacteristicReadError:
        errorString = "Characteristic read error";
        break;
#endif
    case QLowEnergyService::CharacteristicWriteError:
        errorString = "Characteristic write error";
        break;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 5, 0))
    case QLowEnergyService::DescriptorReadError:
        errorString = "Descriptor read error";
        break;
#endif
    case QLowEnergyService::DescriptorWriteError:
        errorString = "Descriptor write error";
        break;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 5, 0))
    case QLowEnergyService::UnknownError:
        errorString = "Unknown error";
        break;
#endif
    default:
        errorString = "Unknown error";
        break;
    }


    qCWarning(dcMultiSensor) << "Service of " << name() << address().toString() << ":" << errorString;
}

#endif // BLUETOOTH_LE
