#include "hardwaremanager.h"

#include "plugintimer.h"
#include "bluetooth/bluetoothscanner.h"
#include "hardware/radio433/radio433.h"
#include "network/networkaccessmanager.h"
#include "network/upnp/upnpdiscovery.h"
#include "network/upnp/upnpdevicedescriptor.h"
#include "network/avahi/qtavahiservicebrowser.h"

HardwareManager::HardwareManager(QObject *parent) : QObject(parent)
{
    // Init hardware resources
    m_pluginTimer = new PluginTimer(10000, this);
    m_hardwareResources.append(m_pluginTimer);

    m_radio433 = new Radio433(this);
    m_hardwareResources.append(m_radio433);
    m_radio433->enable();

    // Create network access manager for all resources, centralized
    m_networkAccessManager = new QNetworkAccessManager(this);
    // Note: configuration and proxy settings could be implemented here

    // Network manager
    m_networkManager = new NetworkAccessManager(m_networkAccessManager, this);
    m_hardwareResources.append(m_networkManager);
    m_networkManager->enable();

    // UPnP discovery
    m_upnpDiscovery = new UpnpDiscovery(m_networkAccessManager, this);

    // Avahi Browser
    m_avahiBrowser = new QtAvahiServiceBrowser(this);
    m_avahiBrowser->enable();

    // Bluetooth LE
    m_bluetoothScanner = new BluetoothScanner(this);
    if (!m_bluetoothScanner->isAvailable()) {
        delete m_bluetoothScanner;
        m_bluetoothScanner = nullptr;
    }
}

Radio433 *HardwareManager::radio433()
{
    return m_radio433;
}

PluginTimer *HardwareManager::pluginTimer()
{
    return m_pluginTimer;
}

NetworkAccessManager *HardwareManager::networkManager()
{
    return m_networkManager;
}

UpnpDiscovery *HardwareManager::upnpDiscovery()
{
    return m_upnpDiscovery;
}

QtAvahiServiceBrowser *HardwareManager::avahiBrowser()
{
    return m_avahiBrowser;
}

BluetoothScanner *HardwareManager::bluetoothScanner()
{
    return m_bluetoothScanner;
}

bool HardwareManager::isAvailable(const HardwareResource::Type &hardwareResourceType) const
{
    foreach (HardwareResource *resource, m_hardwareResources) {
        if (resource->hardwareReourceType() == hardwareResourceType && resource->available()) {
            return true;
        }
    }
    return false;
}

bool HardwareManager::isEnabled(const HardwareResource::Type &hardwareResourceType) const
{
    foreach (HardwareResource *resource, m_hardwareResources) {
        if (resource->hardwareReourceType() == hardwareResourceType && resource->enabled()) {
            return true;
        }
    }
    return false;
}

bool HardwareManager::enableHardwareReource(const HardwareResource::Type &hardwareResourceType)
{
    foreach (HardwareResource *resource, m_hardwareResources) {
        if (resource->hardwareReourceType() == hardwareResourceType) {
            return resource->enabled();
        }
    }
    return false;
}

bool HardwareManager::disableHardwareReource(const HardwareResource::Type &hardwareResourceType)
{
    foreach (HardwareResource *resource, m_hardwareResources) {
        if (resource->hardwareReourceType() == hardwareResourceType) {
             return resource->disable();
        }
    }
    return false;
}

