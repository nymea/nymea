#include "platformplugin.h"

PlatformPlugin::PlatformPlugin(QObject *parent) : QObject(parent)
{

}

PlatformSystemController *PlatformPlugin::systemController() const
{
    return m_systemStub;
}

PlatformUpdateController *PlatformPlugin::updateController() const
{
    return m_updateStub;
}
