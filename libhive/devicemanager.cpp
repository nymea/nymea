/*!
    \class DeviceManager
    \brief The main entry point when interacting with \l{Device}{Devices}

    \ingroup devices
    \inmodule libhive

    The DeviceManager holds all information about supported and configured Devices in the system.

    It is also responsible for loading Plugins and managing common hardware resources between
    \l{DevicePlugin}{device plugins}.

*/

#include "devicemanager.h"

#include "radio433.h"

#include "device.h"
#include "deviceclass.h"
#include "deviceplugin.h"

#include <QPluginLoader>
#include <QtPlugin>
#include <QDebug>
#include <QSettings>
#include <QStringList>

Q_IMPORT_PLUGIN(DevicePluginElro)
Q_IMPORT_PLUGIN(DevicePluginIntertechno)
Q_IMPORT_PLUGIN(DevicePluginMeisterAnker)
Q_IMPORT_PLUGIN(DevicePluginWifiDetector)


DeviceManager::DeviceManager(QObject *parent) :
    QObject(parent),
    m_radio433(0)
{
    m_pluginTimer.setInterval(15000);
    connect(&m_pluginTimer, &QTimer::timeout, this, &DeviceManager::timerEvent);

    QMetaObject::invokeMethod(this, "loadPlugins", Qt::QueuedConnection);
    QMetaObject::invokeMethod(this, "loadConfiguredDevices", Qt::QueuedConnection);
}

QList<DevicePlugin *> DeviceManager::plugins() const
{
    return m_devicePlugins.values();
}

DevicePlugin *DeviceManager::plugin(const QUuid &id) const
{
    return m_devicePlugins.value(id);
}

QList<DeviceClass> DeviceManager::supportedDevices() const
{
    return m_supportedDevices.values();
}

DeviceManager::DeviceError DeviceManager::addConfiguredDevice(const QUuid &deviceClassId, const QVariantMap &params)
{
    DeviceClass deviceClass = findDeviceClass(deviceClassId);
    if (deviceClass.id().isNull()) {
        qWarning() << "cannot find a device class with id" << deviceClassId;
        return DeviceErrorDeviceClassNotFound;
    }
    foreach (const QVariant &param, deviceClass.params()) {
        if (!params.contains(param.toMap().value("name").toString())) {
            qWarning() << "Missing parameter when creating device:" << param.toMap().value("name").toString();
            return DeviceErrorMissingParameter;
        }
        // TODO: Check if parameter type matches
    }

    DevicePlugin *plugin = m_devicePlugins.value(deviceClass.pluginId());
    if (!plugin) {
        qWarning() << "Cannot find a plugin for this device class!";
        return DeviceErrorPluginNotFound;
    }

    Device *device = new Device(plugin->pluginId(), deviceClassId, this);
    device->setName(deviceClass.name());
    device->setParams(params);
    if (setupDevice(device)) {
        m_configuredDevices.append(device);
    } else {
        qWarning() << "Failed to set up device.";
        return DeviceErrorSetupFailed;
    }

    storeConfiguredDevices();

    return DeviceErrorNoError;
}

Device *DeviceManager::findConfiguredDevice(const QUuid &id) const
{
    foreach (Device *device, m_configuredDevices) {
        if (device->id() == id) {
            return device;
        }
    }
    return 0;
}

QList<Device *> DeviceManager::configuredDevices() const
{
    return m_configuredDevices;
}

QList<Device *> DeviceManager::findConfiguredDevices(const QUuid &deviceClassId) const
{
    QList<Device*> ret;
    foreach (Device *device, m_configuredDevices) {
        if (device->deviceClassId() == deviceClassId) {
            ret << device;
        }
    }
    return ret;
}

DeviceClass DeviceManager::findDeviceClassforTrigger(const QUuid &triggerTypeId) const
{
    foreach (const DeviceClass &deviceClass, m_supportedDevices) {
        foreach (const TriggerType &triggerType, deviceClass.triggers()) {
            if (triggerType.id() == triggerTypeId) {
                return deviceClass;
            }
        }
    }
    return DeviceClass(QUuid(), QUuid());
}

DeviceClass DeviceManager::findDeviceClass(const QUuid &deviceClassId) const
{
    foreach (const DeviceClass &deviceClass, m_supportedDevices) {
        if (deviceClass.id() == deviceClassId) {
            return deviceClass;
        }
    }
    return DeviceClass(QUuid(), QUuid());
}

DeviceManager::DeviceError DeviceManager::executeAction(const Action &action)
{
    foreach (Device *device, m_configuredDevices) {
        if (action.deviceId() == device->id()) {
            // found device

            // Make sure this device has an action type with this id
            DeviceClass deviceClass = findDeviceClass(device->deviceClassId());
            bool found = false;
            foreach (const ActionType &actionType, deviceClass.actions()) {
                if (actionType.id() == action.actionTypeId()) {
                    found = true;
                }
            }
            if (!found) {
                return DeviceErrorActionTypeNotFound;
            }

            m_devicePlugins.value(device->pluginId())->executeAction(device, action);
            return DeviceErrorNoError;
        }
    }
    return DeviceErrorDeviceNotFound;
}

void DeviceManager::loadPlugins()
{
    foreach (QObject *pluginObject, QPluginLoader::staticInstances()) {
        DevicePlugin *pluginIface = qobject_cast<DevicePlugin*>(pluginObject);
        if (pluginIface) {
            qDebug() << "*** Loaded plugin" << pluginIface->pluginName();
            pluginIface->initPlugin(this);
            foreach (const DeviceClass &deviceClass, pluginIface->supportedDevices()) {
                qDebug() << "* Loaded device class:" << deviceClass.name();
                m_supportedDevices.insert(deviceClass.id(), deviceClass);
            }
            m_devicePlugins.insert(pluginIface->pluginId(), pluginIface);
            connect(pluginIface, &DevicePlugin::emitTrigger, this, &DeviceManager::emitTrigger);
        }

    }
}

void DeviceManager::loadConfiguredDevices()
{
    QSettings settings;
    qDebug() << "loading devices";
    foreach (const QString &idString, settings.childGroups()) {
        settings.beginGroup(idString);
        Device *device = new Device(settings.value("pluginid").toUuid(), QUuid(idString), settings.value("deviceClassId").toUuid(), this);
        device->setName(settings.value("devicename").toString());
        device->setParams(settings.value("params").toMap());
        settings.endGroup();

        setupDevice(device);

        m_configuredDevices.append(device);
        qDebug() << "found stored device" << device->name() << idString;
    }
}

void DeviceManager::storeConfiguredDevices()
{
    QSettings settings;
    foreach (Device *device, m_configuredDevices) {
        settings.beginGroup(device->id().toString());
        settings.setValue("devicename", device->name());
        settings.setValue("deviceClassId", device->deviceClassId().toString());
        settings.setValue("pluginid", device->pluginId());
        settings.setValue("params", device->params());
        settings.endGroup();
    }
}

void DeviceManager::radio433SignalReceived(QList<int> rawData)
{
    foreach (Device *device, m_configuredDevices) {
        DeviceClass deviceClass = m_supportedDevices.value(device->deviceClassId());
        DevicePlugin *plugin = m_devicePlugins.value(deviceClass.pluginId());
        if (plugin->requiredHardware().testFlag(HardwareResourceRadio433)) {
            plugin->radioData(rawData);
        }
    }
}

void DeviceManager::timerEvent()
{
    foreach (Device *device, m_configuredDevices) {
        DeviceClass deviceClass = m_supportedDevices.value(device->deviceClassId());
        DevicePlugin *plugin = m_devicePlugins.value(deviceClass.pluginId());
        if (plugin->requiredHardware().testFlag(HardwareResourceTimer)) {
            plugin->hiveTimer();
        }
    }
}

bool DeviceManager::setupDevice(Device *device)
{
    DeviceClass deviceClass = findDeviceClass(device->deviceClassId());
    DevicePlugin *plugin = m_devicePlugins.value(deviceClass.pluginId());

    QList<State> states;
    foreach (const StateType &stateType, deviceClass.states()) {
        State state(stateType.id(), device->id());
        state.setValue(stateType.defaultValue());
        states.append(state);
    }
    device->setStates(states);

    if (plugin->requiredHardware().testFlag(HardwareResourceRadio433)) {
        if (!m_radio433) {
            m_radio433 = new Radio433();
            connect(m_radio433, &Radio433::dataReceived, this, &DeviceManager::radio433SignalReceived);
        }
    }

    if (plugin->requiredHardware().testFlag(HardwareResourceTimer)) {
        if (!m_pluginTimer.isActive()) {
            m_pluginTimer.start();
            // Additionally fire off one event to initialize stuff
            QTimer::singleShot(0, this, SLOT(timerEvent()));
        }
    }

    return true;
}
