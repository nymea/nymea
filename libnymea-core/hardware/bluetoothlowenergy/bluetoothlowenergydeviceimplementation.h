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

#ifndef BLUETOOTHLOWENERGYDEVICEIMPLEMENTATION_H
#define BLUETOOTHLOWENERGYDEVICEIMPLEMENTATION_H

#include <QBluetoothAddress>
#include <QBluetoothDeviceInfo>
#include <QBluetoothServiceInfo>
#include <QLowEnergyController>
#include <QObject>

#include "hardware/bluetoothlowenergy/bluetoothlowenergydevice.h"

namespace nymeaserver {

class BluetoothLowEnergyDeviceImplementation : public BluetoothLowEnergyDevice
{
    Q_OBJECT

    friend class BluetoothLowEnergyManagerImplementation;

public:
    explicit BluetoothLowEnergyDeviceImplementation(const QBluetoothDeviceInfo &deviceInfo,
                                                    const QLowEnergyController::RemoteAddressType &addressType = QLowEnergyController::PublicAddress,
                                                    QObject *parent = nullptr);

    QString name() const override;
    QBluetoothAddress address() const override;

    void connectDevice() override;
    void disconnectDevice() override;

    bool autoConnecting() const override;
    void setAutoConnecting(const bool &autoConnecting) override;

    bool connected() const override;
    bool discovered() const override;

    QList<QBluetoothUuid> serviceUuids() const override;
    QLowEnergyController *controller() const override;

private:
    QBluetoothDeviceInfo m_deviceInfo;
    QLowEnergyController *m_controller = nullptr;

    // Default enabled and auto connecting
    bool m_connected = false;
    bool m_autoConnecting = true;
    bool m_discovered = false;
    bool m_enabled = true;

    void setConnected(bool connected);

private slots:
    void onConnected();
    void onDisconnected();
    void onServiceDiscoveryFinished();
    void onStateChanged(const QLowEnergyController::ControllerState &state);
    void onDeviceError(const QLowEnergyController::Error &error);
};

} // namespace nymeaserver

#endif // BLUETOOTHLOWENERGYDEVICEIMPLEMENTATION_H
