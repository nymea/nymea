#ifndef DEVICEPLUGINELRO_H
#define DEVICEPLUGINELRO_H

#include "deviceplugin.h"

class DevicePluginElro : public DevicePlugin
{
    Q_OBJECT

    Q_PLUGIN_METADATA(IID "org.hiveyourhome.DevicePlugin" FILE "devicepluginelro.json")
    Q_INTERFACES(DevicePlugin)

public:
    explicit DevicePluginElro();

    QList<DeviceClass> supportedDevices() const override;
    DeviceManager::HardwareResources requiredHardware() const override;

    QString pluginName() const override;
    QUuid pluginId() const override;

    void radioData(QList<int> rawData) override;

public slots:
    void executeAction(Device *device, const Action &action) override;

};

#endif // DEVICEPLUGINELRO_H
