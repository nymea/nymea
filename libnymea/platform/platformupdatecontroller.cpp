#include "platformupdatecontroller.h"

PlatformUpdateController::PlatformUpdateController(QObject *parent) : QObject(parent)
{

}

/*! Override this to indicate whether update management is available. Defaults to false.
    When a plugin returns true here, it is assumed that the system is capable of updating and nymea
    has permissions to do so.
    */
bool PlatformUpdateController::updateManagementAvailable()
{
    return false;
}

QString PlatformUpdateController::currentVersion() const
{
    return tr("N/A");
}

QString PlatformUpdateController::candidateVersion() const
{
    return tr("N/A");
}

void PlatformUpdateController::checkForUpdates()
{
    // Nothing to do here
}

bool PlatformUpdateController::updateAvailable() const
{
    return false;
}

bool PlatformUpdateController::startUpdate()
{
    return false;
}

bool PlatformUpdateController::rollbackAvailable() const
{
    return false;
}

bool PlatformUpdateController::startRollback()
{
    return false;
}

bool PlatformUpdateController::updateInProgress() const
{
    return false;
}

QStringList PlatformUpdateController::availableChannels() const
{
    return {};
}

QString PlatformUpdateController::currentChannel() const
{
    return tr("N/A");
}

bool PlatformUpdateController::selectChannel(const QString &channel)
{
    Q_UNUSED(channel)
    return false;
}
