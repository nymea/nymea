/****************************************************************************
 *                                                                          *
 *  This file is part of guh.                                               *
 *                                                                          *
 *  Guh is free software: you can redistribute it and/or modify             *
 *  it under the terms of the GNU General Public License as published by    *
 *  the Free Software Foundation, version 2 of the License.                 *
 *                                                                          *
 *  Guh is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *  GNU General Public License for more details.                            *
 *                                                                          *
 *  You should have received a copy of the GNU General Public License       *
 *  along with guh.  If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                          *
 ***************************************************************************/

#include "guhtestbase.h"
#include "guhcore.h"
#include "devicemanager.h"
#include "mocktcpserver.h"

#include <QtTest/QtTest>
#include <QCoreApplication>
#include <QTcpSocket>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QCoreApplication>

class TestRules: public GuhTestBase
{
    Q_OBJECT

private:
    void cleanupMockHistory();
    void cleanupRules();

    void verifyRuleExecuted(const ActionTypeId &actionTypeId);
    void verifyRuleNotExecuted();

private slots:

    void cleanup();

    void addRemoveRules_data();
    void addRemoveRules();

    void removeInvalidRule();

    void loadStoreConfig();

    void evaluateEvent();

    void testStateEvaluator_data();
    void testStateEvaluator();

    void testStateEvaluator2_data();
    void testStateEvaluator2();

    void testStateChange();

    void enableDisableRule();
};

void TestRules::cleanupMockHistory() {
    QNetworkAccessManager nam;
    QSignalSpy spy(&nam, SIGNAL(finished(QNetworkReply*)));
    QNetworkRequest request(QUrl(QString("http://localhost:%1/clearactionhistory").arg(m_mockDevice1Port).arg(mockEvent1Id.toString())));
    QNetworkReply *reply = nam.get(request);
    spy.wait();
    QCOMPARE(spy.count(), 1);
    reply->deleteLater();
}

void TestRules::cleanupRules() {
    QVariant response = injectAndWait("Rules.GetRules");
    foreach (const QVariant &ruleId, response.toMap().value("params").toMap().value("ruleIds").toList()) {
        QVariantMap params;
        params.insert("ruleId", ruleId.toString());
        verifyRuleError(injectAndWait("Rules.RemoveRule", params));
    }
}

void TestRules::cleanup() {
    cleanupMockHistory();
    cleanupRules();
}

void TestRules::verifyRuleExecuted(const ActionTypeId &actionTypeId)
{
    // Verify rule got executed
    QNetworkAccessManager nam;
    QSignalSpy spy(&nam, SIGNAL(finished(QNetworkReply*)));
    QNetworkRequest request(QUrl(QString("http://localhost:%1/actionhistory").arg(m_mockDevice1Port)));
    QNetworkReply *reply = nam.get(request);
    spy.wait();
    QCOMPARE(spy.count(), 1);

    QByteArray actionHistory = reply->readAll();
    qDebug() << "have action history" << actionHistory;
    QVERIFY2(actionTypeId == ActionTypeId(actionHistory), "Action not triggered");
    reply->deleteLater();

}

void TestRules::verifyRuleNotExecuted()
{
    QNetworkAccessManager nam;
    QSignalSpy spy(&nam, SIGNAL(finished(QNetworkReply*)));
    QNetworkRequest request(QUrl(QString("http://localhost:%1/actionhistory").arg(m_mockDevice1Port)));
    QNetworkReply *reply = nam.get(request);
    spy.wait();
    QCOMPARE(spy.count(), 1);

    QByteArray actionHistory = reply->readAll();
    qDebug() << "have action history" << actionHistory;
    QVERIFY2(actionHistory.isEmpty(), "Action is triggered while it should not have been.");
    reply->deleteLater();

}

void TestRules::addRemoveRules_data()
{
    QVariantMap validActionNoParams;
    validActionNoParams.insert("actionTypeId", mockActionIdNoParams);
    validActionNoParams.insert("deviceId", m_mockDeviceId);
    validActionNoParams.insert("ruleActionParams", QVariantList());

    QVariantMap invalidAction;
    invalidAction.insert("actionTypeId", ActionTypeId());
    invalidAction.insert("deviceId", m_mockDeviceId);
    invalidAction.insert("ruleActionParams", QVariantList());

    QVariantMap validExitActionNoParams;
    validExitActionNoParams.insert("actionTypeId", mockActionIdNoParams);
    validExitActionNoParams.insert("deviceId", m_mockDeviceId);
    validExitActionNoParams.insert("ruleActionParams", QVariantList());

    QVariantMap invalidExitAction;
    invalidExitAction.insert("actionTypeId", ActionTypeId());
    invalidExitAction.insert("deviceId", m_mockDeviceId);
    invalidExitAction.insert("ruleActionParams", QVariantList());

    QVariantMap stateDescriptor;
    stateDescriptor.insert("stateTypeId", mockIntStateId);
    stateDescriptor.insert("deviceId", m_mockDeviceId);
    stateDescriptor.insert("operator", JsonTypes::valueOperatorToString(Types::ValueOperatorLess));
    stateDescriptor.insert("value", "20");

    QVariantMap validStateEvaluator;
    validStateEvaluator.insert("stateDescriptor", stateDescriptor);
    validStateEvaluator.insert("operator", JsonTypes::stateOperatorToString(Types::StateOperatorAnd));

    QVariantMap invalidStateEvaluator;
    stateDescriptor.remove("deviceId");
    invalidStateEvaluator.insert("stateDescriptor", stateDescriptor);

    QVariantMap validEventDescriptor1;
    validEventDescriptor1.insert("eventTypeId", mockEvent1Id);
    validEventDescriptor1.insert("deviceId", m_mockDeviceId);
    validEventDescriptor1.insert("paramDescriptors", QVariantList());

    QVariantMap validEventDescriptor2;
    validEventDescriptor2.insert("eventTypeId", mockEvent2Id);
    validEventDescriptor2.insert("deviceId", m_mockDeviceId);
    QVariantList params;
    QVariantMap param1;
    param1.insert("name", "mockParamInt");
    param1.insert("value", 3);
    param1.insert("operator", JsonTypes::valueOperatorToString(Types::ValueOperatorEquals));
    params.append(param1);
    validEventDescriptor2.insert("paramDescriptors", params);

    QVariantList eventDescriptorList;
    eventDescriptorList.append(validEventDescriptor1);
    eventDescriptorList.append(validEventDescriptor2);

    QVariantMap invalidEventDescriptor;
    invalidEventDescriptor.insert("eventTypeId", mockEvent1Id);
    invalidEventDescriptor.insert("deviceId", DeviceId());
    invalidEventDescriptor.insert("paramDescriptors", QVariantList());

    QTest::addColumn<bool>("enabled");
    QTest::addColumn<QVariantMap>("action1");
    QTest::addColumn<QVariantMap>("exitAction1");
    QTest::addColumn<QVariantMap>("eventDescriptor");
    QTest::addColumn<QVariantList>("eventDescriptorList");
    QTest::addColumn<QVariantMap>("stateEvaluator");
    QTest::addColumn<RuleEngine::RuleError>("error");
    QTest::addColumn<bool>("jsonError");
    QTest::addColumn<QString>("name");

    QTest::newRow("valid rule. enabled, 1 Action, 1 Exit Action,  1 StateEvaluator, name") << true << validActionNoParams << validExitActionNoParams << QVariantMap() << QVariantList() << validStateEvaluator << RuleEngine::RuleErrorNoError << false << "TestRule";
    QTest::newRow("valid rule. disabled, 1 Action, 1 Exit Action, 1 StateEvaluator, name") << false << validActionNoParams << validExitActionNoParams << QVariantMap() << QVariantList() << validStateEvaluator << RuleEngine::RuleErrorNoError << false << "TestRule";
    QTest::newRow("valid rule. disabled, 1 Action, 1 invalid Exit Action, 1 StateEvaluator, name") << false << validActionNoParams << invalidExitAction << QVariantMap() << QVariantList() << validStateEvaluator << RuleEngine::RuleErrorActionTypeNotFound << false << "TestRule";
    QTest::newRow("invalid rule. 1 Action, 1 Exit Action, 1 EventDescriptor, 1 StateEvaluator, name") << true << validActionNoParams << validExitActionNoParams << validEventDescriptor1 << QVariantList() << validStateEvaluator << RuleEngine::RuleErrorInvalidRuleFormat << false << "TestRule";
    QTest::newRow("invalid rule. 1 Action, 1 Exit Action, eventDescriptorList, 1 StateEvaluator, name") << true << validActionNoParams << validExitActionNoParams << QVariantMap() << eventDescriptorList << validStateEvaluator << RuleEngine::RuleErrorInvalidRuleFormat << false << "TestRule";


    QTest::newRow("valid rule. enabled, 1 EventDescriptor, StateEvaluator, 1 Action, name") << true << validActionNoParams << QVariantMap() << validEventDescriptor1 << QVariantList() << validStateEvaluator << RuleEngine::RuleErrorNoError << false << "TestRule";
    QTest::newRow("valid rule. diabled, 1 EventDescriptor, StateEvaluator, 1 Action, name") << false << validActionNoParams << QVariantMap() << validEventDescriptor1 << QVariantList() << validStateEvaluator << RuleEngine::RuleErrorNoError << false << "TestRule";
    QTest::newRow("valid rule. 2 EventDescriptors, 1 Action, name") << true << validActionNoParams << QVariantMap() << QVariantMap() << eventDescriptorList << validStateEvaluator << RuleEngine::RuleErrorNoError << false << "TestRule";
    QTest::newRow("invalid rule: eventDescriptor and eventDescriptorList used") << true << validActionNoParams << QVariantMap() << validEventDescriptor1 << eventDescriptorList << validStateEvaluator << RuleEngine::RuleErrorInvalidParameter << false << "TestRule";
    QTest::newRow("invalid action") << true << invalidAction << QVariantMap() << validEventDescriptor1 << QVariantList() << validStateEvaluator << RuleEngine::RuleErrorActionTypeNotFound << false << "TestRule";
    QTest::newRow("invalid event descriptor") << true << validActionNoParams << QVariantMap() << invalidEventDescriptor << QVariantList() << validStateEvaluator << RuleEngine::RuleErrorDeviceNotFound << false << "TestRule";
    QTest::newRow("invalid StateDescriptor") << true << validActionNoParams << QVariantMap() << validEventDescriptor1 << QVariantList() << invalidStateEvaluator << RuleEngine::RuleErrorInvalidParameter << true << "TestRule";
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
        params.insert("eventDescriptor", eventDescriptor);
    }
    if (!eventDescriptorList.isEmpty()) {
        params.insert("eventDescriptorList", eventDescriptorList);
    }
    if (!exitAction1.isEmpty()) {
        QVariantList exitActions;
        exitActions.append(exitAction1);
        params.insert("exitActions", exitActions);
    }
    params.insert("stateEvaluator", stateEvaluator);
    if (!enabled) {
        params.insert("enabled", enabled);
    }
    QVariant response = injectAndWait("Rules.AddRule", params);
    if (!jsonError) {
        verifyRuleError(response, error);
    }

    RuleId newRuleId = RuleId(response.toMap().value("params").toMap().value("ruleId").toString());

    response = injectAndWait("Rules.GetRules");
    QVariantList rules = response.toMap().value("params").toMap().value("ruleIds").toList();

    if (error != RuleEngine::RuleErrorNoError) {
        QVERIFY2(rules.count() == 0, "There should be no rules.");
        return;
    }

    QVERIFY2(rules.count() == 1, "There should be exactly one rule");
    QCOMPARE(RuleId(rules.first().toString()), newRuleId);

    params.clear();
    params.insert("ruleId", newRuleId);
    response = injectAndWait("Rules.GetRuleDetails", params);
    //    verifySuccess(response);

    QVariantMap rule = response.toMap().value("params").toMap().value("rule").toMap();

    qDebug() << rule.value("name").toString();
    QVERIFY2(rule.value("enabled").toBool() == enabled, "Rule enabled state doesn't match");
    QVariantList eventDescriptors = rule.value("eventDescriptors").toList();
    if (!eventDescriptor.isEmpty()) {
        QVERIFY2(eventDescriptors.count() == 1, "There shoud be exactly one eventDescriptor");
        QVERIFY2(eventDescriptors.first().toMap() == eventDescriptor, "Event descriptor doesn't match");
    } else if (eventDescriptorList.isEmpty()){
        QVERIFY2(eventDescriptors.count() == eventDescriptorList.count(), QString("There shoud be exactly %1 eventDescriptor").arg(eventDescriptorList.count()).toLatin1().data());
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
    QVERIFY2(actions == replyActions, "Actions don't match");

    params.clear();
    params.insert("ruleId", newRuleId);
    response = injectAndWait("Rules.RemoveRule", params);
    verifyRuleError(response);

    response = injectAndWait("Rules.GetRules");
    rules = response.toMap().value("params").toMap().value("rules").toList();
    QVERIFY2(rules.count() == 0, "There should be no rules.");
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
    eventDescriptor1.insert("eventTypeId", mockEvent1Id);
    eventDescriptor1.insert("deviceId", m_mockDeviceId);
    eventDescriptor1.insert("paramDescriptors", QVariantList());

    QVariantMap eventDescriptor2;
    eventDescriptor2.insert("eventTypeId", mockEvent2Id);
    eventDescriptor2.insert("deviceId", m_mockDeviceId);
    eventDescriptor2.insert("paramDescriptors", QVariantList());
    QVariantList eventParamDescriptors;
    QVariantMap eventParam1;
    eventParam1.insert("name", "mockParamInt");
    eventParam1.insert("value", 3);
    eventParam1.insert("operator", JsonTypes::valueOperatorToString(Types::ValueOperatorEquals));
    eventParamDescriptors.append(eventParam1);
    eventDescriptor2.insert("paramDescriptors", eventParamDescriptors);

    QVariantList eventDescriptorList;
    eventDescriptorList.append(eventDescriptor1);
    eventDescriptorList.append(eventDescriptor2);

    QVariantMap stateEvaluator1;
    QVariantList childEvaluators;

    QVariantMap stateDescriptor2;
    stateDescriptor2.insert("deviceId", m_mockDeviceId);
    stateDescriptor2.insert("operator", JsonTypes::valueOperatorToString(Types::ValueOperatorEquals));
    stateDescriptor2.insert("stateTypeId", mockIntStateId);
    stateDescriptor2.insert("value", 1);
    QVariantMap stateEvaluator2;
    stateEvaluator2.insert("stateDescriptor", stateDescriptor2);
    stateEvaluator2.insert("operator", JsonTypes::stateOperatorToString(Types::StateOperatorAnd));


    QVariantMap stateDescriptor3;
    stateDescriptor3.insert("deviceId", m_mockDeviceId);
    stateDescriptor3.insert("operator", JsonTypes::valueOperatorToString(Types::ValueOperatorEquals));
    stateDescriptor3.insert("stateTypeId", mockBoolStateId);
    stateDescriptor3.insert("value", true);

    QVariantMap stateEvaluator3;
    stateEvaluator3.insert("stateDescriptor", stateDescriptor3);
    stateEvaluator3.insert("operator", JsonTypes::stateOperatorToString(Types::StateOperatorAnd));

    childEvaluators.append(stateEvaluator2);
    childEvaluators.append(stateEvaluator3);
    stateEvaluator1.insert("childEvaluators", childEvaluators);
    stateEvaluator1.insert("operator", JsonTypes::stateOperatorToString(Types::StateOperatorAnd));

    QVariantMap action1;
    action1.insert("actionTypeId", mockActionIdNoParams);
    action1.insert("deviceId", m_mockDeviceId);
    action1.insert("ruleActionParams", QVariantList());

    QVariantMap action2;
    action2.insert("actionTypeId", mockActionIdWithParams);
    qDebug() << "got action id" << mockActionIdWithParams;
    action2.insert("deviceId", m_mockDeviceId);
    QVariantList action2Params;
    QVariantMap action2Param1;
    action2Param1.insert("name", "mockActionParam1");
    action2Param1.insert("value", 5);
    action2Params.append(action2Param1);
    QVariantMap action2Param2;
    action2Param2.insert("name", "mockActionParam2");
    action2Param2.insert("value", true);
    action2Params.append(action2Param2);
    action2.insert("ruleActionParams", action2Params);

    // First rule
    QVariantMap params;
    QVariantList actions;
    actions.append(action1);
    actions.append(action2);
    params.insert("actions", actions);
    params.insert("eventDescriptorList", eventDescriptorList);
    params.insert("stateEvaluator", stateEvaluator1);
    params.insert("name", "TestRule");
    QVariant response = injectAndWait("Rules.AddRule", params);

    RuleId newRuleId = RuleId(response.toMap().value("params").toMap().value("ruleId").toString());
    verifyRuleError(response);

    // Second rule
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

    restartServer();

    response = injectAndWait("Rules.GetRules");

    QVariantList rules = response.toMap().value("params").toMap().value("ruleIds").toList();

    QVERIFY2(rules.count() == 2, "There should be exactly two rule.");
    QVERIFY2(rules.contains(newRuleId.toString()), "Rule 1 should be in ruleIds list.");
    QVERIFY2(rules.contains(newRuleId2.toString()), "Rule 2 should be in ruleIds list.");


    params.clear();
    params.insert("ruleId", newRuleId);
    response = injectAndWait("Rules.GetRuleDetails", params);

    QVariantMap rule1 = response.toMap().value("params").toMap().value("rule").toMap();

    QVariantList eventDescriptors = rule1.value("eventDescriptors").toList();
    QVERIFY2(eventDescriptors.count() == 2, "There shoud be exactly 2 eventDescriptors");
    foreach (const QVariant &expectedEventDescriptorVariant, eventDescriptorList) {
        bool found = false;
        foreach (const QVariant &replyEventDescriptorVariant, eventDescriptors) {
            if (expectedEventDescriptorVariant.toMap().value("eventTypeId") == replyEventDescriptorVariant.toMap().value("eventTypeId") &&
                    expectedEventDescriptorVariant.toMap().value("deviceId") == replyEventDescriptorVariant.toMap().value("deviceId")) {
                found = true;
                QVERIFY2(replyEventDescriptorVariant == expectedEventDescriptorVariant, "EventDescriptor doesn't match");
            }
        }
        QVERIFY2(found, "missing eventdescriptor");
    }

    QVERIFY2(rule1.value("name").toString() == "TestRule", "Loaded wrong name for rule");
    QVariantMap replyStateEvaluator= rule1.value("stateEvaluator").toMap();
    QVariantList replyChildEvaluators = replyStateEvaluator.value("childEvaluators").toList();
    QVERIFY2(replyStateEvaluator.value("operator") == "StateOperatorAnd", "There should be the AND operator.");
    QVERIFY2(replyChildEvaluators.count() == 2, "There shoud be exactly 2 childEvaluators");

    foreach (const QVariant &childEvaluator, replyChildEvaluators) {
        QVERIFY2(childEvaluator.toMap().contains("stateDescriptor"), "StateDescriptor missing in StateEvaluator");
        QVariantMap stateDescriptor = childEvaluator.toMap().value("stateDescriptor").toMap();
        QVERIFY2(stateDescriptor.value("deviceId") == m_mockDeviceId, "DeviceId of stateDescriptor does not match");
        QVERIFY2(stateDescriptor.value("stateTypeId") == mockIntStateId || stateDescriptor.value("stateTypeId") == mockBoolStateId, "StateTypeId of stateDescriptor doesn't match");
    }

    QVariantList replyActions = rule1.value("actions").toList();
    qDebug() << "----------------------------------------------";
    qDebug() << rule1;
    qDebug() << "----------------------------------------------";

    foreach (const QVariant &actionVariant, actions) {
        bool found = false;
        foreach (const QVariant &replyActionVariant, replyActions) {
            if (actionVariant.toMap().value("actionTypeId") == replyActionVariant.toMap().value("actionTypeId") &&
                    actionVariant.toMap().value("deviceId") == replyActionVariant.toMap().value("deviceId")) {
                found = true;
                QJsonDocument bDoc = QJsonDocument::fromVariant(actionVariant);
                QString bString = bDoc.toJson();
                QJsonDocument aDoc = QJsonDocument::fromVariant(replyActionVariant);
                QString aString = aDoc.toJson();
                QVERIFY2(actionVariant == replyActionVariant, QString("Action doesn't match after loading from config.\nBefore storing: %1\nAfter storing:%2").arg(bString).arg(aString).toUtf8().data());
            }
        }
        QVERIFY2(found, "Action not found after loading from config.");
    }


    params.clear();
    params.insert("ruleId", newRuleId2);
    response = injectAndWait("Rules.GetRuleDetails", params);

    QVariantMap rule2 = response.toMap().value("params").toMap().value("rule").toMap();

    QVERIFY2(rule2.value("name").toString() == "TestRule2", "Loaded wrong name for rule");
    QVariantMap replyStateEvaluator2= rule2.value("stateEvaluator").toMap();
    QVariantList replyChildEvaluators2 = replyStateEvaluator.value("childEvaluators").toList();
    QVERIFY2(replyStateEvaluator2.value("operator") == "StateOperatorAnd", "There should be the AND operator.");
    QVERIFY2(replyChildEvaluators2.count() == 2, "There shoud be exactly 2 childEvaluators");

    foreach (const QVariant &childEvaluator, replyChildEvaluators2) {
        QVERIFY2(childEvaluator.toMap().contains("stateDescriptor"), "StateDescriptor missing in StateEvaluator");
        QVariantMap stateDescriptor = childEvaluator.toMap().value("stateDescriptor").toMap();
        QVERIFY2(stateDescriptor.value("deviceId") == m_mockDeviceId, "DeviceId of stateDescriptor does not match");
        QVERIFY2(stateDescriptor.value("stateTypeId") == mockIntStateId || stateDescriptor.value("stateTypeId") == mockBoolStateId, "StateTypeId of stateDescriptor doesn't match");
    }

    QVariantList replyActions2 = rule2.value("actions").toList();
    QVERIFY2(replyActions2.count() == 1, "Rule 2 should have exactly 1 action");
    foreach (const QVariant &actionVariant, actions2) {
        bool found = false;
        foreach (const QVariant &replyActionVariant, replyActions2) {
            if (actionVariant.toMap().value("actionTypeId") == replyActionVariant.toMap().value("actionTypeId") &&
                    actionVariant.toMap().value("deviceId") == replyActionVariant.toMap().value("deviceId")) {
                found = true;
                QJsonDocument bDoc = QJsonDocument::fromVariant(actionVariant);
                QString bString = bDoc.toJson();
                QJsonDocument aDoc = QJsonDocument::fromVariant(replyActionVariant);
                QString aString = aDoc.toJson();
                QVERIFY2(actionVariant == replyActionVariant, QString("Action doesn't match after loading from config.\nBefore storing: %1\nAfter storing:%2").arg(bString).arg(aString).toUtf8().data());
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
                    exitActionVariant.toMap().value("deviceId") == replyActionVariant.toMap().value("deviceId")) {
                found = true;
                QJsonDocument bDoc = QJsonDocument::fromVariant(exitActionVariant);
                QString bString = bDoc.toJson();
                QJsonDocument aDoc = QJsonDocument::fromVariant(replyActionVariant);
                QString aString = aDoc.toJson();
                QVERIFY2(exitActionVariant == replyActionVariant, QString("Action doesn't match after loading from config.\nBefore storing: %1\nAfter storing:%2").arg(bString).arg(aString).toUtf8().data());
            }
        }
        QVERIFY2(found, "Exit Action not found after loading from config.");
    }

    params.clear();
    params.insert("ruleId", newRuleId);
    response = injectAndWait("Rules.RemoveRule", params);
    verifyRuleError(response);

    params2.clear();
    params2.insert("ruleId", newRuleId2);
    response = injectAndWait("Rules.RemoveRule", params2);
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
    event1.insert("eventTypeId", mockEvent1Id);
    event1.insert("deviceId", m_mockDeviceId);
    events.append(event1);
    addRuleParams.insert("eventDescriptorList", events);

    QVariantList actions;
    QVariantMap action;
    action.insert("actionTypeId", mockActionIdNoParams);
    action.insert("deviceId", m_mockDeviceId);
    actions.append(action);
    addRuleParams.insert("actions", actions);
    QVariant response = injectAndWait("Rules.AddRule", addRuleParams);
    verifyRuleError(response);

    // Trigger an event
    QNetworkAccessManager nam;
    QSignalSpy spy(&nam, SIGNAL(finished(QNetworkReply*)));

    // trigger event in mock device
    QNetworkRequest request(QUrl(QString("http://localhost:%1/generateevent?eventtypeid=%2").arg(m_mockDevice1Port).arg(mockEvent1Id.toString())));
    QNetworkReply *reply = nam.get(request);
    spy.wait();
    QCOMPARE(spy.count(), 1);
    reply->deleteLater();

    verifyRuleExecuted(mockActionIdNoParams);
}

void TestRules::testStateChange() {
    // Add a rule
    QVariantMap addRuleParams;
    QVariantMap stateEvaluator;
    QVariantMap stateDescriptor;
    stateDescriptor.insert("deviceId", m_mockDeviceId);
    stateDescriptor.insert("operator", JsonTypes::valueOperatorToString(Types::ValueOperatorGreaterOrEqual));
    stateDescriptor.insert("stateTypeId", mockIntStateId);
    stateDescriptor.insert("value", 42);
    stateEvaluator.insert("stateDescriptor", stateDescriptor);
    addRuleParams.insert("stateEvaluator", stateEvaluator);
    addRuleParams.insert("name", "TestRule");

    QVariantList actions;
    QVariantMap action;
    action.insert("actionTypeId", mockActionIdNoParams);
    action.insert("deviceId", m_mockDeviceId);
    actions.append(action);
    addRuleParams.insert("actions", actions);
    QVariant response = injectAndWait("Rules.AddRule", addRuleParams);
    verifyRuleError(response);


    // Change the state
    QNetworkAccessManager nam;
    QSignalSpy spy(&nam, SIGNAL(finished(QNetworkReply*)));

    // state state to 42
    qDebug() << "setting mock int state to 42";
    QNetworkRequest request(QUrl(QString("http://localhost:%1/setstate?%2=%3").arg(m_mockDevice1Port).arg(mockIntStateId.toString()).arg(42)));
    QNetworkReply *reply = nam.get(request);
    spy.wait();
    QCOMPARE(spy.count(), 1);
    reply->deleteLater();

    verifyRuleExecuted(mockActionIdNoParams);

    cleanupMockHistory();

    // set state to 45
    qDebug() << "setting mock int state to 45";
    spy.clear();
    request.setUrl(QUrl(QString("http://localhost:%1/setstate?%2=%3").arg(m_mockDevice1Port).arg(mockIntStateId.toString()).arg(45)));
    reply = nam.get(request);
    spy.wait();
    QCOMPARE(spy.count(), 1);
    reply->deleteLater();

    verifyRuleNotExecuted();

    cleanupMockHistory();

    // set state to 30
    qDebug() << "setting mock int state to 30";
    spy.clear();
    request.setUrl(QUrl(QString("http://localhost:%1/setstate?%2=%3").arg(m_mockDevice1Port).arg(mockIntStateId.toString()).arg(30)));
    reply = nam.get(request);
    spy.wait();
    QCOMPARE(spy.count(), 1);
    reply->deleteLater();

    verifyRuleNotExecuted();

    cleanupMockHistory();

    // set state to 100
    qDebug() << "setting mock int state to 100";
    spy.clear();
    request.setUrl(QUrl(QString("http://localhost:%1/setstate?%2=%3").arg(m_mockDevice1Port).arg(mockIntStateId.toString()).arg(100)));
    reply = nam.get(request);
    spy.wait();
    QCOMPARE(spy.count(), 1);

    verifyRuleExecuted(mockActionIdNoParams);
    reply->deleteLater();
}

void TestRules::testStateEvaluator_data()
{
    QTest::addColumn<DeviceId>("deviceId");
    QTest::addColumn<StateTypeId>("stateTypeId");
    QTest::addColumn<QVariant>("value");
    QTest::addColumn<Types::ValueOperator>("operatorType");
    QTest::addColumn<bool>("shouldMatch");

    QTest::newRow("invalid stateId") << m_mockDeviceId << StateTypeId::createStateTypeId() << QVariant(10) << Types::ValueOperatorEquals << false;
    QTest::newRow("invalid deviceId") << DeviceId::createDeviceId() << mockIntStateId << QVariant(10) << Types::ValueOperatorEquals << false;

    QTest::newRow("equals, not matching") << m_mockDeviceId << mockIntStateId << QVariant(7777) << Types::ValueOperatorEquals << false;
    QTest::newRow("equals, matching") << m_mockDeviceId << mockIntStateId << QVariant(10) << Types::ValueOperatorEquals << true;

    QTest::newRow("not equal, not matching") << m_mockDeviceId << mockIntStateId << QVariant(10) << Types::ValueOperatorNotEquals << false;
    QTest::newRow("not equal, matching") << m_mockDeviceId << mockIntStateId << QVariant(7777) << Types::ValueOperatorNotEquals << true;

    QTest::newRow("Greater, not matching") << m_mockDeviceId << mockIntStateId << QVariant(7777) << Types::ValueOperatorGreater << false;
    QTest::newRow("Greater, matching") << m_mockDeviceId << mockIntStateId << QVariant(2) << Types::ValueOperatorGreater << true;
    QTest::newRow("GreaterOrEqual, not matching") << m_mockDeviceId << mockIntStateId << QVariant(7777) << Types::ValueOperatorGreaterOrEqual << false;
    QTest::newRow("GreaterOrEqual, matching (greater)") << m_mockDeviceId << mockIntStateId << QVariant(2) << Types::ValueOperatorGreaterOrEqual << true;
    QTest::newRow("GreaterOrEqual, matching (equals)") << m_mockDeviceId << mockIntStateId << QVariant(10) << Types::ValueOperatorGreaterOrEqual << true;

    QTest::newRow("Less, not matching") << m_mockDeviceId << mockIntStateId << QVariant(2) << Types::ValueOperatorLess << false;
    QTest::newRow("Less, matching") << m_mockDeviceId << mockIntStateId << QVariant(7777) << Types::ValueOperatorLess << true;
    QTest::newRow("LessOrEqual, not matching") << m_mockDeviceId << mockIntStateId << QVariant(2) << Types::ValueOperatorLessOrEqual << false;
    QTest::newRow("LessOrEqual, matching (less)") << m_mockDeviceId << mockIntStateId << QVariant(777) << Types::ValueOperatorLessOrEqual << true;
    QTest::newRow("LessOrEqual, matching (equals)") << m_mockDeviceId << mockIntStateId << QVariant(10) << Types::ValueOperatorLessOrEqual << true;
}

void TestRules::testStateEvaluator()
{
    QFETCH(DeviceId, deviceId);
    QFETCH(StateTypeId, stateTypeId);
    QFETCH(QVariant, value);
    QFETCH(Types::ValueOperator, operatorType);
    QFETCH(bool, shouldMatch);

    StateDescriptor descriptor(stateTypeId, deviceId, value, operatorType);
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

    StateDescriptor descriptor1(mockIntStateId, m_mockDeviceId, intValue, intOperator);
    StateEvaluator evaluator1(descriptor1);

    StateDescriptor descriptor2(mockBoolStateId, m_mockDeviceId, boolValue, boolOperator);
    StateEvaluator evaluator2(descriptor2);

    QList<StateEvaluator> childEvaluators;
    childEvaluators.append(evaluator1);
    childEvaluators.append(evaluator2);

    StateEvaluator mainEvaluator(childEvaluators);
    mainEvaluator.setOperatorType(stateOperator);

    QVERIFY2(mainEvaluator.evaluate() == shouldMatch, shouldMatch ? "State should match" : "State shouldn't match");
}

void TestRules::enableDisableRule()
{
    // Add a rule
    QVariantMap addRuleParams;
    QVariantList events;
    QVariantMap event1;
    event1.insert("eventTypeId", mockEvent1Id);
    event1.insert("deviceId", m_mockDeviceId);
    events.append(event1);
    addRuleParams.insert("eventDescriptorList", events);
    addRuleParams.insert("name", "TestRule");

    QVariantList actions;
    QVariantMap action;
    action.insert("actionTypeId", mockActionIdNoParams);
    action.insert("deviceId", m_mockDeviceId);
    actions.append(action);
    addRuleParams.insert("actions", actions);
    QVariant response = injectAndWait("Rules.AddRule", addRuleParams);
    verifyRuleError(response);
    RuleId id = RuleId(response.toMap().value("params").toMap().value("ruleId").toString());

    // Trigger an event
    QNetworkAccessManager nam;
    QSignalSpy spy(&nam, SIGNAL(finished(QNetworkReply*)));

    // trigger event in mock device
    QNetworkRequest request(QUrl(QString("http://localhost:%1/generateevent?eventtypeid=%2").arg(m_mockDevice1Port).arg(mockEvent1Id.toString())));
    QNetworkReply *reply = nam.get(request);
    spy.wait();
    QCOMPARE(spy.count(), 1);
    reply->deleteLater();

    verifyRuleExecuted(mockActionIdNoParams);

    cleanupMockHistory();

    // Now disable the rule
    QVariantMap disableParams;
    disableParams.insert("ruleId", id.toString());
    response = injectAndWait("Rules.DisableRule", disableParams);
    verifyRuleError(response);


    // trigger event in mock device
    spy.clear();
    request = QNetworkRequest(QUrl(QString("http://localhost:%1/generateevent?eventtypeid=%2").arg(m_mockDevice1Port).arg(mockEvent1Id.toString())));
    reply = nam.get(request);
    spy.wait();
    QCOMPARE(spy.count(), 1);
    reply->deleteLater();

    verifyRuleNotExecuted();

    cleanupMockHistory();

    // Now enable the rule again
    response = injectAndWait("Rules.EnableRule", disableParams);
    verifyRuleError(response);


    // trigger event in mock device
    spy.clear();
    request = QNetworkRequest(QUrl(QString("http://localhost:%1/generateevent?eventtypeid=%2").arg(m_mockDevice1Port).arg(mockEvent1Id.toString())));
    reply = nam.get(request);
    spy.wait();
    QCOMPARE(spy.count(), 1);
    reply->deleteLater();

    verifyRuleExecuted(mockActionIdNoParams);
}

#include "testrules.moc"
QTEST_MAIN(TestRules)
