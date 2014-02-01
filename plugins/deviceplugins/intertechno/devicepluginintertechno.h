#ifndef DEVICEPLUGININTERTECHNO_H
#define DEVICEPLUGININTERTECHNO_H


#include "deviceplugin.h"

class DevicePluginIntertechno : public DevicePlugin
{
    Q_OBJECT

    Q_PLUGIN_METADATA(IID "org.hiveyourhome.DevicePlugin" FILE "devicepluginintertechno.json")
    Q_INTERFACES(DevicePlugin)

public:
    explicit DevicePluginIntertechno();

    QList<DeviceClass> supportedDevices() const override;
    DeviceManager::HardwareResources requiredHardware() const override;

    QString pluginName() const override;
    QUuid pluginId() const override;

    void radioData(QList<int> rawData) override;

public slots:
    void executeAction(Device *device, const Action &action) override;

};

#endif // DEVICEPLUGININTERTECHNO_H
