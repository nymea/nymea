#ifdef BLUETOOTH_LE

#include <QPointer>
#include <QLowEnergyService>
#include "extern-plugininfo.h"
#include "sensortag.h"

SensorTag::SensorTag(const QBluetoothDeviceInfo &deviceInfo, const QLowEnergyController::RemoteAddressType &addressType, QObject *parent) :
    BluetoothLowEnergyDevice(deviceInfo, addressType, parent)
{
    m_temperatureServiceUuid = QBluetoothUuid(QUuid("f000aa00-0451-4000-b000-000000000000"));
    m_temperatureCharacteristicUuid = QBluetoothUuid(QUuid("f000aa01-0451-4000-b000-000000000000"));

    connect(this, SIGNAL(connectionStatusChanged()), this,SLOT(onConnectionStatusChanged()));
    connect(this, SIGNAL(servicesDiscoveryFinished()), this, SLOT(setupServices()));
}

void SensorTag::setupServices()
{
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
}

void SensorTag::onConnectionStatusChanged()
{
    if (!isConnected()) {
        // delete the services, they need to be recreated and
        // rediscovered once the device will be reconnected
        delete m_temperatureService;
        m_temperatureService = 0;

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
        // check which service is discovered
        if (service->serviceUuid() == m_temperatureService->serviceUuid()) {
            qCDebug(dcMultiSensor) << "...temperature service discovered.";

            QLowEnergyCharacteristic config = m_temperatureService->characteristic(
                        QBluetoothUuid(QUuid("f000aa02-0451-4000-b000-000000000000")));

            if (!config.isValid()) {
                qCWarning(dcMultiSensor) << "ERROR: temperature characteristc not found for device " << name() << address().toString();
                return;
            }

            service->writeCharacteristic(config, QByteArray::fromHex("01"));

            m_temperatureCharacteristic = m_temperatureService->characteristic(m_temperatureCharacteristicUuid);

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
            //m_imageService->discoverDetails();
        }
/*
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
