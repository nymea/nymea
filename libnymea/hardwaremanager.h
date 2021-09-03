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

#ifndef HARDWAREMANAGER_H
#define HARDWAREMANAGER_H

#include <QObject>

class Radio433;
class UpnpDiscovery;
class PluginTimerManager;
class NetworkAccessManager;
class UpnpDeviceDescriptor;
class PlatformZeroConfController;
class MqttProvider;
class I2CManager;
class ZigbeeHardwareResource;
class HardwareResource;
class ModbusRtuHardwareResource;
class NetworkDeviceDiscovery;

#ifdef WITH_BLUETOOTH
class BluetoothLowEnergyManager;
#endif

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
    virtual MqttProvider *mqttProvider() = 0;
    virtual I2CManager *i2cManager() = 0;
    virtual ZigbeeHardwareResource *zigbeeResource() = 0;
    virtual ModbusRtuHardwareResource *modbusRtuResource() = 0;
    virtual NetworkDeviceDiscovery *networkDeviceDiscovery() = 0;

#ifdef WITH_BLUETOOTH
    virtual BluetoothLowEnergyManager *bluetoothLowEnergyManager() = 0;
#endif

protected:
    void setResourceEnabled(HardwareResource* resource, bool enabled);
};

#endif // HARDWAREMANAGER_H
