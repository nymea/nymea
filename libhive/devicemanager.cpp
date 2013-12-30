#include "devicemanager.h"

#include "radio433.h"

#include "device.h"
#include "deviceclass.h"
#include "deviceplugin.h"

#include <QPluginLoader>
#include <QtPlugin>
#include <QDebug>

Q_IMPORT_PLUGIN(RfSwitch)

DeviceManager::DeviceManager(QObject *parent) :
    QObject(parent)
{
    m_radio433 = new Radio433(this);

    QMetaObject::invokeMethod(this, "loadPlugins", Qt::QueuedConnection);
}

QList<DeviceClass> DeviceManager::supportedDevices()
{
    return m_supportedDevices;
}

void DeviceManager::createDevice(const DeviceClass &deviceClass)
{
    Device *device = new Device(deviceClass.id(), this);
    device->setName(deviceClass.name());
}

QList<Device *> DeviceManager::configuredDevices() const
{
    return m_configuredDevices;
}

QList<Device *> DeviceManager::findConfiguredDevices(const DeviceClass &deviceClass)
{
    QList<Device*> ret;
    foreach (Device *device, m_configuredDevices) {
        if (device->deviceClassId() == deviceClass.id()) {
            ret << device;
        }
    }
    return ret;
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
        }
    }
}

