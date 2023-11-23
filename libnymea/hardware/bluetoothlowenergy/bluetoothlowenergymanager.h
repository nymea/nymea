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

#ifndef BLUETOOTHLOWENERGYMANAGER_H
#define BLUETOOTHLOWENERGYMANAGER_H

#include <QTimer>
#include <QObject>
#include <QPointer>
#include <QBluetoothDeviceInfo>
#include <QBluetoothLocalDevice>
#include <QBluetoothDeviceDiscoveryAgent>

#include "hardwareresource.h"

#include "bluetoothdiscoveryreply.h"
#include "bluetoothlowenergydevice.h"

#include "libnymea.h"

class NymeaBluetoothAgent;

class LIBNYMEA_EXPORT BluetoothPairingJob: public QObject
{
    Q_OBJECT
public:
    explicit BluetoothPairingJob(const QBluetoothAddress &address, QObject *parent = nullptr);
    virtual ~BluetoothPairingJob() = default;

    QBluetoothAddress address() const;

    virtual bool isFinished() const = 0;
    virtual bool success() const = 0;

    virtual void passKeyEntered(const QString passKey) = 0;

signals:
    void finished(bool success);
    void passKeyRequested();
    void displayPinCode(const QString &pinCode);

private:
    QBluetoothAddress m_address;
};

class LIBNYMEA_EXPORT BluetoothLowEnergyManager : public HardwareResource
{
    Q_OBJECT

public:
    explicit BluetoothLowEnergyManager(QObject *parent = nullptr);
    virtual ~BluetoothLowEnergyManager() = default;


    virtual BluetoothDiscoveryReply *discoverDevices(int interval = 5000) = 0;

    // Bluetooth device registration methods
    virtual BluetoothPairingJob *pairDevice(const QBluetoothAddress &device, const QBluetoothAddress &adapter) = 0;
    virtual void unpairDevice(const QBluetoothAddress &device, const QBluetoothAddress &adapter) = 0;
    virtual BluetoothLowEnergyDevice *registerDevice(const QBluetoothDeviceInfo &deviceInfo, const QLowEnergyController::RemoteAddressType &addressType = QLowEnergyController::RandomAddress) = 0;
    virtual void unregisterDevice(BluetoothLowEnergyDevice *bluetoothDevice) = 0;

public slots:
    Q_SCRIPTABLE void EnableBluetooth(bool enabled);

};

#endif // BLUETOOTHLOWENERGYMANAGER_H
