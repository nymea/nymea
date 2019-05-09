#include "platformsystemcontroller.h"

PlatformSystemController::PlatformSystemController(QObject *parent) : QObject(parent)
{

}

bool PlatformSystemController::powerManagementAvailable() const
{
    return false;
}

bool PlatformSystemController::reboot()
{
    return false;
}

bool PlatformSystemController::shutdown()
{
    return false;
}
