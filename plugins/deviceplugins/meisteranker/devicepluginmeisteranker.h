#ifndef DEVICEPLUGINMEISTERANKER_H
#define DEVICEPLUGINMEISTERANKER_H

#include "deviceplugin.h"


class DevicePluginMeisterAnker : public DevicePlugin
{
    Q_OBJECT

    Q_PLUGIN_METADATA(IID "org.guhyourhome.DevicePlugin" FILE "devicepluginmeisteranker.json")
    Q_INTERFACES(DevicePlugin)

public:
    explicit DevicePluginMeisterAnker();

    QList<DeviceClass> supportedDevices() const override;
    DeviceManager::HardwareResources requiredHardware() const override;

    QString pluginName() const override;
    QUuid pluginId() const override;

    void radioData(QList<int> rawData) override;

public slots:
    void executeAction(Device *device, const Action &action) override;
};

#endif // DEVICEPLUGINMEISTERANKER_H
