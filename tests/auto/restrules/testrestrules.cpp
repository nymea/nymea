/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
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

    void triggerMockEvent();

    QVariant validIntStateBasedRule(const QString &name, const bool &executable, const bool &enabled);

private slots:
    void getRules();
    void findRule();
    void invalidMethod();
    void invalidPath();

    void checkOptionCall();

    void addRemoveRules_data();
    void addRemoveRules();

    void emptyRule();

    void editRules_data();
    void editRules();

    void enableDisableRule();

    void executeRuleActions_data();
    void executeRuleActions();

};

void TestRestRules::cleanupMockHistory()
{
    QNetworkAccessManager nam;
    QSignalSpy spy(&nam, SIGNAL(finished(QNetworkReply*)));
    QNetworkRequest request(QUrl(QString("http://localhost:%1/clearactionhistory").arg(m_mockDevice1Port).arg(mockEvent1Id.toString())));
    QNetworkReply *reply = nam.get(request);
    spy.wait();
    QCOMPARE(spy.count(), 1);
    reply->deleteLater();
}

void TestRestRules::cleanupRules()
{
    // Get all rules
    QNetworkRequest request(QUrl("https://localhost:3333/api/v1/rules"));
    QVariant response = getAndWait(request);
    QVERIFY(!response.isNull());
    QVariantList rulesList = response.toList();

    // delete each rule
    foreach (const QVariant &rule, rulesList) {
        QVariantMap ruleMap = rule.toMap();
        QNetworkRequest request(QUrl(QString("https://localhost:3333/api/v1/rules/%1").arg(ruleMap.value("id").toString())));
        response = deleteAndWait(request);
        QVERIFY(!response.isNull());
    }
}

void TestRestRules::verifyRuleExecuted(const ActionTypeId &actionTypeId)
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

void TestRestRules::triggerMockEvent()
{
    // trigger event in mock device
    QNetworkAccessManager nam;
    QSignalSpy spy(&nam, SIGNAL(finished(QNetworkReply*)));
    QNetworkRequest request = QNetworkRequest(QUrl(QString("http://localhost:%1/generateevent?eventtypeid=%2").arg(m_mockDevice1Port).arg(mockEvent1Id.toString())));
    QNetworkReply *reply = nam.get(request);
    spy.wait();
    QCOMPARE(spy.count(), 1);
    reply->deleteLater();
}

QVariant TestRestRules::validIntStateBasedRule(const QString &name, const bool &executable, const bool &enabled)
{
    QVariantMap params;

    // StateDescriptor
    QVariantMap stateDescriptor;
    stateDescriptor.insert("stateTypeId", mockIntStateId);
    stateDescriptor.insert("deviceId", m_mockDeviceId);
    stateDescriptor.insert("operator", JsonTypes::valueOperatorToString(Types::ValueOperatorLess));
    stateDescriptor.insert("value", 25);

    // StateEvaluator
    QVariantMap stateEvaluator;
    stateEvaluator.insert("stateDescriptor", stateDescriptor);
    stateEvaluator.insert("operator", JsonTypes::stateOperatorToString(Types::StateOperatorAnd));

    // RuleAction
    QVariantMap action;
    action.insert("actionTypeId", mockActionIdWithParams);
    QVariantList actionParams;
    QVariantMap param1;
    param1.insert("paramTypeId", mockActionParam1ParamTypeId);
    param1.insert("value", 5);
    actionParams.append(param1);
    QVariantMap param2;
    param2.insert("paramTypeId", mockActionParam2ParamTypeId);
    param2.insert("value", true);
    actionParams.append(param2);
    action.insert("deviceId", m_mockDeviceId);
    action.insert("ruleActionParams", actionParams);

    // RuleExitAction
    QVariantMap exitAction;
    exitAction.insert("actionTypeId", mockActionIdNoParams);
    exitAction.insert("deviceId", m_mockDeviceId);
    exitAction.insert("ruleActionParams", QVariantList());

    params.insert("name", name);
    params.insert("enabled", enabled);
    params.insert("executable", executable);
    params.insert("stateEvaluator", stateEvaluator);
    params.insert("actions", QVariantList() << action);
    params.insert("exitActions", QVariantList() << exitAction);

    return params;
}

void TestRestRules::getRules()
{
    // Get all rules
    QVariant response = getAndWait(QNetworkRequest(QUrl("https://localhost:3333/api/v1/rules")));
    QVariantList rulesList = response.toList();
    QVERIFY2(rulesList.count() == 0, "Not enought rules");

    foreach (const QVariant &rule, rulesList) {
        QVariantMap ruleMap = rule.toMap();
        QNetworkRequest request(QUrl(QString("https://localhost:3333/api/v1/rules/%1").arg(ruleMap.value("id").toString())));
        request.setHeader(QNetworkRequest::ContentTypeHeader, "text/json");

        response = getAndWait(request);
        QVERIFY2(!response.isNull(), "Could not get rule");
    }
}

void TestRestRules::findRule()
{
    QVariant params = validIntStateBasedRule("Find", true, true);
    QNetworkRequest request(QUrl(QString("https://localhost:3333/api/v1/rules")));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "text/json");
    QVariant response = postAndWait(request, params);

    RuleId ruleId = RuleId(response.toMap().value("id").toString());
    QVERIFY(!ruleId.isNull());

    QUrl url(QString("https://localhost:3333/api/v1/rules"));
    QUrlQuery query;
    query.addQueryItem("deviceId", m_mockDeviceId.toString());
    url.setQuery(query);
    request.setUrl(QUrl(url));
    response = getAndWait(request);
    QVERIFY(!response.isNull());

    // REMOVE rule
    request.setUrl(QUrl(QString("https://localhost:3333/api/v1/rules/%1").arg(ruleId.toString())));
    response = deleteAndWait(request);
    QVERIFY(!response.isNull());
    QCOMPARE(JsonTypes::ruleErrorToString(RuleEngine::RuleErrorNoError), response.toMap().value("error").toString());

    // check if removed
    request.setUrl(QUrl(QString("https://localhost:3333/api/v1/rules/%1").arg(ruleId.toString())));
    response = getAndWait(request, 404);
    QVERIFY(!response.isNull());
}

void TestRestRules::invalidMethod()
{
    QNetworkAccessManager nam;
    connect(&nam, &QNetworkAccessManager::sslErrors, [this, &nam](QNetworkReply *reply, const QList<QSslError> &) {
        reply->ignoreSslErrors();
    });
    QSignalSpy clientSpy(&nam, SIGNAL(finished(QNetworkReply*)));

    QNetworkRequest request;
    request.setUrl(QUrl("https://localhost:3333/api/v1/rules"));
    QNetworkReply *reply = nam.sendCustomRequest(request, "TRACE");

    clientSpy.wait();
    QVERIFY2(clientSpy.count() != 0, "expected at least 1 response from webserver");

    bool ok = false;
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(&ok);
    QVERIFY2(ok, "Could not convert statuscode from response to int");
    QCOMPARE(statusCode, 405);

    reply->deleteLater();
}

void TestRestRules::invalidPath()
{
    QNetworkAccessManager nam;
    connect(&nam, &QNetworkAccessManager::sslErrors, [this, &nam](QNetworkReply *reply, const QList<QSslError> &) {
        reply->ignoreSslErrors();
    });
    QSignalSpy clientSpy(&nam, SIGNAL(finished(QNetworkReply*)));

    QNetworkRequest request;
    request.setUrl(QUrl("https://localhost:3333/api/v1/rules/" + QUuid::createUuid().toString() + "/" + QUuid::createUuid().toString()));
    QNetworkReply *reply = nam.get(request);

    clientSpy.wait();
    QVERIFY2(clientSpy.count() != 0, "expected at least 1 response from webserver");

    bool ok = false;
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(&ok);
    QVERIFY2(ok, "Could not convert statuscode from response to int");
    QCOMPARE(statusCode, 404);

    reply->deleteLater();
}

void TestRestRules::checkOptionCall()
{
    QNetworkRequest request(QUrl(QString("https://localhost:3333/api/v1/rules")));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "text/json");
    QVariant response = postAndWait(request, validIntStateBasedRule("Options", true, true));

    RuleId ruleId = RuleId(response.toMap().value("id").toString());
    QVERIFY(!ruleId.isNull());

    QNetworkAccessManager nam;
    connect(&nam, &QNetworkAccessManager::sslErrors, [this, &nam](QNetworkReply *reply, const QList<QSslError> &) {
        reply->ignoreSslErrors();
    });
    QSignalSpy clientSpy(&nam, SIGNAL(finished(QNetworkReply*)));

    request.setUrl(QUrl(QString("https://localhost:3333/api/v1/rules/%1").arg(ruleId.toString())));
    QNetworkReply *reply = nam.sendCustomRequest(request, "OPTIONS");

    clientSpy.wait();
    QCOMPARE(clientSpy.count(), 1);

    bool ok = false;
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(&ok);
    QVERIFY2(ok, "Could not convert statuscode from response to int");
    QCOMPARE(statusCode, 200);

    reply->deleteLater();

    // REMOVE rule
    request.setUrl(QUrl(QString("https://localhost:3333/api/v1/rules/%1").arg(ruleId.toString())));
    response = deleteAndWait(request);
    QVERIFY(!response.isNull());
    QCOMPARE(JsonTypes::ruleErrorToString(RuleEngine::RuleErrorNoError), response.toMap().value("error").toString());

    // check if removed
    request.setUrl(QUrl(QString("https://localhost:3333/api/v1/rules/%1").arg(ruleId.toString())));
    response = getAndWait(request, 404);
    QVERIFY(!response.isNull());
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
    stateDescriptor.insert("value", 20);

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
    param1.insert("paramTypeId", mockParamIntParamTypeId);
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
    validActionEventBasedParam1.insert("paramTypeId", mockActionParam1ParamTypeId);
    validActionEventBasedParam1.insert("eventTypeId", mockEvent2Id);
    validActionEventBasedParam1.insert("eventParamTypeId", mockParamIntParamTypeId);
    QVariantMap validActionEventBasedParam2;
    validActionEventBasedParam2.insert("paramTypeId", mockActionParam2ParamTypeId);
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
    invalidActionEventBasedParam2.insert("paramTypeId", mockActionParam1ParamTypeId);
    invalidActionEventBasedParam2.insert("eventTypeId", mockEvent1Id);
    invalidActionEventBasedParam2.insert("eventParamTypeId", ParamTypeId::createParamTypeId());
    QVariantMap invalidActionEventBasedParam3;
    invalidActionEventBasedParam3.insert("paramTypeId", mockActionParam2ParamTypeId);
    invalidActionEventBasedParam3.insert("value", 2);
    invalidActionEventBased2.insert("ruleActionParams", QVariantList() << invalidActionEventBasedParam2 << invalidActionEventBasedParam3);

    QVariantMap invalidActionEventBased3;
    invalidActionEventBased3.insert("actionTypeId", mockActionIdWithParams);
    invalidActionEventBased3.insert("deviceId", m_mockDeviceId);
    QVariantMap invalidActionEventBasedParam4;
    invalidActionEventBasedParam4.insert("paramTypeId", mockActionParam1ParamTypeId);
    invalidActionEventBasedParam4.insert("eventTypeId", mockEvent1Id);
    invalidActionEventBasedParam4.insert("eventParamTypeId", mockParamIntParamTypeId);
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

    // Get rules and verify there is no rule added
    QNetworkRequest request(QUrl("https://localhost:3333/api/v1/rules"));
    QVariant response = getAndWait(request);
    QVariantList rulesList = response.toList();
    QVERIFY2(rulesList.count() == 0, "there should be no rules.");

    // ADD rule
    request = QNetworkRequest(QUrl(QString("https://localhost:3333/api/v1/rules")));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "text/json");

    response = postAndWait(request, params, expectedStatusCode);
    if (expectedStatusCode != 200)
        return;

    RuleId ruleId = RuleId(response.toMap().value("id").toString());
    QVERIFY(!ruleId.isNull());

    qDebug() << QJsonDocument::fromVariant(response).toJson();

    // GET rule details
    request = QNetworkRequest(QUrl(QString("https://localhost:3333/api/v1/rules/%1").arg(ruleId.toString())));
    //request.setHeader(QNetworkRequest::ContentTypeHeader, "text/json");
    response = getAndWait(request);
    QVERIFY(!response.isNull());

    // REMOVE rule
    request = QNetworkRequest(QUrl(QString("https://localhost:3333/api/v1/rules/%1").arg(ruleId.toString())));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "text/json");
    response = deleteAndWait(request);
    QVERIFY(!response.isNull());
}

void TestRestRules::emptyRule()
{
    // create add params for rule
    QVariantMap params;
    params.insert("name", QString());
    params.insert("actions", QVariantList());

    // Get rules and verify there is no rule added
    QNetworkRequest request = QNetworkRequest(QUrl(QString("https://localhost:3333/api/v1/rules")));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "text/json");

    QVariant response = postAndWait(request, params, 400);
    QCOMPARE(response.toMap().value("error").toString(), JsonTypes::ruleErrorToString(RuleEngine::RuleErrorInvalidRuleFormat));
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
    stateDescriptor.insert("value", 20);

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
    param1.insert("paramTypeId", mockParamIntParamTypeId);
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
    validActionEventBasedParam1.insert("paramTypeId", mockActionParam1ParamTypeId);
    validActionEventBasedParam1.insert("eventTypeId", mockEvent2Id);
    validActionEventBasedParam1.insert("eventParamTypeId", mockParamIntParamTypeId);
    QVariantMap validActionEventBasedParam2;
    validActionEventBasedParam2.insert("paramTypeId", mockActionParam2ParamTypeId);
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
    invalidActionEventBasedParam2.insert("paramTypeId", mockActionParam1ParamTypeId);
    invalidActionEventBasedParam2.insert("eventTypeId", mockEvent1Id);
    invalidActionEventBasedParam2.insert("eventParamTypeId", mockEvent1Id);
    QVariantMap invalidActionEventBasedParam3;
    invalidActionEventBasedParam3.insert("paramTypeId", mockActionParam2ParamTypeId);
    invalidActionEventBasedParam3.insert("value", 2);
    invalidActionEventBased2.insert("ruleActionParams", QVariantList() << invalidActionEventBasedParam2 << invalidActionEventBasedParam3);

    QVariantMap invalidActionEventBased3;
    invalidActionEventBased3.insert("actionTypeId", mockActionIdWithParams);
    invalidActionEventBased3.insert("deviceId", m_mockDeviceId);
    QVariantMap invalidActionEventBasedParam4;
    invalidActionEventBasedParam4.insert("paramTypeId", mockActionParam1ParamTypeId);
    invalidActionEventBasedParam4.insert("eventTypeId", mockEvent1Id);
    invalidActionEventBasedParam4.insert("eventParamTypeId", mockEvent1Id);
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
    eventParam1.insert("paramTypeId", mockParamIntParamTypeId);
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
    action2.insert("deviceId", m_mockDeviceId);
    QVariantList action2Params;
    QVariantMap action2Param1;
    action2Param1.insert("paramTypeId", mockActionParam1ParamTypeId);
    action2Param1.insert("value", 5);
    action2Params.append(action2Param1);
    QVariantMap action2Param2;
    action2Param2.insert("paramTypeId", mockActionParam2ParamTypeId);
    action2Param2.insert("value", true);
    action2Params.append(action2Param2);
    action2.insert("ruleActionParams", action2Params);

    // RuleAction event based
    QVariantMap validActionEventBased;
    validActionEventBased.insert("actionTypeId", mockActionIdWithParams);
    validActionEventBased.insert("deviceId", m_mockDeviceId);
    QVariantMap validActionEventBasedParam1;
    validActionEventBasedParam1.insert("paramTypeId", mockActionParam1ParamTypeId);
    validActionEventBasedParam1.insert("eventTypeId", mockEvent2Id);
    validActionEventBasedParam1.insert("eventParamTypeId", mockEvent2Id);
    QVariantMap validActionEventBasedParam2;
    validActionEventBasedParam2.insert("paramTypeId", mockActionParam2ParamTypeId);
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
    params.insert("eventDescriptors", eventDescriptorList1);
    params.insert("stateEvaluator", stateEvaluator0);
    params.insert("name", "TestRule");

    // Get rules and verify there is no rule added
    QNetworkRequest request(QUrl("https://localhost:3333/api/v1/rules"));
    QVariant response = getAndWait(request);
    QVariantList rulesList = response.toList();
    QVERIFY2(rulesList.count() == 0, "there should be no rules.");

    // ADD rule
    request.setUrl(QUrl(QString("https://localhost:3333/api/v1/rules")));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "text/json");
    response = postAndWait(request, params);
    QVERIFY(!response.isNull());

    RuleId ruleId = RuleId(response.toMap().value("id").toString());
    QVERIFY(!ruleId.isNull());

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

    // EDIT rule
    request.setUrl(QUrl(QString("https://localhost:3333/api/v1/rules/%1").arg(ruleId.toString())));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "text/json");
    response = putAndWait(request, params, expectedStatusCode);
    QVERIFY(!response.isNull());

    if (expectedStatusCode == 200) {
        // get edit rule and verify params
        request.setUrl(QUrl(QString("https://localhost:3333/api/v1/rules")));
        response = getAndWait(request);
        QVERIFY(!response.isNull());
    }

    // REMOVE rule
    request.setUrl(QUrl(QString("https://localhost:3333/api/v1/rules/%1").arg(ruleId.toString())));
    response = deleteAndWait(request);
    QVERIFY(!response.isNull());

    // check if removed
    request.setUrl(QUrl(QString("https://localhost:3333/api/v1/rules/%1").arg(ruleId.toString())));
    response = getAndWait(request, 404);
    QVERIFY(!response.isNull());
}

void TestRestRules::enableDisableRule()
{
    QVariantMap addRuleParams;
    QVariantList events;
    QVariantMap event1;
    event1.insert("eventTypeId", mockEvent1Id);
    event1.insert("deviceId", m_mockDeviceId);
    events.append(event1);
    addRuleParams.insert("eventDescriptors", events);
    addRuleParams.insert("name", "TestRule");

    QVariantList actions;
    QVariantMap action;
    action.insert("actionTypeId", mockActionIdNoParams);
    action.insert("deviceId", m_mockDeviceId);
    actions.append(action);
    addRuleParams.insert("actions", actions);

    // ADD rule
    QNetworkRequest request = QNetworkRequest(QUrl(QString("https://localhost:3333/api/v1/rules")));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "text/json");
    QVariant response = postAndWait(request, addRuleParams);
    RuleId ruleId = RuleId(response.toMap().value("id").toString());
    QVERIFY(!ruleId.isNull());

    // ENABLE rule
    request = QNetworkRequest(QUrl(QString("https://localhost:3333/api/v1/rules/%1/enable").arg(ruleId.toString())));
    response = postAndWait(request, QVariant());
    QVERIFY2(!response.isNull(), "Could not read response");

    // Trigger an event
    triggerMockEvent();
    verifyRuleExecuted(mockActionIdNoParams);

    cleanupMockHistory();

    // DISABLE the rule
    request = QNetworkRequest(QUrl(QString("https://localhost:3333/api/v1/rules/%1/disable").arg(ruleId.toString())));
    response = postAndWait(request, QVariant());
    QVERIFY2(!response.isNull(), "Could not read response");

    // trigger event in mock device
    triggerMockEvent();
    verifyRuleNotExecuted();

    cleanupMockHistory();

    // ENABLE again
    request = QNetworkRequest(QUrl(QString("https://localhost:3333/api/v1/rules/%1/enable").arg(ruleId.toString())));
    response = postAndWait(request, QVariant());
    QVERIFY2(!response.isNull(), "Could not read response");

    // trigger event in mock device
    triggerMockEvent();
    verifyRuleExecuted(mockActionIdNoParams);

    cleanupRules();
}

void TestRestRules::executeRuleActions_data()
{
    QTest::addColumn<QVariant>("params");
    QTest::addColumn<int>("expectedStatusCode");
    QTest::addColumn<RuleEngine::RuleError>("ruleError");

    QTest::newRow("executable rule, enabled") << validIntStateBasedRule("Executeable", true, true) << 200 << RuleEngine::RuleErrorNoError;
    QTest::newRow("executable rule, disabled") << validIntStateBasedRule("Executeable", true, false) << 200 << RuleEngine::RuleErrorNoError;
    QTest::newRow("not executable rule, enabled") << validIntStateBasedRule("Not Executable", false, true) << 500 << RuleEngine::RuleErrorNotExecutable;
    QTest::newRow("not executable rule, disabled") << validIntStateBasedRule("Not Executable", false, false) << 500 << RuleEngine::RuleErrorNotExecutable;
}

void TestRestRules::executeRuleActions()
{
    QFETCH(QVariant, params);
    QFETCH(int, expectedStatusCode);
    QFETCH(RuleEngine::RuleError, ruleError);

    // ADD rule
    QNetworkRequest request(QUrl(QString("https://localhost:3333/api/v1/rules")));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "text/json");
    QVariant response = postAndWait(request, params);

    RuleId ruleId = RuleId(response.toMap().value("id").toString());
    QVERIFY(!ruleId.isNull());

    request.setUrl(QUrl(QString("https://localhost:3333/api/v1/rules/%1").arg(ruleId.toString())));
    response = getAndWait(request);

    cleanupMockHistory();

    // EXECUTE actions
    request.setUrl(QUrl(QString("https://localhost:3333/api/v1/rules/%1/executeactions").arg(ruleId.toString())));
    response = postAndWait(request, QVariant(), expectedStatusCode);
    QVERIFY(!response.isNull());
    QCOMPARE(JsonTypes::ruleErrorToString(ruleError), response.toMap().value("error").toString());

    if (ruleError == RuleEngine::RuleErrorNoError) {
        verifyRuleExecuted(mockActionIdWithParams);
    } else {
        verifyRuleNotExecuted();
    }

    cleanupMockHistory();

    // EXECUTE exit actions
    request.setUrl(QUrl(QString("https://localhost:3333/api/v1/rules/%1/executeexitactions").arg(ruleId.toString())));
    response = postAndWait(request, QVariant(), expectedStatusCode);
    QVERIFY(!response.isNull());
    QCOMPARE(JsonTypes::ruleErrorToString(ruleError), response.toMap().value("error").toString());

    if (ruleError == RuleEngine::RuleErrorNoError) {
        verifyRuleExecuted(mockActionIdNoParams);
    } else {
        verifyRuleNotExecuted();
    }

    cleanupMockHistory();

    // REMOVE rule
    request.setUrl(QUrl(QString("https://localhost:3333/api/v1/rules/%1").arg(ruleId.toString())));
    response = deleteAndWait(request);
    QVERIFY(!response.isNull());
    QCOMPARE(JsonTypes::ruleErrorToString(RuleEngine::RuleErrorNoError), response.toMap().value("error").toString());

    // check if removed
    request.setUrl(QUrl(QString("https://localhost:3333/api/v1/rules/%1").arg(ruleId.toString())));
    response = getAndWait(request, 404);
    QVERIFY(!response.isNull());
}



#include "testrestrules.moc"
QTEST_MAIN(TestRestRules)
