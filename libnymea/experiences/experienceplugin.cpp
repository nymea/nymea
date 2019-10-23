#include "experienceplugin.h"

ExperiencePlugin::ExperiencePlugin(QObject *parent) : QObject(parent)
{

}

/*! This method will be called when the plugin has been completely loaded and experience
    logic may start operating. A plugin can reimplment this to do initialisation code. */
void ExperiencePlugin::init()
{

}

/*! Returns a pointer to the DeviceManager. The pointer won't be valid unless init() has been called. */
DeviceManager *ExperiencePlugin::deviceManager()
{
    return m_deviceManager;
}

/*! Returns a pointer to the JsonRPCServer. The pointer won't be valid unless init() has been called. */
JsonRPCServer *ExperiencePlugin::jsonRpcServer()
{
    return m_jsonRpcServer;
}


void ExperiencePlugin::initPlugin(DeviceManager *deviceManager, JsonRPCServer *jsonRPCServer)
{
    m_deviceManager = deviceManager;
    m_jsonRpcServer = jsonRPCServer;

    init();
}
