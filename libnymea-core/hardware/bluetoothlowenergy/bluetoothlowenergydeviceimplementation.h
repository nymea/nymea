/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2020, nymea GmbH
* Contact: contact@nymea.io
*
* This file is part of nymea.
* This project including source code and documentation is protected by
* copyright law, and remains the property of nymea GmbH. All rights, including
* reproduction, publication, editing and translation, are reserved. The use of
* this project is subject to the terms of a license agreement to be concluded
* with nymea GmbH in accordance with the terms of use of nymea GmbH, available
* under https://nymea.io/license
*
* GNU General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU General Public License as published by the Free Software
* Foundation, GNU version 3. This project is distributed in the hope that it
* will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
* of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
* Public License for more details.
*
* You should have received a copy of the GNU General Public License along with
* this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef BLUETOOTHLOWENERGYDEVICEIMPLEMENTATION_H
#define BLUETOOTHLOWENERGYDEVICEIMPLEMENTATION_H

#ifdef WITH_BLUETOOTH

#include <QObject>
#include <QBluetoothDeviceInfo>
#include <QBluetoothAddress>
#include <QBluetoothServiceInfo>
#include <QLowEnergyController>

#include "hardware/bluetoothlowenergy/bluetoothlowenergydevice.h"

namespace nymeaserver {

class BluetoothLowEnergyDeviceImplementation : public BluetoothLowEnergyDevice
{
    Q_OBJECT

    friend class BluetoothLowEnergyManagerImplementation;

public:
    explicit BluetoothLowEnergyDeviceImplementation(const QBluetoothDeviceInfo &deviceInfo, const QLowEnergyController::RemoteAddressType &addressType = QLowEnergyController::PublicAddress, QObject *parent = nullptr);

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

    void setConnected(const bool &connected);

    // Methods called from BluetoothLowEnergyManager
    void setEnabled(const bool &enabled);

private slots:
    void onConnected();
    void onDisconnected();
    void onServiceDiscoveryFinished();
    void onStateChanged(const QLowEnergyController::ControllerState &state);
    void onDeviceError(const QLowEnergyController::Error &error);
};

}

#endif // WITH_BLUETOOTH

#endif // BLUETOOTHLOWENERGYDEVICEIMPLEMENTATION_H
