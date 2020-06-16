#ifndef HARDWAREMANAGERMOCK_H
#define HARDWAREMANAGERMOCK_H

#include <QObject>

#include "hardwaremanager.h"

class HardwareManagerMock : public HardwareManager
{
    Q_OBJECT
public:
    explicit HardwareManagerMock(QObject *parent = nullptr);

    virtual Radio433 *radio433() override;
    virtual PluginTimerManager *pluginTimerManager() override;
    virtual NetworkAccessManager *networkManager() override;
    virtual UpnpDiscovery *upnpDiscovery() override;
    virtual PlatformZeroConfController *zeroConfController() override;
    virtual BluetoothLowEnergyManager *bluetoothLowEnergyManager() override;
    virtual MqttProvider *mqttProvider() override;
    virtual I2CManager *i2cManager() override;

signals:

};

#endif // HARDWAREMANAGERMOCK_H
