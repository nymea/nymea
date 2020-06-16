#include "hardwaremanagermock.h"

HardwareManagerMock::HardwareManagerMock(QObject *parent) : HardwareManager(parent)
{

}

Radio433 *HardwareManagerMock::radio433()
{
    Q_ASSERT_X(false, "HardwareManagerMock", "Radio 433 is not mocked yet.");
    return nullptr;
}

PluginTimerManager *HardwareManagerMock::pluginTimerManager()
{
    Q_ASSERT_X(false, "HardwareManagerMock", "Plugin Timer is not mocked yet.");
    return nullptr;
}

NetworkAccessManager *HardwareManagerMock::networkManager()
{
    Q_ASSERT_X(false, "HardwareManagerMock", "NetworkManager is not mocked yet.");
    return nullptr;
}

UpnpDiscovery *HardwareManagerMock::upnpDiscovery()
{
    Q_ASSERT_X(false, "HardwareManagerMock", "UPnP discovery is not mocked yet.");
    return nullptr;
}

PlatformZeroConfController *HardwareManagerMock::zeroConfController()
{
    Q_ASSERT_X(false, "HardwareManagerMock", "ZeroConfControlleris not mocked yet.");
    return nullptr;
}

BluetoothLowEnergyManager *HardwareManagerMock::bluetoothLowEnergyManager()
{
    Q_ASSERT_X(false, "HardwareManagerMock", "BtLE is not mocked yet.");
    return nullptr;
}

MqttProvider *HardwareManagerMock::mqttProvider()
{
    Q_ASSERT_X(false, "HardwareManagerMock", "MqttProvider is not mocked yet.");
    return nullptr;
}

I2CManager *HardwareManagerMock::i2cManager()
{
    Q_ASSERT_X(false, "HardwareManagerMock", "I2C Manager is not mocked yet.");
    return nullptr;
}
