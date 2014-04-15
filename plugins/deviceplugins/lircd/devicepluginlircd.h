#ifndef DEVICEPLUGINLIRCD_H
#define DEVICEPLUGINLIRCD_H

#include "plugin/deviceplugin.h"

#include <QProcess>

class LircClient;

class DevicePluginLircd: public DevicePlugin
{
    Q_OBJECT

    Q_PLUGIN_METADATA(IID "org.hiveyourhome.DevicePlugin" FILE "devicepluginlircd.json")
    Q_INTERFACES(DevicePlugin)

public:
    explicit DevicePluginLircd();

    QList<Vendor> supportedVendors() const override;
    QList<DeviceClass> supportedDevices() const override;
    DeviceManager::HardwareResources requiredHardware() const override;

    QString pluginName() const override;
    PluginId pluginId() const override;

//    QVariantMap configuration() const override;
//    void setConfiguration(const QVariantMap &configuration) override;

private slots:
    void buttonPressed(const QString &remoteName, const QString &buttonName, int repeat);

private:
    LircClient *m_lircClient;
//    QVariantMap m_config;
};

#endif // DEVICEPLUGINBOLIRCD_H
