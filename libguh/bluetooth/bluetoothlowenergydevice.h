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

#ifndef BLUETOOTHLOWENERGYDEVICE_H
#define BLUETOOTHLOWENERGYDEVICE_H

#include <QObject>
#include <QBluetoothDeviceInfo>
#include <QBluetoothAddress>
#include <QBluetoothServiceInfo>
#include <QLowEnergyController>

class BluetoothLowEnergyDevice : public QObject
{
    Q_OBJECT
public:
    explicit BluetoothLowEnergyDevice(const QBluetoothDeviceInfo &deviceInfo, const QLowEnergyController::RemoteAddressType &addressType = QLowEnergyController::PublicAddress, QObject *parent = 0);

    QString name() const;
    QBluetoothAddress address() const;
    QList<QLowEnergyService *> services() const;
    QList<QLowEnergyCharacteristic> characteristics() const;

    void connectDevice();
    void disconnectDevice();

    bool isConnected() const;
    bool isDiscovered() const;

    void discoverService(const QBluetoothUuid &serviceUuid);

private:
    QBluetoothDeviceInfo m_deviceInfo;
    QLowEnergyController *m_controller;
    QList<QLowEnergyService *> m_services;
    QList<QLowEnergyCharacteristic> m_characteristics;
    QList<QLowEnergyDescriptor> m_descriptors;

    bool m_connected;
    bool m_discovered;

signals:
    void connectionStatusChanged(const bool &connectionStatus);
    void servicesDiscovered();

private slots:
    // Device
    void connected();
    void disconnected();
    void deviceError(const QLowEnergyController::Error &error);
    void serviceDiscovered(const QBluetoothUuid &serviceUuid);
    void serviceStateChanged(const QLowEnergyService::ServiceState &newState);
    void serviceScanFinished();

    // Service
    void serviceCharacteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &value);
    void serviceError(const QLowEnergyService::ServiceError &error);
    void confirmedDescriptorWrite(const QLowEnergyDescriptor &descriptor, const QByteArray &value);
};

#endif // BLUETOOTHLOWENERGYDEVICE_H
