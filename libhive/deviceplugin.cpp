#include "deviceplugin.h"

#include "devicemanager.h"

DevicePlugin::~DevicePlugin()
{

}

void DevicePlugin::init(DeviceManager *deviceManager)
{
    m_deviceManager = deviceManager;
}

DeviceManager *DevicePlugin::deviceManager() const
{
    return m_deviceManager;
}

DevicePlugin::DevicePlugin()
{

}
