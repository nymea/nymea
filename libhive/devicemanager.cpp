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

    qDebug() << "creating radio";
    m_radio433 = new Radio433(this);

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


}

QList<DeviceClass> DeviceManager::supportedDevices()
{
    return m_supportedDevices;
}

QList<Device *> DeviceManager::devices() const
{
    return m_devices;
}

Radio433 *DeviceManager::radio() const
{
    return m_radio433;
}

