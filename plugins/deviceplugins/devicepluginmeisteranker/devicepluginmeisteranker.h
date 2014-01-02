#ifndef DEVICEPLUGINMEISTERANKER_H
#define DEVICEPLUGINMEISTERANKER_H

#include "deviceplugin.h"


class DevicePluginMeisterAnker : public DevicePlugin
{
    Q_OBJECT

    Q_PLUGIN_METADATA(IID "org.hiveyourhome.DevicePlugin" FILE "devicepluginmeisteranker.json")
    Q_INTERFACES(DevicePlugin)

public:
    explicit DevicePluginMeisterAnker();

    QList<DeviceClass> supportedDevices() const override;
    DeviceManager::HardwareResource requiredHardware() const override;

    QString pluginName() const;
    QUuid pluginId() const;

    void receiveData(QList<int> rawData);

public slots:
    void executeAction(Device *device, const Action &action) override;
};

#endif // DEVICEPLUGINMEISTERANKER_H
