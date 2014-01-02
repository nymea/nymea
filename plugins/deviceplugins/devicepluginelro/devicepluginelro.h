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

    void init() override;
    QList<DeviceClass> supportedDevices() const override;

    QString pluginName() const;
    QUuid pluginId() const;

public slots:
    void executeAction(Device *device, const Action &action) override;

private slots:
    void dataReceived(QList<int> rawData);
};

#endif // DEVICEPLUGINELRO_H
