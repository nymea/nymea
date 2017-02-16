/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
 *                                                                         *
 *  This file is part of guh.                                              *
 *                                                                         *
 *  This library is free software; you can redistribute it and/or          *
 *  modify it under the terms of the GNU Lesser General Public             *
 *  License as published by the Free Software Foundation; either           *
 *  version 2.1 of the License, or (at your option) any later version.     *
 *                                                                         *
 *  This library is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
 *  Lesser General Public License for more details.                        *
 *                                                                         *
 *  You should have received a copy of the GNU Lesser General Public       *
 *  License along with this library; If not, see                           *
 *  <http://www.gnu.org/licenses/>.                                        *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef BLUETOOTHLOWENERGYDEVICE_H
#define BLUETOOTHLOWENERGYDEVICE_H

#include <QObject>
#include <QBluetoothDeviceInfo>
#include <QBluetoothAddress>
#include <QBluetoothServiceInfo>
#include <QLowEnergyController>

#include "libguh.h"

class LIBGUH_EXPORT BluetoothLowEnergyDevice : public QObject
{
    Q_OBJECT
public:
    explicit BluetoothLowEnergyDevice(const QBluetoothDeviceInfo &deviceInfo, const QLowEnergyController::RemoteAddressType &addressType = QLowEnergyController::PublicAddress, QObject *parent = 0);

    QString name() const;
    QBluetoothAddress address() const;

    void connectDevice();
    void reconnectDevice();
    void disconnectDevice();

    bool isConnected() const;
    bool isDiscovered() const;

protected:
    QLowEnergyController *controller() const;

private:
    QBluetoothDeviceInfo m_deviceInfo;
    QLowEnergyController *m_controller;

    bool m_connected;
    bool m_discovered;

signals:
    void connectionStatusChanged();
    void servicesDiscoveryFinished();

private slots:
    void connected();
    void disconnected();
    void deviceError(const QLowEnergyController::Error &error);
};

#endif // BLUETOOTHLOWENERGYDEVICE_H
