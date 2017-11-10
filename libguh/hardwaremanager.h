#ifndef HARDWAREMANAGER_H
#define HARDWAREMANAGER_H

#include <QObject>

#include "plugintimer.h"
#include "bluetooth/bluetoothscanner.h"
#include "hardware/radio433/radio433.h"
#include "network/networkaccessmanager.h"
#include "network/upnp/upnpdiscovery.h"
#include "network/upnp/upnpdevicedescriptor.h"
#include "network/avahi/qtavahiservicebrowser.h"

class PluginTimer;
class UpnpDiscovery;

class HardwareManager : public QObject
{
    Q_OBJECT
public:
    explicit HardwareManager(QObject *parent = nullptr);

    Radio433 *radio433();
    PluginTimer *pluginTimer();
    NetworkAccessManager *networkManager();
    UpnpDiscovery *upnpDiscovery();
    QtAvahiServiceBrowser *avahiBrowser();
    BluetoothScanner *bluetoothScanner();

private:
    // Hardware Resources
    Radio433 *m_radio433 = nullptr;
    PluginTimer *m_pluginTimer = nullptr;
    NetworkAccessManager *m_networkManager = nullptr;
    UpnpDiscovery *m_upnpDiscovery = nullptr;
    QtAvahiServiceBrowser *m_avahiBrowser = nullptr;
    BluetoothScanner *m_bluetoothScanner = nullptr;

signals:

public slots:

};


#endif // HARDWAREMANAGER_H
