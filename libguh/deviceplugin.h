#ifndef DEVICEPLUGIN_H
#define DEVICEPLUGIN_H

#include "devicemanager.h"
#include "deviceclass.h"
#include "event.h"
#include "action.h"

#include <QObject>

class DeviceManager;
class Device;

class DevicePlugin: public QObject
{
    Q_OBJECT
public:
    DevicePlugin(QObject *parent = 0);
    virtual ~DevicePlugin();

    virtual void init() {}

    virtual QString pluginName() const = 0;
    virtual QUuid pluginId() const = 0;

    virtual QList<DeviceClass> supportedDevices() const = 0;
    virtual DeviceManager::HardwareResources requiredHardware() const = 0;

    // Hardware input
    virtual void radioData(QList<int> rawData) {Q_UNUSED(rawData)}
    virtual void guhTimer() {}

    virtual QVariantMap configuration() const;
    virtual void setConfiguration(const QVariantMap &configuration);

public slots:
    virtual void executeAction(Device *device, const Action &action) {Q_UNUSED(device) Q_UNUSED(action)}


signals:
    void emitEvent(const Event &event);

protected:
    DeviceManager *deviceManager() const;

    void transmitData(QList<int> rawData);

private:
    void initPlugin(DeviceManager *deviceManager);

    DeviceManager *m_deviceManager;

    friend class DeviceManager;
};
Q_DECLARE_INTERFACE(DevicePlugin, "org.guhyourhome.DevicePlugin")

#endif
