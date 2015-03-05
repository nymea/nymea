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

#include "bluetoothlowenergydevice.h"

BluetoothLowEnergyDevice::BluetoothLowEnergyDevice(const QBluetoothDeviceInfo &deviceInfo, const QLowEnergyController::RemoteAddressType &addressType, QObject *parent) :
    QObject(parent), m_deviceInfo(deviceInfo)
{
    m_controller = new QLowEnergyController(address(), this);
    m_controller->setRemoteAddressType(addressType);

    connect(m_controller, &QLowEnergyController::connected, this, &BluetoothLowEnergyDevice::connected);
    connect(m_controller, &QLowEnergyController::disconnected, this, &BluetoothLowEnergyDevice::disconnected);
    connect(m_controller, &QLowEnergyController::serviceDiscovered, this, &BluetoothLowEnergyDevice::serviceDiscovered);
    connect(m_controller, &QLowEnergyController::discoveryFinished, this, &BluetoothLowEnergyDevice::serviceScanFinished);
    connect(m_controller, SIGNAL(error(QLowEnergyController::Error)), this, SLOT(deviceError(QLowEnergyController::Error)));
}

QString BluetoothLowEnergyDevice::name() const
{
    return m_deviceInfo.name();
}

QBluetoothAddress BluetoothLowEnergyDevice::address() const
{
    return m_deviceInfo.address();
}

QList<QLowEnergyService *> BluetoothLowEnergyDevice::services() const
{
    return m_services;
}

QList<QLowEnergyCharacteristic> BluetoothLowEnergyDevice::characteristics() const
{
    return m_characteristics;
}

void BluetoothLowEnergyDevice::connectDevice()
{
    m_controller->connectToDevice();
}

void BluetoothLowEnergyDevice::disconnectDevice()
{
    m_controller->disconnectFromDevice();
}

bool BluetoothLowEnergyDevice::isConnected() const
{
    return m_connected;
}

bool BluetoothLowEnergyDevice::isDiscovered() const
{
    return m_discovered;
}

void BluetoothLowEnergyDevice::discoverService(const QBluetoothUuid &serviceUuid)
{
    Q_UNUSED(serviceUuid);
}

void BluetoothLowEnergyDevice::connected()
{
    m_connected = true;
    qDebug() << "connected successfully to" << name() << address().toString();
    emit connectionStatusChanged(true);
    m_controller->discoverServices();
}

void BluetoothLowEnergyDevice::disconnected()
{
    m_connected = false;
    qWarning() << "disconnected from" << name() << address().toString();
    emit connectionStatusChanged(false);
}

void BluetoothLowEnergyDevice::deviceError(const QLowEnergyController::Error &error)
{
    qWarning() << "ERROR: Bluetooth device" << name() << address().toString() << ": " << error << m_controller->errorString();
}

void BluetoothLowEnergyDevice::serviceDiscovered(const QBluetoothUuid &serviceUuid)
{
    QLowEnergyService *service = m_controller->createServiceObject(serviceUuid);

    if (!service) {
        qWarning() << "Cannot create service for uuid" << serviceUuid;
        return;
    }

    connect(service, SIGNAL(characteristicChanged(QLowEnergyCharacteristic,QByteArray)), this, SLOT(serviceCharacteristicChanged(QLowEnergyCharacteristic,QByteArray)));
    connect(service, SIGNAL(error(QLowEnergyService::ServiceError)), this, SLOT(serviceError(QLowEnergyService::ServiceError)));
    connect(service, SIGNAL(stateChanged(QLowEnergyService::ServiceState)), this, SLOT(serviceStateChanged(QLowEnergyService::ServiceState)));

    m_services.append(service);
}

void BluetoothLowEnergyDevice::serviceStateChanged(const QLowEnergyService::ServiceState &newState)
{
    if (newState != QLowEnergyService::ServiceDiscovered) {
        return;
    }

    QLowEnergyService *service = qobject_cast<QLowEnergyService *>(sender());
    foreach (QLowEnergyCharacteristic characteristic, service->characteristics()) {
        if (!characteristic.descriptors().isEmpty()) {
            foreach (QLowEnergyDescriptor descriptor, characteristic.descriptors()) {
                if (descriptor.isValid()) {
                    m_descriptors.append(descriptor);
                }
            }
        }
        m_characteristics.append(characteristic);
    }
    qDebug() << "details discovered for service " << service->serviceName() << service->serviceUuid().toString();
}

void BluetoothLowEnergyDevice::serviceScanFinished()
{
    qDebug() << "service scan finished";
    emit serviceScanFinished();
}

void BluetoothLowEnergyDevice::serviceCharacteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &value)
{
    qDebug() << "characteristic" << characteristic.name() << "changed:" << value;
}

void BluetoothLowEnergyDevice::serviceError(const QLowEnergyService::ServiceError &error)
{
    QLowEnergyService *service = qobject_cast<QLowEnergyService *>(sender());
    qWarning() << "ERROR: Bluetooth service " << service->serviceName() << service->serviceUuid() << ": " << error;
}

void BluetoothLowEnergyDevice::confirmedDescriptorWrite(const QLowEnergyDescriptor &descriptor, const QByteArray &value)
{
    qDebug() << "descriptor" << descriptor.name() << "written successfully " << value;
}
