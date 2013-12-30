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

<<<<<<< HEAD
    QMetaObject::invokeMethod(this, "loadPlugins", Qt::QueuedConnection);
=======
    qDebug() << "loading plugins";
    foreach (QObject *pluginObject, QPluginLoader::staticInstances()) {
        DevicePlugin *pluginIface = qobject_cast<DevicePlugin*>(pluginObject);
        qDebug() << "got plugin instance";
        if (pluginIface) {
            qDebug() << "got device plugin" << pluginIface->pluginName();
        }
    }

    // TODO: load dynamically
//    RfSwitch *rfSwitch = new RfSwitch(this);
//    m_supportedDevices.append(rfSwitch->supportedDevices());
//    m_devicePlugins.append(rfSwitch);


>>>>>>> radio thread fixed
}

QList<DeviceClass> DeviceManager::supportedDevices()
{
    return m_supportedDevices;
}

QList<Device *> DeviceManager::devices() const
{
    return m_devices;
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

