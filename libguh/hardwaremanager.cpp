#include "hardwaremanager.h"

HardwareManager::HardwareManager(QObject *parent) : QObject(parent)
{
    // Init hardware resources
    m_pluginTimer = new PluginTimer(10000, this);

    m_radio433 = new Radio433(this);
    m_radio433->enable();

    // Network manager
    m_networkManager = new NetworkAccessManager(this);

    // UPnP discovery
    m_upnpDiscovery = new UpnpDiscovery(this);

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

