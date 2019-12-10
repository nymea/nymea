#include "testhelper.h"

TestHelper* TestHelper::s_instance = nullptr;

TestHelper *TestHelper::instance()
{
    if (!s_instance) {
        s_instance = new TestHelper();
    }
    return s_instance;
}

void TestHelper::logEvent(const QString &deviceId, const QString &eventId, const QVariantMap &params)
{
    emit eventLogged(DeviceId(deviceId), eventId, params);
}

void TestHelper::logStateChange(const QString &deviceId, const QString &stateId, const QVariant &value)
{
    emit stateChangeLogged(DeviceId(deviceId), stateId, value);
}

TestHelper::TestHelper(QObject *parent) : QObject(parent)
{

}
