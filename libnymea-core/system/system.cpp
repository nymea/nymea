#include "system.h"

#include "loggingcategories.h"
#include "platform/platform.h"
#include "plugin/platformsystemcontroller.h"
#include "plugin/platformupdatecontroller.h"

namespace nymeaserver {


System::System(Platform *platform, QObject *parent):
    QObject(parent),
    m_platform(platform)
{
    connect(m_platform->updateController(), &PlatformUpdateController::updateStatusChanged, this, &System::updateStatusChanged);
}

bool System::powerManagementAvailable() const
{
    return m_platform->systemController()->capabilities().testFlag(PlatformSystemController::CapabilityPower);
}

bool System::reboot()
{
    return m_platform->systemController()->reboot();
}

bool System::shutdown()
{
    return m_platform->systemController()->shutdown();
}

bool System::updateManagementAvailable() const
{
    return m_platform->updateController()->updateManagementAvailable();
}

bool System::updateAvailable() const
{
    return m_platform->updateController()->updateAvailable();
}

QString System::currentVersion() const
{
    return m_platform->updateController()->currentVersion();
}

QString System::candidateVersion() const
{
    return m_platform->updateController()->candidateVersion();
}

QStringList System::availableChannels() const
{
    return m_platform->updateController()->channels();
}

QString System::currentChannel() const
{
    return m_platform->updateController()->currentChannel();
}

bool System::selectChannel(const QString &channel) const
{
    return m_platform->updateController()->selectChannel(channel);
}

bool System::canUpdate() const
{
    return m_platform->updateController() ;
}

bool System::startUpdate()
{
    return m_platform->updateController()->startUpdate();
}

bool System::updateInProgress() const
{
    return m_platform->updateController()->updateInProgress();
}

bool System::rollbackAvailable() const
{
    return m_platform->updateController()->rollbackAvailable();
}

}
