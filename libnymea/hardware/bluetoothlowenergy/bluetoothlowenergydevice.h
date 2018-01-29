/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015-2018 Simon Stürz <simon.stuerz@guh.io>              *
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

#include "libnymea.h"

class LIBNYMEA_EXPORT BluetoothLowEnergyDevice : public QObject
{
    Q_OBJECT

public:
    explicit BluetoothLowEnergyDevice(QObject *parent = 0);
    virtual ~BluetoothLowEnergyDevice() = default;

    virtual QString name() const = 0;
    virtual QBluetoothAddress address() const = 0;

    virtual void connectDevice() = 0;
    virtual void disconnectDevice() = 0;

    virtual bool autoConnecting() const = 0;
    virtual void setAutoConnecting(const bool &autoConnecting) = 0;

    virtual bool connected() const = 0;
    virtual bool discovered() const = 0;

    virtual QList<QBluetoothUuid> serviceUuids() const = 0;
    virtual QLowEnergyController *controller() const = 0;

signals:
    void connectedChanged(const bool &connected);
    void autoConnectingChanged(const bool &autoConnecting);
    void stateChanged(const QLowEnergyController::ControllerState &state);
    void errorOccured(const QLowEnergyController::Error &error);
    void servicesDiscoveryFinished();

};

#endif // BLUETOOTHLOWENERGYDEVICE_H
