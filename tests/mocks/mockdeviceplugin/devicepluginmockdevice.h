#ifndef DEVICEPLUGINMOCKDEVICE_H
#define DEVICEPLUGINMOCKDEVICE_H

#include "deviceplugin.h"

class DevicePluginMockDevice: public DevicePlugin
{
    Q_OBJECT

    Q_PLUGIN_METADATA(IID "org.guhyourhome.DevicePlugin" FILE "devicepluginmockdevice.json")
    Q_INTERFACES(DevicePlugin)

public:
    explicit DevicePluginMockDevice();

    QList<DeviceClass> supportedDevices() const override;
    DeviceManager::HardwareResources requiredHardware() const override;

    QString pluginName() const override;
    QUuid pluginId() const override;

    void guhTimer() override;

};

#endif // DEVICEPLUGINMOCKDEVICE_H
