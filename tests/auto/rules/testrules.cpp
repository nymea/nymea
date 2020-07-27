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
#include "nymeasettings.h"
#include "servers/mocktcpserver.h"
#include "nymeacore.h"
#include "jsonrpc/jsonhandler.h"

using namespace nymeaserver;

class TestRules: public NymeaTestBase
{
    Q_OBJECT

private:
    void cleanupMockHistory();
    void cleanupRules();

    ThingId addDisplayPinMock();

    QVariantMap createEventDescriptor(const ThingId &thingId, const EventTypeId &eventTypeId);
    QVariantMap createActionWithParams(const ThingId &thingId);
    QVariantMap createStateEvaluatorFromSingleDescriptor(const QVariantMap &stateDescriptor);

    void setWritableStateValue(const ThingId &thingId, const StateTypeId &stateTypeId, const QVariant &value);

    void verifyRuleExecuted(const ActionTypeId &actionTypeId);
    void verifyRuleNotExecuted();

    QVariant validIntStateBasedRule(const QString &name, const bool &executable, const bool &enabled);

    void generateEvent(const EventTypeId &eventTypeId);

    inline void verifyRuleError(const QVariant &response, RuleEngine::RuleError error = RuleEngine::RuleErrorNoError) {
        verifyError(response, "ruleError", enumValueName(error));
    }
    inline void verifyThingError(const QVariant &response, Thing::ThingError error = Thing::ThingErrorNoError) {
        verifyError(response, "thingError", enumValueName(error));
    }

private slots:

    void initTestCase();

    void cleanup();
    void emptyRule();
    void getInvalidRule();

    void addRemoveRules_data();
    void addRemoveRules();

    void editRules_data();
    void editRules();

    void executeRuleActions_data();
    void executeRuleActions();

    void findRule();

    void removeInvalidRule();

    void loadStoreConfig();

    void evaluateEvent();

    void evaluateEventParams();

    void testStateEvaluator_data();
    void testStateEvaluator();

    void testStateEvaluator2_data();
    void testStateEvaluator2();

    void testStateEvaluator3_data();
    void testStateEvaluator3();

    void testChildEvaluator_data();
    void testChildEvaluator();

    void testStateChange();

    void enableDisableRule();

    void testEventBasedAction();
    void testEventBasedRuleWithExitAction();

    void testStateBasedAction();

    void removePolicyUpdate();
    void removePolicyCascade();
    void removePolicyUpdateRendersUselessRule();

    void testRuleActionParams_data();
    void testRuleActionParams();

    void testRuleActionPAramsFromEventParameter_data();
    void testRuleActionPAramsFromEventParameter();

    void testInitStatesActive();

    void testInterfaceBasedEventRule();

    void testInterfaceBasedStateRule();

    void testLoopingRules();

    void testScene();

    void testHousekeeping_data();
    void testHousekeeping();
};

void TestRules::cleanupMockHistory() {
    QNetworkAccessManager nam;
    QSignalSpy spy(&nam, SIGNAL(finished(QNetworkReply*)));
    QNetworkRequest request(QUrl(QString("http://localhost:%1/clearactionhistory").arg(QString::number(m_mockThing1Port))));
    QNetworkReply *reply = nam.get(request);
    spy.wait();
    QCOMPARE(spy.count(), 1);
    reply->deleteLater();
}

void TestRules::cleanupRules() {
    QVariant response = injectAndWait("Rules.GetRules");
    foreach (const QVariant &ruleDescription, response.toMap().value("params").toMap().value("ruleDescriptions").toList()) {
        QVariantMap params;
        params.insert("ruleId", ruleDescription.toMap().value("id").toString());
        verifyRuleError(injectAndWait("Rules.RemoveRule", params));
    }
}

ThingId TestRules::addDisplayPinMock()
{
    // Discover things
    QVariantList discoveryParams;
    QVariantMap resultCountParam;
    resultCountParam.insert("paramTypeId", displayPinMockDiscoveryResultCountParamTypeId);
    resultCountParam.insert("value", 1);
    discoveryParams.append(resultCountParam);

    QVariantMap params;
    params.insert("thingClassId", displayPinMockThingClassId);
    params.insert("discoveryParams", discoveryParams);
    QVariant response = injectAndWait("Integrations.DiscoverThings", params);

    verifyThingError(response, Thing::ThingErrorNoError);

    // Pair mock
    ThingDescriptorId descriptorId = ThingDescriptorId(response.toMap().value("params").toMap().value("thingDescriptors").toList().first().toMap().value("id").toString());
    params.clear();
    params.insert("thingClassId", displayPinMockThingClassId);
    params.insert("name", "Display pin mock");
    params.insert("thingDescriptorId", descriptorId.toString());
    response = injectAndWait("Integrations.PairThing", params);

    verifyThingError(response);

    PairingTransactionId pairingTransactionId(response.toMap().value("params").toMap().value("pairingTransactionId").toString());
    QString displayMessage = response.toMap().value("params").toMap().value("displayMessage").toString();

    qDebug() << "displayMessage" << displayMessage;

    params.clear();
    params.insert("pairingTransactionId", pairingTransactionId.toString());
    params.insert("secret", "243681");
    response = injectAndWait("Integrations.ConfirmPairing", params);

    verifyThingError(response);

    return ThingId(response.toMap().value("params").toMap().value("thingId").toString());
}

QVariantMap TestRules::createEventDescriptor(const ThingId &thingId, const EventTypeId &eventTypeId)
{
    QVariantMap eventDescriptor;
    eventDescriptor.insert("eventTypeId", eventTypeId);
    eventDescriptor.insert("thingId", thingId);
    return eventDescriptor;
}

QVariantMap TestRules::createActionWithParams(const ThingId &thingId)
{
    QVariantMap action;
    QVariantList ruleActionParams;
    QVariantMap param1;
    param1.insert("paramTypeId", mockWithParamsActionParam1ParamTypeId);
    param1.insert("value", 4);
    QVariantMap param2;
    param2.insert("paramTypeId", mockWithParamsActionParam2ParamTypeId);
    param2.insert("value", true);
    ruleActionParams.append(param1);
    ruleActionParams.append(param2);
    action.insert("thingId", thingId);
    action.insert("actionTypeId", mockWithParamsActionTypeId);
    action.insert("ruleActionParams", ruleActionParams);
    return action;
}

QVariantMap TestRules::createStateEvaluatorFromSingleDescriptor(const QVariantMap &stateDescriptor)
{
    QVariantMap stateEvaluator;
    stateEvaluator.insert("stateDescriptor", stateDescriptor);
    return stateEvaluator;
}

void TestRules::setWritableStateValue(const ThingId &thingId, const StateTypeId &stateTypeId, const QVariant &value)
{
    enableNotifications({"Integrations"});
    QVariantMap params;
    params.insert("thingId", thingId);
    params.insert("stateTypeId", stateTypeId);
    QVariant response = injectAndWait("Integrations.GetStateValue", params);
    verifyThingError(response);

    QVariant currentStateValue = response.toMap().value("params").toMap().value("value");
    bool shouldGetNotification = currentStateValue != value;
    QSignalSpy stateSpy(m_mockTcpServer, SIGNAL(outgoingData(QUuid,QByteArray)));

    QVariantMap paramMap;
    paramMap.insert("paramTypeId", stateTypeId.toString());
    paramMap.insert("value", value);

    params.clear(); response.clear();
    params.insert("thingId", thingId);
    params.insert("actionTypeId", stateTypeId.toString());
    params.insert("params", QVariantList() << paramMap);

    printJson(params);

    response = injectAndWait("Integrations.ExecuteAction", params);
    qCDebug(dcTests()) << "Execute action response" << response;
    verifyThingError(response);

    if (shouldGetNotification) {
        stateSpy.wait(200);
        // Wait for state changed notification
        QVariantList stateChangedVariants = checkNotifications(stateSpy, "Integrations.StateChanged");
        QVERIFY2(stateChangedVariants.count() == 1, "Did not get Integrations.StateChanged notification.");

        qCDebug(dcTests()) << "Notification content:" << qUtf8Printable(QJsonDocument::fromVariant(stateChangedVariants).toJson());
        QVariantMap notification = stateChangedVariants.first().toMap().value("params").toMap();
        QVERIFY2(notification.contains("thingId"), "Integrations.StateChanged notification does not contain thingId");
        QVERIFY2(ThingId(notification.value("thingId").toString()) == thingId, "Integrations.StateChanged notification does not contain the correct thingId");
        QVERIFY2(notification.contains("stateTypeId"), "Integrations.StateChanged notification does not contain stateTypeId");
        QVERIFY2(StateTypeId(notification.value("stateTypeId").toString()) == stateTypeId, "Integrations.StateChanged notification does not contain the correct stateTypeId");
        QVERIFY2(notification.contains("value"), "Integrations.StateChanged notification does not contain new state value");
        QVERIFY2(notification.value("value") == QVariant(value), QString("Integrations.StateChanged notification does not contain the new value. Got: %1, Expected: %2").arg(notification.value("value").toString()).arg(QVariant(value).toString()).toUtf8());
    }

}

void TestRules::verifyRuleExecuted(const ActionTypeId &actionTypeId)
{
    // Verify rule got executed
    bool actionFound = false;
    QByteArray actionHistory;
    int i = 0;
    while (!actionFound && i < 50) {
        QNetworkAccessManager nam;
        QSignalSpy spy(&nam, SIGNAL(finished(QNetworkReply*)));
        QNetworkRequest request(QUrl(QString("http://localhost:%1/actionhistory").arg(QString::number(m_mockThing1Port))));
        QNetworkReply *reply = nam.get(request);
        spy.wait();
        QCOMPARE(spy.count(), 1);

        actionHistory = reply->readAll();
        actionFound = actionTypeId == ActionTypeId(actionHistory);
        reply->deleteLater();
        if (!actionFound) {
            QTest::qWait(100);
        }
        i++;
    }
    QVERIFY2(actionFound, "Action not triggered. Current action history: \"" + actionHistory + "\"");
}

void TestRules::verifyRuleNotExecuted()
{
    QNetworkAccessManager nam;
    QSignalSpy spy(&nam, SIGNAL(finished(QNetworkReply*)));
    QNetworkRequest request(QUrl(QString("http://localhost:%1/actionhistory").arg(QString::number(m_mockThing1Port))));
    QNetworkReply *reply = nam.get(request);
    spy.wait();
    QCOMPARE(spy.count(), 1);

    QByteArray actionHistory = reply->readAll();
    QVERIFY2(actionHistory.isEmpty(), "Action is triggered while it should not have been. Current action history: \"" + actionHistory + "\"");
    reply->deleteLater();
}


/***********************************************************************/

void TestRules::cleanup() {
    cleanupMockHistory();
    cleanupRules();
}

void TestRules::emptyRule()
{
    QVariantMap params;
    params.insert("name", QString());
    params.insert("actions", QVariantList());
    QVariant response = injectAndWait("Rules.AddRule", params);
    verifyRuleError(response, RuleEngine::RuleErrorInvalidRuleFormat);
}

void TestRules::getInvalidRule()
{
    QVariantMap params;
    params.insert("ruleId", QUuid::createUuid());
    QVariant response = injectAndWait("Rules.GetRuleDetails", params);
    verifyRuleError(response, RuleEngine::RuleErrorRuleNotFound);
}

QVariant TestRules::validIntStateBasedRule(const QString &name, const bool &executable, const bool &enabled)
{
    QVariantMap params;

    // StateDescriptor
    QVariantMap stateDescriptor;
    stateDescriptor.insert("stateTypeId", mockIntStateTypeId);
    stateDescriptor.insert("thingId", m_mockThingId);
    stateDescriptor.insert("operator", enumValueName(Types::ValueOperatorLess));
    stateDescriptor.insert("value", 25);

    // StateEvaluator
    QVariantMap stateEvaluator;
    stateEvaluator.insert("stateDescriptor", stateDescriptor);
    stateEvaluator.insert("operator", enumValueName(Types::StateOperatorAnd));

    // RuleAction
    QVariantMap action;
    action.insert("actionTypeId", mockWithParamsActionTypeId);
    QVariantList actionParams;
    QVariantMap param1;
    param1.insert("paramTypeId", mockWithParamsActionParam1ParamTypeId);
    param1.insert("value", 5);
    actionParams.append(param1);
    QVariantMap param2;
    param2.insert("paramTypeId", mockWithParamsActionParam2ParamTypeId);
    param2.insert("value", true);
    actionParams.append(param2);
    action.insert("thingId", m_mockThingId);
    action.insert("ruleActionParams", actionParams);

    // RuleExitAction
    QVariantMap exitAction;
    exitAction.insert("actionTypeId", mockWithoutParamsActionTypeId);
    exitAction.insert("thingId", m_mockThingId);
    exitAction.insert("ruleActionParams", QVariantList());

    params.insert("name", name);
    params.insert("enabled", enabled);
    params.insert("executable", executable);
    params.insert("stateEvaluator", stateEvaluator);
    params.insert("actions", QVariantList() << action);
    params.insert("exitActions", QVariantList() << exitAction);

    return params;
}

void TestRules::generateEvent(const EventTypeId &eventTypeId)
{
    // Trigger an event
    QNetworkAccessManager nam;
    QSignalSpy spy(&nam, SIGNAL(finished(QNetworkReply*)));

    // trigger event in mock
    QNetworkRequest request(QUrl(QString("http://localhost:%1/generateevent?eventtypeid=%2").arg(m_mockThing1Port).arg(eventTypeId.toString())));
    QNetworkReply *reply = nam.get(request);
    spy.wait();
    QCOMPARE(spy.count(), 1);
    reply->deleteLater();

}

void TestRules::initTestCase()
{
    NymeaTestBase::initTestCase();
    QLoggingCategory::setFilterRules("*.debug=false\n"
                                     "Tests.debug=true\n"
                                     "RuleEngine.debug=true\n"
//                                     "RuleEngineDebug.debug=true\n"
                                     "JsonRpc.debug=true\n"
                                     "Mock.*=true");
}

void TestRules::addRemoveRules_data()
{
    // RuleAction
    QVariantMap validActionNoParams;
    validActionNoParams.insert("actionTypeId", mockWithoutParamsActionTypeId);
    validActionNoParams.insert("thingId", m_mockThingId);
    validActionNoParams.insert("deviceId", m_mockThingId); // DEPRECATED

    QVariantMap invalidAction;
    invalidAction.insert("actionTypeId", ActionTypeId("f32c7efb-38b6-4576-a496-c75bbb23132f"));
    invalidAction.insert("thingId", m_mockThingId);

    // RuleExitAction
    QVariantMap validExitActionNoParams;
    validExitActionNoParams.insert("actionTypeId", mockWithoutParamsActionTypeId);
    validExitActionNoParams.insert("thingId", m_mockThingId);
    validExitActionNoParams.insert("deviceId", m_mockThingId); // DEPRECATED

    QVariantMap invalidExitAction;
    invalidExitAction.insert("actionTypeId", ActionTypeId("f32c7efb-38b6-4576-a496-c75bbb23132f"));
    invalidExitAction.insert("thingId", m_mockThingId);

    // StateDescriptor
    QVariantMap stateDescriptor;
    stateDescriptor.insert("stateTypeId", mockIntStateTypeId);
    stateDescriptor.insert("thingId", m_mockThingId);
    stateDescriptor.insert("operator", enumValueName(Types::ValueOperatorLess));
    stateDescriptor.insert("value", 20);

    // StateEvaluator
    QVariantMap validStateEvaluator;
    validStateEvaluator.insert("stateDescriptor", stateDescriptor);
    validStateEvaluator.insert("operator", enumValueName(Types::StateOperatorAnd));

    QVariantMap invalidStateEvaluator;
    stateDescriptor.remove("thingId");
    invalidStateEvaluator.insert("stateDescriptor", stateDescriptor);

    // EventDescriptor
    QVariantMap validEventDescriptor1;
    validEventDescriptor1.insert("eventTypeId", mockEvent1EventTypeId);
    validEventDescriptor1.insert("thingId", m_mockThingId);
    validEventDescriptor1.insert("deviceId", m_mockThingId); // DEPRECATED

    QVariantMap validEventDescriptor2;
    validEventDescriptor2.insert("eventTypeId", mockEvent2EventTypeId);
    validEventDescriptor2.insert("thingId", m_mockThingId);
    validEventDescriptor2.insert("deviceId", m_mockThingId); // DEPRECATED

    QVariantList params;
    QVariantMap param1;
    param1.insert("paramTypeId", mockEvent2EventIntParamParamTypeId);
    param1.insert("value", 3);
    param1.insert("operator", enumValueName(Types::ValueOperatorEquals));
    params.append(param1);
    validEventDescriptor2.insert("paramDescriptors", params);

    QVariantMap validEventDescriptor3;
    validEventDescriptor3.insert("eventTypeId", mockEvent2EventTypeId);
    validEventDescriptor3.insert("thingId", m_mockThingId);
    validEventDescriptor3.insert("deviceId", m_mockThingId); // DREPECATED

    // EventDescriptorList
    QVariantList eventDescriptorList;
    eventDescriptorList.append(validEventDescriptor1);
    eventDescriptorList.append(validEventDescriptor2);

    QVariantMap invalidEventDescriptor;
    invalidEventDescriptor.insert("eventTypeId", mockEvent1EventTypeId);
    invalidEventDescriptor.insert("thingId", ThingId("2c4825c8-dfb9-4ba4-bd0e-1d827d945d41"));

    // RuleAction event based
    QVariantMap validActionEventBased;
    validActionEventBased.insert("actionTypeId", mockWithParamsActionTypeId);
    validActionEventBased.insert("thingId", m_mockThingId);
    validActionEventBased.insert("deviceId", m_mockThingId); // DEPRECATED
    QVariantMap validActionEventBasedParam1;
    validActionEventBasedParam1.insert("paramTypeId", mockWithParamsActionParam1ParamTypeId);
    validActionEventBasedParam1.insert("eventTypeId", mockEvent2EventTypeId);
    validActionEventBasedParam1.insert("eventParamTypeId", mockEvent2EventIntParamParamTypeId);

    QVariantMap validActionEventBasedParam2;
    validActionEventBasedParam2.insert("paramTypeId", mockWithParamsActionParam2ParamTypeId);
    validActionEventBasedParam2.insert("value", false);
    validActionEventBased.insert("ruleActionParams", QVariantList() << validActionEventBasedParam1 << validActionEventBasedParam2);

    QVariantMap invalidActionEventBased;
    invalidActionEventBased.insert("actionTypeId", mockWithoutParamsActionTypeId);
    invalidActionEventBased.insert("thingId", m_mockThingId);
    validActionEventBasedParam1.insert("value", 10);
    invalidActionEventBased.insert("ruleActionParams", QVariantList() << validActionEventBasedParam1);

    QVariantMap invalidActionEventBased2;
    invalidActionEventBased2.insert("actionTypeId", mockWithParamsActionTypeId);
    invalidActionEventBased2.insert("thingId", m_mockThingId);
    QVariantMap invalidActionEventBasedParam2;
    invalidActionEventBasedParam2.insert("paramTypeId", mockWithParamsActionParam1ParamTypeId);
    invalidActionEventBasedParam2.insert("eventTypeId", mockEvent1EventTypeId);
    invalidActionEventBasedParam2.insert("eventParamTypeId", ParamTypeId("7dbf5266-5179-4e09-ac31-631cc63f1d7b"));
    QVariantMap invalidActionEventBasedParam3;
    invalidActionEventBasedParam3.insert("paramTypeId", mockWithParamsActionParam2ParamTypeId);
    invalidActionEventBasedParam3.insert("value", 2);
    invalidActionEventBased2.insert("ruleActionParams", QVariantList() << invalidActionEventBasedParam2 << invalidActionEventBasedParam3);

    QVariantMap invalidActionEventBased3;
    invalidActionEventBased3.insert("actionTypeId", mockWithParamsActionTypeId);
    invalidActionEventBased3.insert("thingId", m_mockThingId);
    QVariantMap invalidActionEventBasedParam4;
    invalidActionEventBasedParam4.insert("paramTypeId", mockWithParamsActionParam1ParamTypeId);
    invalidActionEventBasedParam4.insert("eventTypeId", mockEvent1EventTypeId);
    invalidActionEventBasedParam4.insert("eventParamTypeId", mockEvent2EventIntParamParamTypeId);
    invalidActionEventBased3.insert("ruleActionParams", QVariantList() << invalidActionEventBasedParam4);

    QTest::addColumn<bool>("enabled");
    QTest::addColumn<QVariantMap>("action1");
    QTest::addColumn<QVariantMap>("exitAction1");
    QTest::addColumn<QVariantMap>("eventDescriptor");
    QTest::addColumn<QVariantList>("eventDescriptorList");
    QTest::addColumn<QVariantMap>("stateEvaluator");
    QTest::addColumn<RuleEngine::RuleError>("error");
    QTest::addColumn<bool>("jsonError");
    QTest::addColumn<QString>("name");

    // Rules with event based actions
    QTest::newRow("valid rule. enabled, 1 Action (eventBased), 1 EventDescriptor, name")                << true     << validActionEventBased    << QVariantMap()            << validEventDescriptor3    << QVariantList()       << QVariantMap()            << RuleEngine::RuleErrorNoError << true << "ActionEventRule1";
    QTest::newRow("invalid rule. enabled, 1 Action (eventBased), 1 EventDescriptor, name")              << true     << invalidActionEventBased2 << QVariantMap()            << validEventDescriptor3    << QVariantList()       << QVariantMap()            << RuleEngine::RuleErrorInvalidRuleActionParameter << false << "TestRule";

    QTest::newRow("invalid rule. enabled, 1 Action (eventBased), types not matching, name")             << true     << invalidActionEventBased3 << QVariantMap()            << validEventDescriptor1    << QVariantList()       << QVariantMap()            << RuleEngine::RuleErrorTypesNotMatching << false << "TestRule";

    QTest::newRow("invalid rule. enabled, 1 invalid Action (eventBased), 1 EventDescriptor, name")      << true     << invalidActionEventBased  << QVariantMap()            << validEventDescriptor2    << QVariantList()       << QVariantMap()            << RuleEngine::RuleErrorTypesNotMatching << false << "TestRule";
    QTest::newRow("invalid rule. enabled, 1 Action (eventBased), 1 StateEvaluator, name")               << true     << validActionEventBased    << QVariantMap()            << QVariantMap()            << QVariantList()       << validStateEvaluator      << RuleEngine::RuleErrorInvalidRuleActionParameter << false << "TestRule";
    QTest::newRow("invalid rule. enabled, invalid rule format")                                         << true     << validActionEventBased    << validActionEventBased    << validEventDescriptor2    << QVariantList()       << QVariantMap()            << RuleEngine::RuleErrorInvalidRuleFormat << false << "TestRule";
    QTest::newRow("invalid rule. enabled, 1 Action, 1 ExitAction (EventBased), name")                   << true     << validActionNoParams      << validActionEventBased    << validEventDescriptor2    << QVariantList()       << QVariantMap()            << RuleEngine::RuleErrorInvalidRuleFormat << false << "TestRule";

    // Rules with exit actions
    QTest::newRow("valid rule. enabled, 1 Action, 1 Exit Action,  1 StateEvaluator, name")              << true     << validActionNoParams      << validExitActionNoParams  << QVariantMap()            << QVariantList()       << validStateEvaluator      << RuleEngine::RuleErrorNoError << true << "TestRule";
    QTest::newRow("valid rule. disabled, 1 Action, 1 Exit Action, 1 StateEvaluator, name")              << false    << validActionNoParams      << validExitActionNoParams  << QVariantMap()            << QVariantList()       << validStateEvaluator      << RuleEngine::RuleErrorNoError << true << "TestRule";
    QTest::newRow("invalid rule. disabled, 1 Action, 1 invalid Exit Action, 1 StateEvaluator, name")    << false    << validActionNoParams      << invalidExitAction        << QVariantMap()            << QVariantList()       << validStateEvaluator      << RuleEngine::RuleErrorActionTypeNotFound << false << "TestRule";
    QTest::newRow("valid rule. 1 Action, 1 Exit Action, 1 EventDescriptor, 1 StateEvaluator, name")     << true     << validActionNoParams      << validExitActionNoParams  << validEventDescriptor1    << QVariantList()       << validStateEvaluator      << RuleEngine::RuleErrorNoError << false << "TestRule";
    QTest::newRow("invalid rule. 1 Action, 1 Exit Action, eventDescriptorList, NO StateEvaluator, name")<< true     << validActionNoParams      << validExitActionNoParams  << QVariantMap()            << eventDescriptorList  << QVariantMap()            << RuleEngine::RuleErrorInvalidRuleFormat << false << "TestRule";
    QTest::newRow("valid rule. 1 Action, 1 Exit Action, eventDescriptorList, 1 StateEvaluator, name")   << true     << validActionNoParams      << validExitActionNoParams  << QVariantMap()            << eventDescriptorList  << validStateEvaluator      << RuleEngine::RuleErrorNoError << false << "TestRule";

    // Rules without exit actions
    QTest::newRow("valid rule. enabled, 1 EventDescriptor, StateEvaluator, 1 Action, name")             << true     << validActionNoParams      << QVariantMap()            << validEventDescriptor1    << QVariantList()       << validStateEvaluator      << RuleEngine::RuleErrorNoError << true << "TestRule";
    QTest::newRow("valid rule. disabled, 1 EventDescriptor, StateEvaluator, 1 Action, name")             << false    << validActionNoParams      << QVariantMap()            << validEventDescriptor1    << QVariantList()       << validStateEvaluator      << RuleEngine::RuleErrorNoError << true << "TestRule";
    QTest::newRow("valid rule. 2 EventDescriptors, 1 Action, name")                                     << true     << validActionNoParams      << QVariantMap()            << QVariantMap()            << eventDescriptorList  << validStateEvaluator      << RuleEngine::RuleErrorNoError << true << "TestRule";
    QTest::newRow("invalid action")                                                                     << true     << invalidAction            << QVariantMap()            << validEventDescriptor1    << QVariantList()       << validStateEvaluator      << RuleEngine::RuleErrorActionTypeNotFound << false << "TestRule";
    QTest::newRow("invalid event descriptor")                                                           << true     << validActionNoParams      << QVariantMap()            << invalidEventDescriptor   << QVariantList()       << validStateEvaluator      << RuleEngine::RuleErrorThingNotFound << false << "TestRule";
}

void TestRules::addRemoveRules()
{
    QFETCH(bool, enabled);
    QFETCH(QVariantMap, action1);
    QFETCH(QVariantMap, exitAction1);
    QFETCH(QVariantMap, eventDescriptor);
    QFETCH(QVariantList, eventDescriptorList);
    QFETCH(QVariantMap, stateEvaluator);
    QFETCH(RuleEngine::RuleError, error);
    QFETCH(bool, jsonError);
    QFETCH(QString, name);

    QVariantMap params;
    params.insert("name", name);

    QVariantList actions;
    actions.append(action1);
    params.insert("actions", actions);

    if (!eventDescriptor.isEmpty()) {
        params.insert("eventDescriptors", QVariantList() << eventDescriptor);
    }
    if (!eventDescriptorList.isEmpty()) {
        params.insert("eventDescriptors", eventDescriptorList);
    }
    QVariantList exitActions;
    if (!exitAction1.isEmpty()) {
        exitActions.append(exitAction1);
        params.insert("exitActions", exitActions);
    }
    params.insert("stateEvaluator", stateEvaluator);
    if (!enabled) {
        params.insert("enabled", enabled);
    }
    qCDebug(dcTests()) << "Calling with params:" << qUtf8Printable(QJsonDocument::fromVariant(params).toJson());
    QVariant response = injectAndWait("Rules.AddRule", params);
    if (!jsonError) {
        verifyRuleError(response, error);
    }

    RuleId newRuleId = RuleId(response.toMap().value("params").toMap().value("ruleId").toString());

    response = injectAndWait("Rules.GetRules");
    QVariantList rules = response.toMap().value("params").toMap().value("ruleDescriptions").toList();

    if (error != RuleEngine::RuleErrorNoError) {
        QVERIFY2(rules.count() == 0, "There should be no rules.");
        return;
    }

    QVERIFY2(rules.count() == 1, "There should be exactly one rule");
    QCOMPARE(RuleId(rules.first().toMap().value("id").toString()), newRuleId);

    params.clear();
    params.insert("ruleId", newRuleId);
    response = injectAndWait("Rules.GetRuleDetails", params);
    QVariantMap rule = response.toMap().value("params").toMap().value("rule").toMap();

    qDebug() << rule.value("name").toString();
    QVERIFY2(rule.value("enabled").toBool() == enabled, "Rule enabled state doesn't match");
    QVariantList eventDescriptors = rule.value("eventDescriptors").toList();
    if (!eventDescriptor.isEmpty()) {
        QVERIFY2(eventDescriptors.count() == 1, "There should be exactly one eventDescriptor");
        QVariantMap stringifiedEventDescriptor = QJsonDocument::fromVariant(eventDescriptor).toVariant().toMap();
        QVERIFY2(eventDescriptors.first().toMap() == stringifiedEventDescriptor,
                 QString("Event descriptor doesn't match:\nExpected: %1\nGot: %2")
                 .arg(QString(QJsonDocument::fromVariant(eventDescriptor).toJson()))
                 .arg(QString(QJsonDocument::fromVariant(eventDescriptors.first().toMap()).toJson())).toUtf8());
    } else if (eventDescriptorList.isEmpty()){
        QVERIFY2(eventDescriptors.count() == eventDescriptorList.count(), QString("There should be exactly %1 eventDescriptor").arg(eventDescriptorList.count()).toLatin1().data());
        foreach (const QVariant &eventDescriptorVariant, eventDescriptorList) {
            bool found = false;
            foreach (const QVariant &replyEventDescriptorVariant, eventDescriptors) {
                if (eventDescriptorVariant.toMap().value("deviceId") == replyEventDescriptorVariant.toMap().value("deviceId") &&
                        eventDescriptorVariant.toMap().value("eventTypeId") == replyEventDescriptorVariant.toMap().value("eventTypeId")) {
                    found = true;
                    QVERIFY2(eventDescriptorVariant == replyEventDescriptorVariant, "Event descriptor doesn't match");
                }
            }
            QVERIFY2(found, "Missing event descriptor");
        }
    }

    QVariantList replyActions = rule.value("actions").toList();
    QVERIFY2(actions == replyActions,
             QString("Actions don't match.\nExpected: %1\nGot: %2")
             .arg(QString(QJsonDocument::fromVariant(actions).toJson()))
             .arg(QString(QJsonDocument::fromVariant(replyActions).toJson()))
             .toUtf8());

    QVariantList replyExitActions = rule.value("exitActions").toList();
    QVERIFY2(exitActions == replyExitActions, "ExitActions don't match");

    params.clear();
    params.insert("ruleId", newRuleId);
    response = injectAndWait("Rules.RemoveRule", params);
    verifyRuleError(response);

    response = injectAndWait("Rules.GetRules");
    rules = response.toMap().value("params").toMap().value("ruleDescriptions").toList();
    QVERIFY2(rules.count() == 0, "There should be no rules.");
}

void TestRules::editRules_data()
{
    // RuleAction
    QVariantMap validActionNoParams;
    validActionNoParams.insert("actionTypeId", mockWithoutParamsActionTypeId);
    validActionNoParams.insert("thingId", m_mockThingId);
    validActionNoParams.insert("deviceId", m_mockThingId); // DEPRECATED

    QVariantMap invalidAction;
    invalidAction.insert("actionTypeId", ActionTypeId());
    invalidAction.insert("thingId", m_mockThingId);

    // RuleExitAction
    QVariantMap validExitActionNoParams;
    validExitActionNoParams.insert("actionTypeId", mockWithoutParamsActionTypeId);
    validExitActionNoParams.insert("thingId", m_mockThingId);
    validExitActionNoParams.insert("deviceId", m_mockThingId); // DEPRECATED

    QVariantMap invalidExitAction;
    invalidExitAction.insert("actionTypeId", ActionTypeId());
    invalidExitAction.insert("thingId", m_mockThingId);

    // StateDescriptor
    QVariantMap stateDescriptor;
    stateDescriptor.insert("stateTypeId", mockIntStateTypeId);
    stateDescriptor.insert("thingId", m_mockThingId);
    stateDescriptor.insert("operator", enumValueName(Types::ValueOperatorLess));
    stateDescriptor.insert("value", 20);

    // StateEvaluator
    QVariantMap validStateEvaluator;
    validStateEvaluator.insert("stateDescriptor", stateDescriptor);
    validStateEvaluator.insert("operator", enumValueName(Types::StateOperatorAnd));

    QVariantMap invalidStateEvaluator;
    stateDescriptor.remove("thingId");
    invalidStateEvaluator.insert("stateDescriptor", stateDescriptor);

    // EventDescriptor
    QVariantMap validEventDescriptor1;
    validEventDescriptor1.insert("eventTypeId", mockEvent1EventTypeId);
    validEventDescriptor1.insert("thingId", m_mockThingId);
    validEventDescriptor1.insert("deviceId", m_mockThingId); // DEPRECATED

    QVariantMap validEventDescriptor2;
    validEventDescriptor2.insert("eventTypeId", mockEvent2EventTypeId);
    validEventDescriptor2.insert("thingId", m_mockThingId);
    validEventDescriptor2.insert("deviceId", m_mockThingId); // DEPRECATED
    QVariantList params;
    QVariantMap param1;
    param1.insert("paramTypeId", mockEvent2EventIntParamParamTypeId);
    param1.insert("value", 3);
    param1.insert("operator", enumValueName(Types::ValueOperatorEquals));
    params.append(param1);
    validEventDescriptor2.insert("paramDescriptors", params);

    QVariantMap validEventDescriptor3;
    validEventDescriptor3.insert("eventTypeId", mockEvent2EventTypeId);
    validEventDescriptor3.insert("thingId", m_mockThingId);
    validEventDescriptor3.insert("deviceId", m_mockThingId); // DEPRECATED

    // EventDescriptorList
    QVariantList eventDescriptorList;
    eventDescriptorList.append(validEventDescriptor1);
    eventDescriptorList.append(validEventDescriptor2);

    QVariantMap invalidEventDescriptor;
    invalidEventDescriptor.insert("eventTypeId", mockEvent1EventTypeId);
    invalidEventDescriptor.insert("thingId", ThingId());

    // RuleAction event based
    QVariantMap validActionEventBased;
    validActionEventBased.insert("actionTypeId", mockWithParamsActionTypeId);
    validActionEventBased.insert("thingId", m_mockThingId);
    validActionEventBased.insert("deviceId", m_mockThingId); // DEPRECATED
    QVariantMap validActionEventBasedParam1;
    validActionEventBasedParam1.insert("paramTypeId", mockWithParamsActionParam1ParamTypeId);
    validActionEventBasedParam1.insert("eventTypeId", mockEvent2EventTypeId);
    validActionEventBasedParam1.insert("eventParamTypeId", mockEvent2EventIntParamParamTypeId);
    QVariantMap validActionEventBasedParam2;
    validActionEventBasedParam2.insert("paramTypeId", mockWithParamsActionParam2ParamTypeId);
    validActionEventBasedParam2.insert("value", false);
    validActionEventBased.insert("ruleActionParams", QVariantList() << validActionEventBasedParam1 << validActionEventBasedParam2);

    QVariantMap invalidActionEventBased;
    invalidActionEventBased.insert("actionTypeId", mockWithoutParamsActionTypeId);
    invalidActionEventBased.insert("thingId", m_mockThingId);
    validActionEventBasedParam1.insert("value", 10);
    invalidActionEventBased.insert("ruleActionParams", QVariantList() << validActionEventBasedParam1);

    QVariantMap invalidActionEventBased2;
    invalidActionEventBased2.insert("actionTypeId", mockWithParamsActionTypeId);
    invalidActionEventBased2.insert("thingId", m_mockThingId);
    QVariantMap invalidActionEventBasedParam2;
    invalidActionEventBasedParam2.insert("paramTypeId", mockWithParamsActionParam1ParamTypeId);
    invalidActionEventBasedParam2.insert("eventTypeId", mockEvent1EventTypeId);
    invalidActionEventBasedParam2.insert("eventParamTypeId", ParamTypeId("2c4825c8-dfb9-4ba4-bd0e-1d827d945d41"));
    QVariantMap invalidActionEventBasedParam3;
    invalidActionEventBasedParam3.insert("paramTypeId", mockWithParamsActionParam2ParamTypeId);
    invalidActionEventBasedParam3.insert("value", 2);
    invalidActionEventBased2.insert("ruleActionParams", QVariantList() << invalidActionEventBasedParam2 << invalidActionEventBasedParam3);

    QVariantMap invalidActionEventBased3;
    invalidActionEventBased3.insert("actionTypeId", mockWithParamsActionTypeId);
    invalidActionEventBased3.insert("thingId", m_mockThingId);
    QVariantMap invalidActionEventBasedParam4;
    invalidActionEventBasedParam4.insert("paramTypeId", mockWithParamsActionParam1ParamTypeId);
    invalidActionEventBasedParam4.insert("eventTypeId", mockEvent1EventTypeId);
    invalidActionEventBasedParam4.insert("eventParamTypeId", mockEvent2EventIntParamParamTypeId);
    invalidActionEventBased3.insert("ruleActionParams", QVariantList() << invalidActionEventBasedParam4);

    QTest::addColumn<bool>("enabled");
    QTest::addColumn<QVariantMap>("action");
    QTest::addColumn<QVariantMap>("exitAction");
    QTest::addColumn<QVariantMap>("eventDescriptor");
    QTest::addColumn<QVariantList>("eventDescriptorList");
    QTest::addColumn<QVariantMap>("stateEvaluator");
    QTest::addColumn<RuleEngine::RuleError>("error");
    QTest::addColumn<QString>("name");

    // Rules with event based actions
    QTest::newRow("valid rule. enabled, 1 Action (eventBased), 1 EventDescriptor, name")                << true     << validActionEventBased    << QVariantMap()            << validEventDescriptor3    << QVariantList()       << QVariantMap()            << RuleEngine::RuleErrorNoError << "ActionEventRule1";
    QTest::newRow("invalid rule. enabled, 1 Action (eventBased), 1 EventDescriptor, name")              << true     << invalidActionEventBased2 << QVariantMap()            << validEventDescriptor3    << QVariantList()       << QVariantMap()            << RuleEngine::RuleErrorInvalidRuleActionParameter  << "TestRule";

    QTest::newRow("invalid rule. enabled, 1 Action (eventBased), types not matching, name")             << true     << invalidActionEventBased3 << QVariantMap()            << validEventDescriptor1    << QVariantList()       << QVariantMap()            << RuleEngine::RuleErrorTypesNotMatching << "TestRule";

    QTest::newRow("invalid rule. enabled, 1 Action (eventBased), 1 EventDescriptor, name")              << true     << invalidActionEventBased  << QVariantMap()            << validEventDescriptor2    << QVariantList()       << QVariantMap()            << RuleEngine::RuleErrorTypesNotMatching << "TestRule";
    QTest::newRow("invalid rule. enabled, 1 Action (eventBased), 1 StateEvaluator, name")               << true     << validActionEventBased    << QVariantMap()            << QVariantMap()            << QVariantList()       << validStateEvaluator      << RuleEngine::RuleErrorInvalidRuleActionParameter << "TestRule";
    QTest::newRow("invalid rule. enabled, 1 Action (eventBased), 1 EventDescriptor, name")              << true     << validActionEventBased    << validActionEventBased    << validEventDescriptor2    << QVariantList()       << QVariantMap()            << RuleEngine::RuleErrorInvalidRuleFormat << "TestRule";
    QTest::newRow("invalid rule. enabled, 1 Action, 1 ExitAction (EventBased), name")                   << true     << validActionNoParams      << validActionEventBased    << validEventDescriptor2    << QVariantList()       << QVariantMap()            << RuleEngine::RuleErrorInvalidRuleFormat << "TestRule";

    // Rules with exit actions
    QTest::newRow("valid rule. enabled, 1 Action, 1 Exit Action,  1 StateEvaluator, name")              << true     << validActionNoParams      << validExitActionNoParams  << QVariantMap()            << QVariantList()       << validStateEvaluator      << RuleEngine::RuleErrorNoError << "TestRule";
    QTest::newRow("valid rule. disabled, 1 Action, 1 Exit Action, 1 StateEvaluator, name")              << false    << validActionNoParams      << validExitActionNoParams  << QVariantMap()            << QVariantList()       << validStateEvaluator      << RuleEngine::RuleErrorNoError << "TestRule";
    QTest::newRow("valid rule. 1 Action, 1 Exit Action, 1 EventDescriptor, 1 StateEvaluator, name")     << true     << validActionNoParams      << validExitActionNoParams  << validEventDescriptor1    << QVariantList()       << validStateEvaluator      << RuleEngine::RuleErrorNoError << "TestRule";
    QTest::newRow("valid rule. 1 Action, 1 Exit Action, eventDescriptorList, 1 StateEvaluator, name")   << true     << validActionNoParams      << validExitActionNoParams  << QVariantMap()            << eventDescriptorList  << validStateEvaluator      << RuleEngine::RuleErrorNoError << "TestRule";

    // Rules without exit actions
    QTest::newRow("valid rule. enabled, 1 EventDescriptor, StateEvaluator, 1 Action, name")             << true     << validActionNoParams      << QVariantMap()            << validEventDescriptor1    << QVariantList()       << validStateEvaluator      << RuleEngine::RuleErrorNoError << "TestRule";
    QTest::newRow("valid rule. disabled, 1 EventDescriptor, StateEvaluator, 1 Action, name")             << false    << validActionNoParams      << QVariantMap()            << validEventDescriptor1    << QVariantList()       << validStateEvaluator      << RuleEngine::RuleErrorNoError << "TestRule";
    QTest::newRow("valid rule. 2 EventDescriptors, 1 Action, name")                                     << true     << validActionNoParams      << QVariantMap()            << QVariantMap()            << eventDescriptorList  << validStateEvaluator      << RuleEngine::RuleErrorNoError << "TestRule";
}

void TestRules::editRules()
{
    QFETCH(bool, enabled);
    QFETCH(QVariantMap, action);
    QFETCH(QVariantMap, exitAction);
    QFETCH(QVariantMap, eventDescriptor);
    QFETCH(QVariantList, eventDescriptorList);
    QFETCH(QVariantMap, stateEvaluator);
    QFETCH(RuleEngine::RuleError, error);
    QFETCH(QString, name);

    // Add the rule we want to edit
    QVariantList eventParamDescriptors;
    QVariantMap eventDescriptor1;
    eventDescriptor1.insert("eventTypeId", mockEvent1EventTypeId);
    eventDescriptor1.insert("thingId", m_mockThingId);
    eventDescriptor1.insert("deviceId", m_mockThingId); // DEPRECATED
    QVariantMap eventDescriptor2;
    eventDescriptor2.insert("eventTypeId", mockEvent2EventTypeId);
    eventDescriptor2.insert("thingId", m_mockThingId);
    eventDescriptor2.insert("deviceId", m_mockThingId); // DEPRECATED
    QVariantMap eventParam1;
    eventParam1.insert("paramTypeId", mockEvent2EventIntParamParamTypeId);
    eventParam1.insert("value", 3);
    eventParam1.insert("operator", enumValueName(Types::ValueOperatorEquals));
    eventParamDescriptors.append(eventParam1);
    eventDescriptor2.insert("paramDescriptors", eventParamDescriptors);

    QVariantList eventDescriptorList1;
    eventDescriptorList1.append(eventDescriptor1);
    eventDescriptorList1.append(eventDescriptor2);

    QVariantMap stateEvaluator0;
    QVariantMap stateDescriptor1;
    stateDescriptor1.insert("thingId", m_mockThingId);
    stateDescriptor1.insert("operator", enumValueName(Types::ValueOperatorEquals));
    stateDescriptor1.insert("stateTypeId", mockIntStateTypeId);
    stateDescriptor1.insert("value", 1);
    QVariantMap stateDescriptor2;
    stateDescriptor2.insert("thingId", m_mockThingId);
    stateDescriptor2.insert("operator", enumValueName(Types::ValueOperatorEquals));
    stateDescriptor2.insert("stateTypeId", mockBoolStateTypeId);
    stateDescriptor2.insert("value", true);
    QVariantMap stateEvaluator1;
    stateEvaluator1.insert("stateDescriptor", stateDescriptor1);
    stateEvaluator1.insert("operator", enumValueName(Types::StateOperatorAnd));
    QVariantMap stateEvaluator2;
    stateEvaluator2.insert("stateDescriptor", stateDescriptor2);
    stateEvaluator2.insert("operator", enumValueName(Types::StateOperatorAnd));
    QVariantList childEvaluators;
    childEvaluators.append(stateEvaluator1);
    childEvaluators.append(stateEvaluator2);
    stateEvaluator0.insert("childEvaluators", childEvaluators);
    stateEvaluator0.insert("operator", enumValueName(Types::StateOperatorAnd));

    QVariantMap action1;
    action1.insert("actionTypeId", mockWithoutParamsActionTypeId);
    action1.insert("thingId", m_mockThingId);
    action1.insert("ruleActionParams", QVariantList());
    QVariantMap action2;
    action2.insert("actionTypeId", mockWithParamsActionTypeId);
    action2.insert("thingId", m_mockThingId);
    QVariantList action2Params;
    QVariantMap action2Param1;
    action2Param1.insert("paramTypeId", mockWithParamsActionParam1ParamTypeId);
    action2Param1.insert("value", 5);
    action2Params.append(action2Param1);
    QVariantMap action2Param2;
    action2Param2.insert("paramTypeId", mockWithParamsActionParam2ParamTypeId);
    action2Param2.insert("value", true);
    action2Params.append(action2Param2);
    action2.insert("ruleActionParams", action2Params);

    // RuleAction event based
    QVariantMap validActionEventBased;
    validActionEventBased.insert("actionTypeId", mockWithParamsActionTypeId);
    validActionEventBased.insert("thingId", m_mockThingId);
    validActionEventBased.insert("deviceId", m_mockThingId); // DEPRECATED
    QVariantMap validActionEventBasedParam1;
    validActionEventBasedParam1.insert("paramTypeId", mockWithParamsActionParam1ParamTypeId);
    validActionEventBasedParam1.insert("eventTypeId", mockEvent2EventTypeId);
    validActionEventBasedParam1.insert("eventParamTypeId", mockEvent2EventIntParamParamTypeId);
    QVariantMap validActionEventBasedParam2;
    validActionEventBasedParam2.insert("paramTypeId", mockWithParamsActionParam2ParamTypeId);
    validActionEventBasedParam2.insert("value", false);
    validActionEventBased.insert("ruleActionParams", QVariantList() << validActionEventBasedParam1 << validActionEventBasedParam2);

    QVariantList validEventDescriptors3;
    QVariantMap validEventDescriptor3;
    validEventDescriptor3.insert("eventTypeId", mockEvent2EventTypeId);
    validEventDescriptor3.insert("thingId", m_mockThingId);
    validEventDescriptor3.insert("deviceId", m_mockThingId); // DEPRECATED
    validEventDescriptor3.insert("paramDescriptors", QVariantList());
    validEventDescriptors3.append(validEventDescriptor3);

    QVariantMap params;
    QVariantList actions;
    actions.append(action1);
    actions.append(action2);
    params.insert("actions", actions);
    params.insert("eventDescriptors", eventDescriptorList1);
    params.insert("stateEvaluator", stateEvaluator0);
    params.insert("name", "TestRule");
    QVariant response = injectAndWait("Rules.AddRule", params);

    RuleId ruleId = RuleId(response.toMap().value("params").toMap().value("ruleId").toString());
    verifyRuleError(response);

    // enable notifications
    enableNotifications({"Rules"});

    // now create the new rule and edit the original one
    params.clear();
    params.insert("ruleId", ruleId.toString());
    params.insert("name", name);

    if (!eventDescriptor.isEmpty()) {
        params.insert("eventDescriptors", QVariantList() << eventDescriptor);
    }
    if (!eventDescriptorList.isEmpty()) {
        params.insert("eventDescriptors", eventDescriptorList);
    }
    actions.clear();
    actions.append(action);
    params.insert("actions", actions);

    QVariantList exitActions;
    if (!exitAction.isEmpty()) {
        exitActions.append(exitAction);
        params.insert("exitActions", exitActions);
    }
    params.insert("stateEvaluator", stateEvaluator);
    if (!enabled) {
        params.insert("enabled", enabled);
    }

    // Setup connection to mock client
    QSignalSpy clientSpy(m_mockTcpServer, SIGNAL(outgoingData(QUuid,QByteArray)));
    response.clear();
    response = injectAndWait("Rules.EditRule", params);
    verifyRuleError(response, error);
    if (error == RuleEngine::RuleErrorNoError){
        clientSpy.wait(1);
        // We need to get exactly 2 replies. The actual reply and the Changed notification
        // Make sure there are no other notifications (e.g. RuleAdded or similar)
        QCOMPARE(clientSpy.count(), 2);
        QVariant notification = checkNotification(clientSpy, "Rules.RuleConfigurationChanged");
        QVERIFY2(notification != QVariant(), "not received \"Rules.RuleConfigurationChanged\" notification");

        // now check if the received rule matches the our new rule
        QVariantMap rule = response.toMap().value("params").toMap().value("rule").toMap();

        QVERIFY2(rule.value("enabled").toBool() == enabled, "Rule enabled state doesn't match");
        QVariantList eventDescriptors = rule.value("eventDescriptors").toList();
        if (!eventDescriptor.isEmpty()) {
            QVERIFY2(eventDescriptors.count() == 1, "There should be exactly one eventDescriptor");
            QVariantMap stringifiedEventDescriptor = QJsonDocument::fromVariant(eventDescriptor).toVariant().toMap();
            QVERIFY2(eventDescriptors.first().toMap() == stringifiedEventDescriptor,
                     QString("Event descriptor doesn't match.\nExpected:%1\nGot:%2")
                     .arg(QString(QJsonDocument::fromVariant(eventDescriptor).toJson()))
                     .arg(QString(QJsonDocument::fromVariant(eventDescriptors.first().toMap()).toJson())).toUtf8());
        } else if (eventDescriptorList.isEmpty()){
            QVERIFY2(eventDescriptors.count() == eventDescriptorList.count(), QString("There should be exactly %1 eventDescriptor").arg(eventDescriptorList.count()).toLatin1().data());
            foreach (const QVariant &eventDescriptorVariant, eventDescriptorList) {
                bool found = false;
                foreach (const QVariant &replyEventDescriptorVariant, eventDescriptors) {
                    if (eventDescriptorVariant.toMap().value("deviceId") == replyEventDescriptorVariant.toMap().value("deviceId") &&
                            eventDescriptorVariant.toMap().value("eventTypeId") == replyEventDescriptorVariant.toMap().value("eventTypeId")) {
                        found = true;
                        QVERIFY2(eventDescriptorVariant == replyEventDescriptorVariant, "Event descriptor doesn't match");
                    }
                }
                QVERIFY2(found, "Missing event descriptor");
            }
        }

        QVariantList replyActions = rule.value("actions").toList();
        QVERIFY2(actions == replyActions,
                 QString("Actions don't match.\nExpected: %1\nGot: %2")
                 .arg(QString(QJsonDocument::fromVariant(actions).toJson()))
                 .arg(QString(QJsonDocument::fromVariant(replyActions).toJson()))
                 .toUtf8());

        QVariantList replyExitActions = rule.value("exitActions").toList();
        QVERIFY2(exitActions == replyExitActions,
                 QString("Actions don't match.\nExpected: %1\nGot: %2")
                 .arg(QString(QJsonDocument::fromVariant(exitActions).toJson()))
                 .arg(QString(QJsonDocument::fromVariant(replyExitActions).toJson()))
                 .toUtf8());
    }

    // Remove the rule
    params.clear();
    params.insert("ruleId", ruleId);
    response = injectAndWait("Rules.RemoveRule", params);
    verifyRuleError(response);

    // check if removed
    response = injectAndWait("Rules.GetRules");
    QVariantList rules = response.toMap().value("params").toMap().value("rules").toList();
    QVERIFY2(rules.count() == 0, "There should be no rules.");
}

void TestRules::executeRuleActions_data()
{
    QTest::addColumn<QVariantMap>("params");
    QTest::addColumn<RuleEngine::RuleError>("ruleError");

    QTest::newRow("executable rule, enabled") << validIntStateBasedRule("Executeable", true, true).toMap() << RuleEngine::RuleErrorNoError;
    QTest::newRow("executable rule, disabled") << validIntStateBasedRule("Executeable", true, false).toMap() << RuleEngine::RuleErrorNoError;
    QTest::newRow("not executable rule, enabled") << validIntStateBasedRule("Not Executable", false, true).toMap() << RuleEngine::RuleErrorNotExecutable;
    QTest::newRow("not executable rule, disabled") << validIntStateBasedRule("Not Executable", false, false).toMap() << RuleEngine::RuleErrorNotExecutable;
}

void TestRules::executeRuleActions()
{
    QFETCH(QVariantMap, params);
    QFETCH(RuleEngine::RuleError, ruleError);

    // ADD rule
    QVariant response = injectAndWait("Rules.AddRule", params);
    verifyRuleError(response);

    RuleId ruleId = RuleId(response.toMap().value("params").toMap().value("ruleId").toString());
    QVERIFY(!ruleId.isNull());

    cleanupMockHistory();
    QTest::qWait(200);

    // EEXCUTE action invalid ruleId
    QVariantMap executeParams;
    executeParams.insert("ruleId", QUuid::createUuid().toString());
    response = injectAndWait("Rules.ExecuteActions", executeParams);
    verifyRuleError(response, RuleEngine::RuleErrorRuleNotFound);

    // EXECUTE actions
    qDebug() << "Execute rule actions";
    executeParams.clear();
    executeParams.insert("ruleId", ruleId.toString());
    response = injectAndWait("Rules.ExecuteActions", executeParams);
    verifyRuleError(response, ruleError);

    // give the ruleeingine time to execute the actions
    QTest::qWait(1000);

    if (ruleError == RuleEngine::RuleErrorNoError) {
        verifyRuleExecuted(mockWithParamsActionTypeId);
    } else {
        verifyRuleNotExecuted();
    }

    cleanupMockHistory();
    QTest::qWait(200);

    // EXECUTE exit actions invalid ruleId
    executeParams.clear();
    executeParams.insert("ruleId", QUuid::createUuid().toString());
    response = injectAndWait("Rules.ExecuteExitActions", executeParams);
    verifyRuleError(response, RuleEngine::RuleErrorRuleNotFound);

    // EXECUTE exit actions
    qDebug() << "Execute rule exit actions";
    executeParams.clear();
    executeParams.insert("ruleId", ruleId.toString());
    response = injectAndWait("Rules.ExecuteExitActions", executeParams);
    verifyRuleError(response, ruleError);

    // give the ruleeingine time to execute the actions
    QTest::qWait(1000);

    if (ruleError == RuleEngine::RuleErrorNoError) {
        verifyRuleExecuted(mockWithoutParamsActionTypeId);
    } else {
        verifyRuleNotExecuted();
    }

    cleanupMockHistory();

    // REMOVE rule
    QVariantMap removeParams;
    removeParams.insert("ruleId", ruleId);
    response = injectAndWait("Rules.RemoveRule", removeParams);
    verifyRuleError(response);
}

void TestRules::findRule()
{
    // ADD rule
    QVariantMap params = validIntStateBasedRule("Executeable", true, true).toMap();
    QVariant response = injectAndWait("Rules.AddRule", params);
    verifyRuleError(response);

    RuleId ruleId = RuleId(response.toMap().value("params").toMap().value("ruleId").toString());
    QVERIFY(!ruleId.isNull());

    params.clear();
    params.insert("thingId", m_mockThingId);
    response = injectAndWait("Rules.FindRules", params);

    QCOMPARE(response.toMap().value("params").toMap().value("ruleIds").toList().count(), 1);
    QCOMPARE(response.toMap().value("params").toMap().value("ruleIds").toList().first().toUuid().toString(), ruleId.toString());

    // REMOVE rule
    QVariantMap removeParams;
    removeParams.insert("ruleId", ruleId);
    response = injectAndWait("Rules.RemoveRule", removeParams);
    verifyRuleError(response);

}

void TestRules::removeInvalidRule()
{
    QVariantMap params;
    params.insert("ruleId", RuleId::createRuleId());
    QVariant response = injectAndWait("Rules.RemoveRule", params);
    verifyRuleError(response, RuleEngine::RuleErrorRuleNotFound);
}

void TestRules::loadStoreConfig()
{
    QVariantMap eventDescriptor1;
    eventDescriptor1.insert("eventTypeId", mockEvent1EventTypeId);
    eventDescriptor1.insert("thingId", m_mockThingId);
    eventDescriptor1.insert("deviceId", m_mockThingId); // DEPRECATED

    QVariantMap eventDescriptor2;
    eventDescriptor2.insert("eventTypeId", mockEvent2EventTypeId);
    eventDescriptor2.insert("thingId", m_mockThingId);
    eventDescriptor2.insert("deviceId", m_mockThingId); // DEPRECATED
    QVariantList eventParamDescriptors;
    QVariantMap eventParam1;
    eventParam1.insert("paramTypeId", mockEvent2EventIntParamParamTypeId);
    eventParam1.insert("value", 3);
    eventParam1.insert("operator", enumValueName(Types::ValueOperatorEquals));
    eventParamDescriptors.append(eventParam1);
    eventDescriptor2.insert("paramDescriptors", eventParamDescriptors);

    QVariantList eventDescriptorList;
    eventDescriptorList.append(eventDescriptor1);
    eventDescriptorList.append(eventDescriptor2);

    QVariantMap stateEvaluator1;
    QVariantList childEvaluators;

    QVariantMap stateDescriptor2;
    stateDescriptor2.insert("thingId", m_mockThingId);
    stateDescriptor2.insert("deviceId", m_mockThingId); // DEPRECATED
    stateDescriptor2.insert("operator", enumValueName(Types::ValueOperatorEquals));
    stateDescriptor2.insert("stateTypeId", mockIntStateTypeId);
    stateDescriptor2.insert("value", 1);
    QVariantMap stateEvaluator2;
    stateEvaluator2.insert("stateDescriptor", stateDescriptor2);
    stateEvaluator2.insert("operator", enumValueName(Types::StateOperatorAnd));

    QVariantMap stateDescriptor3;
    stateDescriptor3.insert("thingId", m_mockThingId);
    stateDescriptor3.insert("deviceId", m_mockThingId); // DEPRECATED
    stateDescriptor3.insert("operator", enumValueName(Types::ValueOperatorEquals));
    stateDescriptor3.insert("stateTypeId", mockBoolStateTypeId);
    stateDescriptor3.insert("value", true);

    QVariantMap stateEvaluator3;
    stateEvaluator3.insert("stateDescriptor", stateDescriptor3);
    stateEvaluator3.insert("operator", enumValueName(Types::StateOperatorAnd));

    QVariantMap stateDescriptor4;
    stateDescriptor4.insert("interface", "battery");
    stateDescriptor4.insert("interfaceState", "batteryCritical");
    stateDescriptor4.insert("operator", enumValueName(Types::ValueOperatorEquals));
    stateDescriptor4.insert("value", true);

    QVariantMap stateEvaluator4;
    stateEvaluator4.insert("stateDescriptor", stateDescriptor4);
    stateEvaluator4.insert("operator", enumValueName(Types::StateOperatorAnd));

    childEvaluators.append(stateEvaluator2);
    childEvaluators.append(stateEvaluator3);
    childEvaluators.append(stateEvaluator4);
    stateEvaluator1.insert("childEvaluators", childEvaluators);
    stateEvaluator1.insert("operator", enumValueName(Types::StateOperatorAnd));

    QVariantMap action1;
    action1.insert("actionTypeId", mockWithoutParamsActionTypeId);
    action1.insert("thingId", m_mockThingId);
    action1.insert("deviceId", m_mockThingId); // DEPRECATED
    action1.insert("ruleActionParams", QVariantList());

    QVariantMap action2;
    action2.insert("actionTypeId", mockWithParamsActionTypeId);
    action2.insert("thingId", m_mockThingId);
    action2.insert("deviceId", m_mockThingId); // DEPRECATED
    QVariantList action2Params;
    QVariantMap action2Param1;
    action2Param1.insert("paramTypeId", mockWithParamsActionParam1ParamTypeId);
    action2Param1.insert("value", 5);
    action2Params.append(action2Param1);
    QVariantMap action2Param2;
    action2Param2.insert("paramTypeId", mockWithParamsActionParam2ParamTypeId);
    action2Param2.insert("value", true);
    action2Params.append(action2Param2);
    action2.insert("ruleActionParams", action2Params);

    // RuleAction event based
    QVariantMap validActionEventBased;
    validActionEventBased.insert("actionTypeId", mockWithParamsActionTypeId);
    validActionEventBased.insert("thingId", m_mockThingId);
    validActionEventBased.insert("deviceId", m_mockThingId); // DEPRECATED
    QVariantMap validActionEventBasedParam1;
    validActionEventBasedParam1.insert("paramTypeId", mockWithParamsActionParam1ParamTypeId);
    validActionEventBasedParam1.insert("eventTypeId", mockEvent2EventTypeId);
    validActionEventBasedParam1.insert("eventParamTypeId", mockEvent2EventIntParamParamTypeId);
    QVariantMap validActionEventBasedParam2;
    validActionEventBasedParam2.insert("paramTypeId", mockWithParamsActionParam2ParamTypeId);
    validActionEventBasedParam2.insert("value", false);
    validActionEventBased.insert("ruleActionParams", QVariantList() << validActionEventBasedParam1 << validActionEventBasedParam2);

    QVariantList validEventDescriptors3;
    QVariantMap validEventDescriptor3;
    validEventDescriptor3.insert("eventTypeId", mockEvent2EventTypeId);
    validEventDescriptor3.insert("thingId", m_mockThingId);
    validEventDescriptor3.insert("deviceId", m_mockThingId); // DEPRECATED
    validEventDescriptors3.append(validEventDescriptor3);

    // Interface based event descriptor
    QVariantMap eventDescriptorInterfaces;
    eventDescriptorInterfaces.insert("interface", "battery");
    eventDescriptorInterfaces.insert("interfaceEvent", "batteryCritical");
    QVariantMap eventDescriptorInterfacesParam;
    eventDescriptorInterfacesParam.insert("paramName", "batteryCritical");
    eventDescriptorInterfacesParam.insert("value", true);
    eventDescriptorInterfacesParam.insert("operator", "ValueOperatorEquals");
    QVariantList eventDescriptorInterfacesParams;
    eventDescriptorInterfacesParams.append(eventDescriptorInterfacesParam);
    eventDescriptorInterfaces.insert("paramDescriptors", eventDescriptorInterfacesParams);
    QVariantList eventDescriptorsInterfaces;
    eventDescriptorsInterfaces.append(eventDescriptorInterfaces);

    // Interface based state evaluator
    QVariantMap stateDescriptorInterfaces;
    stateDescriptorInterfaces.insert("interface", "battery");
    stateDescriptorInterfaces.insert("interfaceState", "batteryCritical");
    stateDescriptorInterfaces.insert("operator", "ValueOperatorEquals");
    stateDescriptorInterfaces.insert("value", true);
    QVariantMap stateEvaluatorInterfaces;
    stateEvaluatorInterfaces.insert("stateDescriptor", stateDescriptorInterfaces);
    stateEvaluatorInterfaces.insert("operator", "StateOperatorAnd");

    // Interface based actions
    QVariantMap ruleActionParamInterfaces;
    ruleActionParamInterfaces.insert("paramName", "power");
    ruleActionParamInterfaces.insert("value", true);
    QVariantList ruleActionParamsInterfaces;
    ruleActionParamsInterfaces.append(ruleActionParamInterfaces);
    QVariantMap actionInterfaces;
    actionInterfaces.insert("interface", "light");
    actionInterfaces.insert("interfaceAction", "power");
    actionInterfaces.insert("ruleActionParams", ruleActionParamsInterfaces);

    QVariantList actionsInterfaces;
    actionsInterfaces.append(actionInterfaces);

    // rule 1
    qCDebug(dcTests()) << "Adding rule 1";
    QVariantMap params;
    QVariantList actions;
    actions.append(action1);
    actions.append(action2);
    params.insert("actions", actions);
    params.insert("eventDescriptors", eventDescriptorList);
    params.insert("stateEvaluator", stateEvaluator1);
    params.insert("name", "TestRule");
    QVariant response = injectAndWait("Rules.AddRule", params);

    RuleId newRuleId = RuleId(response.toMap().value("params").toMap().value("ruleId").toString());
    verifyRuleError(response);

    // rule 2
    qCDebug(dcTests()) << "Adding rule 2";
    QVariantMap params2;
    QVariantList actions2;
    actions2.append(action1);
    QVariantList exitActions2;
    exitActions2.append(action2);
    params2.insert("actions", actions2);
    params2.insert("exitActions", exitActions2);
    params2.insert("stateEvaluator", stateEvaluator1);
    params2.insert("name", "TestRule2");
    QVariant response2 = injectAndWait("Rules.AddRule", params2);

    RuleId newRuleId2 = RuleId(response2.toMap().value("params").toMap().value("ruleId").toString());
    verifyRuleError(response2);

    // rule 3
    qCDebug(dcTests()) << "Adding rule 3";
    QVariantMap params3;
    QVariantList actions3;
    actions3.append(validActionEventBased);
    params3.insert("actions", actions3);
    params3.insert("eventDescriptors", validEventDescriptors3);
    params3.insert("name", "TestRule3");
    QVariant response3 = injectAndWait("Rules.AddRule", params3);

    RuleId newRuleId3 = RuleId(response3.toMap().value("params").toMap().value("ruleId").toString());
    verifyRuleError(response3);

    // rule 4, interface based
    qCDebug(dcTests()) << "Adding rule 4";
    QVariantMap params4;
    params4.insert("name", "TestRule4 - Interface based");
    params4.insert("eventDescriptors", eventDescriptorsInterfaces);
    params4.insert("stateEvaluator", stateEvaluatorInterfaces);
    params4.insert("actions", actionsInterfaces);
    QVariant response4 = injectAndWait("Rules.AddRule", params4);

    RuleId newRuleId4 = RuleId(response4.toMap().value("params").toMap().value("ruleId").toString());
    verifyRuleError(response4);

    qCDebug(dcTests()) << "Getting rules";
    response = injectAndWait("Rules.GetRules");
    QVariantList rules = response.toMap().value("params").toMap().value("ruleDescriptions").toList();
    qDebug() << "GetRules before server shutdown:" <<  response;

    qCDebug(dcTests()) << "Restarting server";
    restartServer();

    response = injectAndWait("Rules.GetRules");
    rules = response.toMap().value("params").toMap().value("ruleDescriptions").toList();

    QVERIFY2(rules.count() == 4, "There should be exactly four rule.");

    QStringList idList;
    foreach (const QVariant &ruleDescription, rules) {
        idList.append(ruleDescription.toMap().value("id").toString());
    }

    QVERIFY2(idList.contains(newRuleId.toString()), "Rule 1 should be in ruleIds list.");
    QVERIFY2(idList.contains(newRuleId2.toString()), "Rule 2 should be in ruleIds list.");
    QVERIFY2(idList.contains(newRuleId3.toString()), "Rule 3 should be in ruleIds list.");
    QVERIFY2(idList.contains(newRuleId4.toString()), "Rule 4 should be in ruleIds list.");

    // Rule 1
    params.clear();
    params.insert("ruleId", newRuleId);
    response.clear();
    response = injectAndWait("Rules.GetRuleDetails", params);

    QVariantMap rule1 = response.toMap().value("params").toMap().value("rule").toMap();

    QVariantList eventDescriptors = rule1.value("eventDescriptors").toList();
    QVERIFY2(eventDescriptors.count() == 2, "There should be exactly 2 eventDescriptors");
    foreach (const QVariant &expectedEventDescriptorVariant, eventDescriptorList) {
        bool found = false;
        foreach (const QVariant &replyEventDescriptorVariant, eventDescriptors) {
            if (expectedEventDescriptorVariant.toMap().value("eventTypeId") == replyEventDescriptorVariant.toMap().value("eventTypeId") &&
                    expectedEventDescriptorVariant.toMap().value("thingId") == replyEventDescriptorVariant.toMap().value("thingId")) {
                found = true;
                QVariantMap stringifiedExpectedEventDescriptorVariant = QJsonDocument::fromVariant(expectedEventDescriptorVariant).toVariant().toMap();
                QVERIFY2(replyEventDescriptorVariant == stringifiedExpectedEventDescriptorVariant,
                         QString("EventDescriptor doesn't match.\nExpected: %1\nGot: %2")
                         .arg(QString(QJsonDocument::fromVariant(expectedEventDescriptorVariant).toJson()))
                         .arg(QString(QJsonDocument::fromVariant(replyEventDescriptorVariant).toJson()))
                         .toUtf8());
            }
        }
        QVERIFY2(found, "missing eventdescriptor");
    }

    QVERIFY2(rule1.value("name").toString() == "TestRule", "Loaded wrong name for rule");
    QVariantMap replyStateEvaluator= rule1.value("stateEvaluator").toMap();
    QVariantList replyChildEvaluators = replyStateEvaluator.value("childEvaluators").toList();
    QCOMPARE(replyChildEvaluators.count(), 3);
    QVERIFY2(replyStateEvaluator.value("operator") == "StateOperatorAnd", "There should be the AND operator.");

    foreach (const QVariant &childEvaluator, replyChildEvaluators) {
        QVERIFY2(childEvaluator.toMap().contains("stateDescriptor"), "StateDescriptor missing in StateEvaluator");
        QVariantMap stateDescriptor = childEvaluator.toMap().value("stateDescriptor").toMap();
        if (stateDescriptor.contains("thingId") && stateDescriptor.contains("stateTypeId")) {
            QVERIFY2(stateDescriptor.value("thingId").toUuid() == m_mockThingId, "ThingId of stateDescriptor does not match");
            QVERIFY2(stateDescriptor.value("stateTypeId").toUuid() == mockIntStateTypeId || stateDescriptor.value("stateTypeId").toUuid() == mockBoolStateTypeId, "StateTypeId of stateDescriptor doesn't match");
        } else if (stateDescriptor.contains("interface") && stateDescriptor.contains("interfaceState")) {
            QVERIFY2(stateDescriptor.value("interface") == "battery", "Interface of stateDescriptor does not match");
            QVERIFY2(stateDescriptor.value("interfaceState") == "batteryCritical", "InterfaceState of stateDescriptor doesn't match");
        } else {
            QVERIFY2(false, "StateDescriptor must have either thingId/stateTypeId or interface/interfaceState.");
        }
    }

    QVariantList replyActions = rule1.value("actions").toList();
    foreach (const QVariant &actionVariant, actions) {
        bool found = false;
        foreach (const QVariant &replyActionVariant, replyActions) {
            if (actionVariant.toMap().value("actionTypeId") == replyActionVariant.toMap().value("actionTypeId") &&
                    actionVariant.toMap().value("thingId") == replyActionVariant.toMap().value("thingId")) {
                found = true;
                // Check rule action params
                QVariantList actionParams = actionVariant.toMap().value("ruleActionParams").toList();
                actionParams = QJsonDocument::fromVariant(actionParams).toVariant().toList();
                QVariantList replyActionParams = replyActionVariant.toMap().value("ruleActionParams").toList();
                QVERIFY2(actionParams.count() == replyActionParams.count(), "Not the same list size of action params");
                foreach (const QVariant &ruleParam, actionParams) {
                    QVERIFY2(replyActionParams.contains(ruleParam), QString("reply actions are missing param.\nExpected:%1\nGot:%2")
                             .arg(qUtf8Printable(QJsonDocument::fromVariant(ruleParam).toJson()))
                             .arg(qUtf8Printable(QJsonDocument::fromVariant(replyActionParams).toJson()))
                             .toUtf8());
                }
            }
        }
        QVERIFY2(found, "Action not found after loading from config.");
    }

    // Rule 2
    params.clear();
    params.insert("ruleId", newRuleId2);
    response.clear();
    response = injectAndWait("Rules.GetRuleDetails", params);

    QVariantMap rule2 = response.toMap().value("params").toMap().value("rule").toMap();

    QVERIFY2(rule2.value("name").toString() == "TestRule2", "Loaded wrong name for rule");
    QVariantMap replyStateEvaluator2= rule2.value("stateEvaluator").toMap();
    QVariantList replyChildEvaluators2 = replyStateEvaluator.value("childEvaluators").toList();
    QVERIFY2(replyStateEvaluator2.value("operator") == "StateOperatorAnd", "There should be the AND operator.");
    QCOMPARE(replyChildEvaluators2.count(), 3);

    foreach (const QVariant &childEvaluator, replyChildEvaluators2) {
        QVERIFY2(childEvaluator.toMap().contains("stateDescriptor"), "StateDescriptor missing in StateEvaluator");
        QVariantMap stateDescriptor = childEvaluator.toMap().value("stateDescriptor").toMap();
        if (stateDescriptor.contains("thingId") && stateDescriptor.contains("stateTypeId")) {
            QVERIFY2(stateDescriptor.value("thingId").toUuid() == m_mockThingId, "ThingId of stateDescriptor does not match");
            QVERIFY2(stateDescriptor.value("stateTypeId").toUuid() == mockIntStateTypeId || stateDescriptor.value("stateTypeId").toUuid() == mockBoolStateTypeId, "StateTypeId of stateDescriptor doesn't match");
        } else if (stateDescriptor.contains("interface") && stateDescriptor.contains("interfaceState")) {
            QVERIFY2(stateDescriptor.value("interface") == "battery", "Interface of stateDescriptor does not match");
            QVERIFY2(stateDescriptor.value("interfaceState") == "batteryCritical", "InterfaceState of stateDescriptor doesn't match");
        } else {
            QVERIFY2(false, "StateDescriptor must have either deviceId/stateTypeId or interface/interfaceState.");
        }
    }

    QVariantList replyActions2 = rule2.value("actions").toList();
    QVERIFY2(replyActions2.count() == 1, "Rule 2 should have exactly 1 action");
    foreach (const QVariant &actionVariant, actions2) {
        bool found = false;
        foreach (const QVariant &replyActionVariant, replyActions2) {
            if (actionVariant.toMap().value("actionTypeId") == replyActionVariant.toMap().value("actionTypeId") &&
                    actionVariant.toMap().value("thingId") == replyActionVariant.toMap().value("thingId")) {
                found = true;
                // Check rule action params
                QVariantList actionParams = actionVariant.toMap().value("ruleActionParams").toList();
                QVariantList replyActionParams = replyActionVariant.toMap().value("ruleActionParams").toList();
                QVERIFY2(actionParams.count() == replyActionParams.count(), "Not the same list size of action params");
                foreach (const QVariant &ruleParam, actionParams) {
                    QVERIFY(replyActionParams.contains(ruleParam));
                }
            }
        }
        QVERIFY2(found, "Action not found after loading from config.");
    }

    QVariantList replyExitActions2 = rule2.value("exitActions").toList();
    QVERIFY2(replyExitActions2.count() == 1, "Rule 2 should have exactly 1 exitAction");
    foreach (const QVariant &exitActionVariant, replyExitActions2) {
        bool found = false;
        foreach (const QVariant &replyActionVariant, replyExitActions2) {
            if (exitActionVariant.toMap().value("actionTypeId") == replyActionVariant.toMap().value("actionTypeId") &&
                    exitActionVariant.toMap().value("thingId") == replyActionVariant.toMap().value("thingId")) {
                found = true;
                // Check rule action params
                QVariantList actionParams = exitActionVariant.toMap().value("ruleActionParams").toList();
                QVariantList replyActionParams = replyActionVariant.toMap().value("ruleActionParams").toList();
                QVERIFY2(actionParams.count() == replyActionParams.count(), "Not the same list size of action params");
                foreach (const QVariant &ruleParam, actionParams) {
                    QVERIFY(replyActionParams.contains(ruleParam));
                }
            }
        }
        QVERIFY2(found, "Exit Action not found after loading from config.");
    }

    // Rule 3
    params.clear();
    params.insert("ruleId", newRuleId3);
    response.clear();
    response = injectAndWait("Rules.GetRuleDetails", params);

    QVariantMap rule3 = response.toMap().value("params").toMap().value("rule").toMap();

    qDebug() << rule3;

    QVariantList eventDescriptors3 = rule3.value("eventDescriptors").toList();
    QVERIFY2(eventDescriptors3.count() == 1, "There should be exactly 1 eventDescriptor");
    QVariantMap eventDescriptor = eventDescriptors3.first().toMap();
    QVERIFY2(eventDescriptor.value("eventTypeId").toUuid() == mockEvent2EventTypeId, "Loaded the wrong eventTypeId in rule 3");
    QVERIFY2(eventDescriptor.value("thingId").toUuid() == m_mockThingId, "Loaded the wrong deviceId from eventDescriptor in rule 3");

    QVariantList replyExitActions3 = rule3.value("exitActions").toList();
    QVERIFY2(replyExitActions3.isEmpty(), "Rule 3 should not have any exitAction");

    QVariantList replyActions3 = rule3.value("actions").toList();
    QVERIFY2(replyActions3.count() == 1, "Rule 3 should have exactly 1 action");
    foreach (const QVariant &actionVariant, actions3) {
        bool found = false;
        foreach (const QVariant &replyActionVariant, replyActions3) {
            if (actionVariant.toMap().value("actionTypeId") == replyActionVariant.toMap().value("actionTypeId") &&
                    actionVariant.toMap().value("thingId") == replyActionVariant.toMap().value("thingId")) {
                found = true;
                // Check rule action params
                QVariantList actionParams = actionVariant.toMap().value("ruleActionParams").toList();
                actionParams = QJsonDocument::fromVariant(actionParams).toVariant().toList();
                QVariantList replyActionParams = replyActionVariant.toMap().value("ruleActionParams").toList();
                QVERIFY2(actionParams.count() == replyActionParams.count(), "Not the same list size of action params");
                foreach (const QVariant &ruleParam, actionParams) {
                    QVERIFY(replyActionParams.contains(ruleParam));
                }
            }
        }
        QVERIFY2(found, "Action not found after loading from config.");
    }

    // Rule 4
    params.clear();
    params.insert("ruleId", newRuleId4);
    response.clear();
    response = injectAndWait("Rules.GetRuleDetails", params);

    QVariantMap rule4 = response.toMap().value("params").toMap().value("rule").toMap();

    qDebug() << rule4;

    QVariantList eventDescriptors4 = rule4.value("eventDescriptors").toList();
    QVERIFY2(eventDescriptors4.count() == 1, "There should be exactly 1 eventDescriptor");
    eventDescriptor = eventDescriptors4.first().toMap();
    QVERIFY2(eventDescriptor.value("interface").toString() == "battery", "Loaded the wrong interface name in rule 4");
    QVERIFY2(eventDescriptor.value("interfaceEvent").toString() == "batteryCritical", "Loaded the wrong interfaceEvent from eventDescriptor in rule 4");
    QCOMPARE(eventDescriptor.value("paramDescriptors").toList().count(), 1);
    QVERIFY2(eventDescriptor.value("paramDescriptors").toList().first().toMap().value("paramName").toString() == "batteryCritical", "Loaded wrong ParamDescriptor in rule 4");
    QVERIFY2(eventDescriptor.value("paramDescriptors").toList().first().toMap().value("value").toBool() == true, "Loaded wrong ParamDescriptor in rule 3");

    QVariantList replyActions4 = rule4.value("actions").toList();
    QVERIFY2(replyActions4.count() == 1, "Rule 4 should have exactly 1 action");
    foreach (const QVariant &actionVariant, actionsInterfaces) {
        bool found = false;
        foreach (const QVariant &replyActionVariant, replyActions4) {
            if (actionVariant.toMap().value("interface") == replyActionVariant.toMap().value("interface") &&
                    actionVariant.toMap().value("interfaceAction") == replyActionVariant.toMap().value("interfaceAction")) {
                found = true;
                // Check rule action params
                QVariantList actionParams = actionVariant.toMap().value("ruleActionParams").toList();
                QVariantList replyActionParams = replyActionVariant.toMap().value("ruleActionParams").toList();
                QVERIFY2(actionParams.count() == replyActionParams.count(), "Not the same list size of action params");
                foreach (const QVariant &ruleParam, actionParams) {
                    QVERIFY(replyActionParams.contains(ruleParam));
                }
            }
        }
        QVERIFY2(found, "Action not found after loading from config.");
    }
    QVariantList replyExitActions4 = rule4.value("exitActions").toList();
    QVERIFY2(replyExitActions4.isEmpty(), "Rule 4 should not have any exitAction");


    // Remove Rule1
    params.clear();
    params.insert("ruleId", newRuleId);
    response = injectAndWait("Rules.RemoveRule", params);
    verifyRuleError(response);

    // Remove Rule2
    params2.clear();
    params2.insert("ruleId", newRuleId2);
    response = injectAndWait("Rules.RemoveRule", params2);
    verifyRuleError(response);

    // Remove Rule2
    params3.clear();
    params3.insert("ruleId", newRuleId3);
    response = injectAndWait("Rules.RemoveRule", params3);
    verifyRuleError(response);

    restartServer();

    response = injectAndWait("Rules.GetRules");
    rules = response.toMap().value("params").toMap().value("rules").toList();
    QVERIFY2(rules.count() == 0, "There should be no rules.");
}

void TestRules::evaluateEvent()
{
    // Add a rule
    QVariantMap addRuleParams;
    addRuleParams.insert("name", "TestRule");

    QVariantList events;
    QVariantMap event1;
    event1.insert("eventTypeId", mockEvent1EventTypeId);
    event1.insert("thingId", m_mockThingId);
    events.append(event1);
    addRuleParams.insert("eventDescriptors", events);

    QVariantList actions;
    QVariantMap action;
    action.insert("actionTypeId", mockWithoutParamsActionTypeId);
    action.insert("thingId", m_mockThingId);
    actions.append(action);
    addRuleParams.insert("actions", actions);
    QVariant response = injectAndWait("Rules.AddRule", addRuleParams);
    verifyRuleError(response);

    // Trigger an event
    QNetworkAccessManager nam;
    QSignalSpy spy(&nam, SIGNAL(finished(QNetworkReply*)));

    // trigger event in mock
    QNetworkRequest request(QUrl(QString("http://localhost:%1/generateevent?eventtypeid=%2").arg(m_mockThing1Port).arg(mockEvent1EventTypeId.toString())));
    QNetworkReply *reply = nam.get(request);
    spy.wait();
    QCOMPARE(spy.count(), 1);
    reply->deleteLater();

    verifyRuleExecuted(mockWithoutParamsActionTypeId);
}

void TestRules::evaluateEventParams()
{
    // Init bool state to true
    QNetworkAccessManager nam;
    QSignalSpy spy(&nam, SIGNAL(finished(QNetworkReply*)));
    QNetworkRequest request(QUrl(QString("http://localhost:%1/setstate?%2=%3").arg(m_mockThing1Port).arg(mockBoolStateTypeId.toString()).arg("true")));
    QNetworkReply *reply = nam.get(request);
    spy.wait();
    QCOMPARE(spy.count(), 1);
    reply->deleteLater();


    // Add a rule
    QVariantMap addRuleParams;
    addRuleParams.insert("name", "TestRule");

    QVariantList params;
    QVariantMap boolParam;
    boolParam.insert("paramTypeId", mockBoolStateTypeId);
    boolParam.insert("operator", "ValueOperatorEquals");
    boolParam.insert("value", true);
    params.append(boolParam);

    QVariantMap event1;
    event1.insert("eventTypeId", mockBoolStateTypeId);
    event1.insert("thingId", m_mockThingId);
    event1.insert("paramDescriptors", params);

    QVariantList events;
    events.append(event1);
    addRuleParams.insert("eventDescriptors", events);

    QVariantList actions;
    QVariantMap action;
    action.insert("actionTypeId", mockWithoutParamsActionTypeId);
    action.insert("thingId", m_mockThingId);
    actions.append(action);
    addRuleParams.insert("actions", actions);
    QVariant response = injectAndWait("Rules.AddRule", addRuleParams);
    verifyRuleError(response);


    // Trigger a non matching param
    spy.clear();
    request = QNetworkRequest(QUrl(QString("http://localhost:%1/setstate?%2=%3").arg(m_mockThing1Port).arg(mockBoolStateTypeId.toString()).arg("false")));
    reply = nam.get(request);
    spy.wait();
    QCOMPARE(spy.count(), 1);
    reply->deleteLater();

    verifyRuleNotExecuted();

    // Trigger a matching param
    spy.clear();
    request = QNetworkRequest(QUrl(QString("http://localhost:%1/setstate?%2=%3").arg(m_mockThing1Port).arg(mockBoolStateTypeId.toString()).arg("true")));
    reply = nam.get(request);
    spy.wait();
    QCOMPARE(spy.count(), 1);
    reply->deleteLater();

    verifyRuleExecuted(mockWithoutParamsActionTypeId);

    // Reset back to false to not mess with other tests
    spy.clear();
    request = QNetworkRequest(QUrl(QString("http://localhost:%1/setstate?%2=%3").arg(m_mockThing1Port).arg(mockBoolStateTypeId.toString()).arg("false")));
    reply = nam.get(request);
    spy.wait();
    QCOMPARE(spy.count(), 1);
    reply->deleteLater();
}


void TestRules::testStateChange() {
    // Add a rule
    QVariantMap addRuleParams;
    QVariantMap stateEvaluator;
    QVariantMap stateDescriptor;
    stateDescriptor.insert("thingId", m_mockThingId);
    stateDescriptor.insert("operator", enumValueName(Types::ValueOperatorGreaterOrEqual));
    stateDescriptor.insert("stateTypeId", mockIntStateTypeId);
    stateDescriptor.insert("value", 42);
    stateEvaluator.insert("stateDescriptor", stateDescriptor);
    addRuleParams.insert("stateEvaluator", stateEvaluator);
    addRuleParams.insert("name", "TestRule");

    QVariantList actions;
    QVariantMap action;
    action.insert("actionTypeId", mockWithoutParamsActionTypeId);
    action.insert("thingId", m_mockThingId);
    actions.append(action);
    addRuleParams.insert("actions", actions);
    QVariant response = injectAndWait("Rules.AddRule", addRuleParams);
    verifyRuleError(response);


    // Change the state
    QNetworkAccessManager nam;
    QSignalSpy spy(&nam, SIGNAL(finished(QNetworkReply*)));

    // state state to 42
    qDebug() << "setting mock int state to 42";
    QNetworkRequest request(QUrl(QString("http://localhost:%1/setstate?%2=%3").arg(m_mockThing1Port).arg(mockIntStateTypeId.toString()).arg(42)));
    QNetworkReply *reply = nam.get(request);
    spy.wait();
    QCOMPARE(spy.count(), 1);
    reply->deleteLater();

    verifyRuleExecuted(mockWithoutParamsActionTypeId);

    cleanupMockHistory();

    // set state to 45
    qDebug() << "setting mock int state to 45";
    spy.clear();
    request.setUrl(QUrl(QString("http://localhost:%1/setstate?%2=%3").arg(m_mockThing1Port).arg(mockIntStateTypeId.toString()).arg(45)));
    reply = nam.get(request);
    spy.wait();
    QCOMPARE(spy.count(), 1);
    reply->deleteLater();

    verifyRuleNotExecuted();

    cleanupMockHistory();

    // set state to 30
    qDebug() << "setting mock int state to 30";
    spy.clear();
    request.setUrl(QUrl(QString("http://localhost:%1/setstate?%2=%3").arg(m_mockThing1Port).arg(mockIntStateTypeId.toString()).arg(30)));
    reply = nam.get(request);
    spy.wait();
    QCOMPARE(spy.count(), 1);
    reply->deleteLater();

    verifyRuleNotExecuted();

    cleanupMockHistory();

    // set state to 100
    qDebug() << "setting mock int state to 100";
    spy.clear();
    request.setUrl(QUrl(QString("http://localhost:%1/setstate?%2=%3").arg(m_mockThing1Port).arg(mockIntStateTypeId.toString()).arg(100)));
    reply = nam.get(request);
    spy.wait();
    QCOMPARE(spy.count(), 1);

    verifyRuleExecuted(mockWithoutParamsActionTypeId);
    reply->deleteLater();
}

void TestRules::testStateEvaluator_data()
{
    QTest::addColumn<ThingId>("thingId");
    QTest::addColumn<StateTypeId>("stateTypeId");
    QTest::addColumn<QVariant>("value");
    QTest::addColumn<Types::ValueOperator>("operatorType");
    QTest::addColumn<bool>("shouldMatch");

    QTest::newRow("invalid stateId") << m_mockThingId << StateTypeId::createStateTypeId() << QVariant(10) << Types::ValueOperatorEquals << false;
    QTest::newRow("invalid thingId") << ThingId::createThingId() << mockIntStateTypeId << QVariant(10) << Types::ValueOperatorEquals << false;

    QTest::newRow("equals, not matching") << m_mockThingId << mockIntStateTypeId << QVariant(7777) << Types::ValueOperatorEquals << false;
    QTest::newRow("equals, matching") << m_mockThingId << mockIntStateTypeId << QVariant(10) << Types::ValueOperatorEquals << true;

    QTest::newRow("not equal, not matching") << m_mockThingId << mockIntStateTypeId << QVariant(10) << Types::ValueOperatorNotEquals << false;
    QTest::newRow("not equal, matching") << m_mockThingId << mockIntStateTypeId << QVariant(7777) << Types::ValueOperatorNotEquals << true;

    QTest::newRow("Greater, not matching") << m_mockThingId << mockIntStateTypeId << QVariant(7777) << Types::ValueOperatorGreater << false;
    QTest::newRow("Greater, matching") << m_mockThingId << mockIntStateTypeId << QVariant(2) << Types::ValueOperatorGreater << true;
    QTest::newRow("GreaterOrEqual, not matching") << m_mockThingId << mockIntStateTypeId << QVariant(7777) << Types::ValueOperatorGreaterOrEqual << false;
    QTest::newRow("GreaterOrEqual, matching (greater)") << m_mockThingId << mockIntStateTypeId << QVariant(2) << Types::ValueOperatorGreaterOrEqual << true;
    QTest::newRow("GreaterOrEqual, matching (equals)") << m_mockThingId << mockIntStateTypeId << QVariant(10) << Types::ValueOperatorGreaterOrEqual << true;

    QTest::newRow("Less, not matching") << m_mockThingId << mockIntStateTypeId << QVariant(2) << Types::ValueOperatorLess << false;
    QTest::newRow("Less, matching") << m_mockThingId << mockIntStateTypeId << QVariant(7777) << Types::ValueOperatorLess << true;
    QTest::newRow("LessOrEqual, not matching") << m_mockThingId << mockIntStateTypeId << QVariant(2) << Types::ValueOperatorLessOrEqual << false;
    QTest::newRow("LessOrEqual, matching (less)") << m_mockThingId << mockIntStateTypeId << QVariant(777) << Types::ValueOperatorLessOrEqual << true;
    QTest::newRow("LessOrEqual, matching (equals)") << m_mockThingId << mockIntStateTypeId << QVariant(10) << Types::ValueOperatorLessOrEqual << true;
    QTest::newRow("Less, not matching, double") << m_mockThingId << mockDoubleStateTypeId << QVariant(2.1) << Types::ValueOperatorLess << false;
    QTest::newRow("Less, not matching, double as string") << m_mockThingId << mockDoubleStateTypeId << QVariant("2.1") << Types::ValueOperatorLess << false;
    QTest::newRow("Less, matching, double") << m_mockThingId << mockDoubleStateTypeId << QVariant(4.2) << Types::ValueOperatorLess << true;
    QTest::newRow("Less, matching, double as string") << m_mockThingId << mockDoubleStateTypeId << QVariant("4.2") << Types::ValueOperatorLess << true;
}

void TestRules::testStateEvaluator()
{
    QFETCH(ThingId, thingId);
    QFETCH(StateTypeId, stateTypeId);
    QFETCH(QVariant, value);
    QFETCH(Types::ValueOperator, operatorType);
    QFETCH(bool, shouldMatch);

    StateDescriptor descriptor(stateTypeId, thingId, value, operatorType);
    StateEvaluator evaluator(descriptor);

    QVERIFY2(evaluator.evaluate() == shouldMatch, shouldMatch ? "State should match" : "State shouldn't match");
}

void TestRules::testStateEvaluator2_data()
{
    QTest::addColumn<int>("intValue");
    QTest::addColumn<Types::ValueOperator>("intOperator");

    QTest::addColumn<bool>("boolValue");
    QTest::addColumn<Types::ValueOperator>("boolOperator");

    QTest::addColumn<Types::StateOperator>("stateOperator");

    QTest::addColumn<bool>("shouldMatch");

    QTest::newRow("Y: 10 && false") << 10 << Types::ValueOperatorEquals << false << Types::ValueOperatorEquals << Types::StateOperatorAnd << true;
    QTest::newRow("N: 10 && true") << 10 << Types::ValueOperatorEquals << true << Types::ValueOperatorEquals << Types::StateOperatorAnd << false;
    QTest::newRow("N: 11 && false") << 11 << Types::ValueOperatorEquals << false << Types::ValueOperatorEquals << Types::StateOperatorAnd << false;
    QTest::newRow("Y: 11 || false") << 11 << Types::ValueOperatorEquals << false << Types::ValueOperatorEquals << Types::StateOperatorOr << true;
    QTest::newRow("Y: 10 || false") << 10 << Types::ValueOperatorEquals << false << Types::ValueOperatorEquals << Types::StateOperatorOr << true;
    QTest::newRow("Y: 10 || true") << 10 << Types::ValueOperatorEquals << true << Types::ValueOperatorEquals << Types::StateOperatorOr << true;
    QTest::newRow("N: 11 || true") << 11 << Types::ValueOperatorEquals << true << Types::ValueOperatorEquals << Types::StateOperatorOr << false;
}

void TestRules::testStateEvaluator2()
{
    QFETCH(int, intValue);
    QFETCH(Types::ValueOperator, intOperator);
    QFETCH(bool, boolValue);
    QFETCH(Types::ValueOperator, boolOperator);
    QFETCH(Types::StateOperator, stateOperator);
    QFETCH(bool, shouldMatch);

    StateDescriptor descriptor1(mockIntStateTypeId, m_mockThingId, intValue, intOperator);
    StateEvaluator evaluator1(descriptor1);

    StateDescriptor descriptor2(mockBoolStateTypeId, m_mockThingId, boolValue, boolOperator);
    StateEvaluator evaluator2(descriptor2);

    QList<StateEvaluator> childEvaluators;
    childEvaluators.append(evaluator1);
    childEvaluators.append(evaluator2);

    StateEvaluator mainEvaluator(childEvaluators);
    mainEvaluator.setOperatorType(stateOperator);

    QVERIFY2(mainEvaluator.evaluate() == shouldMatch, shouldMatch ? "State should match" : "State shouldn't match");
}

void TestRules::testStateEvaluator3_data()
{
    testStateEvaluator2_data();
}

void TestRules::testStateEvaluator3()
{
    QFETCH(int, intValue);
    QFETCH(Types::ValueOperator, intOperator);
    QFETCH(bool, boolValue);
    QFETCH(Types::ValueOperator, boolOperator);
    QFETCH(Types::StateOperator, stateOperator);
    QFETCH(bool, shouldMatch);

    StateDescriptor descriptor1(mockIntStateTypeId, m_mockThingId, intValue, intOperator);
    StateEvaluator childEvaluator(descriptor1);

    QList<StateEvaluator> childEvaluators;
    childEvaluators.append(childEvaluator);

    StateDescriptor descriptor2(mockBoolStateTypeId, m_mockThingId, boolValue, boolOperator);
    StateEvaluator mainEvaluator(descriptor2);
    mainEvaluator.setChildEvaluators(childEvaluators);
    mainEvaluator.setOperatorType(stateOperator);

    QVERIFY2(mainEvaluator.evaluate() == shouldMatch, shouldMatch ? "State should match" : "State shouldn't match");
}

void TestRules::testChildEvaluator_data()
{
    cleanup();

    ThingId testThingId = addDisplayPinMock();
    QVERIFY2(!testThingId.isNull(), "Could not add push button mock for child evaluators");

    enableNotifications({"Rules"});

    // Create child evaluators
    // Action
    QVariantMap action;
    action.insert("actionTypeId", mockWithoutParamsActionTypeId);
    action.insert("thingId", m_mockThingId);
    action.insert("ruleActionParams", QVariantList());

    // Exit action (with params)
    QVariantMap exitAction;
    QVariantList actionParams;
    QVariantMap param1;
    param1.insert("paramTypeId", mockWithParamsActionParam1ParamTypeId);
    param1.insert("value", 12);
    actionParams.append(param1);
    QVariantMap param2;
    param2.insert("paramTypeId", mockWithParamsActionParam2ParamTypeId);
    param2.insert("value", true);
    actionParams.append(param2);
    exitAction.insert("actionTypeId", mockWithParamsActionTypeId);
    exitAction.insert("thingId", m_mockThingId);
    exitAction.insert("ruleActionParams", actionParams);

    // Stateevaluators
    QVariantMap stateDescriptorPercentage;
    stateDescriptorPercentage.insert("thingId", testThingId);
    stateDescriptorPercentage.insert("operator", enumValueName(Types::ValueOperatorGreaterOrEqual));
    stateDescriptorPercentage.insert("stateTypeId", displayPinMockPercentageStateTypeId);
    stateDescriptorPercentage.insert("value", 50);

    QVariantMap stateDescriptorDouble;
    stateDescriptorDouble.insert("thingId", testThingId);
    stateDescriptorDouble.insert("operator", enumValueName(Types::ValueOperatorEquals));
    stateDescriptorDouble.insert("stateTypeId", displayPinMockDoubleActionDoubleParamTypeId);
    stateDescriptorDouble.insert("value", 20.5);

    QVariantMap stateDescriptorAllowedValues;
    stateDescriptorAllowedValues.insert("thingId", testThingId);
    stateDescriptorAllowedValues.insert("operator", enumValueName(Types::ValueOperatorEquals));
    stateDescriptorAllowedValues.insert("stateTypeId", displayPinMockAllowedValuesStateTypeId);
    stateDescriptorAllowedValues.insert("value", "String value 2");

    QVariantMap stateDescriptorColor;
    stateDescriptorColor.insert("thingId", testThingId);
    stateDescriptorColor.insert("operator", enumValueName(Types::ValueOperatorEquals));
    stateDescriptorColor.insert("stateTypeId", displayPinMockColorStateTypeId);
    stateDescriptorColor.insert("value", "#00FF00");

    QVariantMap firstStateEvaluator;
    firstStateEvaluator.insert("operator", enumValueName(Types::StateOperatorOr));
    firstStateEvaluator.insert("childEvaluators", QVariantList() << createStateEvaluatorFromSingleDescriptor(stateDescriptorPercentage) << createStateEvaluatorFromSingleDescriptor(stateDescriptorDouble));

    QVariantMap secondStateEvaluator;
    secondStateEvaluator.insert("operator", enumValueName(Types::StateOperatorAnd));
    secondStateEvaluator.insert("childEvaluators", QVariantList() << createStateEvaluatorFromSingleDescriptor(stateDescriptorAllowedValues) << createStateEvaluatorFromSingleDescriptor(stateDescriptorColor));

    QVariantMap stateEvaluator;
    stateEvaluator.insert("operator", enumValueName(Types::StateOperatorAnd));
    stateEvaluator.insert("childEvaluators", QVariantList() << firstStateEvaluator << secondStateEvaluator);

    // The rule
    QVariantMap ruleMap;
    ruleMap.insert("name", "Child evaluator rule");
    ruleMap.insert("stateEvaluator", stateEvaluator);
    ruleMap.insert("actions", QVariantList() << action);
    ruleMap.insert("exitActions", QVariantList() << exitAction);

    printJson(ruleMap);


    // (percentage >= 50 || double == 20.5) && (color == #00FF00 && allowedValue == "String value 2") ? action : exit action

    QTest::addColumn<ThingId>("thingId");
    QTest::addColumn<QVariantMap>("ruleMap");
    QTest::addColumn<int>("percentageValue");
    QTest::addColumn<double>("doubleValue");
    QTest::addColumn<QString>("allowedValue");
    QTest::addColumn<QString>("colorValue");
    QTest::addColumn<bool>("trigger");
    QTest::addColumn<bool>("active");

    QTest::newRow("Unchanged | 2 | 2.5 | String value 1 | #FF0000") << testThingId << ruleMap << 2 << 2.5 << "String value 1" << "#ff0000" << false << false;
    QTest::newRow("Unchanged | 60 | 2.5 | String value 2 | #FF0000") << testThingId << ruleMap << 60 << 2.5 << "String value 2" << "#ff0000" << false << false;
    QTest::newRow("Unchanged | 60 | 20.5 | String value 2 | #FF0000") << testThingId << ruleMap << 60 << 20.5 << "String value 2" << "#ff0000" << false << false;
    QTest::newRow("Active | 60 | 20.5 | String value 2 | #00FF00") << testThingId << ruleMap << 60 << 20.5 << "String value 2" << "#00ff00" << true << true;
    QTest::newRow("Active | 60 | 20.5 | String value 2 | #00FF00") << testThingId << ruleMap << 60 << 20.5 << "String value 2" << "#00ff00" << true << true;
}

void TestRules::testChildEvaluator()
{
    QFETCH(ThingId, thingId);
    QFETCH(QVariantMap, ruleMap);
    QFETCH(int, percentageValue);
    QFETCH(double, doubleValue);
    QFETCH(QString, allowedValue);
    QFETCH(QString, colorValue);
    QFETCH(bool, trigger);
    QFETCH(bool, active);

    // Init the states
    setWritableStateValue(thingId, StateTypeId(displayPinMockPercentageStateTypeId.toString()), QVariant(0));
    setWritableStateValue(thingId, StateTypeId(displayPinMockDoubleActionDoubleParamTypeId.toString()), QVariant(0));
    setWritableStateValue(thingId, StateTypeId(displayPinMockAllowedValuesStateTypeId.toString()), QVariant("String value 1"));
    setWritableStateValue(thingId, StateTypeId(displayPinMockColorStateTypeId.toString()), QVariant("#000000"));

    qCDebug(dcTests()) << "Adding rule";

    // Add rule
    QVariant response = injectAndWait("Rules.AddRule", ruleMap);
    verifyRuleError(response);

    RuleId ruleId = RuleId(response.toMap().value("params").toMap().value("ruleId").toString());

    // Set the states
    qCDebug(dcTests()) << "Setting state 1";
    setWritableStateValue(thingId, StateTypeId(displayPinMockPercentageStateTypeId.toString()), QVariant::fromValue(percentageValue));
    qCDebug(dcTests()) << "Setting state 2";
    setWritableStateValue(thingId, StateTypeId(displayPinMockDoubleActionDoubleParamTypeId.toString()), QVariant::fromValue(doubleValue));
    qCDebug(dcTests()) << "Setting state 3";
    setWritableStateValue(thingId, StateTypeId(displayPinMockAllowedValuesStateTypeId.toString()), QVariant::fromValue(allowedValue));
    qCDebug(dcTests()) << "Setting state 4";
    setWritableStateValue(thingId, StateTypeId(displayPinMockColorStateTypeId.toString()), QVariant::fromValue(colorValue));

    // Verfiy if the rule executed successfully
    // Actions
    if (trigger && active) {
        qCDebug(dcTests()) << "Checking if actions were executed";
        verifyRuleExecuted(mockWithoutParamsActionTypeId);
        cleanupMockHistory();
    }

    // Exit actions
    if (trigger && !active) {
        qCDebug(dcTests()) << "Checking if exit actions were executed";
        verifyRuleExecuted(mockWithParamsActionTypeId);
        cleanupMockHistory();
    }

    // Nothing triggert
    if (!trigger) {
        qCDebug(dcTests()) << "Making sure nothing triggered";
        verifyRuleNotExecuted();
    }

    // REMOVE rule
    qCDebug(dcTests()) << "Removing rule";
    QVariantMap removeParams;
    removeParams.insert("ruleId", ruleId);
    response = injectAndWait("Rules.RemoveRule", removeParams);
    verifyRuleError(response);
}

void TestRules::enableDisableRule()
{
    // Add a rule
    QVariantMap addRuleParams;
    QVariantList events;
    QVariantMap event1;
    event1.insert("eventTypeId", mockEvent1EventTypeId);
    event1.insert("thingId", m_mockThingId);
    events.append(event1);
    addRuleParams.insert("eventDescriptors", events);
    addRuleParams.insert("name", "TestRule");

    QVariantList actions;
    QVariantMap action;
    action.insert("actionTypeId", mockWithoutParamsActionTypeId);
    action.insert("thingId", m_mockThingId);
    actions.append(action);
    addRuleParams.insert("actions", actions);
    QVariant response = injectAndWait("Rules.AddRule", addRuleParams);
    verifyRuleError(response);
    RuleId id = RuleId(response.toMap().value("params").toMap().value("ruleId").toString());

    // Trigger an event
    QNetworkAccessManager nam;
    QSignalSpy spy(&nam, SIGNAL(finished(QNetworkReply*)));

    // trigger event in mock
    QNetworkRequest request(QUrl(QString("http://localhost:%1/generateevent?eventtypeid=%2").arg(m_mockThing1Port).arg(mockEvent1EventTypeId.toString())));
    QNetworkReply *reply = nam.get(request);
    spy.wait();
    QCOMPARE(spy.count(), 1);
    reply->deleteLater();

    verifyRuleExecuted(mockWithoutParamsActionTypeId);

    cleanupMockHistory();

    // Now DISABLE the rule invalid ruleId
    QVariantMap disableParams;
    disableParams.insert("ruleId", QUuid::createUuid().toString());
    response = injectAndWait("Rules.DisableRule", disableParams);
    verifyRuleError(response, RuleEngine::RuleErrorRuleNotFound);

    // Now DISABLE the rule
    disableParams.clear();
    disableParams.insert("ruleId", id.toString());
    response = injectAndWait("Rules.DisableRule", disableParams);
    verifyRuleError(response);

    // trigger event in mock
    spy.clear();
    request = QNetworkRequest(QUrl(QString("http://localhost:%1/generateevent?eventtypeid=%2").arg(m_mockThing1Port).arg(mockEvent1EventTypeId.toString())));
    reply = nam.get(request);
    spy.wait();
    QCOMPARE(spy.count(), 1);
    reply->deleteLater();

    verifyRuleNotExecuted();

    cleanupMockHistory();

    // Now ENABLE the rule again invald ruleId
    QVariantMap enableParams;
    enableParams.insert("ruleId", QUuid::createUuid().toString());
    response = injectAndWait("Rules.EnableRule", enableParams);
    verifyRuleError(response, RuleEngine::RuleErrorRuleNotFound);

    // Now ENABLE the rule again
    enableParams.clear();
    enableParams.insert("ruleId", id.toString());
    response = injectAndWait("Rules.EnableRule", enableParams);
    verifyRuleError(response);

    // trigger event in mock
    spy.clear();
    request = QNetworkRequest(QUrl(QString("http://localhost:%1/generateevent?eventtypeid=%2").arg(m_mockThing1Port).arg(mockEvent1EventTypeId.toString())));
    reply = nam.get(request);
    spy.wait();
    QCOMPARE(spy.count(), 1);
    reply->deleteLater();

    verifyRuleExecuted(mockWithoutParamsActionTypeId);
}

void TestRules::testEventBasedAction()
{
    // Add a rule
    QVariantMap addRuleParams;
    QVariantMap eventDescriptor;
    eventDescriptor.insert("eventTypeId", mockIntStateTypeId);
    eventDescriptor.insert("thingId", m_mockThingId);
    addRuleParams.insert("eventDescriptors", QVariantList() << eventDescriptor);
    addRuleParams.insert("name", "TestRule");
    addRuleParams.insert("enabled", true);

    QVariantList actions;
    QVariantMap action;
    QVariantList ruleActionParams;
    QVariantMap param1;
    param1.insert("paramTypeId", mockWithParamsActionParam1ParamTypeId);
    param1.insert("eventTypeId", mockIntStateTypeId);
    param1.insert("eventParamTypeId", mockIntStateTypeId);
    QVariantMap param2;
    param2.insert("paramTypeId", mockWithParamsActionParam2ParamTypeId);
    param2.insert("value", true);
    ruleActionParams.append(param1);
    ruleActionParams.append(param2);
    action.insert("actionTypeId", mockWithParamsActionTypeId);
    action.insert("thingId", m_mockThingId);
    action.insert("ruleActionParams", ruleActionParams);
    actions.append(action);
    addRuleParams.insert("actions", actions);

    qDebug() << addRuleParams;

    QVariant response = injectAndWait("Rules.AddRule", addRuleParams);
    verifyRuleError(response);

    // Change the state
    QNetworkAccessManager nam;
    QSignalSpy spy(&nam, SIGNAL(finished(QNetworkReply*)));

    // state state to 42
    qDebug() << "setting mock int state to 42";
    QNetworkRequest request(QUrl(QString("http://localhost:%1/setstate?%2=%3").arg(m_mockThing1Port).arg(mockIntStateTypeId.toString()).arg(42)));
    QNetworkReply *reply = nam.get(request);
    spy.wait();
    QCOMPARE(spy.count(), 1);
    reply->deleteLater();

    verifyRuleExecuted(mockWithParamsActionTypeId);
    // TODO: check if this action was really executed with the int state value 42
}

void TestRules::testEventBasedRuleWithExitAction()
{
    QNetworkAccessManager nam;
    QSignalSpy spy(&nam, SIGNAL(finished(QNetworkReply*)));

    // Init bool state to true
    spy.clear();
    QNetworkRequest request(QUrl(QString("http://localhost:%1/setstate?%2=%3").arg(m_mockThing1Port).arg(mockBoolStateTypeId.toString()).arg(true)));
    QNetworkReply *reply = nam.get(request);
    spy.wait();
    QCOMPARE(spy.count(), 1);
    reply->deleteLater();

    // Add a rule
    QVariantMap addRuleParams;
    QVariantMap eventDescriptor;
    eventDescriptor.insert("eventTypeId", mockEvent1EventTypeId);
    eventDescriptor.insert("thingId", m_mockThingId);
    addRuleParams.insert("eventDescriptors", QVariantList() << eventDescriptor);
    addRuleParams.insert("name", "TestRule");
    addRuleParams.insert("enabled", true);

    QVariantMap stateEvaluator;
    QVariantMap stateDescriptor;
    stateDescriptor.insert("thingId", m_mockThingId);
    stateDescriptor.insert("stateTypeId", mockBoolStateTypeId);
    stateDescriptor.insert("operator", "ValueOperatorEquals");
    stateDescriptor.insert("value", true);
    stateEvaluator.insert("stateDescriptor", stateDescriptor);
    stateEvaluator.insert("operator", "StateOperatorAnd");
    addRuleParams.insert("stateEvaluator", stateEvaluator);

    QVariantList actions;
    QVariantMap action;
    QVariantList ruleActionParams;
    QVariantMap param1;
    param1.insert("paramTypeId", mockWithParamsActionParam1ParamTypeId);
    param1.insert("value", true);
    QVariantMap param2;
    param2.insert("paramTypeId", mockWithParamsActionParam2ParamTypeId);
    param2.insert("value", true);
    ruleActionParams.append(param1);
    ruleActionParams.append(param2);

    action.insert("actionTypeId", mockWithoutParamsActionTypeId);
    action.insert("thingId", m_mockThingId);
    actions.append(action);
    addRuleParams.insert("actions", actions);

    actions.clear();
    action.insert("actionTypeId", mockWithParamsActionTypeId);
    action.insert("ruleActionParams", ruleActionParams);
    actions.append(action);
    addRuleParams.insert("exitActions", actions);

    qDebug() << addRuleParams;

    QVariant response = injectAndWait("Rules.AddRule", addRuleParams);
    verifyRuleError(response);

    // trigger event
    spy.clear();
    request = QNetworkRequest(QUrl(QString("http://localhost:%1/generateevent?eventtypeid=%2").arg(m_mockThing1Port).arg(mockEvent1EventTypeId.toString())));
    reply = nam.get(request);
    spy.wait();
    QCOMPARE(spy.count(), 1);
    reply->deleteLater();

    // Verify the actions got executed
    verifyRuleExecuted(mockWithoutParamsActionTypeId);

    // set bool state to false
    spy.clear();
    request = QNetworkRequest(QUrl(QString("http://localhost:%1/setstate?%2=%3").arg(m_mockThing1Port).arg(mockBoolStateTypeId.toString()).arg(false)));
    reply = nam.get(request);
    spy.wait();
    QCOMPARE(spy.count(), 1);
    reply->deleteLater();

    // trigger event
    spy.clear();
    request = QNetworkRequest(QUrl(QString("http://localhost:%1/generateevent?eventtypeid=%2").arg(m_mockThing1Port).arg(mockEvent1EventTypeId.toString())));
    reply = nam.get(request);
    spy.wait();
    QCOMPARE(spy.count(), 1);
    reply->deleteLater();

    // Verify the exit actions got executed
    verifyRuleExecuted(mockWithoutParamsActionTypeId);

}

void TestRules::testStateBasedAction()
{
    QNetworkAccessManager nam;
    QSignalSpy spy(&nam, SIGNAL(finished(QNetworkReply*)));

    // Init bool state to true
    spy.clear();
    QNetworkRequest request(QUrl(QString("http://localhost:%1/setstate?%2=%3").arg(m_mockThing1Port).arg(mockBoolStateTypeId.toString()).arg(true)));
    QNetworkReply *reply = nam.get(request);
    spy.wait();
    QCOMPARE(spy.count(), 1);
    reply->deleteLater();

    // Init int state to 11
    spy.clear();
    request = QNetworkRequest(QUrl(QString("http://localhost:%1/setstate?%2=%3").arg(m_mockThing1Port).arg(mockIntStateTypeId.toString()).arg(11)));
    reply = nam.get(request);
    spy.wait();
    QCOMPARE(spy.count(), 1);
    reply->deleteLater();

    // Add a rule
    QVariantMap addRuleParams;
    QVariantMap eventDescriptor;
    eventDescriptor.insert("eventTypeId", mockEvent1EventTypeId);
    eventDescriptor.insert("thingId", m_mockThingId);
    addRuleParams.insert("eventDescriptors", QVariantList() << eventDescriptor);
    addRuleParams.insert("name", "TestRule");
    addRuleParams.insert("enabled", true);

    QVariantList actions;
    QVariantMap action;
    QVariantList ruleActionParams;
    QVariantMap param1;
    param1.insert("paramTypeId", mockWithParamsActionParam1ParamTypeId);
    param1.insert("stateThingId", m_mockThingId);
    param1.insert("stateTypeId", mockIntStateTypeId);
    QVariantMap param2;
    param2.insert("paramTypeId", mockWithParamsActionParam2ParamTypeId);
    param2.insert("stateThingId", m_mockThingId);
    param2.insert("stateTypeId", mockBoolStateTypeId);
    ruleActionParams.append(param1);
    ruleActionParams.append(param2);

    actions.clear();
    action.insert("thingId", m_mockThingId);
    action.insert("actionTypeId", mockWithParamsActionTypeId);
    action.insert("ruleActionParams", ruleActionParams);
    actions.append(action);
    addRuleParams.insert("actions", actions);

    qCDebug(dcTests) << "Adding rule";

    QVariant response = injectAndWait("Rules.AddRule", addRuleParams);
    verifyRuleError(response);

    // trigger event
    spy.clear();
    request = QNetworkRequest(QUrl(QString("http://localhost:%1/generateevent?eventtypeid=%2").arg(m_mockThing1Port).arg(mockEvent1EventTypeId.toString())));
    reply = nam.get(request);
    spy.wait();
    QCOMPARE(spy.count(), 1);
    reply->deleteLater();

    LogFilter filter;
    filter.addThingId(m_mockThingId);
    filter.addTypeId(mockWithParamsActionTypeId);

    LogEntriesFetchJob *job = NymeaCore::instance()->logEngine()->fetchLogEntries(filter);
    QSignalSpy fetchSpy(job, &LogEntriesFetchJob::finished);
    fetchSpy.wait();
    QList<LogEntry> entries = job->results();
    qCDebug(dcTests()) << "Log entries:" << entries;

    // set bool state to false
    spy.clear();
    request = QNetworkRequest(QUrl(QString("http://localhost:%1/setstate?%2=%3").arg(m_mockThing1Port).arg(mockBoolStateTypeId.toString()).arg(false)));
    reply = nam.get(request);
    spy.wait();
    QCOMPARE(spy.count(), 1);
    reply->deleteLater();

    // trigger event
    spy.clear();
    request = QNetworkRequest(QUrl(QString("http://localhost:%1/generateevent?eventtypeid=%2").arg(m_mockThing1Port).arg(mockEvent1EventTypeId.toString())));
    reply = nam.get(request);
    spy.wait();
    QCOMPARE(spy.count(), 1);
    reply->deleteLater();

    job = NymeaCore::instance()->logEngine()->fetchLogEntries(filter);
    QSignalSpy fetchSpy2(job, &LogEntriesFetchJob::finished);
    fetchSpy2.wait();
    entries = job->results();

    qCDebug(dcTests()) << "Log entries:" << entries;
}

void TestRules::removePolicyUpdate()
{
    // ADD parent
    QVariantMap params;
    params.insert("thingClassId", parentMockThingClassId);
    params.insert("name", "Parent");

    QSignalSpy addedSpy(NymeaCore::instance()->thingManager(), &ThingManager::thingAdded);

    QVariant response = injectAndWait("Integrations.AddThing", params);
    verifyThingError(response);

    ThingId parentId = ThingId(response.toMap().value("params").toMap().value("thingId").toString());
    QVERIFY(!parentId.isNull());

    addedSpy.wait();

    // find child
    response = injectAndWait("Integrations.GetThings");

    QVariantList things = response.toMap().value("params").toMap().value("things").toList();

    ThingId childId;
    foreach (const QVariant thingVariant, things) {
        QVariantMap thingMap = thingVariant.toMap();

        if (thingMap.value("thingClassId").toUuid() == childMockThingClassId) {
            if (thingMap.value("parentId").toUuid() == parentId) {
                childId = ThingId(thingMap.value("id").toString());
            }
        }
    }
    QVERIFY2(!childId.isNull(), "Could not find child");

    // Add rule with child
    QVariantList eventDescriptors;
    eventDescriptors.append(createEventDescriptor(childId, childMockBoolValueEventTypeId));
    eventDescriptors.append(createEventDescriptor(parentId, parentMockBoolValueEventTypeId));
    eventDescriptors.append(createEventDescriptor(m_mockThingId, mockEvent1EventTypeId));

    params.clear(); response.clear();
    params.insert("name", "RemovePolicy");
    params.insert("eventDescriptors", eventDescriptors);
    params.insert("actions", QVariantList() << createActionWithParams(m_mockThingId));

    response = injectAndWait("Rules.AddRule", params);
    verifyRuleError(response);
    RuleId ruleId = RuleId(response.toMap().value("params").toMap().value("ruleId").toString());
    QVERIFY2(!ruleId.isNull(), "Could not get ruleId");

    // Try to remove child
    params.clear(); response.clear();
    params.insert("thingId", childId);
    response = injectAndWait("Integrations.RemoveThing", params);
    verifyThingError(response, Thing::ThingErrorThingIsChild);

    // Try to remove child
    params.clear(); response.clear();
    params.insert("thingId", parentId);
    response = injectAndWait("Integrations.RemoveThing", params);
    verifyThingError(response, Thing::ThingErrorThingInRule);

    // Remove policy
    params.clear(); response.clear();
    params.insert("thingId", parentId);
    params.insert("removePolicy", "RemovePolicyUpdate");
    response = injectAndWait("Integrations.RemoveThing", params);
    verifyThingError(response);

    // get updated rule
    params.clear();
    params.insert("ruleId", ruleId);
    response = injectAndWait("Rules.GetRuleDetails", params);
    verifyRuleError(response);

    QVariantMap rule = response.toMap().value("params").toMap().value("rule").toMap();
    qDebug() << "Updated rule:" << QJsonDocument::fromVariant(rule).toJson();
    QVERIFY(rule.value("eventDescriptors").toList().count() == 1);

    // REMOVE rule
    QVariantMap removeParams;
    removeParams.insert("ruleId", ruleId);
    response = injectAndWait("Rules.RemoveRule", removeParams);
    verifyRuleError(response);
}

void TestRules::removePolicyCascade()
{
    // ADD parent
    QVariantMap params;
    params.insert("thingClassId", parentMockThingClassId);
    params.insert("name", "Parent");

    QSignalSpy addedSpy(NymeaCore::instance()->thingManager(), &ThingManager::thingAdded);

    QVariant response = injectAndWait("Integrations.AddThing", params);
    verifyThingError(response);

    ThingId parentId = ThingId(response.toMap().value("params").toMap().value("thingId").toString());
    QVERIFY(!parentId.isNull());

    addedSpy.wait();

    // find child
    response = injectAndWait("Integrations.GetThings");

    QVariantList things = response.toMap().value("params").toMap().value("things").toList();

    ThingId childId;
    foreach (const QVariant thingVariant, things) {
        QVariantMap thingMap = thingVariant.toMap();

        if (thingMap.value("thingClassId").toUuid() == childMockThingClassId) {
            if (thingMap.value("parentId").toUuid() == parentId) {
                childId = ThingId(thingMap.value("id").toString());
            }
        }
    }
    QVERIFY2(!childId.isNull(), "Could not find child");

    // Add rule with child
    QVariantList eventDescriptors;
    eventDescriptors.append(createEventDescriptor(childId, childMockBoolValueEventTypeId));
    eventDescriptors.append(createEventDescriptor(parentId, parentMockBoolValueEventTypeId));
    eventDescriptors.append(createEventDescriptor(m_mockThingId, mockEvent1EventTypeId));

    params.clear(); response.clear();
    params.insert("name", "RemovePolicy");
    params.insert("eventDescriptors", eventDescriptors);
    params.insert("actions", QVariantList() << createActionWithParams(m_mockThingId));

    response = injectAndWait("Rules.AddRule", params);
    verifyRuleError(response);
    RuleId ruleId = RuleId(response.toMap().value("params").toMap().value("ruleId").toString());
    QVERIFY2(!ruleId.isNull(), "Could not get ruleId");

    // Try to remove child
    params.clear(); response.clear();
    params.insert("thingId", childId);
    response = injectAndWait("Integrations.RemoveThing", params);
    verifyThingError(response, Thing::ThingErrorThingIsChild);

    // Try to remove child by removing parent
    params.clear(); response.clear();
    params.insert("thingId", parentId);
    response = injectAndWait("Integrations.RemoveThing", params);
    verifyThingError(response, Thing::ThingErrorThingInRule);

    // Remove policy
    params.clear(); response.clear();
    params.insert("thingId", parentId);
    params.insert("removePolicy", "RemovePolicyCascade");
    response = injectAndWait("Integrations.RemoveThing", params);
    verifyThingError(response);

    // get updated rule
    params.clear();
    params.insert("ruleId", ruleId);
    response = injectAndWait("Rules.GetRuleDetails", params);
    verifyRuleError(response, RuleEngine::RuleErrorRuleNotFound);
}

void TestRules::removePolicyUpdateRendersUselessRule()
{
    // ADD parent
    QVariantMap params;
    params.insert("thingClassId", parentMockThingClassId);
    params.insert("name", "Parent");

    QSignalSpy addedSpy(NymeaCore::instance()->thingManager(), &ThingManager::thingAdded);

    QVariant response = injectAndWait("Integrations.AddThing", params);
    verifyThingError(response);

    ThingId parentId = ThingId(response.toMap().value("params").toMap().value("thingId").toString());
    QVERIFY(!parentId.isNull());

    addedSpy.wait();

    // find child
    qCDebug(dcTests()) << "Get things";
    response = injectAndWait("Integrations.GetThings");

    QVariantList things = response.toMap().value("params").toMap().value("things").toList();

    ThingId childId;
    foreach (const QVariant thingVariant, things) {
        QVariantMap thingMap = thingVariant.toMap();

        if (thingMap.value("thingClassId").toUuid() == childMockThingClassId) {
            if (thingMap.value("parentId").toUuid() == parentId) {
                childId = ThingId(thingMap.value("id").toString());
            }
        }
    }
    QVERIFY2(!childId.isNull(), "Could not find child");

    // Add rule with child
    QVariantList eventDescriptors;
    eventDescriptors.append(createEventDescriptor(childId, childMockBoolValueEventTypeId));
    eventDescriptors.append(createEventDescriptor(parentId, parentMockBoolValueEventTypeId));
    eventDescriptors.append(createEventDescriptor(m_mockThingId, mockEvent1EventTypeId));

    params.clear(); response.clear();
    params.insert("name", "RemovePolicy");
    params.insert("eventDescriptors", eventDescriptors);

    QVariantMap action;
    action.insert("thingId", childId);
    action.insert("actionTypeId", childMockBoolValueActionTypeId);
    QVariantMap ruleActionParam;
    ruleActionParam.insert("paramTypeId", childMockBoolValueActionBoolValueParamTypeId);
    ruleActionParam.insert("value", true);
    action.insert("ruleActionParams", QVariantList() << ruleActionParam);
    params.insert("actions", QVariantList() << action);

    qCDebug(dcTests()) << "Adding Rule";
    response = injectAndWait("Rules.AddRule", params);
    verifyRuleError(response);
    RuleId ruleId = RuleId(response.toMap().value("params").toMap().value("ruleId").toString());
    QVERIFY2(!ruleId.isNull(), "Could not get ruleId");

    // Try to remove child
    qCDebug(dcTests()) << "Removing thing (expecing failure - thing is child)";
    params.clear(); response.clear();
    params.insert("thingId", childId);
    response = injectAndWait("Integrations.RemoveThing", params);
    verifyThingError(response, Thing::ThingErrorThingIsChild);

    // Try to remove child by removing parent
    qCDebug(dcTests()) << "Removing thing (expeciting failure - thing in use)";
    params.clear(); response.clear();
    params.insert("thingId", parentId);
    response = injectAndWait("Integrations.RemoveThing", params);
    verifyThingError(response, Thing::ThingErrorThingInRule);

    // Remove policy
    qCDebug(dcTests()) << "Removing thing with update policy";
    params.clear(); response.clear();
    params.insert("thingId", parentId);
    params.insert("removePolicy", "RemovePolicyUpdate");
    response = injectAndWait("Integrations.RemoveThing", params);
    verifyThingError(response);

    // get updated rule. It should've been deleted given it ended up with no actions
    qCDebug(dcTests()) << "Getting details";
    params.clear();
    params.insert("ruleId", ruleId);
    response = injectAndWait("Rules.GetRuleDetails", params);
    verifyRuleError(response, RuleEngine::RuleErrorRuleNotFound);
}

void TestRules::testRuleActionParams_data()
{
    QVariantMap action;
    QVariantList ruleActionParams;
    QVariantMap param1;
    param1.insert("paramTypeId", mockWithParamsActionParam1ParamTypeId);
    param1.insert("value", 4);
    QVariantMap param2;
    param2.insert("paramTypeId", mockWithParamsActionParam2ParamTypeId);
    param2.insert("value", true);
    ruleActionParams.append(param1);
    ruleActionParams.append(param2);
    action.insert("actionTypeId", mockWithParamsActionTypeId);
    action.insert("thingId", m_mockThingId);
    action.insert("ruleActionParams", ruleActionParams);

    QVariantMap invalidAction1;
    invalidAction1.insert("actionTypeId", mockWithParamsActionTypeId);
    invalidAction1.insert("thingId", m_mockThingId);
    invalidAction1.insert("ruleActionParams", QVariantList() << param2);

    QVariantMap invalidAction2;
    invalidAction2.insert("actionTypeId", mockWithParamsActionTypeId);
    invalidAction2.insert("thingId", m_mockThingId);
    invalidAction2.insert("ruleActionParams", QVariantList() << param1);


    QTest::addColumn<QVariantMap>("action");
    QTest::addColumn<QVariantMap>("exitAction");
    QTest::addColumn<RuleEngine::RuleError>("error");

    QTest::newRow("valid action params") << action << QVariantMap() << RuleEngine::RuleErrorNoError;
    QTest::newRow("valid action and exit action params") << action << action << RuleEngine::RuleErrorNoError;
    QTest::newRow("invalid action params1") << invalidAction1 << QVariantMap() << RuleEngine::RuleErrorMissingParameter;
    QTest::newRow("invalid action params2") << invalidAction2 << QVariantMap() << RuleEngine::RuleErrorMissingParameter;
    QTest::newRow("valid action and invalid exit action params1") << action << invalidAction1 << RuleEngine::RuleErrorMissingParameter;
    QTest::newRow("valid action and invalid exit action params2") << action << invalidAction2 << RuleEngine::RuleErrorMissingParameter;
}

void TestRules::testRuleActionParams()
{
    QFETCH(QVariantMap, action);
    QFETCH(QVariantMap, exitAction);
    QFETCH(RuleEngine::RuleError, error);

    // Add a rule
    QVariantMap addRuleParams;
    addRuleParams.insert("name", "TestRule");
    addRuleParams.insert("enabled", true);
    if (!action.isEmpty())
        addRuleParams.insert("actions", QVariantList() << action);
    if (!exitAction.isEmpty())
        addRuleParams.insert("exitActions", QVariantList() << exitAction);

    QVariant response = injectAndWait("Rules.AddRule", addRuleParams);
    verifyRuleError(response, error);
}

void TestRules::testRuleActionPAramsFromEventParameter_data() {
    QTest::addColumn<QVariantMap>("event");
    QTest::addColumn<QVariantMap>("action");
    QTest::addColumn<RuleEngine::RuleError>("error");

    QVariantMap intEvent;
    intEvent.insert("eventTypeId", mockIntStateTypeId);
    intEvent.insert("thingId", m_mockThingId);

    QVariantMap intAction;
    intAction.insert("actionTypeId", mockWithParamsActionTypeId);
    intAction.insert("thingId", m_mockThingId);
    QVariantList ruleActionParams;
    QVariantMap intParam;
    intParam.insert("paramTypeId", mockWithParamsActionParam1ParamTypeId);
    intParam.insert("eventTypeId", mockIntStateTypeId);
    intParam.insert("eventParamTypeId", mockIntStateTypeId);
    ruleActionParams.append(intParam);
    QVariantMap boolParam;
    boolParam.insert("paramTypeId", mockWithParamsActionParam2ParamTypeId);
    boolParam.insert("value", true);
    ruleActionParams.append(boolParam);
    intAction.insert("ruleActionParams", ruleActionParams);

    QVariantMap boolAction;
    boolAction.insert("actionTypeId", mockWithParamsActionTypeId);
    boolAction.insert("thingId", m_mockThingId);
    ruleActionParams.clear();
    intParam.clear();
    intParam.insert("paramTypeId", mockWithParamsActionParam1ParamTypeId);
    intParam.insert("value", 5);
    ruleActionParams.append(intParam);
    boolParam.clear();
    boolParam.insert("paramTypeId", mockWithParamsActionParam2ParamTypeId);
    boolParam.insert("eventTypeId", mockIntStateTypeId);
    boolParam.insert("eventParamTypeId", mockIntStateTypeId);
    ruleActionParams.append(boolParam);
    boolAction.insert("ruleActionParams", ruleActionParams);

    QTest::newRow("int -> int") << intEvent << intAction << RuleEngine::RuleErrorNoError;
    QTest::newRow("int -> bool") << intEvent << boolAction << RuleEngine::RuleErrorNoError;
}

void TestRules::testRuleActionPAramsFromEventParameter()
{
    QFETCH(QVariantMap, event);
    QFETCH(QVariantMap, action);
    QFETCH(RuleEngine::RuleError, error);

    QVariantMap addRuleParams;
    addRuleParams.insert("name", "TestRule");
    addRuleParams.insert("enabled", true);

    addRuleParams.insert("eventDescriptors", QVariantList() << event);
    addRuleParams.insert("actions", QVariantList() << action);

    QVariant response = injectAndWait("Rules.AddRule", addRuleParams);
    verifyRuleError(response, error);
}

void TestRules::testInitStatesActive()
{

    // Create a rule to toggle the power state on event 1
    QVariantMap params;
    params.insert("name", "testrule");

    QVariantMap eventDescriptor;
    eventDescriptor.insert("thingId", m_mockThingId);
    eventDescriptor.insert("eventTypeId", mockEvent1EventTypeId);
    QVariantList eventDescriptors;
    eventDescriptors.append(eventDescriptor);
    params.insert("eventDescriptors", eventDescriptors);

    QVariantMap stateEvaluator;
    QVariantMap stateDescriptor;
    stateDescriptor.insert("stateTypeId", mockPowerStateTypeId);
    stateDescriptor.insert("operator", "ValueOperatorEquals");
    stateDescriptor.insert("value", false);
    stateDescriptor.insert("thingId", m_mockThingId);
    stateEvaluator.insert("stateDescriptor", stateDescriptor);
    params.insert("stateEvaluator", stateEvaluator);

    QVariantList actions;
    QVariantMap action;
    action.insert("actionTypeId", mockPowerActionTypeId);
    action.insert("thingId", m_mockThingId);
    QVariantList actionParams;
    QVariantMap actionParam;
    actionParam.insert("paramTypeId", mockPowerActionPowerParamTypeId);
    actionParam.insert("value", true);
    actionParams.append(actionParam);
    action.insert("ruleActionParams", actionParams);
    actions.append(action);
    params.insert("actions", actions);

    QVariantList exitActions;
    QVariantMap exitAction;
    exitAction.insert("actionTypeId", mockPowerActionTypeId);
    exitAction.insert("thingId", m_mockThingId);
    QVariantList exitActionParams;
    QVariantMap exitActionParam;
    exitActionParam.insert("paramTypeId", mockPowerActionPowerParamTypeId);
    exitActionParam.insert("value", false);
    exitActionParams.append(exitActionParam);
    exitAction.insert("ruleActionParams", exitActionParams);
    exitActions.append(exitAction);
    params.insert("exitActions", exitActions);

    QVariant response = injectAndWait("Rules.AddRule", params);
    RuleId ruleId = RuleId(response.toMap().value("params").toMap().value("ruleId").toUuid());
    QVERIFY2(!ruleId.isNull(), "Error adding rule");

    // Get the current state value, make sure it's false
    params.clear();
    params.insert("thingId", m_mockThingId);
    params.insert("stateTypeId", mockPowerStateTypeId);

    response = injectAndWait("Integrations.GetStateValue", params);
    QVERIFY2(response.toMap().value("params").toMap().value("value").toBool() == false, "State initially true while it should be false");


    // Trigger the event
    generateEvent(mockEvent1EventTypeId);


    // Make sure the value changed after the event has triggered
    response = injectAndWait("Integrations.GetStateValue", params);
    QVERIFY2(response.toMap().value("params").toMap().value("value").toBool() == true, "State is false while it should have changed to true");


    // Trigger the event again...
    generateEvent(mockEvent1EventTypeId);


    // ... and make sure the value changed back to false
    response = injectAndWait("Integrations.GetStateValue", params);
    QVERIFY2(response.toMap().value("params").toMap().value("value").toBool() == false, "State is true while it should have changed to false");

    restartServer();

    // Make sure the value changed is still false
    response = injectAndWait("Integrations.GetStateValue", params);
    QVERIFY2(response.toMap().value("params").toMap().value("value").toBool() == false, "State is true while it should have changed to false");

    // Trigger the event
    generateEvent(mockEvent1EventTypeId);


    // Make sure the value changed after the event has triggered
    response = injectAndWait("Integrations.GetStateValue", params);
    QVERIFY2(response.toMap().value("params").toMap().value("value").toBool() == true, "State is false while it should have changed to true");

}

void TestRules::testInterfaceBasedEventRule()
{
    QNetworkAccessManager nam;
    QSignalSpy spy(&nam, SIGNAL(finished(QNetworkReply*)));

    // state battery critical state to false initially
    QNetworkRequest request(QUrl(QString("http://localhost:%1/setstate?%2=%3").arg(m_mockThing1Port).arg(mockBatteryCriticalStateTypeId.toString()).arg(false)));
    QNetworkReply *reply = nam.get(request);
    spy.wait();
    QCOMPARE(spy.count(), 1);
    reply->deleteLater();

    QVariantMap powerAction;
    powerAction.insert("interface", "light");
    powerAction.insert("interfaceAction", "power");
    QVariantMap powerActionParam;
    powerActionParam.insert("paramName", "power");
    powerActionParam.insert("value", true);
    powerAction.insert("ruleActionParams", QVariantList() << powerActionParam);

    QVariantMap lowBatteryEvent;
    lowBatteryEvent.insert("interface", "battery");
    lowBatteryEvent.insert("interfaceEvent", "batteryCritical");
    QVariantMap eventParams;
    eventParams.insert("paramName", "batteryCritical");
    eventParams.insert("value", true);
    eventParams.insert("operator", "ValueOperatorEquals");
    lowBatteryEvent.insert("paramDescriptors", QVariantList() << eventParams);

    QVariantMap addRuleParams;
    addRuleParams.insert("name", "TestInterfaceBasedRule");
    addRuleParams.insert("enabled", true);
    addRuleParams.insert("actions", QVariantList() << powerAction);
    addRuleParams.insert("eventDescriptors", QVariantList() << lowBatteryEvent);

    qDebug(dcTests) << "Inserting rule";

    QVariant response = injectAndWait("Rules.AddRule", addRuleParams);
    QCOMPARE(response.toMap().value("status").toString(), QString("success"));
    QCOMPARE(response.toMap().value("params").toMap().value("ruleError").toString(), QString("RuleErrorNoError"));

    QVariantMap getRuleParams;
    getRuleParams.insert("ruleId", response.toMap().value("params").toMap().value("ruleId"));

    qDebug(dcTests) << "Getting rule details";

    response = injectAndWait("Rules.GetRuleDetails", getRuleParams);

    QCOMPARE(response.toMap().value("params").toMap().value("ruleError").toString(), QString("RuleErrorNoError"));

    QCOMPARE(response.toMap().value("params").toMap().value("rule").toMap().value("eventDescriptors").toList().first().toMap().value("interface").toString(), QString("battery"));
    QCOMPARE(response.toMap().value("params").toMap().value("rule").toMap().value("eventDescriptors").toList().first().toMap().value("interfaceEvent").toString(), QString("batteryCritical"));

    QCOMPARE(response.toMap().value("params").toMap().value("rule").toMap().value("actions").toList().first().toMap().value("interface").toString(), QString("light"));
    QCOMPARE(response.toMap().value("params").toMap().value("rule").toMap().value("actions").toList().first().toMap().value("interfaceAction").toString(), QString("power"));
    QCOMPARE(response.toMap().value("params").toMap().value("rule").toMap().value("actions").toList().first().toMap().value("ruleActionParams").toList().first().toMap().value("paramName").toString(), QString("power"));
    QCOMPARE(response.toMap().value("params").toMap().value("rule").toMap().value("actions").toList().first().toMap().value("ruleActionParams").toList().first().toMap().value("value").toString(), QString("true"));

    qDebug(dcTests) << "Clearing action history";

    // Change the state to true, action should trigger
    spy.clear();
    request = QNetworkRequest(QUrl(QString("http://localhost:%1/clearactionhistory").arg(m_mockThing1Port)));
    reply = nam.get(request);

    qDebug(dcTests) << "Changing battery state -> true";

    spy.wait(); spy.clear();
    request = QNetworkRequest(QUrl(QString("http://localhost:%1/setstate?%2=%3").arg(m_mockThing1Port).arg(mockBatteryCriticalStateTypeId.toString()).arg(true)));
    reply = nam.get(request);
    spy.wait();
    QCOMPARE(spy.count(), 1);
    reply->deleteLater();

    verifyRuleExecuted(mockPowerActionTypeId);

    qDebug(dcTests) << "Clearing action history";

    // Change the state to false, action should not trigger
    spy.clear();
    request = QNetworkRequest(QUrl(QString("http://localhost:%1/clearactionhistory").arg(m_mockThing1Port)));
    reply = nam.get(request);

    qDebug(dcTests) << "Changing battery state -> false";

    spy.wait(); spy.clear();
    request = QNetworkRequest(QUrl(QString("http://localhost:%1/setstate?%2=%3").arg(m_mockThing1Port).arg(mockBatteryCriticalStateTypeId.toString()).arg(false)));
    reply = nam.get(request);
    spy.wait();
    QCOMPARE(spy.count(), 1);
    reply->deleteLater();
    verifyRuleNotExecuted();
}

void TestRules::testInterfaceBasedStateRule()
{
    QNetworkAccessManager nam;
    QSignalSpy spy(&nam, SIGNAL(finished(QNetworkReply*)));

    // state battery critical state to false initially
    QNetworkRequest request(QUrl(QString("http://localhost:%1/setstate?%2=%3").arg(m_mockThing1Port).arg(mockBatteryCriticalStateTypeId.toString()).arg(false)));
    QNetworkReply *reply = nam.get(request);
    spy.wait();
    QCOMPARE(spy.count(), 1);
    reply->deleteLater();

    QVariantMap powerAction;
    powerAction.insert("interface", "light");
    powerAction.insert("interfaceAction", "power");
    QVariantMap powerActionParam;
    powerActionParam.insert("paramName", "power");
    powerActionParam.insert("value", true);
    powerAction.insert("ruleActionParams", QVariantList() << powerActionParam);

    QVariantMap lowBatteryState;
    lowBatteryState.insert("interface", "battery");
    lowBatteryState.insert("interfaceState", "batteryCritical");

    QVariantMap stateDescriptor;
    stateDescriptor.insert("interface", "battery");
    stateDescriptor.insert("interfaceState", "batteryCritical");
    stateDescriptor.insert("value", true);
    stateDescriptor.insert("operator", "ValueOperatorEquals");

    QVariantMap stateEvaluator;
    stateEvaluator.insert("stateDescriptor", stateDescriptor);

    QVariantMap addRuleParams;
    addRuleParams.insert("name", "TestInterfaceBasedStateRule");
    addRuleParams.insert("enabled", true);
    addRuleParams.insert("stateEvaluator", stateEvaluator);
    addRuleParams.insert("actions", QVariantList() << powerAction);

    QVariant response = injectAndWait("Rules.AddRule", addRuleParams);
    QCOMPARE(response.toMap().value("status").toString(), QString("success"));
    QCOMPARE(response.toMap().value("params").toMap().value("ruleError").toString(), QString("RuleErrorNoError"));

    QVariantMap getRuleParams;
    getRuleParams.insert("ruleId", response.toMap().value("params").toMap().value("ruleId"));
    response = injectAndWait("Rules.GetRuleDetails", getRuleParams);

    QCOMPARE(response.toMap().value("params").toMap().value("ruleError").toString(), QString("RuleErrorNoError"));

    QCOMPARE(response.toMap().value("params").toMap().value("rule").toMap().value("stateEvaluator").toMap().value("stateDescriptor").toMap().value("interface").toString(), QString("battery"));
    QCOMPARE(response.toMap().value("params").toMap().value("rule").toMap().value("stateEvaluator").toMap().value("stateDescriptor").toMap().value("interfaceState").toString(), QString("batteryCritical"));

    QCOMPARE(response.toMap().value("params").toMap().value("rule").toMap().value("actions").toList().first().toMap().value("interface").toString(), QString("light"));
    QCOMPARE(response.toMap().value("params").toMap().value("rule").toMap().value("actions").toList().first().toMap().value("interfaceAction").toString(), QString("power"));
    QCOMPARE(response.toMap().value("params").toMap().value("rule").toMap().value("actions").toList().first().toMap().value("ruleActionParams").toList().first().toMap().value("paramName").toString(), QString("power"));
    QCOMPARE(response.toMap().value("params").toMap().value("rule").toMap().value("actions").toList().first().toMap().value("ruleActionParams").toList().first().toMap().value("value").toString(), QString("true"));


    // Change the state
    spy.clear();
    request = QNetworkRequest(QUrl(QString("http://localhost:%1/setstate?%2=%3").arg(m_mockThing1Port).arg(mockBatteryCriticalStateTypeId.toString()).arg(true)));
    reply = nam.get(request);
    spy.wait();
    QCOMPARE(spy.count(), 1);
    reply->deleteLater();

    verifyRuleExecuted(mockPowerActionTypeId);
}

void TestRules::testLoopingRules()
{
    QVariantMap powerOnActionParam;
    powerOnActionParam.insert("paramTypeId", mockPowerStateTypeId);
    powerOnActionParam.insert("value", true);

    QVariantMap powerOffActionParam;
    powerOffActionParam.insert("paramTypeId", mockPowerStateTypeId);
    powerOffActionParam.insert("value", false);

    QVariantMap powerOnEventParam = powerOnActionParam;
    powerOnEventParam.insert("operator", "ValueOperatorEquals");

    QVariantMap powerOffEventParam = powerOffActionParam;
    powerOffEventParam.insert("operator", "ValueOperatorEquals");

    QVariantMap onEvent;
    onEvent.insert("eventTypeId", mockPowerStateTypeId);
    onEvent.insert("thingId", m_mockThingId);
    onEvent.insert("paramDescriptors", QVariantList() << powerOnEventParam);

    QVariantMap offEvent;
    offEvent.insert("eventTypeId", mockPowerStateTypeId);
    offEvent.insert("thingId", m_mockThingId);
    offEvent.insert("paramDescriptors", QVariantList() << powerOffEventParam);

    QVariantMap onAction;
    onAction.insert("actionTypeId", mockPowerStateTypeId);
    onAction.insert("thingId", m_mockThingId);
    onAction.insert("ruleActionParams", QVariantList() << powerOnActionParam);

    QVariantMap offAction;
    offAction.insert("actionTypeId", mockPowerStateTypeId);
    offAction.insert("thingId", m_mockThingId);
    offAction.insert("ruleActionParams", QVariantList() << powerOffActionParam);

    // Add rule 1
    QVariantMap addRuleParams;
    addRuleParams.insert("name", "Rule off -> on");
    addRuleParams.insert("eventDescriptors", QVariantList() << offEvent);
    addRuleParams.insert("actions", QVariantList() << onAction);
    QVariant response = injectAndWait("Rules.AddRule", addRuleParams);
    verifyRuleError(response);

    // Add rule 1
    addRuleParams.clear();
    addRuleParams.insert("name", "Rule on -> off");
    addRuleParams.insert("eventDescriptors", QVariantList() << onEvent);
    addRuleParams.insert("actions", QVariantList() << offAction);
    response = injectAndWait("Rules.AddRule", addRuleParams);
    verifyRuleError(response);

    cleanupMockHistory();

    QVariantMap params;
    params.insert("thingId", m_mockThingId);
    params.insert("actionTypeId", mockPowerStateTypeId);
    params.insert("params", QVariantList() << powerOffActionParam);
    response = injectAndWait("Integrations.ExecuteAction", params);
    verifyRuleExecuted(mockPowerActionTypeId);

    cleanupMockHistory();

    params.clear();
    params.insert("thingId", m_mockThingId);
    params.insert("actionTypeId", mockPowerStateTypeId);
    params.insert("params", QVariantList() << powerOnActionParam);
    response = injectAndWait("Integrations.ExecuteAction", params);
    verifyRuleExecuted(mockPowerActionTypeId);

    // No need to check anything else. This test sets up a binding loop and if the core doesn't catch it it'll crash here.
}

void TestRules::testScene()
{
    // Given scenes are rules without stateEvaluator and eventDescriptors, they evaluate to true when asked for "active()"
    // This test should catch the case where such a rule might wrongly be exected by evaluating it
    // when another state change happens or when a time event is evaluated

    NymeaCore::instance()->timeManager()->stopTimer();
    QDateTime now = QDateTime::currentDateTime();
    NymeaCore::instance()->timeManager()->setTime(now);

    QNetworkAccessManager nam;
    QSignalSpy spy(&nam, SIGNAL(finished(QNetworkReply*)));

    // state power state to false initially
    QNetworkRequest request(QUrl(QString("http://localhost:%1/setstate?%2=%3").arg(m_mockThing1Port).arg(mockPowerStateTypeId.toString()).arg(false)));
    QNetworkReply *reply = nam.get(request);
    spy.wait();
    QCOMPARE(spy.count(), 1);
    reply->deleteLater();

    // state battery critical state to false initially
    spy.clear();
    request = QNetworkRequest(QUrl(QString("http://localhost:%1/setstate?%2=%3").arg(m_mockThing1Port).arg(mockBatteryCriticalStateTypeId.toString()).arg(false)));
    reply = nam.get(request);
    spy.wait();
    QCOMPARE(spy.count(), 1);
    reply->deleteLater();

    // Add a scene setting power to true
    QVariantMap powerAction;
    powerAction.insert("thingId", m_mockThingId);
    powerAction.insert("actionTypeId", mockPowerStateTypeId);
    QVariantMap powerActionParam;
    powerActionParam.insert("paramTypeId", mockPowerStateTypeId);
    powerActionParam.insert("value", true);
    powerAction.insert("ruleActionParams", QVariantList() << powerActionParam);

    QVariantMap addRuleParams;
    addRuleParams.insert("name", "TestScene");
    addRuleParams.insert("enabled", true);
    addRuleParams.insert("executable", true);
    addRuleParams.insert("actions", QVariantList() << powerAction);

    QVariant response = injectAndWait("Rules.AddRule", addRuleParams);
    QCOMPARE(response.toMap().value("status").toString(), QString("success"));
    QCOMPARE(response.toMap().value("params").toMap().value("ruleError").toString(), QString("RuleErrorNoError"));

    // trigger state change on battery critical
    spy.clear();
    request = QNetworkRequest(QUrl(QString("http://localhost:%1/setstate?%2=%3").arg(m_mockThing1Port).arg(mockBatteryCriticalStateTypeId.toString()).arg(true)));
    reply = nam.get(request);
    spy.wait();
    QCOMPARE(spy.count(), 1);
    reply->deleteLater();

    verifyRuleNotExecuted();

    // Now trigger a time change
    NymeaCore::instance()->timeManager()->setTime(now.addSecs(1));

    verifyRuleNotExecuted();
}

void TestRules::testHousekeeping_data()
{
    QTest::addColumn<bool>("testAction");
    QTest::addColumn<bool>("testExitAction");
    QTest::addColumn<bool>("testStateEvaluator");
    QTest::addColumn<bool>("testEventDescriptor");

    QTest::newRow("action") << true << false << false << false;
    QTest::newRow("exitAction") << false << true << false << false;
    QTest::newRow("stateDescriptor") << false << false << true << false;
    QTest::newRow("eventDescriptor")<< false << false << false << true;
}

void TestRules::testHousekeeping()
{
    QFETCH(bool, testAction);
    QFETCH(bool, testExitAction);
    QFETCH(bool, testStateEvaluator);
    QFETCH(bool, testEventDescriptor);

    QVariantMap params;
    params.insert("thingClassId", mockThingClassId);
    params.insert("name", "TestThingToBeRemoved");
    QVariantList thingParams;
    QVariantMap httpParam;
    httpParam.insert("paramTypeId", mockThingHttpportParamTypeId);
    httpParam.insert("value", 6667);
    thingParams.append(httpParam);
    params.insert("thingParams", thingParams);
    QVariant response = injectAndWait("Integrations.AddThing", params);
    ThingId thingId = ThingId(response.toMap().value("params").toMap().value("thingId").toUuid());
    QVERIFY2(!thingId.isNull(), "Something went wrong creating the thing for testing.");

    // Create a rule with this thing
    params.clear();
    params.insert("name", "testrule");
    if (testEventDescriptor) {
        QVariantList eventDescriptors;
        QVariantMap eventDescriptor;
        eventDescriptor.insert("eventTypeId", mockEvent1EventTypeId);
        eventDescriptor.insert("thingId", testEventDescriptor ? thingId : m_mockThingId);
        eventDescriptors.append(eventDescriptor);
        params.insert("eventDescriptors", eventDescriptors);
    }

    QVariantMap stateEvaluator;
    QVariantMap stateDescriptor;
    stateDescriptor.insert("stateTypeId", mockIntStateTypeId);
    stateDescriptor.insert("operator", "ValueOperatorGreater");
    stateDescriptor.insert("value", 555);
    stateDescriptor.insert("thingId", testStateEvaluator ? thingId : m_mockThingId);
    stateEvaluator.insert("stateDescriptor", stateDescriptor);
    params.insert("stateEvaluator", stateEvaluator);

    QVariantList actions;
    QVariantMap action;
    action.insert("actionTypeId", mockWithoutParamsActionTypeId);
    action.insert("thingId", testAction ? thingId : m_mockThingId);
    actions.append(action);
    params.insert("actions", actions);

    if (!testEventDescriptor) {
        QVariantList exitActions;
        QVariantMap exitAction;
        exitAction.insert("actionTypeId", mockWithoutParamsActionTypeId);
        exitAction.insert("thingId", testExitAction ? thingId : m_mockThingId);
        exitActions.append(exitAction);
        params.insert("exitActions", exitActions);
    }

    response = injectAndWait("Rules.AddRule", params);
    RuleId ruleId = RuleId(response.toMap().value("params").toMap().value("ruleId").toUuid());


    // Verfy that the rule has been created successfully and our thing is in there.
    params.clear();
    params.insert("ruleId", ruleId);
    response = injectAndWait("Rules.GetRuleDetails", params);
    if (testEventDescriptor) {
        QVERIFY2(response.toMap().value("params").toMap().value("rule").toMap().value("eventDescriptors").toList().first().toMap().value("thingId").toUuid().toString() == (testEventDescriptor ? thingId.toString() : m_mockThingId.toString()), "Couldn't find thing in eventDescriptor of rule");
    }
    QVERIFY2(response.toMap().value("params").toMap().value("rule").toMap().value("stateEvaluator").toMap().value("stateDescriptor").toMap().value("thingId").toUuid().toString() == (testStateEvaluator ? thingId.toString() : m_mockThingId.toString()), "Couldn't find thing in stateEvaluator of rule");
    QVERIFY2(response.toMap().value("params").toMap().value("rule").toMap().value("actions").toList().first().toMap().value("thingId").toUuid().toString() == (testAction ? thingId.toString() : m_mockThingId.toString()), "Couldn't find thing in actions of rule");
    if (!testEventDescriptor) {
        QVERIFY2(response.toMap().value("params").toMap().value("rule").toMap().value("exitActions").toList().first().toMap().value("thingId").toUuid().toString() == (testExitAction ? thingId.toString() : m_mockThingId.toString()), "Couldn't find thing in exitActions of rule");
    }

    // Manually delete this thing from config
    NymeaSettings settings(NymeaSettings::SettingsRoleThings);
    settings.beginGroup("ThingConfig");
    settings.remove(thingId.toString());
    settings.endGroup();

    restartServer();

    // Now make sure the appropriate entries with our thing have disappeared
    response = injectAndWait("Rules.GetRuleDetails", params);
    if (testEventDescriptor) {
        QVERIFY2(response.toMap().value("params").toMap().value("rule").toMap().value("eventDescriptors").toList().count() == (testEventDescriptor ? 0: 1), "EventDescriptor still in rule... should've been removed by housekeeping.");
    }
    QVERIFY2(response.toMap().value("params").toMap().value("rule").toMap().value("stateEvaluator").toMap().value("stateDescriptor").toMap().isEmpty() == (testStateEvaluator ? true : false), "StateEvaluator still in rule... should've been removed by housekeeping.");
    QVERIFY2(response.toMap().value("params").toMap().value("rule").toMap().value("actions").toList().count() == (testAction ? 0 : 1), "Action still in rule... should've been removed by housekeeping.");
    if (!testEventDescriptor) {
        QVERIFY2(response.toMap().value("params").toMap().value("rule").toMap().value("exitActions").toList().count() == (testExitAction ? 0: 1), "ExitAction still in rule... should've been removed by housekeeping.");
    }
}

#include "testrules.moc"
QTEST_MAIN(TestRules)
