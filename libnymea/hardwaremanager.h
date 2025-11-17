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

#ifndef HARDWAREMANAGER_H
#define HARDWAREMANAGER_H

#include <QObject>

class Radio433;
class UpnpDiscovery;
class NetworkAccessManager;
class UpnpDeviceDescriptor;
class PlatformZeroConfController;
class BluetoothLowEnergyManager;
class MqttProvider;
class I2CManager;
class ZigbeeHardwareResource;
class ZWaveHardwareResource;
class HardwareResource;
class ModbusRtuHardwareResource;
class NetworkDeviceDiscovery;

// Note: https://www.qt.io/blog/whats-new-in-qmetatype-qvariant
#include "plugintimer.h"

class HardwareManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(PluginTimerManager* pluginTimerManager READ pluginTimerManager CONSTANT)

public:
    HardwareManager(QObject *parent = nullptr);
    virtual ~HardwareManager() = default;

    virtual Radio433 *radio433() = 0;
    virtual PluginTimerManager *pluginTimerManager() = 0;
    virtual NetworkAccessManager *networkManager() = 0;
    virtual UpnpDiscovery *upnpDiscovery() = 0;
    virtual PlatformZeroConfController *zeroConfController() = 0;
    virtual BluetoothLowEnergyManager *bluetoothLowEnergyManager() = 0;
    virtual MqttProvider *mqttProvider() = 0;
    virtual I2CManager *i2cManager() = 0;
    virtual ZigbeeHardwareResource *zigbeeResource() = 0;
    virtual ZWaveHardwareResource *zwaveResource() = 0;
    virtual ModbusRtuHardwareResource *modbusRtuResource() = 0;
    virtual NetworkDeviceDiscovery *networkDeviceDiscovery() = 0;

protected:
    void setResourceEnabled(HardwareResource* resource, bool enabled);
};

#endif // HARDWAREMANAGER_H
