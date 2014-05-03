#ifndef TESTJSONRPC_H
#define TESTJSONRPC_H

#include "guhtestbase.h"

class TestJSONRPC: public GuhTestBase
{
    Q_OBJECT
public:
    TestJSONRPC(QObject *parent=0): GuhTestBase(parent) {}
    TestJSONRPC (const TestJSONRPC &other) {}
    TestJSONRPC& operator=(const TestJSONRPC &other) {}

private slots:
    void testBasicCall();
    void introspect();

    void executeAction_data();
    void executeAction();

    void getActionTypes_data();
    void getActionTypes();

    void getEventTypes_data();
    void getEventTypes();

    void getStateTypes_data();
    void getStateTypes();

    void enableDisableNotifications_data();
    void enableDisableNotifications();

    void stateChangeEmitsNotifications();

    void getRules();

private:
    QStringList extractRefs(const QVariant &variant);

};
Q_DECLARE_METATYPE(TestJSONRPC)

#endif
