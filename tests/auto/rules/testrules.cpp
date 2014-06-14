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

    QVariantMap validEventDescriptor;
    validEventDescriptor.insert("eventTypeId", mockEvent1Id);
    validEventDescriptor.insert("deviceId", m_mockDeviceId);
    validEventDescriptor.insert("paramDescriptors", QVariantList());

    QVariantMap invalidEventDescriptor;
    invalidEventDescriptor.insert("eventTypeId", mockEvent1Id);
    invalidEventDescriptor.insert("deviceId", DeviceId());
    invalidEventDescriptor.insert("paramDescriptors", QVariantList());

    QTest::addColumn<QVariantMap>("action1");
    QTest::addColumn<QVariantMap>("eventDescriptor1");
    QTest::addColumn<bool>("success");

    QTest::newRow("valid rule") << validAction << validEventDescriptor << true;
    QTest::newRow("invalid action") << invalidAction << validEventDescriptor << false;
    QTest::newRow("invalid event descriptor") << validAction << invalidEventDescriptor << false;

}

void TestRules::addRules()
{
    QFETCH(QVariantMap, action1);
    QFETCH(QVariantMap, eventDescriptor1);
    QFETCH(bool, success);

    QVariantMap params;
    QVariantList actions;
    actions.append(action1);
    params.insert("actions", actions);
    params.insert("eventDescriptor", eventDescriptor1);
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
    QVERIFY2(eventDescriptors.count() == 1, "There shoud be exactly one eventDescriptor");
    QVERIFY2(eventDescriptors.first().toMap() == eventDescriptor1, "Event descriptor doesn't match");

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
    eventDescriptor2.insert("eventTypeId", mockEvent1Id);
    eventDescriptor2.insert("deviceId", m_mockDeviceId);
    eventDescriptor2.insert("paramDescriptors", QVariantList());

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
    params.insert("eventDescriptor", eventDescriptor1);
    QVariant response = injectAndWait("Rules.AddRule", params);

    RuleId newRuleId = RuleId(response.toMap().value("params").toMap().value("ruleId").toString());
    verifySuccess(response, true);

    restartServer();

    response = injectAndWait("Rules.GetRules");

    QVariantList rules = response.toMap().value("params").toMap().value("rules").toList();

    QVERIFY2(rules.count() == 1, "There should be exactly one rule");
    QCOMPARE(RuleId(rules.first().toMap().value("id").toString()), newRuleId);

    QVariantList eventDescriptors = rules.first().toMap().value("eventDescriptors").toList();
    QVERIFY2(eventDescriptors.count() == 1, "There shoud be exactly one eventDescriptor");
    QVERIFY2(eventDescriptors.first().toMap() == eventDescriptor1, "Event descriptor doesn't match");

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

#include "testrules.moc"
QTEST_MAIN(TestRules)
