#ifdef BLUETOOTH_LE

#include <QPointer>
#include <QLowEnergyService>
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

        connect(service.data(), SIGNAL(stateChanged(QLowEnergyService::ServiceState)), this, SLOT(onServiceStateChanged(QLowEnergyService::ServiceState)));
        connect(service.data(), SIGNAL(characteristicChanged(QLowEnergyCharacteristic, QByteArray)), this, SLOT(onServiceCharacteristicChanged(QLowEnergyCharacteristic, QByteArray)));
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
        foreach (auto id, m_services.keys())
            m_services[id].clear();

        //m_commandQueue.clear();

        //m_isAvailable = false;
        //emit availableChanged();
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

            auto configId = service->serviceUuid();
            configId.data1 += 2;
            auto sensorConfig = service->characteristic(configId);

            if (!sensorConfig.isValid()) {
                qCWarning(dcMultiSensor) << "ERROR: characteristc not found for device " << name() << address().toString();
                return;
            }

            service->writeCharacteristic(sensorConfig, QByteArray::fromHex("01"));

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
        }
        break;
    default:
        break;
    }
}

void SensorTag::onServiceCharacteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &value)
{
    qCDebug(dcMultiSensor) << "service characteristic changed" << characteristic.uuid().toString() << value.toHex();

    switch (characteristic.uuid().data1) {
    case 0xf000aa21: {
        const quint16 *data = reinterpret_cast<const quint16 *>(value.constData());
        quint16 rawH = *data;
        rawH &= ~0x0003;
        emit valueChanged(humidityStateTypeId, -6.0 + 125.0/65536 * (double)rawH);
        break;
    }
    case 0xf000aa41: {
        const quint16 *data = reinterpret_cast<const quint16 *>(value.constData());
        quint16 rawH = *data;
        rawH &= ~0x0003;
        emit valueChanged(pressureStateTypeId, -6.0 + 125.0/65536 * (double)rawH);
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
