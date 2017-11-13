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

    bool isAvailable(const HardwareResource::Type &hardwareResourceType) const;
    bool isEnabled(const HardwareResource::Type &hardwareResourceType) const;

private:
    QList<HardwareResource *> m_hardwareResources;

    // Hardware Resources
    Radio433 *m_radio433 = nullptr;
    PluginTimer *m_pluginTimer = nullptr;
    NetworkAccessManager *m_networkManager = nullptr;
    UpnpDiscovery *m_upnpDiscovery = nullptr;
    QtAvahiServiceBrowser *m_avahiBrowser = nullptr;
    BluetoothScanner *m_bluetoothScanner = nullptr;

signals:
    void hardwareResourceAvailableChanged(const HardwareResource::Type &hardwareResourceType, const bool &available);
    void hardwareResourceEnabledChanged(const HardwareResource::Type &hardwareResourceType, const bool &enabled);

public slots:
    bool enableHardwareReource(const HardwareResource::Type &hardwareResourceType);
    bool disableHardwareReource(const HardwareResource::Type &hardwareResourceType);

};


#endif // HARDWAREMANAGER_H
