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
* GNU Lesser General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU Lesser General Public License as published by the Free
* Software Foundation; version 3. This project is distributed in the hope that
* it will be useful, but WITHOUT ANY WARRANTY; without even the implied
* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef BLUETOOTHLOWENERGYDEVICE_H
#define BLUETOOTHLOWENERGYDEVICE_H

#ifdef WITH_BLUETOOTH

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

#endif // WITH_BLUETOOTH

#endif // BLUETOOTHLOWENERGYDEVICE_H
