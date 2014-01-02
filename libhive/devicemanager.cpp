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

Radio433 *DeviceManager::radio433() const
{
    return m_radio433;
}

DeviceManager::DeviceError DeviceManager::executeAction(const Action &action)
{
    foreach (Device *device, m_configuredDevices) {
        if (action.deviceId() == device->id()) {
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
                m_supportedDevices.append(deviceClass);
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
        qDebug() << "found stored device" << idString;
        settings.beginGroup(idString);
        Device *device = new Device(settings.value("pluginid").toUuid(), QUuid(idString), settings.value("deviceClassId").toUuid(), this);
        device->setName(settings.value("devicename").toString());
        device->setParams(settings.value("params").toMap());
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
        settings.setValue("pluginid", device->pluginId());
        settings.setValue("params", device->params());
        settings.endGroup();
    }
}
