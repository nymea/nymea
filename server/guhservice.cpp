#include "guhservice.h"

GuhService::GuhService(int argc, char **argv):
    QtService<QCoreApplication>(argc, argv, "guh daemon")
{
    setServiceDescription("guh daemon");
    setServiceFlags(QtServiceBase::CanBeSuspended);
}

GuhService::~GuhService()
{
}

void GuhService::start()
{
    GuhCore::instance();
}
