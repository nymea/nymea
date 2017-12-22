/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2017 Simon Stürz <simon.stuerz@guh.io>                   *
 *                                                                         *
 *  This file is part of guh.                                              *
 *                                                                         *
 *  This library is free software; you can redistribute it and/or          *
 *  modify it under the terms of the GNU Lesser General Public             *
 *  License as published by the Free Software Foundation; either           *
 *  version 2.1 of the License, or (at your option) any later version.     *
 *                                                                         *
 *  This library is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
 *  Lesser General Public License for more details.                        *
 *                                                                         *
 *  You should have received a copy of the GNU Lesser General Public       *
 *  License along with this library; If not, see                           *
 *  <http://www.gnu.org/licenses/>.                                        *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "loggingcategories.h"

#include "hardwaremanagerimplementation.h"
#include "hardware/network/upnp/upnpdiscoveryimplementation.h"
#include "hardware/network/networkaccessmanagerimpl.h"
#include "hardware/radio433/radio433brennenstuhl.h"
#include "hardware/bluetoothlowenergy/bluetoothlowenergymanagerimplementation.h"
#include "hardware/network/avahi/qtavahiservicebrowserimplementation.h"

namespace guhserver {

HardwareManagerImplementation::HardwareManagerImplementation(QObject *parent) :
    HardwareManager(parent)
{
    // Init hardware resources
    m_pluginTimerManager = new PluginTimerManagerImplementation(this);
    setResourceEnabled(m_pluginTimerManager, true);

    m_radio433 = new Radio433Brennenstuhl(this);
    setResourceEnabled(m_radio433, true);

    // Create network access manager for all resources, centralized
    // Note: configuration and proxy settings could be implemented here
    m_networkAccessManager = new QNetworkAccessManager(this);

    // Network manager
    m_networkManager = new NetworkAccessManagerImpl(m_networkAccessManager, this);
    setResourceEnabled(m_networkManager, true);
    if (m_networkManager->available())
        setResourceEnabled(m_networkManager, true);

    // UPnP discovery
    m_upnpDiscovery = new UpnpDiscoveryImplementation(m_networkAccessManager, this);
    if (m_upnpDiscovery->available())
        setResourceEnabled(m_upnpDiscovery, true);

    // Avahi Browser
    m_avahiBrowser = new QtAvahiServiceBrowserImplementation(this);
    if (m_avahiBrowser->available())
        setResourceEnabled(m_avahiBrowser, true);

    // Bluetooth LE
    m_bluetoothLowEnergyManager = new BluetoothLowEnergyManagerImplementation(m_pluginTimerManager->registerTimer(10), this);
    if (m_bluetoothLowEnergyManager->available())
        setResourceEnabled(m_bluetoothLowEnergyManager, true);


    qCDebug(dcHardware()) << "Hardware manager initialized successfully";

    // Register D-Bus interface for enable/disable hardware resources
    bool status = QDBusConnection::systemBus().registerService("io.guh.nymead");
    if (!status) {
        qCWarning(dcHardware()) << "Failed to register HardwareManager D-Bus service. HardwareManager D-Bus control will not work.";
        return;
    }

    status = QDBusConnection::systemBus().registerObject("/io/guh/nymead/HardwareManager", this, QDBusConnection::ExportScriptableContents);
    if (!status) {
        qCWarning(dcHardware()) << "Failed to register HardwareManager D-Bus object. HardwareManager D-Bus control will not work.";
        return;
    }
    qCDebug(dcHardware()) << "HardwareManager D-Bus service set up.";
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

QtAvahiServiceBrowser *HardwareManagerImplementation::avahiBrowser()
{
    return m_avahiBrowser;
}

BluetoothLowEnergyManager *HardwareManagerImplementation::bluetoothLowEnergyManager()
{
    return m_bluetoothLowEnergyManager;
}

void HardwareManagerImplementation::EnableBluetooth(const bool &enabled)
{
    qCDebug(dcHardware()) << "Bluetooth hardware resource" << (enabled ? "enabled" : "disabled");

    if (enabled) {
        setResourceEnabled(m_bluetoothLowEnergyManager, true);
    } else {
        setResourceEnabled(m_bluetoothLowEnergyManager, false);
    }
}


void HardwareManagerImplementation::timeTick()
{
    m_pluginTimerManager->timeTick();
}

}
