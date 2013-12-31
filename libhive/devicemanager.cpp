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

    Device *device = new Device(deviceClassId, this);
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
    return DeviceClass(QUuid());
}

Radio433 *DeviceManager::radio433() const
{
    return m_radio433;
}

void DeviceManager::loadPlugins()
{
    foreach (QObject *pluginObject, QPluginLoader::staticInstances()) {
        DevicePlugin *pluginIface = qobject_cast<DevicePlugin*>(pluginObject);
        if (pluginIface) {
            qDebug() << "*** Loaded plugin" << pluginIface->pluginName();
            pluginIface->initPlugin(this);
            m_supportedDevices.append(pluginIface->supportedDevices());
            m_devicePlugins.append(pluginIface);
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
        device->setParams(device->params());
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
        settings.endGroup();
    }
}

