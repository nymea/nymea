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

#include "loggingcategories.h"

#include "platform/platform.h"

#include "platform/platformzeroconfcontroller.h"

#include "hardware/bluetoothlowenergy/bluetoothlowenergymanagerimplementation.h"
#include "hardware/i2c/i2cmanagerimplementation.h"
#include "hardware/network/mqtt/mqttproviderimplementation.h"
#include "hardware/network/networkaccessmanagerimpl.h"
#include "hardware/network/upnp/upnpdiscoveryimplementation.h"
#include "hardware/plugintimermanagerimplementation.h"
#include "hardware/radio433/radio433brennenstuhl.h"
#include "hardware/zigbee/zigbeehardwareresourceimplementation.h"
#include "hardware/zwave/zwavehardwareresourceimplementation.h"
#include "hardwaremanagerimplementation.h"

#include "hardware/modbus/modbusrtuhardwareresourceimplementation.h"
#include "hardware/modbus/modbusrtumanager.h"
#include "hardware/network/networkdevicediscoveryimpl.h"

namespace nymeaserver {

HardwareManagerImplementation::HardwareManagerImplementation(
    Platform *platform, MqttBroker *mqttBroker, ZigbeeManager *zigbeeManager, ZWaveManager *zwaveManager, ModbusRtuManager *modbusRtuManager, QObject *parent)
    : HardwareManager(parent)
    , m_platform(platform)
{
    // Create network access manager for all resources, centralized
    // Note: configuration and proxy settings could be implemented here
    m_networkAccessManager = new QNetworkAccessManager(this);

    // Init hardware resources
    m_pluginTimerManager = new PluginTimerManagerImplementation(this);

    // Radio 433 MHz
    m_radio433 = new Radio433Brennenstuhl(this);

    // Network manager
    m_networkManager = new NetworkAccessManagerImpl(m_networkAccessManager, this);

    // UPnP discovery
    m_upnpDiscovery = new UpnpDiscoveryImplementation(m_networkAccessManager, this);

    // Bluetooth LE
    m_bluetoothLowEnergyManager = new BluetoothLowEnergyManagerImplementation(m_pluginTimerManager->registerTimer(10), this);

    m_i2cManager = new I2CManagerImplementation(this);

    m_zigbeeResource = new ZigbeeHardwareResourceImplementation(zigbeeManager, this);

    m_zwaveResource = new ZWaveHardwareResourceImplementation(zwaveManager, this);

    m_modbusRtuResource = new ModbusRtuHardwareResourceImplementation(modbusRtuManager, this);

    m_networkDeviceDiscovery = new NetworkDeviceDiscoveryImpl(this);

    // Enable all the resources
    setResourceEnabled(m_pluginTimerManager, true);
    setResourceEnabled(m_radio433, true);

    if (m_networkManager->available())
        setResourceEnabled(m_networkManager, true);

    if (m_upnpDiscovery->available())
        setResourceEnabled(m_upnpDiscovery, true);

    if (m_platform->zeroConfController()->available())
        setResourceEnabled(m_platform->zeroConfController(), true);

    if (m_bluetoothLowEnergyManager->available())
        setResourceEnabled(m_bluetoothLowEnergyManager, true);

    m_mqttProvider = new MqttProviderImplementation(mqttBroker, this);
    qCDebug(dcHardware()) << "Hardware manager initialized successfully";
}

HardwareManagerImplementation::~HardwareManagerImplementation() {}

Radio433 *HardwareManagerImplementation::radio433()
{
    return m_radio433;
}

PluginTimerManager *HardwareManagerImplementation::pluginTimerManager()
{
    return m_pluginTimerManager;
}

NetworkAccessManager *HardwareManagerImplementation::networkManager()
{
    return m_networkManager;
}

UpnpDiscovery *HardwareManagerImplementation::upnpDiscovery()
{
    return m_upnpDiscovery;
}

PlatformZeroConfController *HardwareManagerImplementation::zeroConfController()
{
    return m_platform->zeroConfController();
}

BluetoothLowEnergyManager *HardwareManagerImplementation::bluetoothLowEnergyManager()
{
    return m_bluetoothLowEnergyManager;
}

MqttProvider *HardwareManagerImplementation::mqttProvider()
{
    return m_mqttProvider;
}

I2CManager *HardwareManagerImplementation::i2cManager()
{
    return m_i2cManager;
}

ZigbeeHardwareResource *HardwareManagerImplementation::zigbeeResource()
{
    return m_zigbeeResource;
}

ZWaveHardwareResource *HardwareManagerImplementation::zwaveResource()
{
    return m_zwaveResource;
}

ModbusRtuHardwareResource *HardwareManagerImplementation::modbusRtuResource()
{
    return m_modbusRtuResource;
}

NetworkDeviceDiscovery *HardwareManagerImplementation::networkDeviceDiscovery()
{
    return m_networkDeviceDiscovery;
}

void HardwareManagerImplementation::thingsLoaded()
{
    m_zigbeeResource->thingsLoaded();
    m_zwaveResource->thingsLoaded();
}

} // namespace nymeaserver
