// SPDX-License-Identifier: LGPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2025, chargebyte austria GmbH
*
* This file is part of nymea.
*
* nymea is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* as published by the Free Software Foundation, either version 3
* of the License, or (at your option) any later version.
*
* nymea is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with nymea. If not, see <https://www.gnu.org/licenses/>.
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef BLUETOOTHLOWENERGYDEVICE_H
#define BLUETOOTHLOWENERGYDEVICE_H

#include <QBluetoothAddress>
#include <QBluetoothDeviceInfo>
#include <QBluetoothServiceInfo>
#include <QLowEnergyController>
#include <QObject>

#include "libnymea.h"

class LIBNYMEA_EXPORT BluetoothLowEnergyDevice : public QObject
{
    Q_OBJECT

public:
    explicit BluetoothLowEnergyDevice(QObject *parent = nullptr);
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
    void errorOccurred(const QLowEnergyController::Error &error);
    void servicesDiscoveryFinished();
};

#endif // BLUETOOTHLOWENERGYDEVICE_H
