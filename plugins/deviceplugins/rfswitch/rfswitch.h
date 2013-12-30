#ifndef RFSWITCH_H
#define RFSWITCH_H

#include "deviceplugin.h"

class RfSwitch : public DevicePlugin
{
    Q_OBJECT

    Q_PLUGIN_METADATA(IID "org.hiveyourhome.DevicePlugin" FILE "rfswitch.json")
    Q_INTERFACES(DevicePlugin)

public:
    explicit RfSwitch();

    void init() override;
    QList<DeviceClass> supportedDevices() const override;

    QString pluginName() const;

private slots:
    void dataReceived(QList<int> rawData);
};

#endif // RFSWITCH_H
