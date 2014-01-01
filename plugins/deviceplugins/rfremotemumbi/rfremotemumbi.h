#ifndef RFREMOTEMUMBI_H
#define RFREMOTEMUMBI_H

#include "deviceplugin.h"

class RfRemoteMumbi : public DevicePlugin
{
    Q_OBJECT

    Q_PLUGIN_METADATA(IID "org.hiveyourhome.DevicePlugin" FILE "rfremotemumbi.json")
    Q_INTERFACES(DevicePlugin)

public:
    explicit RfRemoteMumbi();

    void init() override;
    QList<DeviceClass> supportedDevices() const override;

    QString pluginName() const;
    QUuid pluginId() const;

public slots:
    void executeAction(Device *device, const Action &action) override;

private slots:
    void dataReceived(QList<int> rawData);
};

#endif // RFREMOTEMUMBI_H
