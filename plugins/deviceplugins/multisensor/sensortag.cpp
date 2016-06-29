#ifdef BLUETOOTH_LE

#include <QPointer>
#include <QLowEnergyService>
#include "extern-plugininfo.h"
#include "sensortag.h"

SensorTag::SensorTag(const QBluetoothDeviceInfo &deviceInfo, const QLowEnergyController::RemoteAddressType &addressType, QObject *parent) :
    BluetoothLowEnergyDevice(deviceInfo, addressType, parent)
{
    //m_temperatureServiceUuid = QBluetoothUuid(QUuid("f000aa30-0451-4000-b000-000000000000"));
    //m_temperatureCharacteristicUuid = QBluetoothUuid(QUuid("f000aa31-0451-4000-b000-000000000000"));

    connect(this, SIGNAL(connectionStatusChanged()), this,SLOT(onConnectionStatusChanged()));
    connect(this, SIGNAL(servicesDiscoveryFinished()), this, SLOT(setupServices()));
}

void SensorTag::setupServices()
{
    foreach(auto id, m_services.keys())
    {
        if (!controller()->services().contains(id)) {
            qCWarning(dcMultiSensor) << "ERROR: temperature service not found for device" << name() << address().toString();
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
            qCWarning(dcMultiSensor) << "ERROR: could not create temperature service for device" << name() << address().toString();
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
/*
    if (!controller()->services().contains(m_temperatureServiceUuid)) {
        qCWarning(dcMultiSensor) << "ERROR: temperature service not found for device" << name() << address().toString();
        return;
    }

    if (m_temperatureService) {
        qCWarning(dcMultiSensor) << "ERROR: Attention! bad implementation of service handling!!";
        return;
    }

    qCDebug(dcMultiSensor) << "Setup service";

    // service for temperature
    m_temperatureService = controller()->createServiceObject(m_temperatureServiceUuid, this);

    if (!m_temperatureService) {
        qCWarning(dcMultiSensor) << "ERROR: could not create temperature service for device" << name() << address().toString();
        return;
    }

    connect(m_temperatureService, SIGNAL(stateChanged(QLowEnergyService::ServiceState)), this, SLOT(onServiceStateChanged(QLowEnergyService::ServiceState)));
    connect(m_temperatureService, SIGNAL(characteristicChanged(QLowEnergyCharacteristic, QByteArray)), this, SLOT(onServiceCharacteristicChanged(QLowEnergyCharacteristic, QByteArray)));
    //connect(m_temperatureService, SIGNAL(characteristicWritten(QLowEnergyCharacteristic, QByteArray)), this, SLOT(confirmedCharacteristicWritten(QLowEnergyCharacteristic, QByteArray)));
    //connect(m_temperatureService, SIGNAL(descriptorWritten(QLowEnergyDescriptor, QByteArray)), this, SLOT(confirmedDescriptorWritten(QLowEnergyDescriptor, QByteArray)));
    connect(m_temperatureService, SIGNAL(error(QLowEnergyService::ServiceError)), this, SLOT(onServiceError(QLowEnergyService::ServiceError)));

    m_temperatureService->discoverDetails();

    m_temperatureService2 = controller()->createServiceObject(QBluetoothUuid(QUuid("f000aa20-0451-4000-b000-000000000000")), this);

    if (!m_temperatureService) {
        qCWarning(dcMultiSensor) << "ERROR: could not create temperature service for device" << name() << address().toString();
        return;
    }

    connect(m_temperatureService2, SIGNAL(stateChanged(QLowEnergyService::ServiceState)), this, SLOT(onServiceStateChanged(QLowEnergyService::ServiceState)));
    connect(m_temperatureService2, SIGNAL(characteristicChanged(QLowEnergyCharacteristic, QByteArray)), this, SLOT(onServiceCharacteristicChanged(QLowEnergyCharacteristic, QByteArray)));
    //connect(m_temperatureService, SIGNAL(characteristicWritten(QLowEnergyCharacteristic, QByteArray)), this, SLOT(confirmedCharacteristicWritten(QLowEnergyCharacteristic, QByteArray)));
    //connect(m_temperatureService, SIGNAL(descriptorWritten(QLowEnergyDescriptor, QByteArray)), this, SLOT(confirmedDescriptorWritten(QLowEnergyDescriptor, QByteArray)));
    connect(m_temperatureService2, SIGNAL(error(QLowEnergyService::ServiceError)), this, SLOT(onServiceError(QLowEnergyService::ServiceError)));

        m_temperatureService2->discoverDetails();
        */
}

void SensorTag::onConnectionStatusChanged()
{
    if (!isConnected()) {
        // delete the services, they need to be recreated and
        // rediscovered once the device will be reconnected
        foreach (auto id, m_services.keys())
            m_services[id].clear();

        //delete m_temperatureService;
        //m_temperatureService = 0;

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
            qCDebug(dcMultiSensor) << "...temperature service discovered.";

            auto configId = service->serviceUuid();
            configId.data1 += 2;
            auto sensorConfig = service->characteristic(configId);

            if (!sensorConfig.isValid()) {
                qCWarning(dcMultiSensor) << "ERROR: temperature characteristc not found for device " << name() << address().toString();
                return;
            }

            service->writeCharacteristic(sensorConfig, QByteArray::fromHex("01"));

            auto dataId = service->serviceUuid();
            dataId.data1 += 1;
            auto sensorCharacteristic = service->characteristic(dataId);

            if (!sensorCharacteristic.isValid()) {
                qCWarning(dcMultiSensor) << "ERROR: temperature characteristc not found for device " << name() << address().toString();
                return;
            }

            //service->setProperty()
            //m_temperatureCharacteristic.

            const auto notificationDescriptor = sensorCharacteristic.descriptor(
                        QBluetoothUuid::ClientCharacteristicConfiguration);

            if (notificationDescriptor.isValid()) {
                service->writeDescriptor(notificationDescriptor, QByteArray::fromHex("0100"));
                qCDebug(dcMultiSensor) << "Measuring";
            }
            //m_imageService->discoverDetails();
        }
        /*
        // check which service is discovered
        if (service->serviceUuid() == m_temperatureService->serviceUuid()) {
            qCDebug(dcMultiSensor) << "...temperature service discovered.";

            QLowEnergyCharacteristic config = service->characteristic(
                        QBluetoothUuid(QUuid("f000aa32-0451-4000-b000-000000000000")));

            if (!config.isValid()) {
                qCWarning(dcMultiSensor) << "ERROR: temperature characteristc not found for device " << name() << address().toString();
                return;
            }

            service->writeCharacteristic(config, QByteArray::fromHex("01"));

            m_temperatureCharacteristic = service->characteristic(m_temperatureCharacteristicUuid);

            if (!m_temperatureCharacteristic.isValid()) {
                qCWarning(dcMultiSensor) << "ERROR: temperature characteristc not found for device " << name() << address().toString();
                return;
            }

            //service->setProperty()
            //m_temperatureCharacteristic.

            const QLowEnergyDescriptor m_notificationDesc = m_temperatureCharacteristic.descriptor(
                        QBluetoothUuid::ClientCharacteristicConfiguration);

            if (m_notificationDesc.isValid()) {
                service->writeDescriptor(m_notificationDesc, QByteArray::fromHex("0100"));
                qCDebug(dcMultiSensor) << "Measuring";
            }
            //m_temperatureService->discoverDetails();
        }
        if (service->serviceUuid() == QBluetoothUuid(QUuid("f000aa20-0451-4000-b000-000000000000"))) {
            qCDebug(dcMultiSensor) << "...temperature service discovered.";

            QLowEnergyCharacteristic config = service->characteristic(
                        QBluetoothUuid(QUuid("f000aa22-0451-4000-b000-000000000000")));

            if (!config.isValid()) {
                qCWarning(dcMultiSensor) << "ERROR: temperature characteristc not found for device " << name() << address().toString();
                return;
            }

            service->writeCharacteristic(config, QByteArray::fromHex("01"));

            QLowEnergyCharacteristic m_temperatureCharacteristic2 = service->characteristic(QBluetoothUuid(QUuid("f000aa21-0451-4000-b000-000000000000")));

            if (!m_temperatureCharacteristic2.isValid()) {
                qCWarning(dcMultiSensor) << "ERROR: temperature characteristc not found for device " << name() << address().toString();
                return;
            }

            //service->setProperty()
            //m_temperatureCharacteristic.

            const QLowEnergyDescriptor m_notificationDesc = m_temperatureCharacteristic2.descriptor(
                        QBluetoothUuid::ClientCharacteristicConfiguration);

            if (m_notificationDesc.isValid()) {
                service->writeDescriptor(m_notificationDesc, QByteArray::fromHex("0100"));
                qCDebug(dcMultiSensor) << "Measuring";
            }
            //m_imageService->discoverDetails();
        }

        if (m_colorService->state() == QLowEnergyService::ServiceDiscovered && m_imageService->state() == QLowEnergyService::ServiceDiscovered) {
            m_isAvailable = true;
            emit availableChanged();
        }
        */
        break;
    default:
        break;
    }
}

void SensorTag::onServiceCharacteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &value)
{
    qCDebug(dcMultiSensor) << "service characteristic changed" << characteristic.name() << value.toHex();
    const qint16 *data = reinterpret_cast<const qint16 *>(value.constData());
    emit valueChanged((double)(*data)/128.0);
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
