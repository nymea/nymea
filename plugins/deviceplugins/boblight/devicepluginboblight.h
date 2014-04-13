#ifndef DEVICEPLUGINBOBLIGHT_H
#define DEVICEPLUGINBOBLIGHT_H

#include "plugin/deviceplugin.h"

#include <QProcess>

class BobClient;

class DevicePluginBoblight : public DevicePlugin
{
    Q_OBJECT

    Q_PLUGIN_METADATA(IID "org.hiveyourhome.DevicePlugin" FILE "devicepluginboblight.json")
    Q_INTERFACES(DevicePlugin)

public:
    explicit DevicePluginBoblight();

    QList<Vendor> supportedVendors() const override;
    QList<DeviceClass> supportedDevices() const override;
    DeviceManager::HardwareResources requiredHardware() const override;

    bool configureAutoDevice(QList<Device *> loadedDevices, Device *device) const override;

    QString pluginName() const override;
    PluginId pluginId() const override;

    QVariantMap configuration() const override;
    void setConfiguration(const QVariantMap &configuration) override;

public slots:
    DeviceManager::DeviceError executeAction(Device *device, const Action &action);

private slots:
    void connectToBoblight();

private:
    BobClient *m_bobClient;
    QVariantMap m_config;
};

#endif // DEVICEPLUGINBOBLIGHT_H
