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

#include "loggingcategories.h"

#include "platform/platform.h"

#include "platform/platformzeroconfcontroller.h"

#include "hardwaremanagerimplementation.h"
#include "hardware/plugintimermanagerimplementation.h"
#include "hardware/network/upnp/upnpdiscoveryimplementation.h"
#include "hardware/network/networkaccessmanagerimpl.h"
#include "hardware/radio433/radio433brennenstuhl.h"
#include "hardware/bluetoothlowenergy/bluetoothlowenergymanagerimplementation.h"
#include "hardware/network/mqtt/mqttproviderimplementation.h"
#include "hardware/i2c/i2cmanagerimplementation.h"
#include "hardware/zigbee/zigbeehardwareresourceimplementation.h"

#include "hardware/modbus/modbusrtumanager.h"
#include "hardware/modbus/modbusrtuhardwareresourceimplementation.h"
#include "network/networkdevicediscovery.h"

namespace nymeaserver {

HardwareManagerImplementation::HardwareManagerImplementation(Platform *platform, MqttBroker *mqttBroker, ZigbeeManager *zigbeeManager, ModbusRtuManager *modbusRtuManager, QObject *parent) :
    HardwareManager(parent),
    m_platform(platform)
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

#ifdef WITH_BLUETOOTH
    // Bluetooth LE
    m_bluetoothLowEnergyManager = new BluetoothLowEnergyManagerImplementation(m_pluginTimerManager->registerTimer(10), this);
#endif // WITH_BLUETOOTH

    m_i2cManager = new I2CManagerImplementation(this);

    m_zigbeeResource = new ZigbeeHardwareResourceImplementation(zigbeeManager, this);

    m_modbusRtuResource = new ModbusRtuHardwareResourceImplementation(modbusRtuManager, this);

    m_networkDeviceDiscovery = new NetworkDeviceDiscovery(this);

    // Enable all the resources
    setResourceEnabled(m_pluginTimerManager, true);
    setResourceEnabled(m_radio433, true);

    if (m_networkManager->available())
        setResourceEnabled(m_networkManager, true);

    if (m_upnpDiscovery->available())
        setResourceEnabled(m_upnpDiscovery, true);

    if (m_platform->zeroConfController()->available())
        setResourceEnabled(m_platform->zeroConfController(), true);

#ifdef WITH_BLUETOOTH
    if (m_bluetoothLowEnergyManager->available())
        setResourceEnabled(m_bluetoothLowEnergyManager, true);
#endif // WITH_BLUETOOTH

    m_mqttProvider = new MqttProviderImplementation(mqttBroker, this);
    qCDebug(dcHardware()) << "Hardware manager initialized successfully";
}

HardwareManagerImplementation::~HardwareManagerImplementation()
{

}

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
#ifdef WITH_BLUETOOTH
BluetoothLowEnergyManager *HardwareManagerImplementation::bluetoothLowEnergyManager()
{
    return m_bluetoothLowEnergyManager;
}
#endif // WITH_BLUETOOTH

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
}

}
