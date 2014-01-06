#ifndef DEVICEPLUGIN_H
#define DEVICEPLUGIN_H

#include "devicemanager.h"
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
    virtual DeviceManager::HardwareResources requiredHardware() const = 0;

    // Hardware input
    virtual void radioData(QList<int> rawData) {Q_UNUSED(rawData)}
    virtual void hiveTimer() {}

    virtual QVariantMap configuration() const;
    virtual void setConfiguration(const QVariantMap &configuration);

public slots:
    virtual void executeAction(Device *device, const Action &action) {Q_UNUSED(device) Q_UNUSED(action)}


signals:
    void emitTrigger(const Trigger &trigger);

protected:
    DeviceManager *deviceManager() const;

    void transmitData(QList<int> rawData);

private:
    DeviceManager *m_deviceManager;
};
Q_DECLARE_INTERFACE(DevicePlugin, "org.hiveyourhome.DevicePlugin")

#endif
