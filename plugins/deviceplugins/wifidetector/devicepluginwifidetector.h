#ifndef DEVICEPLUGINWIFIDETECTOR_H
#define DEVICEPLUGINWIFIDETECTOR_H

#include "deviceplugin.h"

#include <QProcess>

class DevicePluginWifiDetector : public DevicePlugin
{
    Q_OBJECT

    Q_PLUGIN_METADATA(IID "org.guhyourhome.DevicePlugin" FILE "devicepluginwifidetector.json")
    Q_INTERFACES(DevicePlugin)

public:
    explicit DevicePluginWifiDetector();

    QList<DeviceClass> supportedDevices() const override;
    DeviceManager::HardwareResources requiredHardware() const override;

    QString pluginName() const override;
    QUuid pluginId() const override;

    void guhTimer() override;

private slots:
    void processFinished(int exitCode, QProcess::ExitStatus exitStatus);
};

#endif // DEVICEPLUGINWIFIDETECTOR_H
