#ifndef DEVICEMANAGER_H
#define DEVICEMANAGER_H

#include "deviceclass.h"
#include "trigger.h"
#include "action.h"

#include <QObject>

class Device;
class DevicePlugin;
class Radio433;

class DeviceManager : public QObject
{
    Q_OBJECT
public:
    enum HardwareResource {
        HardwareResourceRadio433 = 0x01,
        HardwareResourceRadio868 = 0x02
    };

    enum DeviceError {
        DeviceErrorNoError,
        DeviceErrorDeviceNotFound,
        DeviceErrorDeviceClassNotFound,
        DeviceErrorMissingParameter,
        DeviceErrorPluginNotFound
    };

    explicit DeviceManager(QObject *parent = 0);

    QList<DeviceClass> supportedDevices();

    QList<Device*> configuredDevices() const;
    DeviceError addConfiguredDevice(const QUuid &deviceClassId, const QVariantMap &params);

    QList<Device*> findConfiguredDevices(const QUuid &deviceClassId);
    DeviceClass findDeviceClass(const QUuid &deviceClassId);

signals:
    void emitTrigger(const Trigger &trigger);

public slots:
    DeviceError executeAction(const Action &action);

private slots:
    void loadPlugins();
    void loadConfiguredDevices();
    void storeConfiguredDevices();

    void radio433SignalReceived(QList<int> rawData);

private:
    QHash<QUuid, DeviceClass> m_supportedDevices;
    QList<Device*> m_configuredDevices;

    QHash<QUuid, DevicePlugin*> m_devicePlugins;

    Radio433* m_radio433;

    friend class DevicePlugin;
};

#endif // DEVICEMANAGER_H
