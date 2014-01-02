#ifndef DEVICEPLUGIN_H
#define DEVICEPLUGIN_H

#include "deviceclass.h"
#include "trigger.h"
#include "action.h"

#include <QObject>

class DeviceManager;
class Device;

class DevicePlugin: public QObject
{
    Q_OBJECT
public:
    DevicePlugin();
    virtual ~DevicePlugin();

    void initPlugin(DeviceManager *deviceManager);

    virtual void init() {}

    virtual QString pluginName() const = 0;
    virtual QUuid pluginId() const = 0;

    virtual QList<DeviceClass> supportedDevices() const = 0;

public slots:
    virtual void executeAction(Device *device, const Action &action) = 0;

signals:
    void emitTrigger(const Trigger &trigger);

protected:
    DeviceManager *deviceManager() const;

private:
    DeviceManager *m_deviceManager;
};
Q_DECLARE_INTERFACE(DevicePlugin, "org.hiveyourhome.DevicePlugin")

#endif
