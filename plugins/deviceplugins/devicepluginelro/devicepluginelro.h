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
    DeviceManager::HardwareResource requiredHardware() const override;

    QString pluginName() const;
    QUuid pluginId() const;

    void receiveData(QList<int> rawData);

public slots:
    void executeAction(Device *device, const Action &action) override;

};

#endif // DEVICEPLUGINELRO_H
