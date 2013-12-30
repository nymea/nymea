#ifndef DEVICEPLUGIN_H
#define DEVICEPLUGIN_H

#include "deviceclass.h"
#include "trigger.h"

#include <QObject>

class DeviceManager;

class DevicePlugin
{
public:
    DevicePlugin();
    virtual ~DevicePlugin();

    void initPlugin(DeviceManager *deviceManager);

    virtual void init() {}

    virtual QString pluginName() const = 0;

    virtual QList<DeviceClass> supportedDevices() const = 0;

signals:
    void emitTrigger(const Trigger &trigger);

protected:
    DeviceManager *deviceManager() const;

private:
    DeviceManager *m_deviceManager;
};
Q_DECLARE_INTERFACE(DevicePlugin, "org.hoveyourhome.DevicePlugin")

#endif
