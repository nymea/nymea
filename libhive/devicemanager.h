#ifndef DEVICEMANAGER_H
#define DEVICEMANAGER_H

#include "deviceclass.h"
#include "trigger.h"
#include "action.h"

#include <QObject>
#include <QTimer>

class Device;
class DevicePlugin;
class Radio433;

class DeviceManager : public QObject
{
    Q_OBJECT
public:
    enum HardwareResource {
        HardwareResourceNone = 0x00,
        HardwareResourceRadio433 = 0x01,
        HardwareResourceRadio868 = 0x02,
        HardwareResourceTimer = 0x04
    };
    Q_DECLARE_FLAGS(HardwareResources, HardwareResource)

    enum DeviceError {
        DeviceErrorNoError,
        DeviceErrorDeviceNotFound,
        DeviceErrorDeviceClassNotFound,
        DeviceErrorActionTypeNotFound,
        DeviceErrorMissingParameter,
        DeviceErrorPluginNotFound,
        DeviceErrorSetupFailed
    };

    explicit DeviceManager(QObject *parent = 0);

    QList<DevicePlugin*> plugins() const;
    DevicePlugin* plugin(const QUuid &id) const;
    QList<DeviceClass> supportedDevices() const;

    QList<Device*> configuredDevices() const;
    DeviceError addConfiguredDevice(const QUuid &deviceClassId, const QVariantMap &params);

    Device* findConfiguredDevice(const QUuid &id) const;
    QList<Device*> findConfiguredDevices(const QUuid &deviceClassId) const;
    DeviceClass findDeviceClassforTrigger(const QUuid &triggerTypeId) const;
    DeviceClass findDeviceClass(const QUuid &deviceClassId) const;

signals:
    void loaded();
    void emitTrigger(const Trigger &trigger);

public slots:
    DeviceError executeAction(const Action &action);

private slots:
    void loadPlugins();
    void loadConfiguredDevices();
    void storeConfiguredDevices();

    void radio433SignalReceived(QList<int> rawData);
    void timerEvent();

private:
    bool setupDevice(Device *device);

    QHash<QUuid, DeviceClass> m_supportedDevices;
    QList<Device*> m_configuredDevices;

    QHash<QUuid, DevicePlugin*> m_devicePlugins;

    // Hardware Resources
    Radio433* m_radio433;
    QTimer m_pluginTimer;

    friend class DevicePlugin;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(DeviceManager::HardwareResources)

#endif // DEVICEMANAGER_H
