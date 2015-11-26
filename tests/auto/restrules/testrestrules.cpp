/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stuerz <simon.stuerz@guh.guru>                *
 *                                                                         *
 *  This file is part of guh.                                              *
 *                                                                         *
 *  Guh is free software: you can redistribute it and/or modify            *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  Guh is distributed in the hope that it will be useful,                 *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with guh. If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "guhtestbase.h"
#include "guhcore.h"
#include "devicemanager.h"
#include "mocktcpserver.h"
#include "webserver.h"

#include <QtTest/QtTest>
#include <QCoreApplication>
#include <QTcpSocket>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QCoreApplication>
#include <QJsonDocument>
#include <QHttpPart>
#include <QMetaType>

using namespace guhserver;

class TestRestRules: public GuhTestBase
{
    Q_OBJECT

private:
    void cleanupMockHistory();
    void cleanupRules();

    void verifyRuleExecuted(const ActionTypeId &actionTypeId);
    void verifyRuleNotExecuted();

private slots:
    void addRemoveRules_data();
    void addRemoveRules();

    void emptyRule();

    void editRules_data();
    void editRules();

    void enableDisableRule();

    void getRules();
};

void TestRestRules::cleanupMockHistory()
{
    QNetworkAccessManager nam;
    QSignalSpy spy(&nam, SIGNAL(finished(QNetworkReply*)));
    QNetworkRequest request(QUrl(QString("http://localhost:%1/clearactionhistory").arg(m_mockDevice1Port).arg(mockEvent1Id.toString())));
    QNetworkReply *reply = nam.get(request);
    spy.wait(500);
    QCOMPARE(spy.count(), 1);
    reply->deleteLater();
}

void TestRestRules::cleanupRules()
{
    QNetworkAccessManager *nam = new QNetworkAccessManager(this);
    QSignalSpy clientSpy(nam, SIGNAL(finished(QNetworkReply*)));

    // Get all rules
    QNetworkRequest request = QNetworkRequest(QUrl("http://localhost:3333/api/v1/rules"));
    QNetworkReply *reply = nam->get(request);
    clientSpy.wait();
    QVERIFY2(clientSpy.count() == 1, "expected exactly 1 response from webserver");
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QCOMPARE(statusCode, 200);
    QByteArray data = reply->readAll();
    reply->deleteLater();

    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);
    QCOMPARE(error.error, QJsonParseError::NoError);
    QVariantList rulesList = jsonDoc.toVariant().toList();

    // delete each rule
    foreach (const QVariant &rule, rulesList) {
        clientSpy.clear();
        QVariantMap ruleMap = rule.toMap();
        QNetworkRequest request(QUrl(QString("http://localhost:3333/api/v1/rules/%1").arg(ruleMap.value("id").toString())));
        request.setHeader(QNetworkRequest::ContentTypeHeader, "text/json");
        reply = nam->deleteResource(request);
        clientSpy.wait();
        QVERIFY2(clientSpy.count() == 1, "expected exactly 1 response from webserver");
        int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        QCOMPARE(statusCode, 200);
        reply->deleteLater();
    }
    nam->deleteLater();
}

void TestRestRules::verifyRuleExecuted(const ActionTypeId &actionTypeId)
{
    // Verify rule got executed
    QNetworkAccessManager nam;
    QSignalSpy spy(&nam, SIGNAL(finished(QNetworkReply*)));
    QNetworkRequest request(QUrl(QString("http://localhost:%1/actionhistory").arg(m_mockDevice1Port)));
    QNetworkReply *reply = nam.get(request);
    spy.wait(500);
    QCOMPARE(spy.count(), 1);

    QByteArray actionHistory = reply->readAll();
    qDebug() << "have action history" << actionHistory;
    QVERIFY2(actionTypeId == ActionTypeId(actionHistory), "Action not triggered");
    reply->deleteLater();
}

void TestRestRules::verifyRuleNotExecuted()
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

void TestRestRules::addRemoveRules_data()
{
    // RuleAction
    QVariantMap validActionNoParams;
    validActionNoParams.insert("actionTypeId", mockActionIdNoParams);
    validActionNoParams.insert("deviceId", m_mockDeviceId);
    validActionNoParams.insert("ruleActionParams", QVariantList());

    QVariantMap invalidAction;
    invalidAction.insert("actionTypeId", ActionTypeId());
    invalidAction.insert("deviceId", m_mockDeviceId);
    invalidAction.insert("ruleActionParams", QVariantList());

    // RuleExitAction
    QVariantMap validExitActionNoParams;
    validExitActionNoParams.insert("actionTypeId", mockActionIdNoParams);
    validExitActionNoParams.insert("deviceId", m_mockDeviceId);
    validExitActionNoParams.insert("ruleActionParams", QVariantList());

    QVariantMap invalidExitAction;
    invalidExitAction.insert("actionTypeId", ActionTypeId());
    invalidExitAction.insert("deviceId", m_mockDeviceId);
    invalidExitAction.insert("ruleActionParams", QVariantList());

    // StateDescriptor
    QVariantMap stateDescriptor;
    stateDescriptor.insert("stateTypeId", mockIntStateId);
    stateDescriptor.insert("deviceId", m_mockDeviceId);
    stateDescriptor.insert("operator", JsonTypes::valueOperatorToString(Types::ValueOperatorLess));
    stateDescriptor.insert("value", "20");

    // StateEvaluator
    QVariantMap validStateEvaluator;
    validStateEvaluator.insert("stateDescriptor", stateDescriptor);
    validStateEvaluator.insert("operator", JsonTypes::stateOperatorToString(Types::StateOperatorAnd));

    QVariantMap invalidStateEvaluator;
    stateDescriptor.remove("deviceId");
    invalidStateEvaluator.insert("stateDescriptor", stateDescriptor);

    // EventDescriptor
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

    QVariantMap validEventDescriptor3;
    validEventDescriptor3.insert("eventTypeId", mockEvent2Id);
    validEventDescriptor3.insert("deviceId", m_mockDeviceId);
    validEventDescriptor3.insert("paramDescriptors", QVariantList());

    // EventDescriptorList
    QVariantList eventDescriptorList;
    eventDescriptorList.append(validEventDescriptor1);
    eventDescriptorList.append(validEventDescriptor2);

    QVariantMap invalidEventDescriptor;
    invalidEventDescriptor.insert("eventTypeId", mockEvent1Id);
    invalidEventDescriptor.insert("deviceId", DeviceId());
    invalidEventDescriptor.insert("paramDescriptors", QVariantList());

    // RuleAction event based
    QVariantMap validActionEventBased;
    validActionEventBased.insert("actionTypeId", mockActionIdWithParams);
    validActionEventBased.insert("deviceId", m_mockDeviceId);
    QVariantMap validActionEventBasedParam1;
    validActionEventBasedParam1.insert("name", "mockActionParam1");
    validActionEventBasedParam1.insert("eventTypeId", mockEvent2Id);
    validActionEventBasedParam1.insert("eventParamName", "mockParamInt");
    QVariantMap validActionEventBasedParam2;
    validActionEventBasedParam2.insert("name", "mockActionParam2");
    validActionEventBasedParam2.insert("value", false);
    validActionEventBased.insert("ruleActionParams", QVariantList() << validActionEventBasedParam1 << validActionEventBasedParam2);

    QVariantMap invalidActionEventBased;
    invalidActionEventBased.insert("actionTypeId", mockActionIdNoParams);
    invalidActionEventBased.insert("deviceId", m_mockDeviceId);
    validActionEventBasedParam1.insert("value", 10);
    invalidActionEventBased.insert("ruleActionParams", QVariantList() << validActionEventBasedParam1);

    QVariantMap invalidActionEventBased2;
    invalidActionEventBased2.insert("actionTypeId", mockActionIdWithParams);
    invalidActionEventBased2.insert("deviceId", m_mockDeviceId);
    QVariantMap invalidActionEventBasedParam2;
    invalidActionEventBasedParam2.insert("name", "mockActionParam1");
    invalidActionEventBasedParam2.insert("eventTypeId", mockEvent1Id);
    invalidActionEventBasedParam2.insert("eventParamName", "value");
    QVariantMap invalidActionEventBasedParam3;
    invalidActionEventBasedParam3.insert("name", "mockActionParam2");
    invalidActionEventBasedParam3.insert("value", 2);
    invalidActionEventBased2.insert("ruleActionParams", QVariantList() << invalidActionEventBasedParam2 << invalidActionEventBasedParam3);

    QVariantMap invalidActionEventBased3;
    invalidActionEventBased3.insert("actionTypeId", mockActionIdWithParams);
    invalidActionEventBased3.insert("deviceId", m_mockDeviceId);
    QVariantMap invalidActionEventBasedParam4;
    invalidActionEventBasedParam4.insert("name", "mockActionParam1");
    invalidActionEventBasedParam4.insert("eventTypeId", mockEvent1Id);
    invalidActionEventBasedParam4.insert("eventParamName", "mockParamInt");
    invalidActionEventBased3.insert("ruleActionParams", QVariantList() << invalidActionEventBasedParam4);

    QTest::addColumn<bool>("enabled");
    QTest::addColumn<QVariantMap>("action1");
    QTest::addColumn<QVariantMap>("exitAction1");
    QTest::addColumn<QVariantMap>("eventDescriptor");
    QTest::addColumn<QVariantList>("eventDescriptorList");
    QTest::addColumn<QVariantMap>("stateEvaluator");
    QTest::addColumn<int>("expectedStatusCode");
    QTest::addColumn<bool>("jsonError");
    QTest::addColumn<QString>("name");

    // Rules with event based actions
    QTest::newRow("valid rule. enabled, 1 Action (eventBased), 1 EventDescriptor, name")                << true     << validActionEventBased    << QVariantMap()            << validEventDescriptor3    << QVariantList()       << QVariantMap()            << 200 << true << "ActionEventRule1";
    QTest::newRow("invalid rule. enabled, 1 Action (eventBased), 1 EventDescriptor, name")              << true     << invalidActionEventBased2 << QVariantMap()            << validEventDescriptor3    << QVariantList()       << QVariantMap()            << 400 << false << "TestRule";

    QTest::newRow("invalid rule. enabled, 1 Action (eventBased), types not matching, name")             << true     << invalidActionEventBased3 << QVariantMap()            << validEventDescriptor1    << QVariantList()       << QVariantMap()            << 400 << false << "TestRule";

    QTest::newRow("invalid rule. enabled, 1 Action (eventBased), 1 EventDescriptor, name")              << true     << invalidActionEventBased  << QVariantMap()            << validEventDescriptor2    << QVariantList()       << QVariantMap()            << 400 << false << "TestRule";
    QTest::newRow("invalid rule. enabled, 1 Action (eventBased), 1 StateEvaluator, name")               << true     << validActionEventBased    << QVariantMap()            << QVariantMap()            << QVariantList()       << validStateEvaluator      << 400 << false << "TestRule";
    QTest::newRow("invalid rule. enabled, 1 Action (eventBased), 1 EventDescriptor, name")              << true     << validActionEventBased    << validActionEventBased    << validEventDescriptor2    << QVariantList()       << QVariantMap()            << 400 << false << "TestRule";
    QTest::newRow("invalid rule. enabled, 1 Action, 1 ExitAction (EventBased), name")                   << true     << validActionNoParams      << validActionEventBased    << validEventDescriptor2    << QVariantList()       << QVariantMap()            << 400 << false << "TestRule";

    // Rules with exit actions
    QTest::newRow("valid rule. enabled, 1 Action, 1 Exit Action,  1 StateEvaluator, name")              << true     << validActionNoParams      << validExitActionNoParams  << QVariantMap()            << QVariantList()       << validStateEvaluator      << 200 << true << "TestRule";
    QTest::newRow("valid rule. disabled, 1 Action, 1 Exit Action, 1 StateEvaluator, name")              << false    << validActionNoParams      << validExitActionNoParams  << QVariantMap()            << QVariantList()       << validStateEvaluator      << 200 << true << "TestRule";
    QTest::newRow("invalid rule. disabled, 1 Action, 1 invalid Exit Action, 1 StateEvaluator, name")    << false    << validActionNoParams      << invalidExitAction        << QVariantMap()            << QVariantList()       << validStateEvaluator      << 400 << false << "TestRule";
    QTest::newRow("invalid rule. 1 Action, 1 Exit Action, 1 EventDescriptor, 1 StateEvaluator, name")   << true     << validActionNoParams      << validExitActionNoParams  << validEventDescriptor1    << QVariantList()       << validStateEvaluator      << 400 << false << "TestRule";
    QTest::newRow("invalid rule. 1 Action, 1 Exit Action, eventDescriptorList, 1 StateEvaluator, name") << true     << validActionNoParams      << validExitActionNoParams  << QVariantMap()            << eventDescriptorList  << validStateEvaluator      << 400 << false << "TestRule";

    // Rules without exit actions
    QTest::newRow("valid rule. enabled, 1 EventDescriptor, StateEvaluator, 1 Action, name")             << true     << validActionNoParams      << QVariantMap()            << validEventDescriptor1    << QVariantList()       << validStateEvaluator      << 200 << true << "TestRule";
    QTest::newRow("valid rule. diabled, 1 EventDescriptor, StateEvaluator, 1 Action, name")             << false    << validActionNoParams      << QVariantMap()            << validEventDescriptor1    << QVariantList()       << validStateEvaluator      << 200 << true << "TestRule";
    QTest::newRow("valid rule. 2 EventDescriptors, 1 Action, name")                                     << true     << validActionNoParams      << QVariantMap()            << QVariantMap()            << eventDescriptorList  << validStateEvaluator      << 200 << true << "TestRule";
    QTest::newRow("invalid rule: eventDescriptor and eventDescriptorList used")                         << true     << validActionNoParams      << QVariantMap()            << validEventDescriptor1    << eventDescriptorList  << validStateEvaluator      << 400 << false << "TestRule";
    QTest::newRow("invalid action")                                                                     << true     << invalidAction            << QVariantMap()            << validEventDescriptor1    << QVariantList()       << validStateEvaluator      << 400 << false << "TestRule";
    QTest::newRow("invalid event descriptor")                                                           << true     << validActionNoParams      << QVariantMap()            << invalidEventDescriptor   << QVariantList()       << validStateEvaluator      << 400 << false << "TestRule";
}

void TestRestRules::addRemoveRules()
{
    QFETCH(bool, enabled);
    QFETCH(QVariantMap, action1);
    QFETCH(QVariantMap, exitAction1);
    QFETCH(QVariantMap, eventDescriptor);
    QFETCH(QVariantList, eventDescriptorList);
    QFETCH(QVariantMap, stateEvaluator);
    QFETCH(int, expectedStatusCode);
    QFETCH(bool, jsonError);
    QFETCH(QString, name);

    Q_UNUSED(jsonError)

    // create add params for rule
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

    QVariantList exitActions;
    if (!exitAction1.isEmpty()) {
        exitActions.append(exitAction1);
        params.insert("exitActions", exitActions);
    }
    params.insert("stateEvaluator", stateEvaluator);
    if (!enabled) {
        params.insert("enabled", enabled);
    }

    QNetworkAccessManager *nam = new QNetworkAccessManager(this);
    QSignalSpy clientSpy(nam, SIGNAL(finished(QNetworkReply*)));

    // Get rules and verify there is no rule added
    QNetworkRequest request;
    request.setUrl(QUrl("http://localhost:3333/api/v1/rules"));
    QNetworkReply *reply = nam->get(request);
    clientSpy.wait();
    QVERIFY2(clientSpy.count() == 1, "expected exactly 1 response from webserver");
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QCOMPARE(statusCode, 200);
    QByteArray data = reply->readAll();
    reply->deleteLater();

    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);
    QCOMPARE(error.error, QJsonParseError::NoError);
    QVariantList rulesList = jsonDoc.toVariant().toList();
    QVERIFY2(rulesList.count() == 0, "there should be no rules.");

    // ADD rule
    clientSpy.clear();
    request = QNetworkRequest(QUrl(QString("http://localhost:3333/api/v1/rules")));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "text/json");
    reply = nam->post(request, QJsonDocument::fromVariant(params).toJson(QJsonDocument::Compact));
    clientSpy.wait();
    QVERIFY2(clientSpy.count() == 1, "expected exactly 1 response from webserver");
    statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QCOMPARE(statusCode, expectedStatusCode);

    if (expectedStatusCode != 200)
        return;

    jsonDoc = QJsonDocument::fromJson(reply->readAll(), &error);
    QCOMPARE(error.error, QJsonParseError::NoError);
    reply->deleteLater();

    RuleId ruleId = RuleId(jsonDoc.toVariant().toMap().value("id").toString());
    QVERIFY(!ruleId.isNull());

    // GET rule details
    clientSpy.clear();
    request = QNetworkRequest(QUrl(QString("http://localhost:3333/api/v1/rules/%1").arg(ruleId.toString())));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "text/json");
    reply = nam->get(request);
    clientSpy.wait();
    QVERIFY2(clientSpy.count() == 1, "expected exactly 1 response from webserver");
    statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QCOMPARE(statusCode, 200);
    data = reply->readAll();
    reply->deleteLater();
    jsonDoc = QJsonDocument::fromJson(data, &error);
    QCOMPARE(error.error, QJsonParseError::NoError);

    // REMOVE rule
    clientSpy.clear();
    request = QNetworkRequest(QUrl(QString("http://localhost:3333/api/v1/rules/%1").arg(ruleId.toString())));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "text/json");
    reply = nam->deleteResource(request);
    clientSpy.wait();
    QVERIFY2(clientSpy.count() == 1, "expected exactly 1 response from webserver");
    statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QCOMPARE(statusCode, 200);
    reply->deleteLater();

    nam->deleteLater();
}

void TestRestRules::emptyRule()
{
    // create add params for rule
    QVariantMap params;
    params.insert("name", QString());
    params.insert("actions", QVariantList());

    QNetworkAccessManager *nam = new QNetworkAccessManager(this);
    QSignalSpy clientSpy(nam, SIGNAL(finished(QNetworkReply*)));

    // Get rules and verify there is no rule added
    QNetworkRequest request = QNetworkRequest(QUrl(QString("http://localhost:3333/api/v1/rules")));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "text/json");
    QNetworkReply *reply = nam->post(request, QJsonDocument::fromVariant(params).toJson(QJsonDocument::Compact));
    clientSpy.wait();
    QVERIFY2(clientSpy.count() == 1, "expected exactly 1 response from webserver");
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QCOMPARE(statusCode, 400);
    QByteArray data = reply->readAll();
    reply->deleteLater();

    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);
    QCOMPARE(error.error, QJsonParseError::NoError);

    QVERIFY2(jsonDoc.toVariant().toMap().contains("error"), "The error message is missing");
    QVERIFY2(jsonDoc.toVariant().toMap().value("error").toString() == "RuleErrorMissingParameter", "Wrong RuleError.");
    nam->deleteLater();
}

void TestRestRules::editRules_data()
{
    // RuleAction
    QVariantMap validActionNoParams;
    validActionNoParams.insert("actionTypeId", mockActionIdNoParams);
    validActionNoParams.insert("deviceId", m_mockDeviceId);
    validActionNoParams.insert("ruleActionParams", QVariantList());

    QVariantMap invalidAction;
    invalidAction.insert("actionTypeId", ActionTypeId());
    invalidAction.insert("deviceId", m_mockDeviceId);
    invalidAction.insert("ruleActionParams", QVariantList());

    // RuleExitAction
    QVariantMap validExitActionNoParams;
    validExitActionNoParams.insert("actionTypeId", mockActionIdNoParams);
    validExitActionNoParams.insert("deviceId", m_mockDeviceId);
    validExitActionNoParams.insert("ruleActionParams", QVariantList());

    QVariantMap invalidExitAction;
    invalidExitAction.insert("actionTypeId", ActionTypeId());
    invalidExitAction.insert("deviceId", m_mockDeviceId);
    invalidExitAction.insert("ruleActionParams", QVariantList());

    // StateDescriptor
    QVariantMap stateDescriptor;
    stateDescriptor.insert("stateTypeId", mockIntStateId);
    stateDescriptor.insert("deviceId", m_mockDeviceId);
    stateDescriptor.insert("operator", JsonTypes::valueOperatorToString(Types::ValueOperatorLess));
    stateDescriptor.insert("value", "20");

    // StateEvaluator
    QVariantMap validStateEvaluator;
    validStateEvaluator.insert("stateDescriptor", stateDescriptor);
    validStateEvaluator.insert("operator", JsonTypes::stateOperatorToString(Types::StateOperatorAnd));

    QVariantMap invalidStateEvaluator;
    stateDescriptor.remove("deviceId");
    invalidStateEvaluator.insert("stateDescriptor", stateDescriptor);

    // EventDescriptor
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

    QVariantMap validEventDescriptor3;
    validEventDescriptor3.insert("eventTypeId", mockEvent2Id);
    validEventDescriptor3.insert("deviceId", m_mockDeviceId);
    validEventDescriptor3.insert("paramDescriptors", QVariantList());

    // EventDescriptorList
    QVariantList eventDescriptorList;
    eventDescriptorList.append(validEventDescriptor1);
    eventDescriptorList.append(validEventDescriptor2);

    QVariantMap invalidEventDescriptor;
    invalidEventDescriptor.insert("eventTypeId", mockEvent1Id);
    invalidEventDescriptor.insert("deviceId", DeviceId());
    invalidEventDescriptor.insert("paramDescriptors", QVariantList());

    // RuleAction event based
    QVariantMap validActionEventBased;
    validActionEventBased.insert("actionTypeId", mockActionIdWithParams);
    validActionEventBased.insert("deviceId", m_mockDeviceId);
    QVariantMap validActionEventBasedParam1;
    validActionEventBasedParam1.insert("name", "mockActionParam1");
    validActionEventBasedParam1.insert("eventTypeId", mockEvent2Id);
    validActionEventBasedParam1.insert("eventParamName", "mockParamInt");
    QVariantMap validActionEventBasedParam2;
    validActionEventBasedParam2.insert("name", "mockActionParam2");
    validActionEventBasedParam2.insert("value", false);
    validActionEventBased.insert("ruleActionParams", QVariantList() << validActionEventBasedParam1 << validActionEventBasedParam2);

    QVariantMap invalidActionEventBased;
    invalidActionEventBased.insert("actionTypeId", mockActionIdNoParams);
    invalidActionEventBased.insert("deviceId", m_mockDeviceId);
    validActionEventBasedParam1.insert("value", 10);
    invalidActionEventBased.insert("ruleActionParams", QVariantList() << validActionEventBasedParam1);

    QVariantMap invalidActionEventBased2;
    invalidActionEventBased2.insert("actionTypeId", mockActionIdWithParams);
    invalidActionEventBased2.insert("deviceId", m_mockDeviceId);
    QVariantMap invalidActionEventBasedParam2;
    invalidActionEventBasedParam2.insert("name", "mockActionParam1");
    invalidActionEventBasedParam2.insert("eventTypeId", mockEvent1Id);
    invalidActionEventBasedParam2.insert("eventParamName", "value");
    QVariantMap invalidActionEventBasedParam3;
    invalidActionEventBasedParam3.insert("name", "mockActionParam2");
    invalidActionEventBasedParam3.insert("value", 2);
    invalidActionEventBased2.insert("ruleActionParams", QVariantList() << invalidActionEventBasedParam2 << invalidActionEventBasedParam3);

    QVariantMap invalidActionEventBased3;
    invalidActionEventBased3.insert("actionTypeId", mockActionIdWithParams);
    invalidActionEventBased3.insert("deviceId", m_mockDeviceId);
    QVariantMap invalidActionEventBasedParam4;
    invalidActionEventBasedParam4.insert("name", "mockActionParam1");
    invalidActionEventBasedParam4.insert("eventTypeId", mockEvent1Id);
    invalidActionEventBasedParam4.insert("eventParamName", "mockParamInt");
    invalidActionEventBased3.insert("ruleActionParams", QVariantList() << invalidActionEventBasedParam4);

    QTest::addColumn<bool>("enabled");
    QTest::addColumn<QVariantMap>("action");
    QTest::addColumn<QVariantMap>("exitAction");
    QTest::addColumn<QVariantMap>("eventDescriptor");
    QTest::addColumn<QVariantList>("eventDescriptorList");
    QTest::addColumn<QVariantMap>("stateEvaluator");
    QTest::addColumn<int>("expectedStatusCode");
    QTest::addColumn<QString>("name");

    // Rules with event based actions
    QTest::newRow("valid rule. enabled, 1 Action (eventBased), 1 EventDescriptor, name")                << true     << validActionEventBased    << QVariantMap()            << validEventDescriptor3    << QVariantList()       << QVariantMap()            << 200 << "ActionEventRule1";
    QTest::newRow("invalid rule. enabled, 1 Action (eventBased), 1 EventDescriptor, name")              << true     << invalidActionEventBased2 << QVariantMap()            << validEventDescriptor3    << QVariantList()       << QVariantMap()            << 400  << "TestRule";

    QTest::newRow("invalid rule. enabled, 1 Action (eventBased), types not matching, name")             << true     << invalidActionEventBased3 << QVariantMap()            << validEventDescriptor1    << QVariantList()       << QVariantMap()            << 400 << "TestRule";

    QTest::newRow("invalid rule. enabled, 1 Action (eventBased), 1 EventDescriptor, name")              << true     << invalidActionEventBased  << QVariantMap()            << validEventDescriptor2    << QVariantList()       << QVariantMap()            << 400 << "TestRule";
    QTest::newRow("invalid rule. enabled, 1 Action (eventBased), 1 StateEvaluator, name")               << true     << validActionEventBased    << QVariantMap()            << QVariantMap()            << QVariantList()       << validStateEvaluator      << 400 << "TestRule";
    QTest::newRow("invalid rule. enabled, 1 Action (eventBased), 1 EventDescriptor, name")              << true     << validActionEventBased    << validActionEventBased    << validEventDescriptor2    << QVariantList()       << QVariantMap()            << 400 << "TestRule";
    QTest::newRow("invalid rule. enabled, 1 Action, 1 ExitAction (EventBased), name")                   << true     << validActionNoParams      << validActionEventBased    << validEventDescriptor2    << QVariantList()       << QVariantMap()            << 400 << "TestRule";

    // Rules with exit actions
    QTest::newRow("valid rule. enabled, 1 Action, 1 Exit Action,  1 StateEvaluator, name")              << true     << validActionNoParams      << validExitActionNoParams  << QVariantMap()            << QVariantList()       << validStateEvaluator      << 200 << "TestRule";
    QTest::newRow("valid rule. disabled, 1 Action, 1 Exit Action, 1 StateEvaluator, name")              << false    << validActionNoParams      << validExitActionNoParams  << QVariantMap()            << QVariantList()       << validStateEvaluator      << 200 << "TestRule";
    QTest::newRow("invalid rule. 1 Action, 1 Exit Action, 1 EventDescriptor, 1 StateEvaluator, name")   << true     << validActionNoParams      << validExitActionNoParams  << validEventDescriptor1    << QVariantList()       << validStateEvaluator      << 400 << "TestRule";
    QTest::newRow("invalid rule. 1 Action, 1 Exit Action, eventDescriptorList, 1 StateEvaluator, name") << true     << validActionNoParams      << validExitActionNoParams  << QVariantMap()            << eventDescriptorList  << validStateEvaluator      << 400 << "TestRule";

    // Rules without exit actions
    QTest::newRow("valid rule. enabled, 1 EventDescriptor, StateEvaluator, 1 Action, name")             << true     << validActionNoParams      << QVariantMap()            << validEventDescriptor1    << QVariantList()       << validStateEvaluator      << 200 << "TestRule";
    QTest::newRow("valid rule. diabled, 1 EventDescriptor, StateEvaluator, 1 Action, name")             << false    << validActionNoParams      << QVariantMap()            << validEventDescriptor1    << QVariantList()       << validStateEvaluator      << 200 << "TestRule";
    QTest::newRow("valid rule. 2 EventDescriptors, 1 Action, name")                                     << true     << validActionNoParams      << QVariantMap()            << QVariantMap()            << eventDescriptorList  << validStateEvaluator      << 200 << "TestRule";
    QTest::newRow("invalid rule: eventDescriptor and eventDescriptorList used")                         << true     << validActionNoParams      << QVariantMap()            << validEventDescriptor1    << eventDescriptorList  << validStateEvaluator      << 400 << "TestRule";
}

void TestRestRules::editRules()
{
    QFETCH(bool, enabled);
    QFETCH(QVariantMap, action);
    QFETCH(QVariantMap, exitAction);
    QFETCH(QVariantMap, eventDescriptor);
    QFETCH(QVariantList, eventDescriptorList);
    QFETCH(QVariantMap, stateEvaluator);
    QFETCH(int, expectedStatusCode);
    QFETCH(QString, name);

    // Add the rule we want to edit
    QVariantList eventParamDescriptors;
    QVariantMap eventDescriptor1;
    eventDescriptor1.insert("eventTypeId", mockEvent1Id);
    eventDescriptor1.insert("deviceId", m_mockDeviceId);
    eventDescriptor1.insert("paramDescriptors", QVariantList());
    QVariantMap eventDescriptor2;
    eventDescriptor2.insert("eventTypeId", mockEvent2Id);
    eventDescriptor2.insert("deviceId", m_mockDeviceId);
    eventDescriptor2.insert("paramDescriptors", QVariantList());
    QVariantMap eventParam1;
    eventParam1.insert("name", "mockParamInt");
    eventParam1.insert("value", 3);
    eventParam1.insert("operator", JsonTypes::valueOperatorToString(Types::ValueOperatorEquals));
    eventParamDescriptors.append(eventParam1);
    eventDescriptor2.insert("paramDescriptors", eventParamDescriptors);

    QVariantList eventDescriptorList1;
    eventDescriptorList1.append(eventDescriptor1);
    eventDescriptorList1.append(eventDescriptor2);

    QVariantMap stateEvaluator0;
    QVariantMap stateDescriptor1;
    stateDescriptor1.insert("deviceId", m_mockDeviceId);
    stateDescriptor1.insert("operator", JsonTypes::valueOperatorToString(Types::ValueOperatorEquals));
    stateDescriptor1.insert("stateTypeId", mockIntStateId);
    stateDescriptor1.insert("value", 1);
    QVariantMap stateDescriptor2;
    stateDescriptor2.insert("deviceId", m_mockDeviceId);
    stateDescriptor2.insert("operator", JsonTypes::valueOperatorToString(Types::ValueOperatorEquals));
    stateDescriptor2.insert("stateTypeId", mockBoolStateId);
    stateDescriptor2.insert("value", true);
    QVariantMap stateEvaluator1;
    stateEvaluator1.insert("stateDescriptor", stateDescriptor1);
    stateEvaluator1.insert("operator", JsonTypes::stateOperatorToString(Types::StateOperatorAnd));
    QVariantMap stateEvaluator2;
    stateEvaluator2.insert("stateDescriptor", stateDescriptor2);
    stateEvaluator2.insert("operator", JsonTypes::stateOperatorToString(Types::StateOperatorAnd));
    QVariantList childEvaluators;
    childEvaluators.append(stateEvaluator1);
    childEvaluators.append(stateEvaluator2);
    stateEvaluator0.insert("childEvaluators", childEvaluators);
    stateEvaluator0.insert("operator", JsonTypes::stateOperatorToString(Types::StateOperatorAnd));

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

    // RuleAction event based
    QVariantMap validActionEventBased;
    validActionEventBased.insert("actionTypeId", mockActionIdWithParams);
    validActionEventBased.insert("deviceId", m_mockDeviceId);
    QVariantMap validActionEventBasedParam1;
    validActionEventBasedParam1.insert("name", "mockActionParam1");
    validActionEventBasedParam1.insert("eventTypeId", mockEvent2Id);
    validActionEventBasedParam1.insert("eventParamName", "mockParamInt");
    QVariantMap validActionEventBasedParam2;
    validActionEventBasedParam2.insert("name", "mockActionParam2");
    validActionEventBasedParam2.insert("value", false);
    validActionEventBased.insert("ruleActionParams", QVariantList() << validActionEventBasedParam1 << validActionEventBasedParam2);

    QVariantList validEventDescriptors3;
    QVariantMap validEventDescriptor3;
    validEventDescriptor3.insert("eventTypeId", mockEvent2Id);
    validEventDescriptor3.insert("deviceId", m_mockDeviceId);
    validEventDescriptor3.insert("paramDescriptors", QVariantList());
    validEventDescriptors3.append(validEventDescriptor3);

    QVariantMap params;
    QVariantList actions;
    actions.append(action1);
    actions.append(action2);
    params.insert("actions", actions);
    params.insert("eventDescriptorList", eventDescriptorList1);
    params.insert("stateEvaluator", stateEvaluator0);
    params.insert("name", "TestRule");


    QNetworkAccessManager *nam = new QNetworkAccessManager(this);
    QSignalSpy clientSpy(nam, SIGNAL(finished(QNetworkReply*)));

    // Get rules and verify there is no rule added
    QNetworkRequest request;
    request.setUrl(QUrl("http://localhost:3333/api/v1/rules"));
    QNetworkReply *reply = nam->get(request);
    clientSpy.wait();
    QVERIFY2(clientSpy.count() == 1, "expected exactly 1 response from webserver");
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QCOMPARE(statusCode, 200);
    QByteArray data = reply->readAll();
    reply->deleteLater();

    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);
    QCOMPARE(error.error, QJsonParseError::NoError);
    QVariantList rulesList = jsonDoc.toVariant().toList();
    QVERIFY2(rulesList.count() == 0, "there should be no rules.");

    // ADD rule
    clientSpy.clear();
    request = QNetworkRequest(QUrl(QString("http://localhost:3333/api/v1/rules")));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "text/json");
    reply = nam->post(request, QJsonDocument::fromVariant(params).toJson(QJsonDocument::Compact));
    clientSpy.wait();
    QVERIFY2(clientSpy.count() == 1, "expected exactly 1 response from webserver");
    statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QCOMPARE(statusCode, 200);

    jsonDoc = QJsonDocument::fromJson(reply->readAll(), &error);
    QCOMPARE(error.error, QJsonParseError::NoError);
    reply->deleteLater();

    RuleId ruleId = RuleId(jsonDoc.toVariant().toMap().value("id").toString());
    QVERIFY(!ruleId.isNull());

    // now create the new rule and edit the original one
    params.clear();
    params.insert("ruleId", ruleId.toString());
    params.insert("name", name);

    if (!eventDescriptor.isEmpty()) {
        params.insert("eventDescriptor", eventDescriptor);
    }
    if (!eventDescriptorList.isEmpty()) {
        params.insert("eventDescriptorList", eventDescriptorList);
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

    // EDIT rule
    clientSpy.clear();
    request = QNetworkRequest(QUrl(QString("http://localhost:3333/api/v1/rules/%1").arg(ruleId.toString())));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "text/json");
    reply = nam->put(request, QJsonDocument::fromVariant(params).toJson(QJsonDocument::Compact));
    clientSpy.wait();
    QVERIFY2(clientSpy.count() == 1, "expected exactly 1 response from webserver");
    statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QCOMPARE(statusCode, expectedStatusCode);

    if (expectedStatusCode == 200) {
        // get edit rule and verify params
        clientSpy.clear();
        request = QNetworkRequest(QUrl(QString("http://localhost:3333/api/v1/rules")));
        request.setHeader(QNetworkRequest::ContentTypeHeader, "text/json");
        reply = nam->get(request);
        clientSpy.wait();
        QVERIFY2(clientSpy.count() == 1, "expected exactly 1 response from webserver");
        statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        QCOMPARE(statusCode, 200);
        jsonDoc = QJsonDocument::fromJson(reply->readAll(), &error);
        QCOMPARE(error.error, QJsonParseError::NoError);
        reply->deleteLater();
    }

    // REMOVE rule
    clientSpy.clear();
    request = QNetworkRequest(QUrl(QString("http://localhost:3333/api/v1/rules/%1").arg(ruleId.toString())));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "text/json");
    reply = nam->deleteResource(request);
    clientSpy.wait();
    QVERIFY2(clientSpy.count() == 1, "expected exactly 1 response from webserver");
    statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QCOMPARE(statusCode, 200);
    reply->deleteLater();


    // check if removed
    clientSpy.clear();
    request = QNetworkRequest(QUrl(QString("http://localhost:3333/api/v1/rules/%1").arg(ruleId.toString())));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "text/json");
    reply = nam->get(request);
    clientSpy.wait();
    QVERIFY2(clientSpy.count() == 1, "expected exactly 1 response from webserver");
    statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QCOMPARE(statusCode, 404);
    reply->deleteLater();

    nam->deleteLater();
}

void TestRestRules::enableDisableRule()
{
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

    QNetworkAccessManager *nam = new QNetworkAccessManager(this);
    QSignalSpy clientSpy(nam, SIGNAL(finished(QNetworkReply*)));

    // ADD rule
    QNetworkRequest request = QNetworkRequest(QUrl(QString("http://localhost:3333/api/v1/rules")));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "text/json");
    QNetworkReply *reply = nam->post(request, QJsonDocument::fromVariant(addRuleParams).toJson(QJsonDocument::Compact));
    clientSpy.wait();
    QVERIFY2(clientSpy.count() == 1, "expected exactly 1 response from webserver");
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QCOMPARE(statusCode, 200);
    QByteArray data = reply->readAll();
    reply->deleteLater();

    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);
    QCOMPARE(error.error, QJsonParseError::NoError);

    RuleId ruleId = RuleId(jsonDoc.toVariant().toMap().value("id").toString());
    QVERIFY(!ruleId.isNull());

    // ENABLE rule
    clientSpy.clear();
    request = QNetworkRequest(QUrl(QString("http://localhost:3333/api/v1/rules/%1/enable").arg(ruleId.toString())));
    reply = nam->post(request, QByteArray());
    clientSpy.wait();
    QVERIFY2(clientSpy.count() == 1, "expected exactly 1 response from webserver");
    statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QCOMPARE(statusCode, 200);
    reply->deleteLater();

    // Trigger an event

    // trigger event in mock device
    clientSpy.clear();
    request = QNetworkRequest(QUrl(QString("http://localhost:%1/generateevent?eventtypeid=%2").arg(m_mockDevice1Port).arg(mockEvent1Id.toString())));
    reply = nam->get(request);
    clientSpy.wait();
    QCOMPARE(clientSpy.count(), 1);
    reply->deleteLater();

    verifyRuleExecuted(mockActionIdNoParams);

    cleanupMockHistory();

    // DISABLE the rule
    clientSpy.clear();
    request = QNetworkRequest(QUrl(QString("http://localhost:3333/api/v1/rules/%1/disable").arg(ruleId.toString())));
    reply = nam->post(request, QByteArray());
    clientSpy.wait();
    QVERIFY2(clientSpy.count() == 1, "expected exactly 1 response from webserver");
    statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QCOMPARE(statusCode, 200);
    reply->deleteLater();


    // trigger event in mock device
    clientSpy.clear();
    request = QNetworkRequest(QUrl(QString("http://localhost:%1/generateevent?eventtypeid=%2").arg(m_mockDevice1Port).arg(mockEvent1Id.toString())));
    reply = nam->get(request);
    clientSpy.wait();
    QCOMPARE(clientSpy.count(), 1);
    reply->deleteLater();

    verifyRuleNotExecuted();

    cleanupMockHistory();

    // ENABLE again
    clientSpy.clear();
    request = QNetworkRequest(QUrl(QString("http://localhost:3333/api/v1/rules/%1/enable").arg(ruleId.toString())));
    reply = nam->post(request, QByteArray());
    clientSpy.wait();
    QVERIFY2(clientSpy.count() == 1, "expected exactly 1 response from webserver");
    statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QCOMPARE(statusCode, 200);
    reply->deleteLater();

    // trigger event in mock device
    clientSpy.clear();
    request = QNetworkRequest(QUrl(QString("http://localhost:%1/generateevent?eventtypeid=%2").arg(m_mockDevice1Port).arg(mockEvent1Id.toString())));
    reply = nam->get(request);
    clientSpy.wait();
    QCOMPARE(clientSpy.count(), 1);
    reply->deleteLater();

    verifyRuleExecuted(mockActionIdNoParams);

    cleanupRules();
}

void TestRestRules::getRules()
{
    QNetworkAccessManager *nam = new QNetworkAccessManager(this);
    QSignalSpy clientSpy(nam, SIGNAL(finished(QNetworkReply*)));

    // Get all rules
    QNetworkRequest request;
    request.setUrl(QUrl("http://localhost:3333/api/v1/rules"));
    QNetworkReply *reply = nam->get(request);
    clientSpy.wait();
    QVERIFY2(clientSpy.count() == 1, "expected exactly 1 response from webserver");
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QCOMPARE(statusCode, 200);
    QByteArray data = reply->readAll();
    reply->deleteLater();

    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);
    QCOMPARE(error.error, QJsonParseError::NoError);
    QVariantList rulesList = jsonDoc.toVariant().toList();
    QVERIFY2(rulesList.count() == 0, "there should be at least one vendor.");

    // Get each of thouse rules individualy
    foreach (const QVariant &rule, rulesList) {
        QVariantMap ruleMap = rule.toMap();
        QNetworkRequest request(QUrl(QString("http://localhost:3333/api/v1/rules/%1").arg(ruleMap.value("id").toString())));
        request.setHeader(QNetworkRequest::ContentTypeHeader, "text/json");
        clientSpy.clear();
        QNetworkReply *reply = nam->get(request);
        clientSpy.wait();
        QVERIFY2(clientSpy.count() == 1, "expected exactly 1 response from webserver");
        int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        QCOMPARE(statusCode, 200);
        jsonDoc = QJsonDocument::fromJson(reply->readAll(), &error);
        QCOMPARE(error.error, QJsonParseError::NoError);
        reply->deleteLater();

    }
    nam->deleteLater();
}


#include "testrestrules.moc"
QTEST_MAIN(TestRestRules)
