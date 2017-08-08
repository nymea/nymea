/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
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

#include <QtTest/QtTest>
#include <QCoreApplication>
#include <QTcpSocket>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QCoreApplication>
#include <QMetaType>

using namespace guhserver;

class TestJSONRPC: public GuhTestBase
{
    Q_OBJECT

private slots:
    void testHandshake();

    void testInitialSetup();

    void testRevokeToken();

    void testBasicCall_data();
    void testBasicCall();

    void introspect();

    void enableDisableNotifications_data();
    void enableDisableNotifications();

    void deviceAddedRemovedNotifications();
    void ruleAddedRemovedNotifications();

    void ruleActiveChangedNotifications();

    void deviceChangedNotifications();

    void stateChangeEmitsNotifications();

    void pluginConfigChangeEmitsNotification();

private:
    QStringList extractRefs(const QVariant &variant);

};

QStringList TestJSONRPC::extractRefs(const QVariant &variant)
{
    if (variant.canConvert(QVariant::String)) {
        if (variant.toString().startsWith("$ref")) {
            return QStringList() << variant.toString();
        }
    }
    if (variant.canConvert(QVariant::List)) {
        QStringList refs;
        foreach (const QVariant tmp, variant.toList()) {
            refs << extractRefs(tmp);
        }
        return refs;
    }
    if (variant.canConvert(QVariant::Map)) {
        QStringList refs;
        foreach (const QVariant tmp, variant.toMap()) {
            refs << extractRefs(tmp);
        }
        return refs;
    }
    return QStringList();
}

void TestJSONRPC::testHandshake()
{
    QSignalSpy spy(m_mockTcpServer, SIGNAL(outgoingData(QUuid,QByteArray)));

    QUuid newClientId = QUuid::createUuid();
    m_mockTcpServer->clientConnected(newClientId);
    QVERIFY2(spy.count() > 0, "Did not get the handshake message upon connect.");
    QVERIFY2(spy.first().first() == newClientId, "Handshake message addressed at the wrong client.");

    QJsonDocument jsonDoc = QJsonDocument::fromJson(spy.first().at(1).toByteArray());
    QVariantMap handShake = jsonDoc.toVariant().toMap();
    QString guhVersionString(GUH_VERSION_STRING);
    QVERIFY2(handShake.value("version").toString() == guhVersionString, "Handshake version doesn't match Guh version.");

    m_mockTcpServer->clientDisconnected(newClientId);
}

void TestJSONRPC::testInitialSetup()
{
    foreach (const QString &user, GuhCore::instance()->userManager()->users()) {
        GuhCore::instance()->userManager()->removeUser(user);
    }
    QCOMPARE(GuhCore::instance()->userManager()->users().count(), 0);

    QSignalSpy spy(m_mockTcpServer, SIGNAL(outgoingData(QUuid,QByteArray)));
    QVERIFY(spy.isValid());

    // Introspect call should work in any case
    m_mockTcpServer->injectData(m_clientId, "{\"id\": 555, \"method\": \"JSONRPC.Introspect\"}");
    if (spy.count() == 0) {
        spy.wait();
    }
    QVERIFY(spy.count() == 1);
    QJsonDocument jsonDoc = QJsonDocument::fromJson(spy.first().at(1).toByteArray());
    QVariantMap response = jsonDoc.toVariant().toMap();
    qWarning() << "Calling introspect on uninitialized instance:" << response.value("status").toString() << response.value("error").toString();
    QCOMPARE(response.value("status").toString(), QStringLiteral("success"));


    // Any other call should fail with "unauthorized" even if we use a previously valid token
    spy.clear();
    m_mockTcpServer->injectData(m_clientId, "{\"id\": 555, \"token\": \"" + m_apiToken + "\", \"method\": \"JSONRPC.Version\"}");
    if (spy.count() == 0) {
        spy.wait();
    }
    QVERIFY(spy.count() == 1);
    jsonDoc = QJsonDocument::fromJson(spy.first().at(1).toByteArray());
    response = jsonDoc.toVariant().toMap();
    qWarning() << "Calling Version on uninitialized instance:" << response.value("status").toString() << response.value("error").toString();
    QCOMPARE(response.value("status").toString(), QStringLiteral("unauthorized"));

    // Except CreateUser

    // But it should still fail when giving a an invalid username
    spy.clear();
    m_mockTcpServer->injectData(m_clientId, "{\"id\": 555, \"method\": \"JSONRPC.CreateUser\", \"params\": {\"username\": \"dummy\", \"password\": \"DummyPW1!\"}}");
    if (spy.count() == 0) {
        spy.wait();
    }
    QVERIFY(spy.count() == 1);
    jsonDoc = QJsonDocument::fromJson(spy.first().at(1).toByteArray());
    response = jsonDoc.toVariant().toMap();
    qWarning() << "Calling CreateUser on uninitialized instance with invalid user:" << response.value("status").toString() << response.value("params").toMap().value("error").toString();
    QCOMPARE(response.value("status").toString(), QStringLiteral("success"));
    QCOMPARE(GuhCore::instance()->userManager()->users().count(), 0);

    // or when giving a bad password
    spy.clear();
    m_mockTcpServer->injectData(m_clientId, "{\"id\": 555, \"method\": \"JSONRPC.CreateUser\", \"params\": {\"username\": \"dummy@guh.io\", \"password\": \"weak\"}}");
    if (spy.count() == 0) {
        spy.wait();
    }
    QVERIFY(spy.count() == 1);
    jsonDoc = QJsonDocument::fromJson(spy.first().at(1).toByteArray());
    response = jsonDoc.toVariant().toMap();
    qWarning() << "Calling CreateUser on uninitialized instance with weak password:" << response.value("status").toString() << response.value("params").toMap().value("error").toString();
    QCOMPARE(response.value("status").toString(), QStringLiteral("success"));
    QCOMPARE(GuhCore::instance()->userManager()->users().count(), 0);

    // Now lets play by the rules
    spy.clear();
    m_mockTcpServer->injectData(m_clientId, "{\"id\": 555, \"method\": \"JSONRPC.CreateUser\", \"params\": {\"username\": \"dummy@guh.io\", \"password\": \"DummyPW1!\"}}");
    if (spy.count() == 0) {
        spy.wait();
    }
    QVERIFY(spy.count() == 1);
    jsonDoc = QJsonDocument::fromJson(spy.first().at(1).toByteArray());
    response = jsonDoc.toVariant().toMap();
    qWarning() << "Calling CreateUser on uninitialized instance:" << response.value("status").toString() << response.value("error").toString();
    QCOMPARE(response.value("status").toString(), QStringLiteral("success"));
    QCOMPARE(GuhCore::instance()->userManager()->users().count(), 1);

    // Calls should still fail, given we didn't get a new token yet
    spy.clear();
    m_mockTcpServer->injectData(m_clientId, "{\"id\": 555, \"token\": \"" + m_apiToken + "\", \"method\": \"JSONRPC.Version\"}");
    if (spy.count() == 0) {
        spy.wait();
    }
    QVERIFY(spy.count() == 1);
    jsonDoc = QJsonDocument::fromJson(spy.first().at(1).toByteArray());
    response = jsonDoc.toVariant().toMap();
    qWarning() << "Calling Version with old token:" << response.value("status").toString() << response.value("error").toString();
    QCOMPARE(response.value("status").toString(), QStringLiteral("unauthorized"));

    // Now lets authenticate with a wrong user
    spy.clear();
    m_mockTcpServer->injectData(m_clientId, "{\"id\": 555, \"method\": \"JSONRPC.Authenticate\", \"params\": {\"username\": \"dummy@wrong.domain\", \"password\": \"DummyPW1!\", \"deviceName\": \"testcase\"}}");
    if (spy.count() == 0) {
        spy.wait();
    }
    QVERIFY(spy.count() == 1);
    jsonDoc = QJsonDocument::fromJson(spy.first().at(1).toByteArray());
    response = jsonDoc.toVariant().toMap();
    qWarning() << "Calling Authenticate with wrong user:" << response.value("params").toMap().value("success").toString() << response.value("params").toMap().value("token").toString();
    QCOMPARE(response.value("status").toString(), QStringLiteral("success"));
    QCOMPARE(response.value("params").toMap().value("success").toBool(), false);
    QVERIFY(response.value("params").toMap().value("token").toByteArray().isEmpty());


    // Now lets authenticate with a wrong password
    spy.clear();
    m_mockTcpServer->injectData(m_clientId, "{\"id\": 555, \"method\": \"JSONRPC.Authenticate\", \"params\": {\"username\": \"dummy@guh.io\", \"password\": \"wrongpw\", \"deviceName\": \"testcase\"}}");
    if (spy.count() == 0) {
        spy.wait();
    }
    QVERIFY(spy.count() == 1);
    jsonDoc = QJsonDocument::fromJson(spy.first().at(1).toByteArray());
    response = jsonDoc.toVariant().toMap();
    qWarning() << "Calling Authenticate with wrong password:" << response.value("params").toMap().value("success").toString() << response.value("params").toMap().value("token").toString();
    QCOMPARE(response.value("status").toString(), QStringLiteral("success"));
    QCOMPARE(response.value("params").toMap().value("success").toBool(), false);
    QVERIFY(response.value("params").toMap().value("token").toByteArray().isEmpty());


    // Now lets authenticate for real
    spy.clear();
    m_mockTcpServer->injectData(m_clientId, "{\"id\": 555, \"method\": \"JSONRPC.Authenticate\", \"params\": {\"username\": \"dummy@guh.io\", \"password\": \"DummyPW1!\", \"deviceName\": \"testcase\"}}");
    if (spy.count() == 0) {
        spy.wait();
    }
    QVERIFY(spy.count() == 1);
    jsonDoc = QJsonDocument::fromJson(spy.first().at(1).toByteArray());
    response = jsonDoc.toVariant().toMap();
    qWarning() << "Calling Authenticate with valid credentials:" << response.value("params").toMap().value("success").toString() << response.value("params").toMap().value("token").toString();
    QCOMPARE(response.value("status").toString(), QStringLiteral("success"));
    QCOMPARE(response.value("params").toMap().value("success").toBool(), true);
    m_apiToken = response.value("params").toMap().value("token").toByteArray();
    QVERIFY(!m_apiToken.isEmpty());

    // Now do a Version call with the valid token and it should work
    spy.clear();
    m_mockTcpServer->injectData(m_clientId, "{\"id\": 555, \"token\": \"" + m_apiToken + "\", \"method\": \"JSONRPC.Version\"}");
    if (spy.count() == 0) {
        spy.wait();
    }
    QVERIFY(spy.count() == 1);
    jsonDoc = QJsonDocument::fromJson(spy.first().at(1).toByteArray());
    response = jsonDoc.toVariant().toMap();
    qWarning() << "Calling Version with valid token:" << response.value("status").toString() << response.value("error").toString();
    QCOMPARE(response.value("status").toString(), QStringLiteral("success"));

}

void TestJSONRPC::testRevokeToken()
{
    QSignalSpy spy(m_mockTcpServer, SIGNAL(outgoingData(QUuid,QByteArray)));
    QVERIFY(spy.isValid());

    // Now get all the tokens
    spy.clear();
    m_mockTcpServer->injectData(m_clientId, "{\"id\": 123, \"token\": \"" + m_apiToken + "\", \"method\": \"JSONRPC.Tokens\"}");
    if (spy.count() == 0) {
        spy.wait();
    }
    QVERIFY(spy.count() == 1);
    QJsonDocument jsonDoc = QJsonDocument::fromJson(spy.first().at(1).toByteArray());
    QVariantMap response = jsonDoc.toVariant().toMap();
    qWarning() << "Getting existing Tokens" << response.value("status").toString() << response;
    QCOMPARE(response.value("status").toString(), QStringLiteral("success"));
    QVariantList tokenList = response.value("params").toMap().value("tokenInfoList").toList();
    QCOMPARE(tokenList.count(), 1);
    QUuid oldTokenId = tokenList.first().toMap().value("id").toUuid();

    // Authenticate and create a new token
    spy.clear();
    m_mockTcpServer->injectData(m_clientId, "{\"id\": 555, \"method\": \"JSONRPC.Authenticate\", \"params\": {\"username\": \"dummy@guh.io\", \"password\": \"DummyPW1!\", \"deviceName\": \"testcase\"}}");
    if (spy.count() == 0) {
        spy.wait();
    }
    QVERIFY(spy.count() == 1);
    jsonDoc = QJsonDocument::fromJson(spy.first().at(1).toByteArray());
    response = jsonDoc.toVariant().toMap();
    qWarning() << "Calling Authenticate with valid credentials:" << response.value("params").toMap().value("success").toString() << response.value("params").toMap().value("token").toString();
    QCOMPARE(response.value("status").toString(), QStringLiteral("success"));
    QCOMPARE(response.value("params").toMap().value("success").toBool(), true);
    QByteArray newToken = response.value("params").toMap().value("token").toByteArray();
    QVERIFY(!newToken.isEmpty());

    // Now do a Version call with the new token and it should work
    spy.clear();
    m_mockTcpServer->injectData(m_clientId, "{\"id\": 555, \"token\": \"" + newToken + "\", \"method\": \"JSONRPC.Version\"}");
    if (spy.count() == 0) {
        spy.wait();
    }
    QVERIFY(spy.count() == 1);
    jsonDoc = QJsonDocument::fromJson(spy.first().at(1).toByteArray());
    response = jsonDoc.toVariant().toMap();
    qWarning() << "Calling Version with valid token:" << response.value("status").toString() << response.value("error").toString();
    QCOMPARE(response.value("status").toString(), QStringLiteral("success"));

    // Now get all the tokens using the old token
    spy.clear();
    m_mockTcpServer->injectData(m_clientId, "{\"id\": 123, \"token\": \"" + m_apiToken + "\", \"method\": \"JSONRPC.Tokens\"}");
    if (spy.count() == 0) {
        spy.wait();
    }
    QVERIFY(spy.count() == 1);
    jsonDoc = QJsonDocument::fromJson(spy.first().at(1).toByteArray());
    response = jsonDoc.toVariant().toMap();
    qWarning() << "Calling Tokens" << response.value("status").toString();
    QCOMPARE(response.value("status").toString(), QStringLiteral("success"));
    tokenList = response.value("params").toMap().value("tokenInfoList").toList();
    QCOMPARE(tokenList.count(), 2);

    // find the new token
    QUuid newTokenId;
    foreach (const QVariant &tokenInfo, tokenList) {
        if (tokenInfo.toMap().value("id").toUuid() != oldTokenId) {
            newTokenId = tokenInfo.toMap().value("id").toUuid();
            break;
        }
    }

    // Revoke the new token
    spy.clear();
    m_mockTcpServer->injectData(m_clientId, "{\"id\": 123, \"token\": \"" + m_apiToken + "\", \"method\": \"JSONRPC.RemoveToken\", \"params\": {\"tokenId\": \"" + newTokenId.toByteArray() + "\"}}");
    if (spy.count() == 0) {
        spy.wait();
    }
    QVERIFY(spy.count() == 1);
    jsonDoc = QJsonDocument::fromJson(spy.first().at(1).toByteArray());
    response = jsonDoc.toVariant().toMap();
    qWarning() << "Calling RemoveToken" << response.value("status").toString() << response;
    QCOMPARE(response.value("status").toString(), QStringLiteral("success"));

    // Do a call with the now removed token, it should be forbidden
    spy.clear();
    m_mockTcpServer->injectData(m_clientId, "{\"id\": 555, \"token\": \"" + newToken + "\", \"method\": \"JSONRPC.Version\"}");
    if (spy.count() == 0) {
        spy.wait();
    }
    QVERIFY(spy.count() == 1);
    jsonDoc = QJsonDocument::fromJson(spy.first().at(1).toByteArray());
    response = jsonDoc.toVariant().toMap();
    qWarning() << "Calling Version with valid token:" << response.value("status").toString() << response.value("error").toString();
    QCOMPARE(response.value("status").toString(), QStringLiteral("unauthorized"));
}

void TestJSONRPC::testBasicCall_data()
{
    QTest::addColumn<QByteArray>("call");
    QTest::addColumn<bool>("idValid");
    QTest::addColumn<bool>("valid");

    QTest::newRow("valid call") << QByteArray("{\"id\":42, \"method\":\"JSONRPC.Introspect\"}") << true << true;
    QTest::newRow("missing id") << QByteArray("{\"method\":\"JSONRPC.Introspect\"}") << false << false;
    QTest::newRow("missing method") << QByteArray("{\"id\":42}") << true << false;
    QTest::newRow("borked") << QByteArray("{\"id\":42, \"method\":\"JSO") << false << false;
    QTest::newRow("invalid function") << QByteArray("{\"id\":42, \"method\":\"JSONRPC.Foobar\"}") << true << false;
    QTest::newRow("invalid namespace") << QByteArray("{\"id\":42, \"method\":\"FOO.Introspect\"}") << true << false;
    QTest::newRow("missing dot") << QByteArray("{\"id\":42, \"method\":\"JSONRPCIntrospect\"}") << true << false;
    QTest::newRow("invalid params") << QByteArray("{\"id\":42, \"method\":\"JSONRPC.Introspect\", \"params\":{\"törööö\":\"chooo-chooo\"}}") << true << false;
}

void TestJSONRPC::testBasicCall()
{
    QFETCH(QByteArray, call);
    QFETCH(bool, idValid);
    QFETCH(bool, valid);

    QSignalSpy spy(m_mockTcpServer, SIGNAL(outgoingData(QUuid,QByteArray)));
    QVERIFY(spy.isValid());

    m_mockTcpServer->injectData(m_clientId, call);

    if (spy.count() == 0) {
        spy.wait();
    }

    // Make sure we got exactly one response
    QVERIFY(spy.count() == 1);

    // Make sure the response goes to the correct clientId
    QCOMPARE(spy.first().first().toString(), m_clientId.toString());

    // Make sure the response it a valid JSON string
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(spy.first().last().toByteArray(), &error);
    QCOMPARE(error.error, QJsonParseError::NoError);

    // Make sure the response\"s id is the same as our command
    if (idValid) {
        QCOMPARE(jsonDoc.toVariant().toMap().value("id").toInt(), 42);
    }
    if (valid) {
        QVERIFY2(jsonDoc.toVariant().toMap().value("status").toString() == "success", "Call wasn't parsed correctly by guh.");
    }
}

void TestJSONRPC::introspect()
{
    QVariant response = injectAndWait("JSONRPC.Introspect");
    QVariantMap methods = response.toMap().value("params").toMap().value("methods").toMap();
    QVariantMap notifications = response.toMap().value("params").toMap().value("notifications").toMap();
    QVariantMap types = response.toMap().value("params").toMap().value("types").toMap();

    QVERIFY2(methods.count() > 0, "No methods in Introspect response!");
    QVERIFY2(notifications.count() > 0, "No notifications in Introspect response!");
    QVERIFY2(types.count() > 0, "No types in Introspect response!");

    // Make sure all $ref: pointers have their according type defined
    QVariantMap allItems = methods.unite(notifications).unite(types);
    foreach (const QVariant &item, allItems) {
        foreach (const QString &ref, extractRefs(item)) {
            QString typeId = ref;
            typeId.remove("$ref:");
            QVERIFY2(types.contains(typeId), QString("Undefined ref: %1").arg(ref).toLatin1().data());
        }
    }
}

void TestJSONRPC::enableDisableNotifications_data()
{
    QTest::addColumn<QString>("enabled");

    QTest::newRow("enabled") << "true";
    QTest::newRow("disabled") << "false";
}

void TestJSONRPC::enableDisableNotifications()
{
    QFETCH(QString, enabled);

    QVariantMap params;
    params.insert("enabled", enabled);
    QVariant response = injectAndWait("JSONRPC.SetNotificationStatus", params);

    QCOMPARE(response.toMap().value("params").toMap().value("enabled").toString(), enabled);
}

void TestJSONRPC::deviceAddedRemovedNotifications()
{
    // enable notificartions
    QCOMPARE(enableNotifications(), true);

    // Setup connection to mock client
    QSignalSpy clientSpy(m_mockTcpServer, SIGNAL(outgoingData(QUuid,QByteArray)));

    // add device and wait for notification
    QVariantList deviceParams;
    QVariantMap httpportParam;
    httpportParam.insert("paramTypeId", httpportParamTypeId);
    httpportParam.insert("value", 8765);
    deviceParams.append(httpportParam);

    QVariantMap params; clientSpy.clear();
    params.insert("deviceClassId", mockDeviceClassId);
    params.insert("name", "Mock device");
    params.insert("deviceParams", deviceParams);
    QVariant response = injectAndWait("Devices.AddConfiguredDevice", params);
    clientSpy.wait(2000);
    verifyDeviceError(response);
    QVariantMap notificationDeviceMap = checkNotification(clientSpy, "Devices.DeviceAdded").toMap().value("params").toMap().value("device").toMap();

    DeviceId deviceId = DeviceId(response.toMap().value("params").toMap().value("deviceId").toString());
    QVERIFY(!deviceId.isNull());

    // check the DeviceAdded notification
    QCOMPARE(notificationDeviceMap.value("deviceClassId").toString(), mockDeviceClassId.toString());
    QCOMPARE(notificationDeviceMap.value("id").toString(), deviceId.toString());
    foreach (const QVariant &param, notificationDeviceMap.value("params").toList()) {
        if (param.toMap().value("name").toString() == "httpport") {
            QCOMPARE(param.toMap().value("value").toInt(), httpportParam.value("value").toInt());
        }
    }

    // now remove the device and check the device removed notification
    params.clear(); response.clear(); clientSpy.clear();
    params.insert("deviceId", deviceId);
    response = injectAndWait("Devices.RemoveConfiguredDevice", params);
    clientSpy.wait(2000);
    verifyDeviceError(response);
    checkNotification(clientSpy, "Devices.DeviceRemoved");

    QCOMPARE(disableNotifications(), true);
}

void TestJSONRPC::ruleAddedRemovedNotifications()
{
    // enable notificartions
    QCOMPARE(enableNotifications(), true);

    // Setup connection to mock client
    QSignalSpy clientSpy(m_mockTcpServer, SIGNAL(outgoingData(QUuid,QByteArray)));

    // Add rule and wait for notification
    // StateDescriptor
    QVariantMap stateDescriptor;
    stateDescriptor.insert("stateTypeId", mockIntStateId);
    stateDescriptor.insert("deviceId", m_mockDeviceId);
    stateDescriptor.insert("operator", JsonTypes::valueOperatorToString(Types::ValueOperatorLess));
    stateDescriptor.insert("value", "20");

    QVariantMap stateEvaluator;
    stateEvaluator.insert("stateDescriptor", stateDescriptor);

    // RuleAction
    QVariantMap actionNoParams;
    actionNoParams.insert("actionTypeId", mockActionIdNoParams);
    actionNoParams.insert("deviceId", m_mockDeviceId);
    actionNoParams.insert("ruleActionParams", QVariantList());

    // EventDescriptor
    QVariantMap eventDescriptor;
    eventDescriptor.insert("eventTypeId", mockEvent1Id);
    eventDescriptor.insert("deviceId", m_mockDeviceId);
    eventDescriptor.insert("paramDescriptors", QVariantList());

    QVariantMap params;
    params.insert("name", "Test Rule notifications");
    params.insert("actions", QVariantList() << actionNoParams);
    params.insert("eventDescriptors", QVariantList() << eventDescriptor);
    params.insert("stateEvaluator", stateEvaluator);

    QVariant response = injectAndWait("Rules.AddRule", params);
    clientSpy.wait(2000);
    QVariantMap notificationRuleMap = checkNotification(clientSpy, "Rules.RuleAdded").toMap().value("params").toMap().value("rule").toMap();
    verifyRuleError(response);

    RuleId ruleId = RuleId(response.toMap().value("params").toMap().value("ruleId").toString());
    QVERIFY(!ruleId.isNull());

    QCOMPARE(notificationRuleMap.value("enabled").toBool(), true);
    QCOMPARE(notificationRuleMap.value("name").toString(), params.value("name").toString());
    QCOMPARE(notificationRuleMap.value("id").toString(), ruleId.toString());
    QCOMPARE(notificationRuleMap.value("actions").toList(), QVariantList() << actionNoParams);
    QCOMPARE(notificationRuleMap.value("stateEvaluator").toMap().value("stateDescriptor").toMap(), stateDescriptor);
    QCOMPARE(notificationRuleMap.value("eventDescriptors").toList(), QVariantList() << eventDescriptor);
    QCOMPARE(notificationRuleMap.value("exitActions").toList(), QVariantList());

    // now remove the rule and check the RuleRemoved notification
    params.clear(); response.clear(); clientSpy.clear();
    params.insert("ruleId", ruleId);
    response = injectAndWait("Rules.RemoveRule", params);
    clientSpy.wait(2000);
    checkNotification(clientSpy, "Devices.DeviceRemoved");
    verifyRuleError(response);

    QCOMPARE(disableNotifications(), true);
}

void TestJSONRPC::ruleActiveChangedNotifications()
{
    // enable notificartions
    QVariantMap params;
    params.insert("enabled", true);
    QVariant response = injectAndWait("JSONRPC.SetNotificationStatus", params);
    QCOMPARE(response.toMap().value("params").toMap().value("enabled").toBool(), true);

    // Add rule and wait for notification
    // StateDescriptor
    QVariantMap stateDescriptor;
    stateDescriptor.insert("stateTypeId", mockIntStateId);
    stateDescriptor.insert("deviceId", m_mockDeviceId);
    stateDescriptor.insert("operator", JsonTypes::valueOperatorToString(Types::ValueOperatorEquals));
    stateDescriptor.insert("value", "20");

    QVariantMap stateEvaluator;
    stateEvaluator.insert("stateDescriptor", stateDescriptor);

    // RuleAction
    QVariantMap actionNoParams;
    actionNoParams.insert("actionTypeId", mockActionIdNoParams);
    actionNoParams.insert("deviceId", m_mockDeviceId);
    actionNoParams.insert("ruleActionParams", QVariantList());

    params.clear(); response.clear();
    params.insert("name", "Test Rule notifications");
    params.insert("actions", QVariantList() << actionNoParams);
    params.insert("stateEvaluator", stateEvaluator);

    // Setup connection to mock client
    QSignalSpy clientSpy(m_mockTcpServer, SIGNAL(outgoingData(QUuid,QByteArray)));

    response = injectAndWait("Rules.AddRule", params);
    clientSpy.wait();
    QVariant notificationVariant = checkNotification(clientSpy, "Rules.RuleAdded");
    verifyRuleError(response);

    QVariantMap notificationRuleMap = notificationVariant.toMap().value("params").toMap().value("rule").toMap();
    RuleId ruleId = RuleId(notificationRuleMap.value("id").toString());
    QVERIFY(!ruleId.isNull());
    QCOMPARE(notificationRuleMap.value("enabled").toBool(), true);
    QCOMPARE(notificationRuleMap.value("name").toString(), params.value("name").toString());
    QCOMPARE(notificationRuleMap.value("id").toString(), ruleId.toString());
    QCOMPARE(notificationRuleMap.value("actions").toList(), QVariantList() << actionNoParams);
    QCOMPARE(notificationRuleMap.value("stateEvaluator").toMap().value("stateDescriptor").toMap(), stateDescriptor);
    QCOMPARE(notificationRuleMap.value("exitActions").toList(), QVariantList());

    // set the rule active
    QNetworkAccessManager nam;
    QSignalSpy spy(&nam, SIGNAL(finished(QNetworkReply*)));

    // state state to 20
    qDebug() << "setting mock int state to 20";
    QNetworkRequest request(QUrl(QString("http://localhost:%1/setstate?%2=%3").arg(m_mockDevice1Port).arg(mockIntStateId.toString()).arg(20)));
    QNetworkReply *reply = nam.get(request);
    connect(reply, SIGNAL(finished()), reply, SLOT(deleteLater()));

    spy.wait();
    notificationVariant = checkNotification(clientSpy, "Rules.RuleActiveChanged");
    verifyRuleError(response);

    QCOMPARE(notificationVariant.toMap().value("params").toMap().value("ruleId").toString(), ruleId.toString());
    QCOMPARE(notificationVariant.toMap().value("params").toMap().value("active").toBool(), true);

    spy.clear(); clientSpy.clear();

    // set the rule inactive
    qDebug() << "setting mock int state to 42";
    QNetworkRequest request2(QUrl(QString("http://localhost:%1/setstate?%2=%3").arg(m_mockDevice1Port).arg(mockIntStateId.toString()).arg(42)));
    QNetworkReply *reply2 = nam.get(request2);
    spy.wait();
    QCOMPARE(spy.count(), 1);
    connect(reply2, SIGNAL(finished()), reply2, SLOT(deleteLater()));


    clientSpy.wait();
    notificationVariant = checkNotification(clientSpy, "Rules.RuleActiveChanged");
    verifyRuleError(response);

    QCOMPARE(notificationVariant.toMap().value("params").toMap().value("ruleId").toString(), ruleId.toString());
    QCOMPARE(notificationVariant.toMap().value("params").toMap().value("active").toBool(), false);

    // now remove the rule and check the RuleRemoved notification
    params.clear(); response.clear();
    params.insert("ruleId", ruleId);
    response = injectAndWait("Rules.RemoveRule", params);

    clientSpy.wait();
    notificationVariant = checkNotification(clientSpy, "Rules.RuleRemoved");
    checkNotification(clientSpy, "Logging.LogDatabaseUpdated");
    verifyRuleError(response);

    QCOMPARE(notificationVariant.toMap().value("params").toMap().value("ruleId").toString(), ruleId.toString());
}

void TestJSONRPC::deviceChangedNotifications()
{
    // enable notificartions
    QVariantMap params;
    params.insert("enabled", true);
    QVariant response = injectAndWait("JSONRPC.SetNotificationStatus", params);
    QCOMPARE(response.toMap().value("params").toMap().value("enabled").toBool(), true);

    // Setup connection to mock client
    QSignalSpy clientSpy(m_mockTcpServer, SIGNAL(outgoingData(QUuid,QByteArray)));

    // ADD
    // add device and wait for notification
    QVariantList deviceParams;
    QVariantMap httpportParam;
    httpportParam.insert("paramTypeId", httpportParamTypeId);
    httpportParam.insert("value", 23234);
    deviceParams.append(httpportParam);

    params.clear(); response.clear(); clientSpy.clear();
    params.insert("deviceClassId", mockDeviceClassId);
    params.insert("name", "Mock");
    params.insert("deviceParams", deviceParams);
    response = injectAndWait("Devices.AddConfiguredDevice", params);
    DeviceId deviceId = DeviceId(response.toMap().value("params").toMap().value("deviceId").toString());
    QVERIFY(!deviceId.isNull());
    clientSpy.wait();
    verifyDeviceError(response);
    QVariantMap notificationDeviceMap = checkNotification(clientSpy, "Devices.DeviceAdded").toMap().value("params").toMap().value("device").toMap();

    QCOMPARE(notificationDeviceMap.value("deviceClassId").toString(), mockDeviceClassId.toString());
    QCOMPARE(notificationDeviceMap.value("id").toString(), deviceId.toString());
    foreach (const QVariant &param, notificationDeviceMap.value("params").toList()) {
        if (param.toMap().value("name").toString() == "httpport") {
            QCOMPARE(param.toMap().value("value").toInt(), httpportParam.value("value").toInt());
        }
    }

    // RECONFIGURE
    // now reconfigure the device and check the deviceChanged notification
    QVariantList newDeviceParams;
    QVariantMap newHttpportParam;
    newHttpportParam.insert("paramTypeId", httpportParamTypeId);
    newHttpportParam.insert("value", 45473);
    newDeviceParams.append(newHttpportParam);

    params.clear(); response.clear(); clientSpy.clear();
    params.insert("deviceId", deviceId);
    params.insert("deviceParams", newDeviceParams);
    response = injectAndWait("Devices.ReconfigureDevice", params);
    clientSpy.wait(2000);
    verifyDeviceError(response);
    QVariantMap reconfigureDeviceNotificationMap = checkNotification(clientSpy, "Devices.DeviceChanged").toMap().value("params").toMap().value("device").toMap();
    QCOMPARE(reconfigureDeviceNotificationMap.value("deviceClassId").toString(), mockDeviceClassId.toString());
    QCOMPARE(reconfigureDeviceNotificationMap.value("id").toString(), deviceId.toString());
    foreach (const QVariant &param, reconfigureDeviceNotificationMap.value("params").toList()) {
        if (param.toMap().value("name").toString() == "httpport") {
            QCOMPARE(param.toMap().value("value").toInt(), newHttpportParam.value("value").toInt());
        }
    }

    // EDIT device name
    QString deviceName = "Test device 1234";
    params.clear(); response.clear(); clientSpy.clear();
    params.insert("deviceId", deviceId);
    params.insert("name", deviceName);
    response = injectAndWait("Devices.EditDevice", params);
    clientSpy.wait(2000);
    verifyDeviceError(response);
    QVariantMap editDeviceNotificationMap = checkNotification(clientSpy, "Devices.DeviceChanged").toMap().value("params").toMap().value("device").toMap();
    QCOMPARE(editDeviceNotificationMap.value("deviceClassId").toString(), mockDeviceClassId.toString());
    QCOMPARE(editDeviceNotificationMap.value("id").toString(), deviceId.toString());
    QCOMPARE(editDeviceNotificationMap.value("name").toString(), deviceName);

    // REMOVE
    // now remove the device and check the device removed notification
    params.clear(); response.clear(); clientSpy.clear();
    params.insert("deviceId", deviceId);
    response = injectAndWait("Devices.RemoveConfiguredDevice", params);
    clientSpy.wait();
    verifyDeviceError(response);
    checkNotification(clientSpy, "Devices.DeviceRemoved");
    checkNotification(clientSpy, "Logging.LogDatabaseUpdated");
}

void TestJSONRPC::stateChangeEmitsNotifications()
{
    QCOMPARE(enableNotifications(), true);
    bool found = false;

    // Setup connection to mock client
    QNetworkAccessManager nam;
    QSignalSpy clientSpy(m_mockTcpServer, SIGNAL(outgoingData(QUuid,QByteArray)));

    // trigger state change in mock device
    int newVal = 38;
    QUuid stateTypeId("80baec19-54de-4948-ac46-31eabfaceb83");
    QNetworkRequest request(QUrl(QString("http://localhost:%1/setstate?%2=%3").arg(m_mockDevice1Port).arg(stateTypeId.toString()).arg(QString::number(newVal))));
    QNetworkReply *reply = nam.get(request);
    connect(reply, SIGNAL(finished()), reply, SLOT(deleteLater()));
    QSignalSpy replySpy(reply, SIGNAL(finished()));
    replySpy.wait();

    // Make sure the notification contains all the stuff we expect
    QVariantList stateChangedVariants = checkNotifications(clientSpy, "Devices.StateChanged");
    QVERIFY2(!stateChangedVariants.isEmpty(), "Did not get Devices.StateChanged notification.");
    qDebug() << "got" << stateChangedVariants.count() << "Devices.StateChanged notifications";
    foreach (const QVariant &stateChangedVariant, stateChangedVariants) {
        if (stateChangedVariant.toMap().value("params").toMap().value("stateTypeId").toUuid() == stateTypeId) {
            found = true;
            QCOMPARE(stateChangedVariant.toMap().value("params").toMap().value("value").toInt(), newVal);
            break;
        }
    }
    if (!found)
        qDebug() << QJsonDocument::fromVariant(stateChangedVariants).toJson();

    QVERIFY2(found, "Could not find the correct Devices.StateChanged notification");


    // Make sure the logg notification contains all the stuff we expect
    QVariantList loggEntryAddedVariants = checkNotifications(clientSpy, "Logging.LogEntryAdded");
    QVERIFY2(!loggEntryAddedVariants.isEmpty(), "Did not get Logging.LogEntryAdded notification.");
    qDebug() << "got" << loggEntryAddedVariants.count() << "Logging.LogEntryAdded notifications";
    found = false;
    foreach (const QVariant &loggEntryAddedVariant, loggEntryAddedVariants) {
        if (loggEntryAddedVariant.toMap().value("params").toMap().value("logEntry").toMap().value("typeId").toUuid() == stateTypeId) {
            found = true;
            QCOMPARE(loggEntryAddedVariant.toMap().value("params").toMap().value("logEntry").toMap().value("source").toString(), QString("LoggingSourceStates"));
            QCOMPARE(loggEntryAddedVariant.toMap().value("params").toMap().value("logEntry").toMap().value("value").toInt(), newVal);
            break;
        }
    }
    if (!found)
        qDebug() << QJsonDocument::fromVariant(loggEntryAddedVariants).toJson();

    QVERIFY2(found, "Could not find the corresponding Logging.LogEntryAdded notification");


    // Make sure the notification contains all the stuff we expect
    QVariantList eventTriggeredVariants = checkNotifications(clientSpy, "Events.EventTriggered");
    QVERIFY2(!eventTriggeredVariants.isEmpty(), "Did not get Events.EventTriggered notification.");
    found = false;
    qDebug() << "got" << eventTriggeredVariants.count() << "Events.EventTriggered notifications";
    foreach (const QVariant &eventTriggeredVariant, eventTriggeredVariants) {
        if (eventTriggeredVariant.toMap().value("params").toMap().value("event").toMap().value("eventTypeId").toUuid() == stateTypeId) {
            found = true;
            QCOMPARE(eventTriggeredVariant.toMap().value("params").toMap().value("event").toMap().value("params").toList().first().toMap().value("value").toInt(), newVal);
            break;
        }
    }
    if (!found)
        qDebug() << QJsonDocument::fromVariant(eventTriggeredVariants).toJson();

    QVERIFY2(found, "Could not find the corresponding Events.EventTriggered notification");

    // Now turn off notifications
    QCOMPARE(disableNotifications(), true);

    // Fire the a statechange once again
    clientSpy.clear();
    newVal = 42;
    request.setUrl(QUrl(QString("http://localhost:%1/setstate?%2=%3").arg(m_mockDevice1Port).arg(stateTypeId.toString()).arg(newVal)));
    reply = nam.get(request);
    connect(reply, SIGNAL(finished()), reply, SLOT(deleteLater()));

    // Lets wait a max of 500ms for notifications
    clientSpy.wait(500);
    // but make sure it doesn't come
    QCOMPARE(clientSpy.count(), 0);

    // Now check that the state has indeed changed even though we didn't get a notification
    QVariantMap params;
    params.insert("deviceId", m_mockDeviceId);
    params.insert("stateTypeId", stateTypeId);
    QVariant response = injectAndWait("Devices.GetStateValue", params);

    QCOMPARE(response.toMap().value("params").toMap().value("value").toInt(), newVal);
}

void TestJSONRPC::pluginConfigChangeEmitsNotification()
{
    QSignalSpy clientSpy(m_mockTcpServer, SIGNAL(outgoingData(QUuid,QByteArray)));

    QVariantMap params;
    params.insert("enabled", true);
    QVariant response = injectAndWait("JSONRPC.SetNotificationStatus", params);
    QCOMPARE(response.toMap().value("params").toMap().value("enabled").toBool(), true);

    params.clear();
    params.insert("pluginId", mockPluginId);
    QVariantList pluginParams;
    QVariantMap param1;
    param1.insert("paramTypeId",  configParamIntParamTypeId);
    param1.insert("value", 42);
    pluginParams.append(param1);
    params.insert("configuration", pluginParams);

    response = injectAndWait("Devices.SetPluginConfiguration", params);

    QVariantList notificationData = checkNotifications(clientSpy, "Devices.PluginConfigurationChanged");
    QCOMPARE(notificationData.first().toMap().value("notification").toString() == "Devices.PluginConfigurationChanged", true);
}

#include "testjsonrpc.moc"

QTEST_MAIN(TestJSONRPC)
