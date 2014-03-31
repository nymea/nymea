#ifndef DEVICEPLUGINCONRAD_H
#define DEVICEPLUGINCONRAD_H

#include "deviceplugin.h"

class DevicePluginConrad : public DevicePlugin
{
    Q_OBJECT

    Q_PLUGIN_METADATA(IID "org.guhyourhome.DevicePlugin" FILE "devicepluginconrad.json")
    Q_INTERFACES(DevicePlugin)

public:
    explicit DevicePluginConrad();

    QList<DeviceClass> supportedDevices() const override;
    DeviceManager::HardwareResources requiredHardware() const override;

    QString pluginName() const override;
    QUuid pluginId() const override;

    void radioData(QList<int> rawData) override;

public slots:
    void executeAction(Device *device, const Action &action) override;

};

#endif // DEVICEPLUGINCONRAD_H
