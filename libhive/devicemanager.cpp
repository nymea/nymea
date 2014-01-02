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

Q_IMPORT_PLUGIN(RfRemoteMumbi)
Q_IMPORT_PLUGIN(RfRemoteIntertechno)


DeviceManager::DeviceManager(QObject *parent) :
    QObject(parent)
{
    m_radio433 = new Radio433(this);

    QMetaObject::invokeMethod(this, "loadPlugins", Qt::QueuedConnection);
    QMetaObject::invokeMethod(this, "loadConfiguredDevices", Qt::QueuedConnection);
}

QList<DeviceClass> DeviceManager::supportedDevices()
{
    return m_supportedDevices;
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
    QList<Trigger> triggers;
    foreach (const TriggerType &triggerType, deviceClass.triggers()) {
        Trigger trigger(QUuid::createUuid());
        trigger.setName(triggerType.name());
        trigger.setParams(triggerType.parameters());
        triggers.append(trigger);
    }
    device->setTriggers(triggers);
    QList<Action> actions;
    foreach (const ActionType &actionType, deviceClass.actions()) {
        Action action(QUuid::createUuid());
        action.setName(actionType.name());
        action.setParams(actionType.parameters());
        actions.append(action);
    }
    device->setActions(actions);
    m_configuredDevices.append(device);

    storeConfiguredDevices();

    return DeviceErrorNoError;
}

QList<Device *> DeviceManager::configuredDevices() const
{
    return m_configuredDevices;
}

QList<Device *> DeviceManager::findConfiguredDevices(const QUuid &deviceClassId)
{
    QList<Device*> ret;
    foreach (Device *device, m_configuredDevices) {
        if (device->deviceClassId() == deviceClassId) {
            ret << device;
        }
    }
    return ret;
}

DeviceClass DeviceManager::findDeviceClass(const QUuid &deviceClassId)
{
    foreach (const DeviceClass &deviceClass, m_supportedDevices) {
        if (deviceClass.id() == deviceClassId) {
            return deviceClass;
        }
    }
    return DeviceClass(QUuid(), QUuid());
}

Trigger DeviceManager::findTrigger(const QUuid &triggerId)
{
    foreach (Device *device, m_configuredDevices) {
        foreach (const Trigger &trigger, device->triggers()) {
            if (trigger.id() == triggerId) {
                return trigger;
            }
        }
    }
    return Trigger();
}

Action DeviceManager::findAction(const QUuid &actionId)
{
    foreach (Device *device, m_configuredDevices) {
        foreach (const Action &action, device->actions()) {
            if (action.id() == actionId) {
                return action;
            }
        }
    }
    return Action();
}

Radio433 *DeviceManager::radio433() const
{
    return m_radio433;
}

void DeviceManager::executeAction(const QUuid &actionId, const QVariantMap &params)
{
    foreach (Device *device, m_configuredDevices) {
        foreach (const Action &action, device->actions()) {
            if (action.id() == actionId) {
                m_devicePlugins.value(device->pluginId())->executeAction(device, action);
                return;
            }
        }
    }
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
                m_supportedDevices.append(deviceClass);
            }
            m_devicePlugins.insert(pluginIface->pluginId(), pluginIface);
            connect(pluginIface,SIGNAL(emitTrigger(QUuid,QVariantMap)),this,SIGNAL(emitTrigger(QUuid,QVariantMap)));
        }

    }
}

void DeviceManager::loadConfiguredDevices()
{
    QSettings settings;
    qDebug() << "loading devices";
    foreach (const QString &idString, settings.childGroups()) {
        qDebug() << "found stored device" << idString;
        settings.beginGroup(idString);
        Device *device = new Device(QUuid(idString), settings.value("deviceClassId").toUuid(), this);
        device->setName(settings.value("devicename").toString());
        device->setParams(settings.value("params").toMap());
        settings.beginGroup("triggers");
        QList<Trigger> triggerList;
        foreach (const QString &triggerId, settings.childGroups()) {
            settings.beginGroup(triggerId);
            QUuid id(triggerId);
            Trigger trigger(id);
            trigger.setName(settings.value("triggername").toString());
            trigger.setParams(settings.value("params").toList());
            settings.endGroup();
            triggerList.append(trigger);
        }
        device->setTriggers(triggerList);
        settings.endGroup();
        settings.beginGroup("actions");
        QList<Action> actionList;
        foreach (const QString &actionId, settings.childGroups()) {
            settings.beginGroup(actionId);
            QUuid id(actionId);
            Action action(id);
            action.setName(settings.value("actionname").toString());
            action.setParams(settings.value("params").toList());
            settings.endGroup();
            actionList.append(action);
        }
        device->setActions(actionList);
        settings.endGroup();
        settings.endGroup();
        m_configuredDevices.append(device);
    }
}

void DeviceManager::storeConfiguredDevices()
{
    QSettings settings;
    foreach (Device *device, m_configuredDevices) {
        settings.beginGroup(device->id().toString());
        settings.setValue("devicename", device->name());
        settings.setValue("deviceClassId", device->deviceClassId().toString());
        settings.setValue("params", device->params());
        settings.beginGroup("triggers");
        foreach (const Trigger &trigger, device->triggers()) {
            settings.beginGroup(trigger.id().toString());
            settings.setValue("triggername", trigger.name());
            settings.setValue("params", trigger.params());
            settings.endGroup();
        }
        settings.endGroup();
        settings.beginGroup("actions");
        foreach (const Action &action, device->actions()) {
            settings.beginGroup(action.id().toString());
            settings.setValue("actionname", action.name());
            settings.setValue("params", action.params());
            settings.endGroup();
        }
        settings.endGroup();
        settings.endGroup();
    }
}
