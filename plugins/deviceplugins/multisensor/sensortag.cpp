/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.guru>                 *
 *  Copyright (C) 2016 nicc                                                *
 *                                                                         *
 *  This file is part of guh.                                              *
 *                                                                         *
 *  Guh is free software: you can redistribute it and/or modify            *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  Guh is distributed in the hope that it will be useful,                 *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with guh. If not, see <http://www.gnu.org/licenses/>.            *
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

void SensorTag::setupServices()
{
    foreach(auto id, m_services.keys())
    {
        if (!controller()->services().contains(id)) {
            qCWarning(dcMultiSensor) << "ERROR: service not found for device" << name() << address().toString();
            return;
        }

        if (m_services.value(id)) {
            qCWarning(dcMultiSensor) << "ERROR: Attention! bad implementation of service handling!!";
            return;
        }

        qCDebug(dcMultiSensor) << "Setup service";

        // service for temperature
        QSharedPointer<QLowEnergyService> service{controller()->createServiceObject(id, this)};

        if (!service) {
            qCWarning(dcMultiSensor) << "ERROR: could not create service for device" << name() << address().toString();
            return;
        }

        m_services.insert(id, service);

        connect(service.data(), &QLowEnergyService::stateChanged, this, &SensorTag::onServiceStateChanged);
        connect(service.data(), &QLowEnergyService::characteristicChanged, this, &SensorTag::onServiceCharacteristicChanged);
        connect(service.data(), &QLowEnergyService::characteristicRead, this, &SensorTag::onServiceCharacteristicChanged);
        //connect(m_temperatureService, SIGNAL(characteristicWritten(QLowEnergyCharacteristic, QByteArray)), this, SLOT(confirmedCharacteristicWritten(QLowEnergyCharacteristic, QByteArray)));
        //connect(m_temperatureService, SIGNAL(descriptorWritten(QLowEnergyDescriptor, QByteArray)), this, SLOT(confirmedDescriptorWritten(QLowEnergyDescriptor, QByteArray)));
        connect(service.data(), SIGNAL(error(QLowEnergyService::ServiceError)), this, SLOT(onServiceError(QLowEnergyService::ServiceError)));


        service->discoverDetails();
    }
}

void SensorTag::onConnectionStatusChanged()
{
    if (!isConnected()) {
        // delete the services, they need to be recreated and
        // rediscovered once the device will be reconnected
        foreach (auto service, m_services)
            service.clear();

        //m_commandQueue.clear();

        //m_isAvailable = false;
        emit valueChanged(availableStateTypeId, false);
    }
}

void SensorTag::onServiceStateChanged(const QLowEnergyService::ServiceState &state)
{
    QPointer<QLowEnergyService> service = qobject_cast<QLowEnergyService *>(sender());

    switch (state) {
    case QLowEnergyService::DiscoveringServices:
        qCDebug(dcMultiSensor) << "start discovering service" << service->serviceUuid();
        break;
    case QLowEnergyService::ServiceDiscovered:
        if (m_services.contains(service->serviceUuid())) {
            qCDebug(dcMultiSensor) << "... service discovered.";

            auto dataId = service->serviceUuid();
            dataId.data1 += 1;
            auto sensorCharacteristic = service->characteristic(dataId);

            if (!sensorCharacteristic.isValid()) {
                qCWarning(dcMultiSensor) << "ERROR: characteristic not found for device " << name() << address().toString();
                return;
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
                qCWarning(dcMultiSensor) << "ERROR: characteristic not found for device " << name() << address().toString();
                return;
            }

            service->writeCharacteristic(sensorConfig, QByteArray::fromHex("01"));

            if (service->serviceUuid().data1 == 0xf000aa40) {
                service->writeCharacteristic(sensorConfig, QByteArray::fromHex("02"));
                auto calibId  = service->serviceUuid();
                calibId.data1 += 3;
                service->readCharacteristic(service->characteristic(calibId));
            }
        }

        if (std::all_of(m_services.begin(), m_services.end(),
                        [](QSharedPointer<QLowEnergyService> service){ return service->state() == QLowEnergyService::ServiceDiscovered; }))
            emit valueChanged(availableStateTypeId, true);
        break;
    default:
        break;
    }
}

void SensorTag::onServiceCharacteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &value)
{
    qCDebug(dcMultiSensor) << "service characteristic changed" << characteristic.uuid().toString() << value.toHex();

    switch (characteristic.uuid().data1) {
    case 0xf000aa01: {
        const quint16 *data = reinterpret_cast<const quint16 *>(value.constData());
        qint16 rawTamb = data[0];
        emit valueChanged(temperatureStateTypeId, (double)rawTamb/128);

        double Vobj2 = (double)data[1];
        Vobj2 *= 0.00000015625;
        double Tdie2 = ((double)rawTamb/128) + 273.15;
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
        tObj = (tObj - 273.15);
        emit valueChanged(IRtemperatureStateTypeId, tObj);
        break;
    }
    case 0xf000aa21: {
        const quint16 *data = reinterpret_cast<const quint16 *>(value.constData());
        quint16 rawH = data[1];
        rawH &= ~0x0003;
        emit valueChanged(humidityStateTypeId, -6.0 + 125.0/65536 * (double)rawH);
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
        emit valueChanged(pressureStateTypeId, (double)pres/100);
        break;
    }
    case 0xf000aa43: {
        const quint16 *data = reinterpret_cast<const quint16 *>(value.constData());
        m_c.resize(4);
        m_c2.resize(4);
        m_c[0] = data[0];
        m_c[1] = data[1];
        m_c[2] = data[2];
        m_c[3] = data[3];
        m_c2[0] = data[4];
        m_c2[1] = data[5];
        m_c2[2] = data[6];
        m_c2[3] = data[7];
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
    case QLowEnergyService::CharacteristicWriteError:
        errorString = "Characteristic write error";
        break;
    case QLowEnergyService::DescriptorWriteError:
        errorString = "Descriptor write error";
        break;
    default:
        break;
    }

    qCWarning(dcMultiSensor) << "ERROR: service of " << name() << address().toString() << ":" << errorString;
}

#endif // BLUETOOTH_LE
