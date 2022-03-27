/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2020, nymea GmbH
* Contact: contact@nymea.io
*
* This file is part of nymea.
* This project including source code and documentation is protected by
* copyright law, and remains the property of nymea GmbH. All rights, including
* reproduction, publication, editing and translation, are reserved. The use of
* this project is subject to the terms of a license agreement to be concluded
* with nymea GmbH in accordance with the terms of use of nymea GmbH, available
* under https://nymea.io/license
*
* GNU General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU General Public License as published by the Free Software
* Foundation, GNU version 3. This project is distributed in the hope that it
* will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
* of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
* Public License for more details.
*
* You should have received a copy of the GNU General Public License along with
* this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "nymeatestbase.h"
#include "testhelper.h"

#include "nymeasettings.h"
#include "nymeacore.h"
#include "scriptengine/scriptengine.h"

#include <QtQml/qqml.h>

using namespace nymeaserver;

static QObject* helperProvider(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)
    return TestHelper::instance();
}

class TestScripts: public NymeaTestBase
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
    NymeaCore::instance()->thingManager()->executeAction(action);
}

void TestScripts::testScriptEventById()
{
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
                            "}\n").arg(m_mockThingId.toString()).arg(mockEvent2EventTypeId.toString());

    qCDebug(dcTests()) << "Adding script:\n" << qUtf8Printable(script);
    ScriptEngine::AddScriptReply reply = NymeaCore::instance()->scriptEngine()->addScript("TestEvent", script.toUtf8());
    QCOMPARE(reply.scriptError, ScriptEngine::ScriptErrorNoError);

    QSignalSpy spy(TestHelper::instance(), &TestHelper::eventLogged);

    // trigger event in mock device
    Thing* thing = NymeaCore::instance()->thingManager()->findConfiguredThing(m_mockThingId);
    int port = thing->paramValue(mockThingHttpportParamTypeId).toInt();
    QNetworkRequest request(QUrl(QString("http://localhost:%1/generateevent?eventtypeid=%2&%3=%4")
                                 .arg(port)
                                 .arg(mockEvent2EventTypeId.toString())
                                 .arg(mockEvent2EventIntParamParamTypeId.toString())
                                 .arg(23)));
    QNetworkAccessManager nam;
    QNetworkReply *r = nam.get(request);
    connect(r, &QNetworkReply::finished, r, &QNetworkReply::deleteLater);

    spy.wait();

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().at(0).value<ThingId>(), m_mockThingId);
    QCOMPARE(EventTypeId(spy.first().at(1).toUuid()), mockEvent2EventTypeId);
    QVariantMap expectedParams;
    expectedParams.insert(mockEvent2EventIntParamParamTypeId.toString().remove(QRegExp("[{}]")), 23);
    expectedParams.insert("intParam", 23);
    QVERIFY2(spy.first().at(2).toMap() == expectedParams, QString("Params not matching.\nExpected: %1\nGot: %2")
             .arg(QString(QJsonDocument::fromVariant(expectedParams).toJson(QJsonDocument::Indented)))
             .arg(QString(QJsonDocument::fromVariant(spy.first().at(2).toMap()).toJson(QJsonDocument::Indented)))
             .toUtf8());
}

void TestScripts::testScriptEventByName()
{
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
                            "}\n").arg(m_mockThingId.toString()).arg("event2");

    qCDebug(dcTests()) << "Adding script:\n" << qUtf8Printable(script);
    ScriptEngine::AddScriptReply reply = NymeaCore::instance()->scriptEngine()->addScript("TestEvent", script.toUtf8());
    QCOMPARE(reply.scriptError, ScriptEngine::ScriptErrorNoError);

    QSignalSpy spy(TestHelper::instance(), &TestHelper::eventLogged);

    // trigger event in mock device
    Thing* thing = NymeaCore::instance()->thingManager()->findConfiguredThing(m_mockThingId);
    int port = thing->paramValue(mockThingHttpportParamTypeId).toInt();
    QNetworkRequest request(QUrl(QString("http://localhost:%1/generateevent?eventtypeid=%2&%3=%4")
                                 .arg(port)
                                 .arg(mockEvent2EventTypeId.toString())
                                 .arg(mockEvent2EventIntParamParamTypeId.toString())
                                 .arg(10)));
    QNetworkAccessManager nam;
    QNetworkReply *r = nam.get(request);
    connect(r, &QNetworkReply::finished, r, &QNetworkReply::deleteLater);

    spy.wait();

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().at(0).value<ThingId>(), m_mockThingId);
    QCOMPARE(spy.first().at(1).toString(), QString("event2"));
    QVariantMap expectedParams;
    expectedParams.insert(mockEvent2EventIntParamParamTypeId.toString().remove(QRegExp("[{}]")), 10);
    expectedParams.insert("intParam", 10);
    QCOMPARE(spy.first().at(2).toMap(), expectedParams);
}

void TestScripts::testReadScriptStateById()
{
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
                            "}\n").arg(m_mockThingId.toString()).arg(mockPowerStateTypeId.toString());

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
                            "}\n").arg(m_mockThingId.toString()).arg("power");

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
                            "}\n").arg(m_mockThingId.toString()).arg(mockPowerStateTypeId.toString());

    qCDebug(dcTests()) << "Adding script:\n" << qUtf8Printable(script);
    ScriptEngine::AddScriptReply reply = NymeaCore::instance()->scriptEngine()->addScript("TestState", script.toUtf8());
    QCOMPARE(reply.scriptError, ScriptEngine::ScriptErrorNoError);

    QSignalSpy spy(NymeaCore::instance()->thingManager(), &ThingManager::thingStateChanged);

    TestHelper::instance()->setState(true);

    spy.wait(1);

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().at(0).value<Thing*>()->id(), m_mockThingId);
    QCOMPARE(spy.first().at(1).value<StateTypeId>(), mockPowerStateTypeId);
    QCOMPARE(spy.first().at(2).toBool(), true);
}

void TestScripts::testWriteScriptStateByName()
{
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
                            "}\n").arg(m_mockThingId.toString()).arg("power");

    qCDebug(dcTests()) << "Adding script:\n" << qUtf8Printable(script);
    ScriptEngine::AddScriptReply reply = NymeaCore::instance()->scriptEngine()->addScript("TestState", script.toUtf8());
    QCOMPARE(reply.scriptError, ScriptEngine::ScriptErrorNoError);

    QSignalSpy spy(NymeaCore::instance()->thingManager(), &ThingManager::thingStateChanged);

    TestHelper::instance()->setState(true);

    spy.wait(1);

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().at(0).value<Thing*>()->id(), m_mockThingId);
    QCOMPARE(spy.first().at(1).value<StateTypeId>(), mockPowerStateTypeId);
    QCOMPARE(spy.first().at(2).toBool(), true);
}

void TestScripts::testScriptActionById()
{
    QString script = QString("import QtQuick 2.0\n"
                            "import nymea 1.0\n"
                            "Item {\n"
                            "    ThingAction {\n"
                            "        id: thingAction\n"
                            "        thingId: \"%1\"\n"
                            "        actionTypeId: \"%2\"\n"
                            "    }\n"
                            "    Connections {\n"
                            "        target: TestHelper\n"
                            "        onExecuteAction: {\n"
                            "            thingAction.execute(params)\n"
                            "        }\n"
                            "    }\n"
                            "}\n").arg(m_mockThingId.toString()).arg(mockPowerActionTypeId.toString());

    qCDebug(dcTests()) << "Adding script:\n" << qUtf8Printable(script);
    ScriptEngine::AddScriptReply reply = NymeaCore::instance()->scriptEngine()->addScript("TestAction", script.toUtf8());
    QCOMPARE(reply.scriptError, ScriptEngine::ScriptErrorNoError);

    QSignalSpy spy(NymeaCore::instance()->thingManager(), &ThingManager::thingStateChanged);

    QVariantMap params;
    params.insert(mockPowerActionPowerParamTypeId.toString(), true);
    TestHelper::instance()->executeAction(params);

    spy.wait(1);

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().at(0).value<Thing*>()->id(), m_mockThingId);
    QCOMPARE(spy.first().at(1).value<StateTypeId>(), mockPowerStateTypeId);
    QCOMPARE(spy.first().at(2).toBool(), true);
}

void TestScripts::testScriptActionByName()
{
    QString script = QString("import QtQuick 2.0\n"
                            "import nymea 1.0\n"
                            "Item {\n"
                            "    ThingAction {\n"
                            "        id: thingAction\n"
                            "        thingId: \"%1\"\n"
                            "        actionName: \"%2\"\n"
                            "    }\n"
                            "    Connections {\n"
                            "        target: TestHelper\n"
                            "        onExecuteAction: {\n"
                            "            thingAction.execute(params)\n"
                            "        }\n"
                            "    }\n"
                            "}\n").arg(m_mockThingId.toString()).arg("power");

    qCDebug(dcTests()) << "Adding script:\n" << qUtf8Printable(script);
    ScriptEngine::AddScriptReply reply = NymeaCore::instance()->scriptEngine()->addScript("TestAction", script.toUtf8());
    QCOMPARE(reply.scriptError, ScriptEngine::ScriptErrorNoError);

    QSignalSpy spy(NymeaCore::instance()->thingManager(), &ThingManager::thingStateChanged);

    QVariantMap params;
    params.insert("power", true);
    TestHelper::instance()->executeAction(params);

    spy.wait(1);

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().at(0).value<Thing*>()->id(), m_mockThingId);
    QCOMPARE(spy.first().at(1).value<StateTypeId>(), mockPowerStateTypeId);
    QCOMPARE(spy.first().at(2).toBool(), true);
}

void TestScripts::testScriptAlarm_data()
{
    QTest::addColumn<QTime>("time");
    QTest::addColumn<QTime>("endTime");
    QTest::addColumn<bool>("active");

    QTest::newRow("active, regular") << QTime(12, 05);
}

void TestScripts::testScriptAlarm()
{

}

void TestScripts::testInterfaceEvent()
{
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
                            "}\n").arg("button").arg("pressed");

    qCDebug(dcTests()) << "Adding script:\n" << qUtf8Printable(script);
    ScriptEngine::AddScriptReply reply = NymeaCore::instance()->scriptEngine()->addScript("TestEvent", script.toUtf8());
    QCOMPARE(reply.scriptError, ScriptEngine::ScriptErrorNoError);

    QSignalSpy spy(TestHelper::instance(), &TestHelper::eventLogged);

    // trigger event in mock device
    Thing* thing = NymeaCore::instance()->thingManager()->findConfiguredThing(m_mockThingId);
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
    expectedParams.insert(mockPressedEventButtonNameParamTypeId.toString().remove(QRegExp("[{}]")), "xxx");
    expectedParams.insert("buttonName", "xxx");
    QCOMPARE(spy.first().at(2).toMap(), expectedParams);

}

void TestScripts::testInterfaceState()
{
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
                            "}\n").arg("power").arg("power");

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
                            "}\n").arg("power").arg("power");

    qCDebug(dcTests()) << "Adding script:\n" << qUtf8Printable(script);
    ScriptEngine::AddScriptReply reply = NymeaCore::instance()->scriptEngine()->addScript("TestInterfaceAction", script.toUtf8());
    QCOMPARE(reply.scriptError, ScriptEngine::ScriptErrorNoError);

    QSignalSpy spy(NymeaCore::instance()->thingManager(), &ThingManager::thingStateChanged);

    QVariantMap params;
    params.insert("power", true);
    TestHelper::instance()->executeAction(params);

    spy.wait(1);

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().at(0).value<Thing*>()->id(), m_mockThingId);
    QCOMPARE(spy.first().at(1).value<StateTypeId>(), mockPowerStateTypeId);
    QCOMPARE(spy.first().at(2).toBool(), true);

}


#include "testscripts.moc"
QTEST_MAIN(TestScripts)
