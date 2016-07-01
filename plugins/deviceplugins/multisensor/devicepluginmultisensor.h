/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stuerz <simon.stuerz@guh.guru>                *
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
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the           *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with guh. If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef DEVICEPLUGINMULTISENSOR_H
#define DEVICEPLUGINMULTISENSOR_H

#ifdef BLUETOOTH_LE

#include <QPointer>
#include <QHash>
#include "plugin/deviceplugin.h"
#include "devicemanager.h"
#include "bluetooth/bluetoothlowenergydevice.h"
#include "sensortag.h"

class DevicePluginMultiSensor : public DevicePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "guru.guh.DevicePlugin" FILE "devicepluginmultisensor.json")
    Q_INTERFACES(DevicePlugin)

public:
    explicit DevicePluginMultiSensor();

    DeviceManager::HardwareResources requiredHardware() const override;
    DeviceManager::DeviceError discoverDevices(const DeviceClassId &deviceClassId, const ParamList &params) override;
    void bluetoothDiscoveryFinished(const QList<QBluetoothDeviceInfo> &deviceInfos) override;
    DeviceManager::DeviceSetupStatus setupDevice(Device *device) override;
    void deviceRemoved(Device *device) override;

private:
    bool verifyExistingDevices(const QBluetoothDeviceInfo &deviceInfo);

    QHash<QSharedPointer<SensorTag>,QPointer<Device>> m_tags;

private slots:
    void updateValue(StateTypeId state, double value);
};

#endif // BLUETOOTH_LE

#endif // DEVICEPLUGINMULTISENSOR_H
