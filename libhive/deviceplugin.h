#ifndef DEVICEPLUGIN_H
#define DEVICEPLUGIN_H

#include "deviceclass.h"

#include <QObject>

class DeviceManager;

class DevicePlugin
{
public:
    DevicePlugin();
    virtual ~DevicePlugin();

    void init(DeviceManager *deviceManager);

    virtual QString pluginName() const = 0;

    virtual QList<DeviceClass> supportedDevices() const = 0;

protected:
    DeviceManager *deviceManager() const;

private:
    DeviceManager *m_deviceManager;
};
Q_DECLARE_INTERFACE(DevicePlugin, "org.hoveyourhome.DevicePlugin")

#endif
