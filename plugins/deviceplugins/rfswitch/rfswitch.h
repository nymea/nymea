#ifndef RFSWITCH_H
#define RFSWITCH_H

#include "deviceplugin.h"

class RfSwitch : public QObject, public DevicePlugin
{
    Q_OBJECT

    Q_PLUGIN_METADATA(IID "org.hiveyourhome.DevicePlugin" FILE "rfswitch.json")
    Q_INTERFACES(DevicePlugin)

public:
    explicit RfSwitch();

    QList<DeviceClass> supportedDevices() const override;

    QString pluginName() const;

signals:

public slots:

};

#endif // RFSWITCH_H
