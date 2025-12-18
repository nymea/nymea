// SPDX-License-Identifier: GPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2025, chargebyte austria GmbH
*
* This file is part of nymea.
*
* nymea is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* nymea is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with nymea. If not, see <https://www.gnu.org/licenses/>.
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "nymeatestbase.h"
#include "testhelper.h"

#include "nymeacore.h"
#include "scriptengine/scriptengine.h"

#include "../plugins/mock/extern-plugininfo.h"

#include <QtQml/qqml.h>

using namespace nymeaserver;

static QObject *helperProvider(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)
    return TestHelper::instance();
}

class TestScripts : public NymeaTestBase
{
    Q_OBJECT
public:
    TestScripts();

private:
private slots:
    void init();

    void testScriptEventById();
    void testScriptEventByName();

    void testReadScriptStateById();
    void testReadScriptStateByNyme();

    void testWriteScriptStateById();
    void testWriteScriptStateByName();

    void testScriptActionById();
    void testScriptActionByName();

    void testScriptAlarm_data();
    void testScriptAlarm();

    void testInterfaceEvent();
    void testInterfaceState();
    void testInterfaceAction();

    void testScriptThingAction();
    void testScriptThingReadState();
    void testScriptThingWriteState();
    void testScriptThingEvent();

    void testThingsFindThing();
};

TestScripts::TestScripts()
{
    qmlRegisterSingletonType<TestHelper>("nymea", 1, 0, "TestHelper", &helperProvider);
}

void TestScripts::init()
{
    // Make sure no scripts are in the engine when we start a test
    foreach (const Script &script, NymeaCore::instance()->scriptEngine()->scripts()) {
        NymeaCore::instance()->scriptEngine()->removeScript(script.id());
    }

    // Set initial state values of mock device
    Action action(mockPowerActionTypeId, m_mockThingId);
    action.setParams(ParamList() << Param(mockPowerActionPowerParamTypeId, false));
    ThingActionInfo *info = NymeaCore::instance()->thingManager()->executeAction(action);
    QSignalSpy spy(info, &ThingActionInfo::finished);
    spy.wait();
}

void TestScripts::testScriptEventById()
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QString script = QString("import QtQuick 2.0\n"
                             "import nymea 1.0\n"
                             "Item {\n"
                             "    ThingEvent {\n"
                             "        thingId: \"%1\"\n"
                             "        eventTypeId: \"%2\"\n"
                             "        onTriggered: (params) => {\n"
                             "            TestHelper.logEvent(thingId, eventTypeId, params);\n"
                             "        }\n"
                             "    }\n"
                             "}\n")
                         .arg(m_mockThingId.toString(), mockEvent2EventTypeId.toString());
#else
    QString script = QString("import QtQuick 2.0\n"
                             "import nymea 1.0\n"
                             "Item {\n"
                             "    ThingEvent {\n"
                             "        thingId: \"%1\"\n"
                             "        eventTypeId: \"%2\"\n"
                             "        onTriggered: {\n"
                             "            TestHelper.logEvent(thingId, eventTypeId, params);\n"
                             "        }\n"
                             "    }\n"
                             "}\n")
                         .arg(m_mockThingId.toString())
                         .arg(mockEvent2EventTypeId.toString());
#endif

    qCDebug(dcTests()) << "Adding script:\n" << qUtf8Printable(script);
    ScriptEngine::AddScriptReply reply = NymeaCore::instance()->scriptEngine()->addScript("TestEvent", script.toUtf8());
    QCOMPARE(reply.scriptError, ScriptEngine::ScriptErrorNoError);

    QSignalSpy spy(TestHelper::instance(), &TestHelper::eventLogged);

    // trigger event in mock device
    Thing *thing = NymeaCore::instance()->thingManager()->findConfiguredThing(m_mockThingId);
    int port = thing->paramValue(mockThingHttpportParamTypeId).toInt();
    QNetworkRequest request(QUrl(
        QString("http://localhost:%1/generateevent?eventtypeid=%2&%3=%4").arg(port).arg(mockEvent2EventTypeId.toString()).arg(mockEvent2EventIntParamParamTypeId.toString()).arg(23)));
    QNetworkAccessManager nam;
    QNetworkReply *r = nam.get(request);
    connect(r, &QNetworkReply::finished, r, &QNetworkReply::deleteLater);

    spy.wait();

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().at(0).value<ThingId>(), m_mockThingId);
    QCOMPARE(EventTypeId(spy.first().at(1).toUuid()), mockEvent2EventTypeId);
    QVariantMap expectedParams;
    expectedParams.insert(mockEvent2EventIntParamParamTypeId.toString().remove(QRegularExpression("[{}]")), 23);
    expectedParams.insert("intParam", 23);
    QVERIFY2(spy.first().at(2).toMap() == expectedParams,
             QString("Params not matching.\nExpected: %1\nGot: %2")
                 .arg(QString(QJsonDocument::fromVariant(expectedParams).toJson(QJsonDocument::Indented)),
                      QString(QJsonDocument::fromVariant(spy.first().at(2).toMap()).toJson(QJsonDocument::Indented)))
                 .toUtf8());
}

void TestScripts::testScriptEventByName()
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QString script = QString("import QtQuick 2.0\n"
                             "import nymea 1.0\n"
                             "Item {\n"
                             "    ThingEvent {\n"
                             "        thingId: \"%1\"\n"
                             "        eventName: \"%2\"\n"
                             "        onTriggered: (params) => {\n"
                             "            TestHelper.logEvent(thingId, eventName, params);\n"
                             "        }\n"
                             "    }\n"
                             "}\n")
                         .arg(m_mockThingId.toString(), "event2");
#else
    QString script = QString("import QtQuick 2.0\n"
                             "import nymea 1.0\n"
                             "Item {\n"
                             "    ThingEvent {\n"
                             "        thingId: \"%1\"\n"
                             "        eventName: \"%2\"\n"
                             "        onTriggered: {\n"
                             "            TestHelper.logEvent(thingId, eventName, params);\n"
                             "        }\n"
                             "    }\n"
                             "}\n")
                         .arg(m_mockThingId.toString())
                         .arg("event2");
#endif

    qCDebug(dcTests()) << "Adding script:\n" << qUtf8Printable(script);
    ScriptEngine::AddScriptReply reply = NymeaCore::instance()->scriptEngine()->addScript("TestEvent", script.toUtf8());
    QCOMPARE(reply.scriptError, ScriptEngine::ScriptErrorNoError);

    QSignalSpy spy(TestHelper::instance(), &TestHelper::eventLogged);

    // trigger event in mock device
    Thing *thing = NymeaCore::instance()->thingManager()->findConfiguredThing(m_mockThingId);
    int port = thing->paramValue(mockThingHttpportParamTypeId).toInt();
    QNetworkRequest request(QUrl(
        QString("http://localhost:%1/generateevent?eventtypeid=%2&%3=%4").arg(port).arg(mockEvent2EventTypeId.toString()).arg(mockEvent2EventIntParamParamTypeId.toString()).arg(10)));
    QNetworkAccessManager nam;
    QNetworkReply *r = nam.get(request);
    connect(r, &QNetworkReply::finished, r, &QNetworkReply::deleteLater);

    spy.wait();

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().at(0).value<ThingId>(), m_mockThingId);
    QCOMPARE(spy.first().at(1).toString(), QString("event2"));
    QVariantMap expectedParams;
    expectedParams.insert(mockEvent2EventIntParamParamTypeId.toString().remove(QRegularExpression("[{}]")), 10);
    expectedParams.insert("intParam", 10);
    QCOMPARE(spy.first().at(2).toMap(), expectedParams);
}

void TestScripts::testReadScriptStateById()
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QString script = QString("import QtQuick 2.0\n"
                             "import nymea 1.0\n"
                             "Item {\n"
                             "    ThingState {\n"
                             "        thingId: \"%1\"\n"
                             "        stateTypeId: \"%2\"\n"
                             "        onValueChanged: () => {\n"
                             "            TestHelper.logStateChange(thingId, stateTypeId, value);\n"
                             "        }\n"
                             "    }\n"
                             "}\n")
                         .arg(m_mockThingId.toString(), mockPowerStateTypeId.toString());
#else
    QString script = QString("import QtQuick 2.0\n"
                             "import nymea 1.0\n"
                             "Item {\n"
                             "    ThingState {\n"
                             "        thingId: \"%1\"\n"
                             "        stateTypeId: \"%2\"\n"
                             "        onValueChanged: {\n"
                             "            TestHelper.logStateChange(thingId, stateTypeId, value);\n"
                             "        }\n"
                             "    }\n"
                             "}\n")
                         .arg(m_mockThingId.toString())
                         .arg(mockPowerStateTypeId.toString());
#endif
    qCDebug(dcTests()) << "Adding script:\n" << qUtf8Printable(script);
    ScriptEngine::AddScriptReply reply = NymeaCore::instance()->scriptEngine()->addScript("TestState", script.toUtf8());
    QCOMPARE(reply.scriptError, ScriptEngine::ScriptErrorNoError);

    QSignalSpy spy(TestHelper::instance(), &TestHelper::stateChangeLogged);

    // Generate state change
    Action action(mockPowerActionTypeId, m_mockThingId);
    action.setParams(ParamList() << Param(mockPowerActionPowerParamTypeId, true));
    NymeaCore::instance()->thingManager()->executeAction(action);

    spy.wait(1);

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().at(0).value<ThingId>(), m_mockThingId);
    QCOMPARE(StateTypeId(spy.first().at(1).toString()), mockPowerStateTypeId);
    QCOMPARE(spy.first().at(2).toBool(), true);
}

void TestScripts::testReadScriptStateByNyme()
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QString script = QString("import QtQuick 2.0\n"
                             "import nymea 1.0\n"
                             "Item {\n"
                             "    ThingState {\n"
                             "        thingId: \"%1\"\n"
                             "        stateName: \"%2\"\n"
                             "        onValueChanged: () => {\n"
                             "            TestHelper.logStateChange(thingId, stateName, value);\n"
                             "        }\n"
                             "    }\n"
                             "}\n")
                         .arg(m_mockThingId.toString(), "power");
#else
    QString script = QString("import QtQuick 2.0\n"
                             "import nymea 1.0\n"
                             "Item {\n"
                             "    ThingState {\n"
                             "        thingId: \"%1\"\n"
                             "        stateName: \"%2\"\n"
                             "        onValueChanged: {\n"
                             "            TestHelper.logStateChange(thingId, stateName, value);\n"
                             "        }\n"
                             "    }\n"
                             "}\n")
                         .arg(m_mockThingId.toString())
                         .arg("power");
#endif
    qCDebug(dcTests()) << "Adding script:\n" << qUtf8Printable(script);
    ScriptEngine::AddScriptReply reply = NymeaCore::instance()->scriptEngine()->addScript("TestState", script.toUtf8());
    QCOMPARE(reply.scriptError, ScriptEngine::ScriptErrorNoError);

    QSignalSpy spy(TestHelper::instance(), &TestHelper::stateChangeLogged);

    // Generate state change
    Action action(mockPowerActionTypeId, m_mockThingId);
    action.setParams(ParamList() << Param(mockPowerActionPowerParamTypeId, true));
    NymeaCore::instance()->thingManager()->executeAction(action);

    spy.wait(1);

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().at(0).value<ThingId>(), m_mockThingId);
    QCOMPARE(spy.first().at(1).toString(), QString("power"));
    QCOMPARE(spy.first().at(2).toBool(), true);
}

void TestScripts::testWriteScriptStateById()
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QString script = QString("import QtQuick 2.0\n"
                             "import nymea 1.0\n"
                             "Item {\n"
                             "    ThingState {\n"
                             "        id: thingState\n"
                             "        thingId: \"%1\"\n"
                             "        stateTypeId: \"%2\"\n"
                             "    }\n"
                             "    Connections {\n"
                             "        target: TestHelper\n"
                             "        function onSetState(value) {\n"
                             "            thingState.value = value\n"
                             "        }\n"
                             "    }\n"
                             "}\n")
                         .arg(m_mockThingId.toString(), mockPowerStateTypeId.toString());
#else
    QString script = QString("import QtQuick 2.0\n"
                             "import nymea 1.0\n"
                             "Item {\n"
                             "    ThingState {\n"
                             "        id: thingState\n"
                             "        thingId: \"%1\"\n"
                             "        stateTypeId: \"%2\"\n"
                             "    }\n"
                             "    Connections {\n"
                             "        target: TestHelper\n"
                             "        onSetState: {\n"
                             "            thingState.value = value\n"
                             "        }\n"
                             "    }\n"
                             "}\n")
                         .arg(m_mockThingId.toString())
                         .arg(mockPowerStateTypeId.toString());
#endif
    qCDebug(dcTests()) << "Adding script:\n" << qUtf8Printable(script);
    ScriptEngine::AddScriptReply reply = NymeaCore::instance()->scriptEngine()->addScript("TestState", script.toUtf8());
    QCOMPARE(reply.scriptError, ScriptEngine::ScriptErrorNoError);

    QSignalSpy spy(NymeaCore::instance()->thingManager(), &ThingManager::thingStateChanged);

    TestHelper::instance()->setState(true);

    spy.wait(1);

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().at(0).value<Thing *>()->id(), m_mockThingId);
    QCOMPARE(spy.first().at(1).value<StateTypeId>(), mockPowerStateTypeId);
    QCOMPARE(spy.first().at(2).toBool(), true);
}

void TestScripts::testWriteScriptStateByName()
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QString script = QString("import QtQuick 2.0\n"
                             "import nymea 1.0\n"
                             "Item {\n"
                             "    ThingState {\n"
                             "        id: thingState\n"
                             "        thingId: \"%1\"\n"
                             "        stateName: \"%2\"\n"
                             "    }\n"
                             "    Connections {\n"
                             "        target: TestHelper\n"
                             "        function onSetState(value) {\n"
                             "            thingState.value = value\n"
                             "        }\n"
                             "    }\n"
                             "}\n")
                         .arg(m_mockThingId.toString(), "power");
#else
    QString script = QString("import QtQuick 2.0\n"
                             "import nymea 1.0\n"
                             "Item {\n"
                             "    ThingState {\n"
                             "        id: thingState\n"
                             "        thingId: \"%1\"\n"
                             "        stateName: \"%2\"\n"
                             "    }\n"
                             "    Connections {\n"
                             "        target: TestHelper\n"
                             "        onSetState: {\n"
                             "            thingState.value = value\n"
                             "        }\n"
                             "    }\n"
                             "}\n")
                         .arg(m_mockThingId.toString())
                         .arg("power");
#endif

    qCDebug(dcTests()) << "Adding script:\n" << qUtf8Printable(script);
    ScriptEngine::AddScriptReply reply = NymeaCore::instance()->scriptEngine()->addScript("TestState", script.toUtf8());
    QCOMPARE(reply.scriptError, ScriptEngine::ScriptErrorNoError);

    QSignalSpy spy(NymeaCore::instance()->thingManager(), &ThingManager::thingStateChanged);

    TestHelper::instance()->setState(true);

    spy.wait(1);

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().at(0).value<Thing *>()->id(), m_mockThingId);
    QCOMPARE(spy.first().at(1).value<StateTypeId>(), mockPowerStateTypeId);
    QCOMPARE(spy.first().at(2).toBool(), true);
}

void TestScripts::testScriptActionById()
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QString script = QString("import QtQuick 2.0\n"
                             "import nymea 1.0\n"
                             "Item {\n"
                             "    ThingAction {\n"
                             "        id: thingAction\n"
                             "        thingId: \"%1\"\n"
                             "        actionTypeId: \"%2\"\n"
                             "        onExecuted: (params, status, triggeredBy) => {\n"
                             "            TestHelper.logActionExecuted(\"%1\", \"%2\", params, status, triggeredBy)\n"
                             "        }\n"
                             "    }\n"
                             "    Connections {\n"
                             "        target: TestHelper\n"
                             "        function onExecuteAction(params) {\n"
                             "            thingAction.execute(params)\n"
                             "        }\n"
                             "    }\n"
                             "}\n")
                         .arg(m_mockThingId.toString(), mockPowerActionTypeId.toString());
#else
    QString script = QString("import QtQuick 2.0\n"
                             "import nymea 1.0\n"
                             "Item {\n"
                             "    ThingAction {\n"
                             "        id: thingAction\n"
                             "        thingId: \"%1\"\n"
                             "        actionTypeId: \"%2\"\n"
                             "        onExecuted: {\n"
                             "            TestHelper.logActionExecuted(\"%1\", \"%2\", params, status, triggeredBy)\n"
                             "        }\n"
                             "    }\n"
                             "    Connections {\n"
                             "        target: TestHelper\n"
                             "        onExecuteAction: {\n"
                             "            thingAction.execute(params)\n"
                             "        }\n"
                             "    }\n"
                             "}\n")
                         .arg(m_mockThingId.toString())
                         .arg(mockPowerActionTypeId.toString());
#endif

    qCDebug(dcTests()) << "Adding script:\n" << qUtf8Printable(script);
    ScriptEngine::AddScriptReply reply = NymeaCore::instance()->scriptEngine()->addScript("TestAction", script.toUtf8());
    QCOMPARE(reply.scriptError, ScriptEngine::ScriptErrorNoError);

    QSignalSpy spy(NymeaCore::instance()->thingManager(), &ThingManager::thingStateChanged);
    QSignalSpy actionExecutedSpy(TestHelper::instance(), &TestHelper::actionExecutionLogged);

    QVariantMap params;
    params.insert(mockPowerActionPowerParamTypeId.toString(), true);
    TestHelper::instance()->executeAction(params);

    spy.wait(1);

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().at(0).value<Thing *>()->id(), m_mockThingId);
    QCOMPARE(spy.first().at(1).value<StateTypeId>(), mockPowerStateTypeId);
    QCOMPARE(spy.first().at(2).toBool(), true);

    QCOMPARE(actionExecutedSpy.count(), 1);
    QCOMPARE(actionExecutedSpy.first().at(0).value<ThingId>(), m_mockThingId);
    QCOMPARE(ActionTypeId(actionExecutedSpy.first().at(1).toString()), mockPowerActionTypeId);
    QCOMPARE(actionExecutedSpy.first().at(2).toMap().value(mockPowerActionTypeId.toString().remove(QRegularExpression("[{}]"))).toBool(), true);
    QCOMPARE(actionExecutedSpy.first().at(3).value<Thing::ThingError>(), Thing::ThingErrorNoError);
    QCOMPARE(actionExecutedSpy.first().at(4).value<Action::TriggeredBy>(), Action::TriggeredByScript);
}

void TestScripts::testScriptActionByName()
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QString script = QString("import QtQuick 2.0\n"
                             "import nymea 1.0\n"
                             "Item {\n"
                             "    ThingAction {\n"
                             "        id: thingAction\n"
                             "        thingId: \"%1\"\n"
                             "        actionName: \"%2\"\n"
                             "        onExecuted: (params, status, triggeredBy) => {\n"
                             "            TestHelper.logActionExecuted(\"%1\", \"%2\", params, status, triggeredBy)\n"
                             "        }\n"
                             "    }\n"
                             "    Connections {\n"
                             "        target: TestHelper\n"
                             "        function onExecuteAction(params) {\n"
                             "            thingAction.execute(params)\n"
                             "        }\n"
                             "    }\n"
                             "}\n")
                         .arg(m_mockThingId.toString(), "power");
#else
    QString script = QString("import QtQuick 2.0\n"
                             "import nymea 1.0\n"
                             "Item {\n"
                             "    ThingAction {\n"
                             "        id: thingAction\n"
                             "        thingId: \"%1\"\n"
                             "        actionName: \"%2\"\n"
                             "        onExecuted: {\n"
                             "            TestHelper.logActionExecuted(\"%1\", \"%2\", params, status, triggeredBy)\n"
                             "        }\n"
                             "    }\n"
                             "    Connections {\n"
                             "        target: TestHelper\n"
                             "        onExecuteAction: {\n"
                             "            thingAction.execute(params)\n"
                             "        }\n"
                             "    }\n"
                             "}\n")
                         .arg(m_mockThingId.toString())
                         .arg("power");
#endif

    qCDebug(dcTests()) << "Adding script:\n" << qUtf8Printable(script);
    ScriptEngine::AddScriptReply reply = NymeaCore::instance()->scriptEngine()->addScript("TestAction", script.toUtf8());
    QCOMPARE(reply.scriptError, ScriptEngine::ScriptErrorNoError);

    QSignalSpy spy(NymeaCore::instance()->thingManager(), &ThingManager::thingStateChanged);
    QSignalSpy actionExecutedSpy(TestHelper::instance(), &TestHelper::actionExecutionLogged);

    QVariantMap params;
    params.insert("power", true);
    TestHelper::instance()->executeAction(params);

    spy.wait(1);

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().at(0).value<Thing *>()->id(), m_mockThingId);
    QCOMPARE(spy.first().at(1).value<StateTypeId>(), mockPowerStateTypeId);
    QCOMPARE(spy.first().at(2).toBool(), true);

    QCOMPARE(actionExecutedSpy.count(), 1);
    QCOMPARE(actionExecutedSpy.first().at(0).value<ThingId>(), m_mockThingId);
    QCOMPARE(actionExecutedSpy.first().at(1).toString(), "power");
    QCOMPARE(actionExecutedSpy.first().at(2).toMap().value("power").toBool(), true);
    QCOMPARE(actionExecutedSpy.first().at(3).value<Thing::ThingError>(), Thing::ThingErrorNoError);
    QCOMPARE(actionExecutedSpy.first().at(4).value<Action::TriggeredBy>(), Action::TriggeredByScript);
}

void TestScripts::testScriptAlarm_data()
{
    QTest::addColumn<QTime>("time");
    QTest::addColumn<QTime>("endTime");
    QTest::addColumn<bool>("active");

    QTest::newRow("active, regular") << QTime(12, 05);
}

void TestScripts::testScriptAlarm() {}

void TestScripts::testInterfaceEvent()
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QString script = QString("import QtQuick 2.0\n"
                             "import nymea 1.0\n"
                             "Item {\n"
                             "    InterfaceEvent {\n"
                             "        interfaceName: \"%1\"\n"
                             "        eventName: \"%2\"\n"
                             "        onTriggered: (thingId, params) => {\n"
                             "            TestHelper.logEvent(thingId, eventName, params);\n"
                             "        }\n"
                             "    }\n"
                             "}\n")
                         .arg("button", "pressed");
#else
    QString script = QString("import QtQuick 2.0\n"
                             "import nymea 1.0\n"
                             "Item {\n"
                             "    InterfaceEvent {\n"
                             "        interfaceName: \"%1\"\n"
                             "        eventName: \"%2\"\n"
                             "        onTriggered: {\n"
                             "            TestHelper.logEvent(thingId, eventName, params);\n"
                             "        }\n"
                             "    }\n"
                             "}\n")
                         .arg("button")
                         .arg("pressed");
#endif

    qCDebug(dcTests()) << "Adding script:\n" << qUtf8Printable(script);
    ScriptEngine::AddScriptReply reply = NymeaCore::instance()->scriptEngine()->addScript("TestEvent", script.toUtf8());
    QCOMPARE(reply.scriptError, ScriptEngine::ScriptErrorNoError);

    QSignalSpy spy(TestHelper::instance(), &TestHelper::eventLogged);

    // trigger event in mock device
    Thing *thing = NymeaCore::instance()->thingManager()->findConfiguredThing(m_mockThingId);
    int port = thing->paramValue(mockThingHttpportParamTypeId).toInt();
    QNetworkRequest request(QUrl(QString("http://localhost:%1/generateevent?eventtypeid=%2&%3=%4")
                                     .arg(port)
                                     .arg(mockPressedEventTypeId.toString())
                                     .arg(mockPressedEventButtonNameParamTypeId.toString())
                                     .arg("xxx")));
    QNetworkAccessManager nam;
    QNetworkReply *r = nam.get(request);
    connect(r, &QNetworkReply::finished, r, &QNetworkReply::deleteLater);

    spy.wait();

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().at(0).value<ThingId>(), m_mockThingId);
    QCOMPARE(spy.first().at(1).toString(), QString("pressed"));
    QVariantMap expectedParams;
    expectedParams.insert(mockPressedEventButtonNameParamTypeId.toString().remove(QRegularExpression("[{}]")), "xxx");
    expectedParams.insert("buttonName", "xxx");
    QCOMPARE(spy.first().at(2).toMap(), expectedParams);
}

void TestScripts::testInterfaceState()
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QString script = QString("import QtQuick 2.0\n"
                             "import nymea 1.0\n"
                             "Item {\n"
                             "    InterfaceState {\n"
                             "        interfaceName: \"%1\"\n"
                             "        stateName: \"%2\"\n"
                             "        onStateChanged: (thingId, value) => {\n"
                             "            TestHelper.logStateChange(thingId, stateName, value);\n"
                             "        }\n"
                             "    }\n"
                             "}\n")
                         .arg("power", "power");
#else
    QString script = QString("import QtQuick 2.0\n"
                             "import nymea 1.0\n"
                             "Item {\n"
                             "    InterfaceState {\n"
                             "        interfaceName: \"%1\"\n"
                             "        stateName: \"%2\"\n"
                             "        onStateChanged: {\n"
                             "            TestHelper.logStateChange(thingId, stateName, value);\n"
                             "        }\n"
                             "    }\n"
                             "}\n")
                         .arg("power")
                         .arg("power");
#endif

    qCDebug(dcTests()) << "Adding script:\n" << qUtf8Printable(script);
    ScriptEngine::AddScriptReply reply = NymeaCore::instance()->scriptEngine()->addScript("TestInterfaceState", script.toUtf8());
    QCOMPARE(reply.scriptError, ScriptEngine::ScriptErrorNoError);

    QSignalSpy spy(TestHelper::instance(), &TestHelper::stateChangeLogged);

    // Generate event by setting state value of powerState
    Action action(mockPowerActionTypeId, m_mockThingId);
    action.setParams(ParamList() << Param(mockPowerActionPowerParamTypeId, true));
    NymeaCore::instance()->thingManager()->executeAction(action);

    spy.wait(1);

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().at(0).value<ThingId>(), m_mockThingId);
    QCOMPARE(spy.first().at(1).toString(), QString("power"));
    QCOMPARE(spy.first().at(2).toBool(), true);
}

void TestScripts::testInterfaceAction()
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QString script = QString("import QtQuick 2.0\n"
                             "import nymea 1.0\n"
                             "Item {\n"
                             "    InterfaceAction {\n"
                             "        id: interfaceAction\n"
                             "        interfaceName: \"%1\"\n"
                             "        actionName: \"%2\"\n"
                             "    }\n"
                             "    Connections {\n"
                             "        target: TestHelper\n"
                             "        function onExecuteAction(params) {\n"
                             "            interfaceAction.execute(params)\n"
                             "        }\n"
                             "    }\n"
                             "}\n")
                         .arg("power", "power");
#else
    QString script = QString("import QtQuick 2.0\n"
                             "import nymea 1.0\n"
                             "Item {\n"
                             "    InterfaceAction {\n"
                             "        id: interfaceAction\n"
                             "        interfaceName: \"%1\"\n"
                             "        actionName: \"%2\"\n"
                             "    }\n"
                             "    Connections {\n"
                             "        target: TestHelper\n"
                             "        onExecuteAction: {\n"
                             "            interfaceAction.execute(params)\n"
                             "        }\n"
                             "    }\n"
                             "}\n")
                         .arg("power")
                         .arg("power");
#endif

    qCDebug(dcTests()) << "Adding script:\n" << qUtf8Printable(script);
    ScriptEngine::AddScriptReply reply = NymeaCore::instance()->scriptEngine()->addScript("TestInterfaceAction", script.toUtf8());
    QCOMPARE(reply.scriptError, ScriptEngine::ScriptErrorNoError);

    QSignalSpy spy(NymeaCore::instance()->thingManager(), &ThingManager::thingStateChanged);

    QVariantMap params;
    params.insert("power", true);
    TestHelper::instance()->executeAction(params);

    spy.wait(1);

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().at(0).value<Thing *>()->id(), m_mockThingId);
    QCOMPARE(spy.first().at(1).value<StateTypeId>(), mockPowerStateTypeId);
    QCOMPARE(spy.first().at(2).toBool(), true);
}

void TestScripts::testScriptThingAction()
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QString script = QString("import QtQuick 2.0\n"
                             "import nymea 1.0\n"
                             "Item {\n"
                             "    Thing {\n"
                             "        id: thing\n"
                             "        thingId: \"%1\"\n"
                             "        onActionExecuted: (actionName, params, status, triggeredBy) => {\n"
                             "            TestHelper.logActionExecuted(\"%1\", actionName, params, status, triggeredBy)\n"
                             "        }\n"
                             "    }\n"
                             "    Connections {\n"
                             "        target: TestHelper\n"
                             "        function onExecuteAction(params) {\n"
                             "            thing.executeAction(\"%2\", params)\n"
                             "        }\n"
                             "    }\n"
                             "}\n")
                         .arg(m_mockThingId.toString(), "power");
#else
    QString script = QString("import QtQuick 2.0\n"
                             "import nymea 1.0\n"
                             "Item {\n"
                             "    Thing {\n"
                             "        id: thing\n"
                             "        thingId: \"%1\"\n"
                             "        onActionExecuted: {\n"
                             "            TestHelper.logActionExecuted(\"%1\", actionName, params, status, triggeredBy)\n"
                             "        }\n"
                             "    }\n"
                             "    Connections {\n"
                             "        target: TestHelper\n"
                             "        onExecuteAction: {\n"
                             "            thing.executeAction(\"%2\", params)\n"
                             "        }\n"
                             "    }\n"
                             "}\n")
                         .arg(m_mockThingId.toString())
                         .arg("power");
#endif
    qCDebug(dcTests()) << "Adding script:\n" << qUtf8Printable(script);
    ScriptEngine::AddScriptReply reply = NymeaCore::instance()->scriptEngine()->addScript("TestAction", script.toUtf8());
    QCOMPARE(reply.scriptError, ScriptEngine::ScriptErrorNoError);

    QSignalSpy spy(NymeaCore::instance()->thingManager(), &ThingManager::thingStateChanged);
    QSignalSpy actionExecutedSpy(TestHelper::instance(), &TestHelper::actionExecutionLogged);

    QVariantMap params;
    params.insert("power", true);
    TestHelper::instance()->executeAction(params);

    spy.wait(1);

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().at(0).value<Thing *>()->id(), m_mockThingId);
    QCOMPARE(spy.first().at(1).value<StateTypeId>(), mockPowerStateTypeId);
    QCOMPARE(spy.first().at(2).toBool(), true);

    QCOMPARE(actionExecutedSpy.count(), 1);
    QCOMPARE(actionExecutedSpy.first().at(0).value<ThingId>(), m_mockThingId);
    QCOMPARE(actionExecutedSpy.first().at(1).toString(), "power");
    QCOMPARE(actionExecutedSpy.first().at(2).toMap().value("power").toBool(), true);
    QCOMPARE(actionExecutedSpy.first().at(3).value<Thing::ThingError>(), Thing::ThingErrorNoError);
    QCOMPARE(actionExecutedSpy.first().at(4).value<Action::TriggeredBy>(), Action::TriggeredByScript);
}

void TestScripts::testScriptThingReadState()
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QString script = QString("import QtQuick 2.0\n"
                             "import nymea 1.0\n"
                             "Item {\n"
                             "    Thing {\n"
                             "        thingId: \"%1\"\n"
                             "        onStateValueChanged: (stateName, value) => {\n"
                             "            TestHelper.logStateChange(thingId, stateName, value);\n"
                             "        }\n"
                             "    }\n"
                             "}\n")
                         .arg(m_mockThingId.toString(), "power");
#else
    QString script = QString("import QtQuick 2.0\n"
                             "import nymea 1.0\n"
                             "Item {\n"
                             "    Thing {\n"
                             "        thingId: \"%1\"\n"
                             "        onStateValueChanged: {\n"
                             "            TestHelper.logStateChange(thingId, stateName, value);\n"
                             "        }\n"
                             "    }\n"
                             "}\n")
                         .arg(m_mockThingId.toString())
                         .arg("power");
#endif

    qCDebug(dcTests()) << "Adding script:\n" << qUtf8Printable(script);
    ScriptEngine::AddScriptReply reply = NymeaCore::instance()->scriptEngine()->addScript("TestState", script.toUtf8());
    QCOMPARE(reply.scriptError, ScriptEngine::ScriptErrorNoError);

    QSignalSpy spy(TestHelper::instance(), &TestHelper::stateChangeLogged);

    // Generate state change
    Action action(mockPowerActionTypeId, m_mockThingId);
    action.setParams(ParamList() << Param(mockPowerActionPowerParamTypeId, true));
    NymeaCore::instance()->thingManager()->executeAction(action);

    spy.wait(1);

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().at(0).value<ThingId>(), m_mockThingId);
    QCOMPARE(spy.first().at(1).toString(), QString("power"));
    QCOMPARE(spy.first().at(2).toBool(), true);
}

void TestScripts::testScriptThingWriteState()
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QString script = QString("import QtQuick 2.0\n"
                             "import nymea 1.0\n"
                             "Item {\n"
                             "    Thing {\n"
                             "        id: thing\n"
                             "        thingId: \"%1\"\n"
                             "    }\n"
                             "    Connections {\n"
                             "        target: TestHelper\n"
                             "        function onSetState(value) {\n"
                             "            thing.setStateValue(\"%2\", value)\n"
                             "        }\n"
                             "    }\n"
                             "}\n")
                         .arg(m_mockThingId.toString(), "power");
#else
    QString script = QString("import QtQuick 2.0\n"
                             "import nymea 1.0\n"
                             "Item {\n"
                             "    Thing {\n"
                             "        id: thing\n"
                             "        thingId: \"%1\"\n"
                             "    }\n"
                             "    Connections {\n"
                             "        target: TestHelper\n"
                             "        onSetState: {\n"
                             "            thing.setStateValue(\"%2\", value)\n"
                             "        }\n"
                             "    }\n"
                             "}\n")
                         .arg(m_mockThingId.toString())
                         .arg("power");
#endif

    qCDebug(dcTests()) << "Adding script:\n" << qUtf8Printable(script);
    ScriptEngine::AddScriptReply reply = NymeaCore::instance()->scriptEngine()->addScript("TestState", script.toUtf8());
    QCOMPARE(reply.scriptError, ScriptEngine::ScriptErrorNoError);

    QSignalSpy spy(NymeaCore::instance()->thingManager(), &ThingManager::thingStateChanged);

    TestHelper::instance()->setState(true);

    spy.wait(1);

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().at(0).value<Thing *>()->id(), m_mockThingId);
    QCOMPARE(spy.first().at(1).value<StateTypeId>(), mockPowerStateTypeId);
    QCOMPARE(spy.first().at(2).toBool(), true);
}

void TestScripts::testScriptThingEvent()
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QString script = QString("import QtQuick 2.0\n"
                             "import nymea 1.0\n"
                             "Item {\n"
                             "    Thing {\n"
                             "        thingId: \"%1\"\n"
                             "        onEventTriggered: (eventName, params) => {\n"
                             "            TestHelper.logEvent(thingId, eventName, params);\n"
                             "        }\n"
                             "    }\n"
                             "}\n")
                         .arg(m_mockThingId.toString());
#else
    QString script = QString("import QtQuick 2.0\n"
                             "import nymea 1.0\n"
                             "Item {\n"
                             "    Thing {\n"
                             "        thingId: \"%1\"\n"
                             "        onEventTriggered: {\n"
                             "            TestHelper.logEvent(thingId, eventName, params);\n"
                             "        }\n"
                             "    }\n"
                             "}\n")
                         .arg(m_mockThingId.toString());
#endif

    qCDebug(dcTests()) << "Adding script:\n" << qUtf8Printable(script);
    ScriptEngine::AddScriptReply reply = NymeaCore::instance()->scriptEngine()->addScript("TestEvent", script.toUtf8());
    QCOMPARE(reply.scriptError, ScriptEngine::ScriptErrorNoError);

    QSignalSpy spy(TestHelper::instance(), &TestHelper::eventLogged);

    // trigger event in mock device
    Thing *thing = NymeaCore::instance()->thingManager()->findConfiguredThing(m_mockThingId);
    int port = thing->paramValue(mockThingHttpportParamTypeId).toInt();
    QNetworkRequest request(QUrl(
        QString("http://localhost:%1/generateevent?eventtypeid=%2&%3=%4").arg(port).arg(mockEvent2EventTypeId.toString()).arg(mockEvent2EventIntParamParamTypeId.toString()).arg(10)));
    QNetworkAccessManager nam;
    QNetworkReply *r = nam.get(request);
    connect(r, &QNetworkReply::finished, r, &QNetworkReply::deleteLater);

    spy.wait();

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().at(0).value<ThingId>(), m_mockThingId);
    QCOMPARE(spy.first().at(1).toString(), QString("event2"));
    QVariantMap expectedParams;
    expectedParams.insert(mockEvent2EventIntParamParamTypeId.toString().remove(QRegularExpression("[{}]")), 10);
    expectedParams.insert("intParam", 10);
    QCOMPARE(spy.first().at(2).toMap(), expectedParams);
}

void TestScripts::testThingsFindThing()
{
    QSignalSpy spy(TestHelper::instance(), &TestHelper::testResult);

    QString script = QString("import QtQuick 2.0\n"
                             "import nymea 1.0\n"
                             "Item {\n"
                             "    id: root\n"
                             "    property string thingId: \"%1\"\n"
                             "    Things {\n"
                             "        id: things\n"
                             "    }\n"
                             "    Component.onCompleted: {\n"
                             "        var thing = things.getThing(root.thingId)\n"
                             "        TestHelper.setTestResult(thing.thingId == root.thingId);\n"
                             "    }\n"
                             "}\n")
                         .arg(m_mockThingId.toString());

    qCDebug(dcTests()) << "Adding script:\n" << qUtf8Printable(script);
    ScriptEngine::AddScriptReply reply = NymeaCore::instance()->scriptEngine()->addScript("TestEvent", script.toUtf8());
    QCOMPARE(reply.scriptError, ScriptEngine::ScriptErrorNoError);

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().at(0).toBool(), true);
}

#include "testscripts.moc"
QTEST_MAIN(TestScripts)
