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

    void init() override;
    QList<DeviceClass> supportedDevices() const override;

    QString pluginName() const;
    QUuid pluginId() const;

public slots:
    void executeAction(Device *device, const Action &action) override;

private slots:
    void dataReceived(QList<int> rawData);
};

#endif // DEVICEPLUGINMEISTERANKER_H
