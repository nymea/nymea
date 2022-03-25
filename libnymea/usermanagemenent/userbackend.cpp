#include "userbackend.h"

UserBackend::UserBackend(QObject *parent) : QObject(parent)
{

}

bool UserBackend::pushButtonAuthAvailable() const
{
    return false;
}

int UserBackend::requestPushButtonAuth(const QString &deviceName)
{
    Q_UNUSED(deviceName)
    return -1;
}

void UserBackend::cancelPushButtonAuth(int transactionId)
{
    Q_UNUSED(transactionId)
}

