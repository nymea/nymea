#ifndef RFREMOTEINTERTECHNO_H
#define RFREMOTEINTERTECHNO_H

#include "deviceplugin.h"

class RfRemoteIntertechno : public DevicePlugin
{
    Q_OBJECT

    Q_PLUGIN_METADATA(IID "org.hiveyourhome.DevicePlugin" FILE "rfremoteintertechno.json")
    Q_INTERFACES(DevicePlugin)

public:
    explicit RfRemoteIntertechno();

    void init() override;
    QList<DeviceClass> supportedDevices() const override;

    QString pluginName() const;

private slots:
    void dataReceived(QList<int> rawData);
};

#endif // RFREMOTEINTERTECHNO_H
