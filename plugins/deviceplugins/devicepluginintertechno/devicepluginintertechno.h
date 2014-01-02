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

    void init() override;
    QList<DeviceClass> supportedDevices() const override;

    QString pluginName() const;
    QUuid pluginId() const;

public slots:
    void executeAction(Device *device, const Action &action) override;

private slots:
    void dataReceived(QList<int> rawData);
};

#endif // DEVICEPLUGININTERTECHNO_H
