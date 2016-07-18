/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2016 Simon Stürz <simon.stuerz@guh.guru>                 *
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

#include "nuimo.h"
#include "extern-plugininfo.h"

#include <QBitArray>

Nuimo::Nuimo(const QBluetoothDeviceInfo &deviceInfo, const QLowEnergyController::RemoteAddressType &addressType, QObject *parent) :
    BluetoothLowEnergyDevice(deviceInfo, addressType, parent),
    m_deviceInfoService(NULL),
    m_batteryService(NULL),
    m_inputService(NULL),
    m_ledMatrixService(NULL),
    m_isAvailable(false)
{
    connect(this, SIGNAL(connectionStatusChanged()), this,SLOT(onConnectionStatusChanged()));
    connect(this, SIGNAL(servicesDiscoveryFinished()), this, SLOT(serviceScanFinished()));
}

bool Nuimo::isAvailable()
{
    return m_isAvailable;
}

void Nuimo::showGuhLogo()
{
    QByteArray matrix(
            "         "
            " *       "
            " **   ** "
            "  ****   "
            "   **   "
            "   *     "
            "  *      "
            " *      "
            "         ");

    QBitArray bits;
    bits.resize(81);
    for (int i = 0; i < matrix.size(); i++) {
        if (matrix.at(i) != ' ') {
            bits[i] = true;
        }
    }

    QByteArray bytes;
    bytes.resize(bits.count() / 8+1);
    bytes.fill(0);
    // Convert from QBitArray to QByteArray
    for(int b = 0; b < bits.count(); ++b)
        bytes[b / 8] = (bytes.at(b / 8) | ((bits[b] ? 1 : 0) << (b % 8)));

    bytes.append(QByteArray::fromHex("FF"));
    bytes.append(QByteArray::fromHex("AA"));

    m_ledMatrixService->writeCharacteristic(m_ledMatrixCharacteristic, bytes);
}

void Nuimo::registerService(QLowEnergyService *service)
{
    connect(service, SIGNAL(stateChanged(QLowEnergyService::ServiceState)), this, SLOT(serviceStateChanged(QLowEnergyService::ServiceState)));
    connect(service, SIGNAL(characteristicChanged(QLowEnergyCharacteristic, QByteArray)), this, SLOT(serviceCharacteristicChanged(QLowEnergyCharacteristic, QByteArray)));
    connect(service, SIGNAL(characteristicWritten(QLowEnergyCharacteristic, QByteArray)), this, SLOT(confirmedCharacteristicWritten(QLowEnergyCharacteristic, QByteArray)));
    connect(service, SIGNAL(descriptorWritten(QLowEnergyDescriptor, QByteArray)), this, SLOT(confirmedDescriptorWritten(QLowEnergyDescriptor, QByteArray)));
    connect(service, SIGNAL(error(QLowEnergyService::ServiceError)), this, SLOT(serviceError(QLowEnergyService::ServiceError)));

    service->discoverDetails();
}


void Nuimo::serviceScanFinished()
{
    QBluetoothUuid deviceInfoUuid(QUuid("0000180A-0000-1000-8000-00805F9B34FB"));
    QBluetoothUuid batteryUuid(QUuid("0000180F-0000-1000-8000-00805F9B34FB"));
    QBluetoothUuid inputUuid(QUuid("F29B1525-CB19-40F3-BE5C-7241ECB82FD2"));
    QBluetoothUuid ledMatrixUuid(QUuid("F29B1523-CB19-40F3-BE5C-7241ECB82FD1"));

    qCDebug(dcSenic()) << "Service scan finised";

    if (!controller()->services().contains(deviceInfoUuid)) {
        qCWarning(dcSenic()) << "Device Information service not found for device" << name() << address().toString();
        return;
    }

    if (!controller()->services().contains(batteryUuid)) {
        qCWarning(dcSenic()) << "Battery service not found for device" << name() << address().toString();
        return;
    }

    if (!controller()->services().contains(ledMatrixUuid)) {
        qCWarning(dcSenic()) << "Led matrix service not found for device" << name() << address().toString();
        return;
    }

    if (!controller()->services().contains(inputUuid)) {
        qCWarning(dcSenic()) << "Input service not found for device" << name() << address().toString();
        return;
    }

    m_isAvailable = true;
    emit availableChanged();

    // Device information
    m_deviceInfoService = controller()->createServiceObject(deviceInfoUuid, this);
    if (!m_deviceInfoService) {
        qCWarning(dcSenic()) << "Could not create device information service for device" << name() << address().toString();
        return;
    }

    // Battery service
    m_batteryService = controller()->createServiceObject(batteryUuid, this);
    if (!m_batteryService) {
        qCWarning(dcSenic()) << "Could not create battery service for device" << name() << address().toString();
        return;
    }

    // Input service
    m_inputService = controller()->createServiceObject(inputUuid, this);
    if (!m_inputService) {
        qCWarning(dcSenic()) << "Could not create input service for device" << name() << address().toString();
        return;
    }

    // Led Matrix
    m_ledMatrixService = controller()->createServiceObject(ledMatrixUuid, this);
    if (!m_ledMatrixService) {
        qCWarning(dcSenic()) << "Could not create led matrix service for device" << name() << address().toString();
        return;
    }

    registerService(m_deviceInfoService);
    registerService(m_batteryService);
    registerService(m_inputService);
    registerService(m_ledMatrixService);

}

void Nuimo::onConnectionStatusChanged()
{
    if (!isConnected()) {
        // delete the services, they need to be recreated and
        // rediscovered once the device will be reconnected

        if (m_deviceInfoService) {
            delete m_deviceInfoService;
            m_deviceInfoService = 0;
        }

        if (m_batteryService) {
            delete m_batteryService;
            m_batteryService = 0;
        }

        if (m_inputService) {
            delete m_inputService;
            m_inputService = 0;
        }

        if (m_ledMatrixService) {
            delete m_ledMatrixService;
            m_ledMatrixService = 0;
        }

        m_isAvailable = false;
        emit availableChanged();
    }
}

void Nuimo::serviceStateChanged(const QLowEnergyService::ServiceState &state)
{
    QLowEnergyService *service =static_cast<QLowEnergyService *>(sender());

    switch (state) {
    case QLowEnergyService::DiscoveringServices:
        if (service == m_batteryService) {
            qCDebug(dcSenic()) << "Start discovering battery service...";
        }
        if (service == m_deviceInfoService) {
            qCDebug(dcSenic()) << "Start discovering device information service...";
        }
        if (service == m_inputService) {
            qCDebug(dcSenic()) << "Start discovering input service...";
        }
        if (service == m_inputService) {
            qCDebug(dcSenic()) << "Start discovering led matrix service...";
        }
        break;
    case QLowEnergyService::ServiceDiscovered:

        // Device information
        if (service == m_deviceInfoService) {
            qCDebug(dcSenic()) << "Device information service discovered";
            m_deviceInfoCharacteristic = m_deviceInfoService->characteristic(QBluetoothUuid(QUuid("00002A29-0000-1000-8000-00805F9B34FB")));
            if (!m_deviceInfoCharacteristic.isValid()) {
                qCWarning(dcSenic()) << "Device information characteristc not found for device " << name() << address().toString();
                return;
            }

            qCDebug(dcSenic()) << "Device information:" << m_deviceInfoCharacteristic.value();
        }

        // Battery
        if (service == m_batteryService) {
            qCDebug(dcSenic()) << "Battery service discovered";
            m_batteryCharacteristic = m_batteryService->characteristic(QBluetoothUuid(QUuid("00002A19-0000-1000-8000-00805F9B34FB")));
            if (!m_batteryCharacteristic.isValid()) {
                qCWarning(dcSenic()) << "Battery characteristc not found for device " << name() << address().toString();
                return;
            }

            int batteryPercentage = m_batteryCharacteristic.value().toHex().toUInt(0, 16);
            qCDebug(dcSenic()) << "Battery:" << batteryPercentage << "%";

            // Enable notification
            foreach (const QLowEnergyDescriptor &descriptor, m_batteryCharacteristic.descriptors()) {
                qCDebug(dcSenic()) << descriptor.name() << descriptor.uuid().toString();
                m_batteryService->writeDescriptor(descriptor, QByteArray::fromHex("0100"));
            }

            emit batteryValueChaged(batteryPercentage);
        }

        // Input
        if (service == m_inputService) {
            qCDebug(dcSenic()) << "Input service discovered";
            m_inputButtonCharacteristic = m_inputService->characteristic(QBluetoothUuid(QUuid("F29B1529-CB19-40F3-BE5C-7241ECB82FD2")));
            if (!m_inputButtonCharacteristic.isValid()) {
                qCWarning(dcSenic()) << "Button characteristc not found for device " << name() << address().toString();
                return;
            }

            foreach (const QLowEnergyDescriptor &descriptor, m_inputButtonCharacteristic.descriptors()) {
                qCDebug(dcSenic()) << descriptor.name() << descriptor.uuid().toString();
                m_inputService->writeDescriptor(descriptor, QByteArray::fromHex("0100"));
            }
        }

        // Input
        if (service == m_ledMatrixService) {
            qCDebug(dcSenic()) << "Led matrix service discovered";
            m_ledMatrixCharacteristic = m_ledMatrixService->characteristic(QBluetoothUuid(QUuid("F29B1524-CB19-40F3-BE5C-7241ECB82FD1")));
            if (!m_ledMatrixCharacteristic.isValid()) {
                qCWarning(dcSenic()) << "Led matrix characteristc not found for device " << name() << address().toString();
                return;
            }

            showGuhLogo();
        }

        break;
    default:
        break;
    }
}

void Nuimo::serviceCharacteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &value)
{
    if (characteristic.uuid() == m_batteryCharacteristic.uuid()) {
        uint batteryValue = value.toHex().toUInt(0, 16);
        qCDebug(dcSenic()) << "Battery:" << batteryValue;
        emit batteryValueChaged(batteryValue);
        return;
    }

    if (characteristic.uuid() == m_inputButtonCharacteristic.uuid()) {
        bool pressed = (bool)value.toHex().toUInt(0, 16);
        qCDebug(dcSenic()) << "Button:" << (pressed ? "pressed": "released");
        if (pressed) {
            emit buttonPressed();
        } else {
            emit buttonReleased();
        }
        return;
    }

    qCDebug(dcSenic()) << "Service characteristic changed" << characteristic.name() << value.toHex();
}

void Nuimo::confirmedCharacteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &value)
{
    qCDebug(dcSenic()) << characteristic.name() << value;
}

void Nuimo::confirmedDescriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &value)
{
    qCDebug(dcSenic()) << "Descriptor:" << descriptor.name() << "value:" << value.toHex() << "written successfully";
}

void Nuimo::serviceError(const QLowEnergyService::ServiceError &error)
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

    qCWarning(dcSenic()) << "Service of " << name() << address().toString() << ":" << errorString;
}

#endif // BLUETOOTH_LE
