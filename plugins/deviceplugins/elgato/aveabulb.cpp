#include "aveabulb.h"


AveaBulb::AveaBulb(const QBluetoothDeviceInfo &deviceInfo, const QLowEnergyController::RemoteAddressType &addressType, QObject *parent) :
    BluetoothLowEnergyDevice(deviceInfo, addressType, parent),
    m_colorService(0)
{
    m_colorSeviceUuid = QBluetoothUuid(QUuid("f815e810-456c-6761-746f-4d756e696368"));
    m_colorCharacteristicUuid = QBluetoothUuid(QUuid("f815e811-456c-6761-746f-4d756e696368"));

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

    if (m_colorService) {
        qWarning() << "ERROR: Attention! bad implementation of service handling!!";
        return;
    }

    // create color service and discover it
    m_colorService = controller()->createServiceObject(m_colorSeviceUuid, this);

    if (!m_colorService) {
        qWarning() << "ERROR: could not create color service for device" << name() << address().toString();
        return;
    }

    connect(m_colorService, SIGNAL(stateChanged(QLowEnergyService::ServiceState)), this, SLOT(serviceStateChanged(QLowEnergyService::ServiceState)));
    connect(m_colorService, SIGNAL(characteristicChanged(QLowEnergyCharacteristic,QByteArray)), this, SLOT(serviceCharacteristicChanged(QLowEnergyCharacteristic,QByteArray)));
    connect(m_colorService, SIGNAL(characteristicWritten(QLowEnergyCharacteristic,QByteArray)), this, SLOT(confirmedCharacteristicWritten(QLowEnergyCharacteristic,QByteArray)));
    connect(m_colorService, SIGNAL(error(QLowEnergyService::ServiceError)), this, SLOT(serviceError(QLowEnergyService::ServiceError)));

    m_colorService->discoverDetails();
}

void AveaBulb::onConnectionStatusChanged()
{
    if (!isConnected()) {
        // delete the service, needs to be recreatedand rediscovered once the device will be reconnected
        delete m_colorService;
        m_colorService = 0;

        m_isAvailable = false;
        emit availableChanged();
    }
}

void AveaBulb::serviceStateChanged(const QLowEnergyService::ServiceState &state)
{
    switch (state) {
    case QLowEnergyService::ServiceDiscovered:
        m_colorCharacteristic = m_colorService->characteristic(m_colorCharacteristicUuid);

        if (!m_colorCharacteristic.isValid()) {
            qWarning() << "ERROR: color characteristc not found for device " << name() << address().toString();
            return;
        }
        m_isAvailable = true;
        emit availableChanged();
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
        if (m_actions.contains(value.toHex())) {
            ActionId actionId = m_actions.take(value.toHex());
            emit actionExecutionFinished(actionId, true);
        }
    }
}

void AveaBulb::serviceError(const QLowEnergyService::ServiceError &error)
{
    qWarning() << "ERROR: color service of " << name() << address().toString() << ":" << error;
}

bool AveaBulb::enableNotification()
{
    if (!isAvailable())
        return false;

    qDebug() << "enable notify";
    QByteArray value = "0100";
    m_colorService->writeCharacteristic(m_colorCharacteristic, value, QLowEnergyService::WriteWithResponse);

    return true;
}

bool AveaBulb::testMethod()
{
    if (!isAvailable())
        return false;

    QByteArray value = "34";
    qDebug() << "test" << value;
    m_colorService->writeCharacteristic(m_colorCharacteristic, QByteArray::fromHex(value), QLowEnergyService::WriteWithResponse);

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
