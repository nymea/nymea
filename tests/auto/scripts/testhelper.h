#ifndef TESTHELPER_H
#define TESTHELPER_H

#include <QObject>

#include "typeutils.h"

class TestHelper : public QObject
{
    Q_OBJECT
public:
    static TestHelper* instance();

    Q_INVOKABLE void logEvent(const QString &deviceId, const QString &eventId, const QVariantMap &params);
    Q_INVOKABLE void logStateChange(const QString &deviceId, const QString &stateId, const QVariant &value);

signals:
    void setState(const QVariant &value);
    void executeAction(const QVariantMap &params);

    void eventLogged(const DeviceId &deviceId, const QString &eventId, const QVariantMap &params);
    void stateChangeLogged(const DeviceId &deviceId, const QString stateId, const QVariant &value);
private:
    explicit TestHelper(QObject *parent = nullptr);
    static TestHelper* s_instance;
};

#endif // TESTHELPER_H
