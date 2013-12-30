#include "deviceplugin.h"

#include "devicemanager.h"

DevicePlugin::~DevicePlugin()
{

}

void DevicePlugin::initPlugin(DeviceManager *deviceManager)
{
    m_deviceManager = deviceManager;
    init();
}

DeviceManager *DevicePlugin::deviceManager() const
{
    return m_deviceManager;
}

DevicePlugin::DevicePlugin()
{

}
