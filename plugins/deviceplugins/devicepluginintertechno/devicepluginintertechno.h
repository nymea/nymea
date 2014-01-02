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
    DeviceManager::HardwareResource requiredHardware() const override;

    QString pluginName() const;
    QUuid pluginId() const;

    void receiveData(QList<int> rawData);

public slots:
    void executeAction(Device *device, const Action &action) override;

};

#endif // DEVICEPLUGININTERTECHNO_H
