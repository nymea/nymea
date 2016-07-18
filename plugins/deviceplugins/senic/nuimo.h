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

#ifndef NUIMO_H
#define NUIMO_H

#ifdef BLUETOOTH_LE

#include <QObject>
#include <QBluetoothUuid>

#include "typeutils.h"
#include "bluetooth/bluetoothlowenergydevice.h"

class Nuimo : public BluetoothLowEnergyDevice
{
    Q_OBJECT
public:
    explicit Nuimo(const QBluetoothDeviceInfo &deviceInfo, const QLowEnergyController::RemoteAddressType &addressType, QObject *parent = 0);

    bool isAvailable();

    void showGuhLogo();

private:
    QLowEnergyService *m_deviceInfoService;
    QLowEnergyService *m_batteryService;
    QLowEnergyService *m_inputService;
    QLowEnergyService *m_ledMatrixService;

    QLowEnergyCharacteristic m_deviceInfoCharacteristic;
    QLowEnergyCharacteristic m_batteryCharacteristic;
    QLowEnergyCharacteristic m_inputButtonCharacteristic;
    QLowEnergyCharacteristic m_ledMatrixCharacteristic;

    bool m_isAvailable;

    void registerService(QLowEnergyService *service);

signals:
    void availableChanged();
    void buttonPressed();
    void buttonReleased();
    void batteryValueChaged(const uint &percentage);

private slots:
    void serviceScanFinished();
    void onConnectionStatusChanged();

    void serviceStateChanged(const QLowEnergyService::ServiceState &state);
    void serviceCharacteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &value);
    void confirmedCharacteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &value);
    void confirmedDescriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &value);
    void serviceError(const QLowEnergyService::ServiceError &error);

};

#endif // BLUETOOTH_LE

#endif // NUIMO_H
