/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.guru>                 *
 *  Copyright (C) 2016 nicc                                                *
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

#ifndef SENSORTAG_H
#define SENSORTAG_H

#ifdef BLUETOOTH_LE

#include <QHash>
#include <QSharedPointer>
#include <QLowEnergyService>
#include "bluetooth/bluetoothlowenergydevice.h"
#include "extern-plugininfo.h"

class SensorTag : public BluetoothLowEnergyDevice
{
    Q_OBJECT
public:
    explicit SensorTag(const QBluetoothDeviceInfo &deviceInfo, const QLowEnergyController::RemoteAddressType &addressType, QObject *parent = 0);

signals:
    void valueChanged(StateTypeId state, QVariant value);
    void event(EventTypeId event);

private:
    QHash<QBluetoothUuid, QSharedPointer<QLowEnergyService>> m_services{
        {QBluetoothUuid(QUuid("f000aa00-0451-4000-b000-000000000000")), QSharedPointer<QLowEnergyService>()},
        {QBluetoothUuid(QUuid("f000aa20-0451-4000-b000-000000000000")), QSharedPointer<QLowEnergyService>()},
        {QBluetoothUuid(QUuid("f000aa40-0451-4000-b000-000000000000")), QSharedPointer<QLowEnergyService>()},
        {QBluetoothUuid(QUuid("0000ffe0-0000-1000-8000-00805f9b34fb")), QSharedPointer<QLowEnergyService>()}
    };

    //QHash<QByteArray, ActionId> m_actions;

    //QQueue<CommandRequest> m_commandQueue;
    //CommandRequest m_currentRequest;
    //bool m_queueRunning;

    QVector<quint16> m_c;
    QVector<qint16> m_c2;

private slots:
    void setupServices();
    void onConnectionStatusChanged();

    // Service
    void onServiceStateChanged(const QLowEnergyService::ServiceState &state);
    void onServiceCharacteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &value);
    //void confirmedCharacteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &value);
    //void confirmedDescriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &value);
    void onServiceError(const QLowEnergyService::ServiceError &error);
};
#endif // BLUETOOTH_LE

#endif // SENSORTAG_H
