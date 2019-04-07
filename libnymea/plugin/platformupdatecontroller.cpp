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
