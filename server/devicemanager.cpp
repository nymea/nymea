#include "devicemanager.h"

#include "radio433.h"

#include "device.h"
#include "deviceclass.h"
#include "deviceplugin.h"

#include "deviceplugins/rfswitch/rfswitch.h"

DeviceManager::DeviceManager(QObject *parent) :
    QObject(parent)
{

    m_radio433 = new Radio433(this);


    // TODO: load dynamically
    RfSwitch *rfSwitch = new RfSwitch(this);
    m_supportedDevices.append(rfSwitch->supportedDevices());
    m_devicePlugins.append(rfSwitch);


}

QList<DeviceClass> DeviceManager::supportedDevices()
{
    return m_supportedDevices;
}

QList<Device *> DeviceManager::devices() const
{
    return m_devices;
}

