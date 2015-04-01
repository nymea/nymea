#include "aveabulb.h"


AveaBulb::AveaBulb(const QBluetoothDeviceInfo &deviceInfo, const QLowEnergyController::RemoteAddressType &addressType, QObject *parent) :
    BluetoothLowEnergyDevice(deviceInfo, addressType, parent),
    m_colorService(0),
    m_imageService(0)
{
    m_colorSeviceUuid = QBluetoothUuid(QUuid("f815e810-456c-6761-746f-4d756e696368"));
    m_colorCharacteristicUuid = QBluetoothUuid(QUuid("f815e811-456c-6761-746f-4d756e696368"));

    m_imageSeviceUuid = QBluetoothUuid(QUuid("f815e500-456c-6761-746f-4d756e696368"));
    m_imageCharacteristicUuid = QBluetoothUuid(QUuid("f815e501-456c-6761-746f-4d756e696368"));

    connect(this, SIGNAL(connectionStatusChanged()), this,SLOT(onConnectionStatusChanged()));
    connect(this, SIGNAL(servicesDiscoveryFinished()), this, SLOT(serviceScanFinished()));
}

bool AveaBulb::isAvailable()
{
    return m_isAvailable;
}

void AveaBulb::serviceScanFinished()
{
    if (!controller()->services().contains(m_colorSeviceUuid)) {
        qWarning() << "ERROR: Color service not found for device" << name() << address().toString();
        return;
    }

    if (m_colorService || m_imageService) {
        qWarning() << "ERROR: Attention! bad implementation of service handling!!";
        return;
    }

    // service for colors
    m_colorService = controller()->createServiceObject(m_colorSeviceUuid, this);

    if (!m_colorService) {
        qWarning() << "ERROR: could not create color service for device" << name() << address().toString();
        return;
    }

    connect(m_colorService, SIGNAL(stateChanged(QLowEnergyService::ServiceState)), this, SLOT(serviceStateChanged(QLowEnergyService::ServiceState)));
    connect(m_colorService, SIGNAL(characteristicChanged(QLowEnergyCharacteristic, QByteArray)), this, SLOT(serviceCharacteristicChanged(QLowEnergyCharacteristic, QByteArray)));
    connect(m_colorService, SIGNAL(characteristicWritten(QLowEnergyCharacteristic, QByteArray)), this, SLOT(confirmedCharacteristicWritten(QLowEnergyCharacteristic, QByteArray)));
    connect(m_colorService, SIGNAL(descriptorWritten(QLowEnergyDescriptor, QByteArray)), this, SLOT(confirmedDescriptorWritten(QLowEnergyDescriptor, QByteArray)));
    connect(m_colorService, SIGNAL(error(QLowEnergyService::ServiceError)), this, SLOT(serviceError(QLowEnergyService::ServiceError)));

    // service for images
    m_imageService = controller()->createServiceObject(m_imageSeviceUuid, this);

    if (!m_imageService) {
        qWarning() << "ERROR: could not create color service for device" << name() << address().toString();
        return;
    }

    connect(m_imageService, SIGNAL(stateChanged(QLowEnergyService::ServiceState)), this, SLOT(serviceStateChanged(QLowEnergyService::ServiceState)));
    connect(m_imageService, SIGNAL(characteristicChanged(QLowEnergyCharacteristic, QByteArray)), this, SLOT(serviceCharacteristicChanged(QLowEnergyCharacteristic, QByteArray)));
    connect(m_imageService, SIGNAL(characteristicWritten(QLowEnergyCharacteristic, QByteArray)), this, SLOT(confirmedCharacteristicWritten(QLowEnergyCharacteristic, QByteArray)));
    connect(m_imageService, SIGNAL(descriptorWritten(QLowEnergyDescriptor, QByteArray)), this, SLOT(confirmedDescriptorWritten(QLowEnergyDescriptor, QByteArray)));
    connect(m_imageService, SIGNAL(error(QLowEnergyService::ServiceError)), this, SLOT(serviceError(QLowEnergyService::ServiceError)));

    m_colorService->discoverDetails();
}

void AveaBulb::onConnectionStatusChanged()
{
    if (!isConnected()) {
        // delete the services, they need to be recreated and
        // rediscovered once the device will be reconnected
        delete m_colorService;
        m_colorService = 0;

        delete m_imageService;
        m_imageService = 0;

        m_isAvailable = false;
        emit availableChanged();
    }
}

void AveaBulb::serviceStateChanged(const QLowEnergyService::ServiceState &state)
{
    QLowEnergyService *service =static_cast<QLowEnergyService *>(sender());

    switch (state) {
    case QLowEnergyService::DiscoveringServices:
        if (service->serviceUuid() == m_colorService->serviceUuid()) {
            qDebug() << "start discovering color service...";
        } else if (service->serviceUuid() == m_imageService->serviceUuid()) {
            qDebug() << "start discovering image service...";
        }
        break;
    case QLowEnergyService::ServiceDiscovered:
        // check which service is discovered
        if (service->serviceUuid() == m_colorService->serviceUuid()) {
            qDebug() << "...color service discovered.";

            m_colorCharacteristic = m_colorService->characteristic(m_colorCharacteristicUuid);

            if (!m_colorCharacteristic.isValid()) {
                qWarning() << "ERROR: color characteristc not found for device " << name() << address().toString();
                return;
            }

            m_imageService->discoverDetails();
        }
        if (service->serviceUuid() == m_imageService->serviceUuid()) {
            qDebug() << "...image service discovered.";

            m_imageCharacteristic = m_imageService->characteristic(m_imageCharacteristicUuid);

            if (!m_imageCharacteristic.isValid()) {
                qWarning() << "ERROR: image characteristc not found for device " << name() << address().toString();
                return;
            }
        }

        if (m_colorService->state() == QLowEnergyService::ServiceDiscovered && m_imageService->state() == QLowEnergyService::ServiceDiscovered) {
            m_isAvailable = true;
            emit availableChanged();
        }
        break;
    default:
        break;
    }
}

void AveaBulb::serviceCharacteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &value)
{
    qDebug() << "service characteristic changed" << characteristic.name() << value.toHex();
}

void AveaBulb::confirmedCharacteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &value)
{
    if (characteristic.handle() == m_colorCharacteristic.handle()) {
        qDebug() << "color char written successfully" << value.toHex();
        if (m_actions.contains(value.toHex())) {
            ActionId actionId = m_actions.take(value.toHex());
            emit actionExecutionFinished(actionId, true);
        }
    } else if (characteristic.handle() == m_imageCharacteristic.handle()) {
        qDebug() << "image char written successfully" << value.toHex();
    }

}

void AveaBulb::confirmedDescriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &value)
{
    qDebug() << "descriptor:" << descriptor.name() << "value:" << value.toHex() << "written successfully";
}

void AveaBulb::serviceError(const QLowEnergyService::ServiceError &error)
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

    qWarning() << "ERROR: color service of " << name() << address().toString() << ":" << errorString;
}

bool AveaBulb::enableNotification()
{
    if (!isAvailable())
        return false;

    qDebug() << "enable notify";
    m_colorService->writeCharacteristic(m_colorCharacteristic, QByteArray::fromHex("0100"), QLowEnergyService::WriteWithResponse);

    return true;
}

bool AveaBulb::testMethod()
{
    if (!isAvailable())
        return false;

    setZauberwald();
    return true;
}

bool AveaBulb::actionPowerOff(ActionId actionId)
{
    if (!isAvailable())
        return false;

    QByteArray value = "35f4010a00008000b000a00090";
    qDebug() << "set" << name() << "power OFF";
    m_actions.insert(value, actionId);
    m_colorService->writeCharacteristic(m_colorCharacteristic, QByteArray::fromHex(value), QLowEnergyService::WriteWithResponse);

    return true;
}

bool AveaBulb::setWhite(ActionId actionId)
{
    if (!isAvailable())
        return false;

    QByteArray value = "3532000a00ff0f003000200010";
    qDebug() << "set" << name() << "white";
    m_actions.insert(value, actionId);
    m_colorService->writeCharacteristic(m_colorCharacteristic, QByteArray::fromHex(value), QLowEnergyService::WriteWithResponse);

    return true;
}

bool AveaBulb::setZauberwald()
{
    if (!isAvailable())
        return false;

    m_colorService->writeCharacteristic(m_colorCharacteristic, QByteArray::fromHex("340040143058220010"), QLowEnergyService::WriteWithResponse);

    QByteArray value = "1500cd0110cd0020cd0030cd2850cd5842cd14608d0a0ecd04708216c200a04d";
    m_imageService->writeCharacteristic(m_imageCharacteristic, QByteArray::fromHex(value), QLowEnergyService::WriteWithResponse);

    value = "1600cd0210cd1280a1646301cd14608d080e"
            "cd9041f190017402b564f200608d0a7d0ecd"
            "6851b1c87402b564f200608d0a7d0ecdff47"
            "cd6450cdb470823c44028564c200604d8d0a"
            "0ecd17a0";
    m_imageService->writeCharacteristic(m_imageCharacteristic, QByteArray::fromHex(value), QLowEnergyService::WriteWithResponse);

    value = "1700cd0310cd1280a1646301cde041cd2850"
            "b1f07402b564f200608d0a7d0ecd04708216"
            "c200a04d";
    m_imageService->writeCharacteristic(m_imageCharacteristic, QByteArray::fromHex(value), QLowEnergyService::WriteWithResponse);

    value = "1800cd0410cd1280a1646301cdb841cdb470"
            "823c7402b564c200604d8d0a0ecd0e41cd28"
            "50b1c87402b564f200608d0a7d0ecd6440f1"
            "90017402b564f200608d0a7d0e8d0a0ecd04"
            "708216c200a04d";
    m_imageService->writeCharacteristic(m_imageCharacteristic, QByteArray::fromHex(value), QLowEnergyService::WriteWithResponse);

    value = "1900cd0510cd1280a1646301cd6841cdb470"
            "823cc200604d8d0a0ecda050f12c017402b5"
            "64f200608d0a7d0ecdf040cd2850f1680174"
            "02b564f200608d0a7d0ecdcc41b1c87402b5"
            "64f200608d0a7d0ecd04708216c200a04d";
    m_imageService->writeCharacteristic(m_imageCharacteristic, QByteArray::fromHex(value), QLowEnergyService::WriteWithResponse);

    m_colorService->writeCharacteristic(m_colorCharacteristic, QByteArray::fromHex("2a15"), QLowEnergyService::WriteWithResponse);

    return true;
}
