#include "deviceplugin.h"

#include "devicemanager.h"
#include "radio433.h"

#include <QDebug>

DevicePlugin::DevicePlugin():
    m_deviceManager(0)
{

}

DevicePlugin::~DevicePlugin()
{

}

void DevicePlugin::initPlugin(DeviceManager *deviceManager)
{
    m_deviceManager = deviceManager;
    init();
}

QVariantMap DevicePlugin::configuration() const
{
    return QVariantMap();
}

void DevicePlugin::setConfiguration(const QVariantMap &configuration)
{
    Q_UNUSED(configuration)
    qWarning() << "Plugin" << pluginName() << pluginId() << "does not support any configuration";
}

DeviceManager *DevicePlugin::deviceManager() const
{
    return m_deviceManager;
}

void DevicePlugin::transmitData(QList<int> rawData)
{
    switch (requiredHardware()) {
    case DeviceManager::HardwareResourceRadio433:
        deviceManager()->m_radio433->sendData(rawData);
        break;
    case DeviceManager::HardwareResourceRadio868:
        qDebug() << "Radio868 not connected yet";
        break;
    default:
        qWarning() << "Unknown harware type. Cannot send.";
    }
}

