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

private slots:
    void addRules_data();
    void addRules();

    void loadStoreConfig();

    void evaluateEvent();
};

void TestRules::addRules_data()
{
    QVariantMap validAction;
    validAction.insert("actionTypeId", mockActionIdNoParams);
    validAction.insert("deviceId", m_mockDeviceId);
    validAction.insert("params", QVariantList());

    QVariantMap invalidAction;
    invalidAction.insert("actionTypeId", ActionTypeId());
    invalidAction.insert("deviceId", m_mockDeviceId);
    invalidAction.insert("params", QVariantList());

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
    param1.insert("operator", "OperatorTypeEquals");
    params.append(param1);
    validEventDescriptor2.insert("paramDescriptors", params);

    QVariantList eventDescriptorList;
    eventDescriptorList.append(validEventDescriptor1);
    eventDescriptorList.append(validEventDescriptor2);

    QVariantMap invalidEventDescriptor;
    invalidEventDescriptor.insert("eventTypeId", mockEvent1Id);
    invalidEventDescriptor.insert("deviceId", DeviceId());
    invalidEventDescriptor.insert("paramDescriptors", QVariantList());

    QTest::addColumn<QVariantMap>("action1");
    QTest::addColumn<QVariantMap>("eventDescriptor");
    QTest::addColumn<QVariantList>("eventDescriptorList");
    QTest::addColumn<bool>("success");


    QTest::newRow("valid rule. 1 EventDescriptor, 1 Action") << validAction << validEventDescriptor1 << QVariantList() << true;
    QTest::newRow("valid rule. 2 EventDescriptors, 1 Action") << validAction << QVariantMap() << eventDescriptorList << true;
    QTest::newRow("invalid rule: eventDescriptor and eventDescriptorList used") << validAction << validEventDescriptor1 << eventDescriptorList << false;
    QTest::newRow("invalid action") << invalidAction << validEventDescriptor1 << QVariantList() << false;
    QTest::newRow("invalid event descriptor") << validAction << invalidEventDescriptor << QVariantList() << false;

}

void TestRules::addRules()
{
    QFETCH(QVariantMap, action1);
    QFETCH(QVariantMap, eventDescriptor);
    QFETCH(QVariantList, eventDescriptorList);
    QFETCH(bool, success);

    QVariantMap params;
    QVariantList actions;
    actions.append(action1);
    params.insert("actions", actions);

    if (!eventDescriptor.isEmpty()) {
        params.insert("eventDescriptor", eventDescriptor);
    }
    if (!eventDescriptorList.isEmpty()) {
        params.insert("eventDescriptorList", eventDescriptorList);
    }
    QVariant response = injectAndWait("Rules.AddRule", params);

    RuleId newRuleId = RuleId(response.toMap().value("params").toMap().value("ruleId").toString());
    verifySuccess(response, success);

    response = injectAndWait("Rules.GetRules");
    QVariantList rules = response.toMap().value("params").toMap().value("rules").toList();

    if (!success) {
        QVERIFY2(rules.count() == 0, "There should be no rules.");
        return;
    }

    QVERIFY2(rules.count() == 1, "There should be exactly one rule");
    QCOMPARE(RuleId(rules.first().toMap().value("id").toString()), newRuleId);


    QVariantList eventDescriptors = rules.first().toMap().value("eventDescriptors").toList();
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

    QVariantList replyActions = rules.first().toMap().value("actions").toList();
    QVERIFY2(actions == replyActions, "Actions don't match");


    params.clear();
    params.insert("ruleId", newRuleId);
    response = injectAndWait("Rules.RemoveRule", params);
    verifySuccess(response, true);

    response = injectAndWait("Rules.GetRules");
    rules = response.toMap().value("params").toMap().value("rules").toList();
    QVERIFY2(rules.count() == 0, "There should be no rules.");
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
    eventParam1.insert("operator", "OperatorTypeEquals");
    eventParamDescriptors.append(eventParam1);
    eventDescriptor2.insert("paramDescriptors", eventParamDescriptors);

    QVariantList eventDescriptorList;
    eventDescriptorList.append(eventDescriptor1);
    eventDescriptorList.append(eventDescriptor2);

    QVariantMap action1;
    action1.insert("actionTypeId", mockActionIdNoParams);
    action1.insert("deviceId", m_mockDeviceId);
    action1.insert("params", QVariantList());

    QVariantMap action2;
    action2.insert("actionTypeId", mockActionIdWithParams);
    qDebug() << "got action id" << mockActionIdWithParams;
    action2.insert("deviceId", m_mockDeviceId);
    QVariantList action2Params;
    QVariantMap action2Param1;
    action2Param1.insert("mockActionParam1", 5);
    action2Params.append(action2Param1);
    QVariantMap action2Param2;
    action2Param2.insert("mockActionParam2", true);
    action2Params.append(action2Param2);
    action2.insert("params", action2Params);


    QVariantMap params;
    QVariantList actions;
    actions.append(action1);
    actions.append(action2);
    params.insert("actions", actions);
    params.insert("eventDescriptorList", eventDescriptorList);
    QVariant response = injectAndWait("Rules.AddRule", params);

    RuleId newRuleId = RuleId(response.toMap().value("params").toMap().value("ruleId").toString());
    verifySuccess(response, true);

    restartServer();

    response = injectAndWait("Rules.GetRules");

    QVariantList rules = response.toMap().value("params").toMap().value("rules").toList();

    QVERIFY2(rules.count() == 1, "There should be exactly one rule");
    QCOMPARE(RuleId(rules.first().toMap().value("id").toString()), newRuleId);

    QVariantList eventDescriptors = rules.first().toMap().value("eventDescriptors").toList();
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

    QVariantList replyActions = rules.first().toMap().value("actions").toList();
    foreach (const QVariant &actionVariant, actions) {
        bool found = false;
        foreach (const QVariant &replyActionVariant, replyActions) {
            if (actionVariant.toMap().value("actionTypeId") == replyActionVariant.toMap().value("actionTypeId") &&
                    actionVariant.toMap().value("deviceId") == replyActionVariant.toMap().value("deviceId")) {
                found = true;
                QVERIFY2(actionVariant == replyActionVariant, "Action doesn't match after loading from config.");
            }
        }
        QVERIFY2(found, "Action not found after loading from config.");
    }

    params.clear();
    params.insert("ruleId", newRuleId);
    response = injectAndWait("Rules.RemoveRule", params);
    verifySuccess(response, true);

    restartServer();

    response = injectAndWait("Rules.GetRules");
    rules = response.toMap().value("params").toMap().value("rules").toList();
    QVERIFY2(rules.count() == 0, "There should be no rules.");
}

void TestRules::evaluateEvent()
{
    // Add a rule
    QVariantMap addRuleParams;
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
    verifySuccess(response, true);

    // Trigger an event
    QNetworkAccessManager nam;
    QSignalSpy spy(&nam, SIGNAL(finished(QNetworkReply*)));

    // trigger event in mock device
    QNetworkRequest request(QUrl(QString("http://localhost:%1/generateevent?eventtypeid=%2").arg(m_mockDevice1Port).arg(mockEvent1Id.toString())));
    QNetworkReply *reply = nam.get(request);
    spy.wait();
    QCOMPARE(spy.count(), 1);
    reply->deleteLater();

    // Verify rule got executed
    spy.clear();
    request.setUrl(QUrl(QString("http://localhost:%1/actionhistory").arg(m_mockDevice1Port)));
    reply = nam.get(request);
    spy.wait();
    QCOMPARE(spy.count(), 1);

    reply->deleteLater();

    QByteArray actionHistory = reply->readAll();
    QVERIFY2(mockActionIdNoParams == ActionTypeId(actionHistory), "Action not triggered");
}

#include "testrules.moc"
QTEST_MAIN(TestRules)
