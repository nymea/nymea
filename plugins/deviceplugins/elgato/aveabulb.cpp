/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
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

#include "aveabulb.h"

AveaBulb::AveaBulb(const QBluetoothDeviceInfo &deviceInfo, const QLowEnergyController::RemoteAddressType &addressType, QObject *parent) :
    BluetoothLowEnergyDevice(deviceInfo, addressType, parent),
    m_colorService(0),
    m_imageService(0),
    m_queueRunning(false),
    m_brigthness("57ff0f"),
    m_liveliness("493300")
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

        m_commandQueue.clear();

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
        //qDebug() << "color char written successfully" << value.toHex();
        if (m_actions.contains(value.toHex())) {
            ActionId actionId = m_actions.take(value.toHex());
            emit actionExecutionFinished(actionId, true);
        }
    } else if (characteristic.handle() == m_imageCharacteristic.handle()) {
        //qDebug() << "image char written successfully" << value.toHex();
        if (m_actions.contains(value.toHex())) {
            ActionId actionId = m_actions.take(value.toHex());
            emit actionExecutionFinished(actionId, true);
        }
    }
    if (characteristic.uuid() == m_currentRequest.characteristic().uuid()) {
        if (m_commandQueue.isEmpty()) {
            m_queueRunning = false;
            //qDebug() << "queue finished";
        }
        sendNextCommand();
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

void AveaBulb::enqueueCommand(QLowEnergyService *service, const QLowEnergyCharacteristic &characteristic, const QByteArray &value)
{
    CommandRequest r = CommandRequest(service, characteristic, QByteArray::fromHex(value));
    m_commandQueue.enqueue(r);
}

void AveaBulb::sendNextCommand()
{
    if (!isAvailable()) {
        m_commandQueue.clear();
        return;
    }

    if (m_commandQueue.isEmpty())
        return;

    //qDebug() << "send next command";
    m_queueRunning = true;
    m_currentRequest = m_commandQueue.dequeue();


    QLowEnergyService *service = m_currentRequest.service();
    service->writeCharacteristic(m_currentRequest.characteristic(), m_currentRequest.value());
}

bool AveaBulb::actionPowerOff(ActionId actionId)
{
    if (!isAvailable())
        return false;

    QByteArray value = "35f4010a00008000b000a00090";
    m_actions.insert(value, actionId);
    m_colorService->writeCharacteristic(m_colorCharacteristic, QByteArray::fromHex(value), QLowEnergyService::WriteWithResponse);

    return true;
}

bool AveaBulb::setWhite(ActionId actionId)
{
    // warm white 3514000a00ff0f703bd2259d11
    // cool white 3514000a00ff0f0030a220a31f

    if (!isAvailable())
        return false;

    QByteArray value = "3532000a00ff8f00b058a4899d";
    m_actions.insert(value, actionId);
    m_colorService->writeCharacteristic(m_colorCharacteristic, QByteArray::fromHex(value), QLowEnergyService::WriteWithResponse);

    return true;
}

bool AveaBulb::setRed(ActionId actionId)
{
    if (!isAvailable())
        return false;

    QByteArray value = "3532000a000080ffbf00a0b390";
    m_actions.insert(value, actionId);
    m_colorService->writeCharacteristic(m_colorCharacteristic, QByteArray::fromHex(value), QLowEnergyService::WriteWithResponse);

    return true;
}

bool AveaBulb::setGreen(ActionId actionId)
{
    if (!isAvailable())
        return false;

    QByteArray value = "3514000a00a3005231ff2f0010";
    m_actions.insert(value, actionId);
    m_colorService->writeCharacteristic(m_colorCharacteristic, QByteArray::fromHex(value), QLowEnergyService::WriteWithResponse);

    return true;
}

bool AveaBulb::setBlue(ActionId actionId)
{
    if (!isAvailable())
        return false;

    QByteArray value = "3532000a00c8805fb6ada2ff9f";
    m_actions.insert(value, actionId);
    m_colorService->writeCharacteristic(m_colorCharacteristic, QByteArray::fromHex(value), QLowEnergyService::WriteWithResponse);

    return true;
}

bool AveaBulb::setYellow(ActionId actionId)
{
    if (!isAvailable())
        return false;

    QByteArray value = "3532000a001481e2bfffaf0090";
    m_actions.insert(value, actionId);
    m_colorService->writeCharacteristic(m_colorCharacteristic, QByteArray::fromHex(value), QLowEnergyService::WriteWithResponse);

    return true;
}

bool AveaBulb::setPurple(ActionId actionId)
{
    if (!isAvailable())
        return false;

    QByteArray value = "3532000a000080f6b700a02498";
    m_actions.insert(value, actionId);
    m_colorService->writeCharacteristic(m_colorCharacteristic, QByteArray::fromHex(value), QLowEnergyService::WriteWithResponse);

    return true;
}

bool AveaBulb::setOrange(ActionId actionId)
{
    if (!isAvailable())
        return false;

    QByteArray value = "3532000a000080ffbf36aa0090";
    m_actions.insert(value, actionId);
    m_colorService->writeCharacteristic(m_colorCharacteristic, QByteArray::fromHex(value), QLowEnergyService::WriteWithResponse);

    return true;
}

bool AveaBulb::setCalmProvence(ActionId actionId)
{
    if (!isAvailable())
        return false;

    enqueueCommand(m_colorService, m_colorCharacteristic, m_brigthness);
    enqueueCommand(m_colorService, m_colorCharacteristic, m_liveliness);
    enqueueCommand(m_colorService, m_colorCharacteristic, "340040203300208017");
    enqueueCommand(m_imageService, m_imageCharacteristic, "1500cd0110cd1280811543016100a40acd00"
                   "20cd0040f1f80773027302f200507df13403"
                   "7202f200507db1147400b50af200608d0a7d"
                   "0ecd16a0");
    enqueueCommand(m_imageService, m_imageCharacteristic, "1600cd0210cd1280811543016100a40af1f8"
                   "077302f200507df1c4097202f200507db128"
                   "7301f200608d0a7d0ef134037202f200507d"
                   "b13c7301f200607d8d0a0ecdd453cd4036b1"
                   "c87400b50af200607d8d0a0ecd17a0");
    enqueueCommand(m_imageService, m_imageCharacteristic, "1700cd0310cd1280811543016100a40af1cc"
                   "017202f200507df1f8077302f200507db16e"
                   "7400b50af200607d8d0a0ecd18a0");
    enqueueCommand(m_imageService, m_imageCharacteristic, "1800cd0410cd1280811543016100a40af1d4"
                   "037202f200507db15a7400b50af200607d8d"
                   "0a0ef134037202f200507df1f807f200507d"
                   "b1c87400b50af200607d8d0a0ecd15a0");

    enqueueCommand(m_colorService, m_colorCharacteristic, "2a15");

    m_actions.insert("2a15", actionId);

    sendNextCommand();
    return true;
}

bool AveaBulb::setCozyFlames(ActionId actionId)
{
    if (!isAvailable())
        return false;

    enqueueCommand(m_colorService, m_colorCharacteristic, m_brigthness);
    enqueueCommand(m_colorService, m_colorCharacteristic, m_liveliness);
    enqueueCommand(m_colorService, m_colorCharacteristic, "340040ff3fc8200010");
    enqueueCommand(m_imageService, m_imageCharacteristic, "1500cd0110cd0020cdff5fcdc840cd0030cd"
                   "14608d0a0ecd05708216c200a04d");
    enqueueCommand(m_imageService, m_imageCharacteristic, "1600cd0280a16e6300cd0210cdff5fcdc840"
                   "cd0a608d0a0ecd1ba0");
    enqueueCommand(m_imageService, m_imageCharacteristic, "1700cd0310cd02807100b464f5c4097bcdc8"
                   "708278c200404dcd5070823cc200604d8d0b"
                   "0ecdc870c200404dcd64608d0b0e0ccdff5b"
                   "cd0f608d0a0ecd05708216c200a04d");
    enqueueCommand(m_imageService, m_imageCharacteristic, "1800cd0410cdc840cd0a608d0a0ecd028071"
                   "00b464f5c4097bcd0072c2004082284dcd20"
                   "738232c200604d8d0b0e0ccdff5fcdf842f1"
                   "f4017400b564f200607d8d110ecd05708216"
                   "c200a04d");
    enqueueCommand(m_imageService, m_imageCharacteristic, "1900cd0280a16e6300cd0510cdff5fcdc840"
                   "cd0a608d0a0ecd1ba0");
    enqueueCommand(m_imageService, m_imageCharacteristic, "1a00cd0610cd0280a16e63007100b464f5c4"
                   "097bcd207382c844028564c200404dcd5070"
                   "82a0c200604d8d0b0ecdc870c200404dcd64"
                   "608d0b0e0ccda040cdff5bcd0a608d0a0ecd"
                   "15a0");
    enqueueCommand(m_imageService, m_imageCharacteristic, "1b00cd0280a16e6300cd07107100b464f5e8"
                   "03b3017bcdff73c22c0144028564c200404d"
                   "cd50708264c200604d8d0b0ecd587282c844"
                   "028564c200404dcd64608d0b0e0ccdff5bcd"
                   "05608d0a0ecd05708216c200a04d");

    enqueueCommand(m_colorService, m_colorCharacteristic, "2a15");

    m_actions.insert("2a15", actionId);

    sendNextCommand();
    return true;
}

bool AveaBulb::setCherryBlossom(ActionId actionId)
{
    if (!isAvailable())
        return false;

    enqueueCommand(m_colorService, m_colorCharacteristic, m_brigthness);
    enqueueCommand(m_colorService, m_colorCharacteristic, m_liveliness);
    enqueueCommand(m_colorService, m_colorCharacteristic, "346c47483d00207013");
    enqueueCommand(m_imageService, m_imageCharacteristic, "1500cd0110cd128081654301cd0040cd5c23"
                   "cd7033f19e0c7201f200507db1507201f200"
                   "607d8d0a0ecd1aa0");
    enqueueCommand(m_imageService, m_imageCharacteristic, "1600cd0210cd128081654301f108077201f2"
                   "00207df1660d7201f200507df104017301f2"
                   "00608d0a7d0e8d0a0ecdc023b1aa7301f200"
                   "608d0a7d0eb16e73007ecd17a0");
    enqueueCommand(m_imageService, m_imageCharacteristic, "1700cd0310cd128081654301cd782acd9838"
                   "b1967301f200608d0a7d0ef1780a7301f200"
                   "207df1280f7201f200507db1dc7301f20060"
                   "8d0a7d0ecd5827cd7033b1967301f200608d"
                   "0a7d0ecd18a0");
    enqueueCommand(m_imageService, m_imageCharacteristic, "1800cd0410cd128081654301cd6c27cd345d"
                   "b1a07301f200608d0a7d0ecdc023b1b47301"
                   "f200608d0a7d0ecd402bcd6039b1a07301f2"
                   "00607d8d0a0ecdc023cdf05fb1a07301f200"
                   "607d8d0a0ecd19a0");
    enqueueCommand(m_imageService, m_imageCharacteristic, "1900cd0410cd128081654301cd502acd7033"
                   "b1a07301f200608d0a7d0ecd1aa0");
    enqueueCommand(m_imageService, m_imageCharacteristic, "1a00cd0510cd128081654301cd2033cdcc21"
                   "7101b20af200607d8d0a0ecd04708216c200"
                   "a04d");

    enqueueCommand(m_colorService, m_colorCharacteristic, "2a15");

    m_actions.insert("2a15", actionId);

    sendNextCommand();
    return true;
}

bool AveaBulb::setMountainBreeze(ActionId actionId)
{
    if (!isAvailable())
        return false;

    enqueueCommand(m_colorService, m_colorCharacteristic, m_brigthness);
    enqueueCommand(m_colorService, m_colorCharacteristic, m_liveliness);
    enqueueCommand(m_colorService, m_colorCharacteristic, "34584200300020ff1f");
    enqueueCommand(m_imageService, m_imageCharacteristic, "1500cd0110cd5822cd0050cd0040cdff3fcd"
                   "14608d0a0ecd03708216c200a04d");
    enqueueCommand(m_imageService, m_imageCharacteristic, "1600cd0210cd1280a1646301cd9828f16801"
                   "7402b564f200608d0a7d0ecd5822f1040174"
                   "02b564f200608d0a7d0ecd03708216c200a0"
                   "4d");
    enqueueCommand(m_imageService, m_imageCharacteristic, "1700cd0310cd1280a1646301cd283acd4c24"
                   "cddc70825044028564c200608d0a4d0ecda0"
                   "20cddc70825044028564c200604d8d0a0ecd"
                   "03708216c200a04d");
    enqueueCommand(m_imageService, m_imageCharacteristic, "1800cd0410cd1280a1646301f178057302f2"
                   "00207df12c017402b564f200608d0a7d0ecd"
                   "2023f1480d7302f200307df178057302f200"
                   "207dcdb470825044028564c200604d8d0a0e"
                   "cd03708216c200a04d");
    enqueueCommand(m_colorService, m_colorCharacteristic, "2a15");

    m_actions.insert("2a15", actionId);

    sendNextCommand();
    return true;
}

bool AveaBulb::setNorthernGlow(ActionId actionId)
{
    if (!isAvailable())
        return false;

    enqueueCommand(m_colorService, m_colorCharacteristic, m_brigthness);
    enqueueCommand(m_colorService, m_colorCharacteristic, m_liveliness);
    enqueueCommand(m_colorService, m_colorCharacteristic, "34004000301420e011");
    enqueueCommand(m_imageService, m_imageCharacteristic, "1500cd1280cd0110cd0020cd0050cd1440cd"
                   "e031cd14608d0a0ecdf07082e64201c20030"
                   "4db1b47301f200607d8d0a0ecd04708216c2"
                   "00a04d");
    enqueueCommand(m_imageService, m_imageCharacteristic, "1600cd0210cd1280b1a07301f200607dcd50"
                   "7082504301c200404dcdf070c2b8014201c2"
                   "00304d8d0a0ecd03708217c200a04d");
    enqueueCommand(m_imageService, m_imageCharacteristic, "1700cd0310cd1280b1967301f200607dcd2c"
                   "40cd0050cdb4308d0a0ecd04708216c200a0"
                   "4d");
    enqueueCommand(m_imageService, m_imageCharacteristic, "1800cd0410cd1280b1aa7301f200607dcd50"
                   "70821ec200404dcd0050c168014201c20030"
                   "4d8d0a0ecd04708216c200a04d");
    enqueueCommand(m_imageService, m_imageCharacteristic, "1900cd0510cd1280b1a07301f200607de170"
                   "08e31c026401a564f170087302f200407dcd"
                   "28308d110eb1a07301f200607df1f8027200"
                   "f200407dcd28308d110ecddc70823c4301c2"
                   "00604dcd3c40cda0308d0a0ecd15a0");

    enqueueCommand(m_colorService, m_colorCharacteristic, "2a15");

    m_actions.insert("2a15", actionId);

    sendNextCommand();
    return true;
}

bool AveaBulb::setFairyWoods(ActionId actionId)
{
    if (!isAvailable())
        return false;

    enqueueCommand(m_colorService, m_colorCharacteristic, "57ff0f");
    enqueueCommand(m_colorService, m_colorCharacteristic, "340040143058220010");
    enqueueCommand(m_imageService, m_imageCharacteristic, "1500cd0110cd0020cd0030cd2850cd5842cd"
                   "14608d0a0ecd04708216c200a04d");
    enqueueCommand(m_imageService, m_imageCharacteristic, "1600cd0210cd1280a1646301cd14608d080e"
                   "cd9041f190017402b564f200608d0a7d0ecd"
                   "6851b1c87402b564f200608d0a7d0ecdff47"
                   "cd6450cdb470823c44028564c200604d8d0a"
                   "0ecd17a0");
    enqueueCommand(m_imageService, m_imageCharacteristic, "1700cd0310cd1280a1646301cde041cd2850"
                   "b1f07402b564f200608d0a7d0ecd04708216"
                   "c200a04d");
    enqueueCommand(m_imageService, m_imageCharacteristic, "1800cd0410cd1280a1646301cdb841cdb470"
                   "823c7402b564c200604d8d0a0ecd0e41cd28"
                   "50b1c87402b564f200608d0a7d0ecd6440f1"
                   "90017402b564f200608d0a7d0e8d0a0ecd04"
                   "708216c200a04d");
    enqueueCommand(m_imageService, m_imageCharacteristic, "1900cd0510cd1280a1646301cd6841cdb470"
                   "823cc200604d8d0a0ecda050f12c017402b5"
                   "64f200608d0a7d0ecdf040cd2850f1680174"
                   "02b564f200608d0a7d0ecdcc41b1c87402b5"
                   "64f200608d0a7d0ecd04708216c200a04d");
    enqueueCommand(m_colorService, m_colorCharacteristic, "2a15");

    m_actions.insert("2a15", actionId);

    sendNextCommand();
    return true;
}

bool AveaBulb::setMagicHour(ActionId actionId)
{
    if (!isAvailable())
        return false;

    enqueueCommand(m_colorService, m_colorCharacteristic, m_brigthness);
    enqueueCommand(m_colorService, m_colorCharacteristic, m_liveliness);
    enqueueCommand(m_colorService, m_colorCharacteristic, "340040d03c90210010");
    enqueueCommand(m_imageService, m_imageCharacteristic, "1500cd0110cd0020cdd05ccd9041cd0030cd"
                   "14608d0a0ecd02708216c200a04d");
    enqueueCommand(m_imageService, m_imageCharacteristic, "1600cd0210cd0a60cd0030cd0020cd29408d"
                   "0a0ecd128081664301e1ff0fe3ff07a50a64"
                   "01a50a5100cd0a706400a50af1ff0f7302f2"
                   "00507dcd28708229c200404dcd32704201c2"
                   "00604d8d0a0ecd17a0");
    enqueueCommand(m_imageService, m_imageCharacteristic, "1700cd0310cd1460cd0030cd00208d0a0ecd"
                   "1280a16663018d0d85808250c200404dcd32"
                   "70820a4202c200604d8d0a0ecd16a0");
    enqueueCommand(m_colorService, m_colorCharacteristic, "2a15");

    m_actions.insert("2a15", actionId);

    sendNextCommand();
    return true;
}
