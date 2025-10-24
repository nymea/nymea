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

#include "testusermanager.h"
#include "nymeacore.h"
#include "nymeatestbase.h"
#include "usermanager/usermanager.h"
#include "servers/mocktcpserver.h"
#include "nymeadbusservice.h"

#include "../../utils/pushbuttonagent.h"

#include "../plugins/mock/extern-plugininfo.h"

using namespace nymeaserver;

TestUsermanager::TestUsermanager(QObject *parent): NymeaTestBase(parent)
{
    QCoreApplication::instance()->setOrganizationName("nymea-test");
}

void TestUsermanager::initTestCase()
{
    NymeaDBusService::setBusType(QDBusConnection::SessionBus);
    NymeaTestBase::initTestCase("*.debug=false\n"
                                "Application.debug=true\n"
                                "Tests.debug=true\n"
                                "UserManager.debug=true\n"
                                "PushButtonAgent.debug=true\n"
                                "MockDevice.debug=true");
}

void TestUsermanager::init()
{
    UserManager *userManager = NymeaCore::instance()->userManager();
    foreach (const UserInfo &userInfo, userManager->users()) {
        qCDebug(dcTests()) << "Removing user" << userInfo.username();
        userManager->removeUser(userInfo.username());
    }
    userManager->removeUser("");
}

void TestUsermanager::loginValidation_data() {
    QTest::addColumn<QString>("username");
    QTest::addColumn<QString>("password");
    QTest::addColumn<UserManager::UserError>("expectedError");

    QTest::newRow("foo@bar.baz,    Bla1234*, NoError")       << "foo@bar.baz"    << "Bla1234*" << UserManager::UserErrorNoError;
    QTest::newRow("foo@bar.co.uk,  Bla1234*, NoError")       << "foo@bar.co.uk"  << "Bla1234*" << UserManager::UserErrorNoError;
    QTest::newRow("foo@bar.com.au, Bla1234*, NoError")       << "foo@bar.com.au" << "Bla1234*" << UserManager::UserErrorNoError;

    QTest::newRow("n,              Bla1234*, InvalidUserId") << "n"              << "Bla1234*" << UserManager::UserErrorInvalidUserId;
    QTest::newRow("@,              Bla1234*, InvalidUserId") << "@"              << "Bla1234*" << UserManager::UserErrorInvalidUserId;
    QTest::newRow("nymea,          Bla1234*, InvalidUserId") << "nymea"          << "Bla1234*" << UserManager::UserErrorNoError;

    QTest::newRow("foo@bar.baz, a,        BadPassword") << "foo@bar.baz" << "a"        << UserManager::UserErrorBadPassword;
    QTest::newRow("foo@bar.baz, a1,       BadPassword") << "foo@bar.baz" << "a1"       << UserManager::UserErrorBadPassword;
    QTest::newRow("foo@bar.baz, a1!,      BadPassword") << "foo@bar.baz" << "a1!"      << UserManager::UserErrorBadPassword;
    QTest::newRow("foo@bar.baz, aaaaaaaa, BadPassword") << "foo@bar.baz" << "aaaaaaaa" << UserManager::UserErrorBadPassword;
    QTest::newRow("foo@bar.baz, aaaaaaa1, BadPassword") << "foo@bar.baz" << "aaaaaaa1" << UserManager::UserErrorBadPassword;
    QTest::newRow("foo@bar.baz, aaaaaaa!, BadPassword") << "foo@bar.baz" << "aaaaaaa!" << UserManager::UserErrorBadPassword;
    QTest::newRow("foo@bar.baz, aaaaaaaA, BadPassword") << "foo@bar.baz" << "aaaaaaaA" << UserManager::UserErrorBadPassword;
    QTest::newRow("foo@bar.baz, aaaaaa!A, BadPassword") << "foo@bar.baz" << "aaaaaa!A" << UserManager::UserErrorBadPassword;
    QTest::newRow("foo@bar.baz, aaaaaa!1, BadPassword") << "foo@bar.baz" << "aaaaaa!1" << UserManager::UserErrorBadPassword;

    QTest::newRow("foo@bar.baz, aaaaa!1A, NoError")            << "foo@bar.baz" << "aaaaa!1A" << UserManager::UserErrorNoError;
    QTest::newRow("foo@bar.baz, Bla1234*a, NoError")           << "foo@bar.baz" << "Bla1234*a" << UserManager::UserErrorNoError;
    QTest::newRow("foo@bar.baz, #1-Nymea-is-awesome, NoError") << "foo@bar.baz" << "#1-Nymea-is-awesome" << UserManager::UserErrorNoError;
    QTest::newRow("foo@bar.baz, Bla1234.a, NoError")           << "foo@bar.baz" << "Bla1234.a" << UserManager::UserErrorNoError;
    QTest::newRow("foo@bar.baz, Bla1234\\a, NoError")          << "foo@bar.baz" << "Bla1234\\a" << UserManager::UserErrorNoError;
    QTest::newRow("foo@bar.baz, Bla1234@a, NoError")           << "foo@bar.baz" << "Bla1234@a" << UserManager::UserErrorNoError;

}

void TestUsermanager::loginValidation()
{
    QFETCH(QString, username);
    QFETCH(QString, password);
    QFETCH(UserManager::UserError, expectedError);

    UserManager *userManager = NymeaCore::instance()->userManager();
    UserManager::UserError error = userManager->createUser(username, password, "", "", Types::PermissionScopeAdmin);
    qDebug() << "Error:" << error << "Expected:" << expectedError;
    QCOMPARE(error, expectedError);
}

void TestUsermanager::createUser()
{
    QVariantMap params;
    params.insert("username", "valid@user.test");
    params.insert("password", "Bla1234*");
    QVariant response = injectAndWait("JSONRPC.CreateUser", params);

    QVERIFY2(response.toMap().value("status").toString() == "success", "Error creating user");
    QVERIFY2(response.toMap().value("params").toMap().value("error").toString() == "UserErrorNoError", "Error creating user");
}

void TestUsermanager::authenticate()
{
    m_apiToken.clear();
    injectAndWait("JSONRPC.Hello");

    createUser();

    QVariantMap params;
    params.insert("username", "valid@user.test");
    params.insert("password", "Bla1234*");
    params.insert("deviceName", "autotests");
    QVariant response = injectAndWait("JSONRPC.Authenticate", params);

    m_apiToken = response.toMap().value("params").toMap().value("token").toByteArray();

    QVERIFY2(response.toMap().value("status").toString() == "success", "Error authenticating");
    QVERIFY2(response.toMap().value("params").toMap().value("success").toString() == "true", "Error authenticating");
}

void TestUsermanager::authenticatePushButton()
{
    PushButtonAgent pushButtonAgent;
    pushButtonAgent.init(QDBusConnection::SessionBus);

    QVariantMap params;
    params.insert("deviceName", "pbtestdevice");
    QVariant response = injectAndWait("JSONRPC.RequestPushButtonAuth", params);
    qCDebug(dcTests()) << "Pushbutton auth response:" << qUtf8Printable(QJsonDocument::fromVariant(response).toJson(QJsonDocument::Indented));
    QCOMPARE(response.toMap().value("params").toMap().value("success").toBool(), true);
    int transactionId = response.toMap().value("params").toMap().value("transactionId").toInt();

    // Setup connection to mock client
    QSignalSpy clientSpy(m_mockTcpServer, &MockTcpServer::outgoingData);

    pushButtonAgent.sendButtonPressed();

    if (clientSpy.count() == 0) clientSpy.wait();
    QVariantMap rsp = checkNotification(clientSpy, "JSONRPC.PushButtonAuthFinished").toMap();

    for (int i = 0; i < clientSpy.count(); i++) {
        qCDebug(dcTests()) << "Notification:" << clientSpy.at(i);
    }

    QCOMPARE(rsp.value("params").toMap().value("transactionId").toInt(), transactionId);
    QVERIFY2(!rsp.value("params").toMap().value("token").toByteArray().isEmpty(), "Token not in push button auth notification");

    m_apiToken = rsp.value("params").toMap().value("token").toByteArray();

    qCDebug(dcTests()) << "Invoking Version";
    // Test a regular call to verify we're actually authenticated
    response = injectAndWait("JSONRPC.Version");
    QVERIFY2(response.toMap().value("status").toString() == "success", "JSONRPC.Version call failed after push button auth!");
}

void TestUsermanager::authenticatePushButtonAuthInterrupt()
{
    PushButtonAgent pushButtonAgent;
    pushButtonAgent.init(QDBusConnection::SessionBus);

    // m_clientId is registered in gutTestbase already, just using it here to improve readability of the test
    QUuid aliceId = m_clientId;

    // Create a new clientId for mallory and connect it to the server
    QUuid malloryId = QUuid::createUuid();
    m_mockTcpServer->clientConnected(malloryId);
    QSignalSpy responseSpy(m_mockTcpServer, &MockTcpServer::outgoingData);
    m_mockTcpServer->injectData(malloryId, "{\"id\": 0, \"method\": \"JSONRPC.Hello\"}");
    if (responseSpy.count() == 0) responseSpy.wait();

    // Snoop in on everything the TCP server sends to its clients.
    QSignalSpy clientSpy(m_mockTcpServer, &MockTcpServer::outgoingData);

    // request push button auth for client 1 (alice) and check for OK reply
    QVariantMap params;
    params.insert("deviceName", "alice");
    QVariant response = injectAndWait("JSONRPC.RequestPushButtonAuth", params, aliceId);
    QCOMPARE(response.toMap().value("params").toMap().value("success").toBool(), true);
    int transactionId1 = response.toMap().value("params").toMap().value("transactionId").toInt();


    // Request push button auth for client 2 (mallory)
    clientSpy.clear();
    params.clear();
    params.insert("deviceName", "mallory");
    response = injectAndWait("JSONRPC.RequestPushButtonAuth", params, malloryId, "");
    QCOMPARE(response.toMap().value("params").toMap().value("success").toBool(), true);
    int transactionId2 = response.toMap().value("params").toMap().value("transactionId").toInt();

    // Both clients should receive something. Wait for it
    if (clientSpy.count() < 2) {
        clientSpy.wait();
    }

    // spy.at(0) should be the failed notification for alice
    // spy.at(1) shpuld be the OK reply for mallory


    // alice should have received a failed notification. She knows something's wrong.
    QVariantMap notification = QJsonDocument::fromJson(clientSpy.first().at(1).toByteArray()).toVariant().toMap();
    QCOMPARE(clientSpy.first().first().toUuid(), aliceId);
    QCOMPARE(notification.value("notification").toString(), QLatin1String("JSONRPC.PushButtonAuthFinished"));
    QCOMPARE(notification.value("params").toMap().value("transactionId").toInt(), transactionId1);
    QCOMPARE(notification.value("params").toMap().value("success").toBool(), false);

    // Mallory instead should have received an OK
    QVariantMap reply = QJsonDocument::fromJson(clientSpy.at(1).at(1).toByteArray()).toVariant().toMap();
    QCOMPARE(clientSpy.at(1).first().toUuid(), malloryId);
    QCOMPARE(reply.value("params").toMap().value("success").toBool(), true);


    // Alice tries once more
    clientSpy.clear();
    params.clear();
    params.insert("deviceName", "alice");
    response = injectAndWait("JSONRPC.RequestPushButtonAuth", params, aliceId);
    QCOMPARE(response.toMap().value("params").toMap().value("success").toBool(), true);
    int transactionId3 = response.toMap().value("params").toMap().value("transactionId").toInt();

    // Both clients should receive something. Wait for it
    if (clientSpy.count() < 2) {
        clientSpy.wait();
    }

    // spy.at(0) should be the failed notification for mallory
    // spy.at(1) shpuld be the OK reply for alice

    // mallory should have received a failed notification. She knows something's wrong.
    notification = QJsonDocument::fromJson(clientSpy.first().at(1).toByteArray()).toVariant().toMap();
    QCOMPARE(clientSpy.first().first().toUuid(), malloryId);
    QCOMPARE(notification.value("notification").toString(), QLatin1String("JSONRPC.PushButtonAuthFinished"));
    QCOMPARE(notification.value("params").toMap().value("transactionId").toInt(), transactionId2);
    QCOMPARE(notification.value("params").toMap().value("success").toBool(), false);

    // Alice instead should have received an OK
    reply = QJsonDocument::fromJson(clientSpy.at(1).at(1).toByteArray()).toVariant().toMap();
    QCOMPARE(clientSpy.at(1).first().toUuid(), aliceId);
    QCOMPARE(reply.value("params").toMap().value("success").toBool(), true);

    clientSpy.clear();

    // do the button press
    pushButtonAgent.sendButtonPressed();

    // Wait for things to happen
    if (clientSpy.count() == 0) {
        clientSpy.wait();
    }

    // There should have been only exactly one message sent, the token for alice
    // Mallory should not have received anything
    QCOMPARE(clientSpy.count(), 1);
    notification = QJsonDocument::fromJson(clientSpy.first().at(1).toByteArray()).toVariant().toMap();
    QCOMPARE(clientSpy.first().first().toUuid(), aliceId);
    QCOMPARE(notification.value("notification").toString(), QLatin1String("JSONRPC.PushButtonAuthFinished"));
    QCOMPARE(notification.value("params").toMap().value("transactionId").toInt(), transactionId3);
    QCOMPARE(notification.value("params").toMap().value("success").toBool(), true);
    QVERIFY2(!notification.value("params").toMap().value("token").toByteArray().isEmpty(), "Token is empty while it shouldn't be");
}

void TestUsermanager::authenticatePushButtonAuthConnectionDrop()
{
    PushButtonAgent pushButtonAgent;
    pushButtonAgent.init(QDBusConnection::SessionBus);

    // Snoop in on everything the TCP server sends to its clients.
    QSignalSpy clientSpy(m_mockTcpServer, &MockTcpServer::outgoingData);

    // Create a new clientId for alice and connect it to the server
    QUuid aliceId = QUuid::createUuid();
    emit m_mockTcpServer->clientConnected(aliceId);
    m_mockTcpServer->injectData(aliceId, "{\"id\": 0, \"method\": \"JSONRPC.Hello\"}");
    if (clientSpy.count() == 0) clientSpy.wait();

    // request push button auth for client 1 (alice) and check for OK reply
    QVariantMap params;
    params.insert("deviceName", "alice");
    QVariant response = injectAndWait("JSONRPC.RequestPushButtonAuth", params, aliceId, "");
    QCOMPARE(response.toMap().value("params").toMap().value("success").toBool(), true);

    // Disconnect alice
    emit m_mockTcpServer->clientDisconnected(aliceId);

    // Now try with bob
    // Create a new clientId for bob and connect it to the server
    QUuid bobId = QUuid::createUuid();
    emit m_mockTcpServer->clientConnected(bobId);
    clientSpy.clear();
    m_mockTcpServer->injectData(bobId, "{\"id\": 0, \"method\": \"JSONRPC.Hello\"}");
    if (clientSpy.count() == 0) clientSpy.wait();

    // request push button auth for client 2 (bob) and check for OK reply
    params.clear();
    params.insert("deviceName", "bob");
    response = injectAndWait("JSONRPC.RequestPushButtonAuth", params, bobId, "");
    QCOMPARE(response.toMap().value("params").toMap().value("success").toBool(), true);
    int transactionId = response.toMap().value("params").toMap().value("transactionId").toInt();

    clientSpy.clear();

    pushButtonAgent.sendButtonPressed();

    // Wait for things to happen
    if (clientSpy.count() == 0) {
        clientSpy.wait();
    }

    // There should have been only exactly one message sent, the token for bob
    QCOMPARE(clientSpy.count(), 1);
    QVariantMap notification = QJsonDocument::fromJson(clientSpy.first().at(1).toByteArray()).toVariant().toMap();
    QCOMPARE(clientSpy.first().first().toUuid(), bobId);
    QCOMPARE(notification.value("notification").toString(), QLatin1String("JSONRPC.PushButtonAuthFinished"));
    QCOMPARE(notification.value("params").toMap().value("transactionId").toInt(), transactionId);
    QCOMPARE(notification.value("params").toMap().value("success").toBool(), true);
    QVERIFY2(!notification.value("params").toMap().value("token").toByteArray().isEmpty(), "Token is empty while it shouldn't be");

}

void TestUsermanager::createDuplicateUser()
{
    authenticate();

    QVariantMap params;
    params.insert("username", "valid@user.test");
    params.insert("password", "Bla1234*");
    QVariant response = injectAndWait("Users.CreateUser", params);

    QVERIFY2(response.toMap().value("status").toString() == "success", "Unexpected error code creating duplicate user");
    QVERIFY2(response.toMap().value("params").toMap().value("error").toString() == "UserErrorDuplicateUserId", "Unexpected error creating duplicate user");
}

void TestUsermanager::getTokens()
{
    authenticate();

    QVariant response = injectAndWait("Users.GetTokens");
    QVERIFY2(response.toMap().value("status").toString() == "success", "Unexpected error code creating duplicate user");
    QCOMPARE(response.toMap().value("params").toMap().value("error").toString(), QString("UserErrorNoError"));

    QVariantList tokenInfoList = response.toMap().value("params").toMap().value("tokenInfoList").toList();
    QCOMPARE(tokenInfoList.count(), 1);

    m_tokenId = tokenInfoList.first().toMap().value("id").toUuid();
    QVERIFY2(!m_tokenId.isNull(), "Token ID should not be null");
    QCOMPARE(tokenInfoList.first().toMap().value("username").toString(), QString("valid@user.test"));
    QCOMPARE(tokenInfoList.first().toMap().value("deviceName").toString(), QString("autotests"));
}

void TestUsermanager::removeToken()
{
    getTokens();

    QVariantMap params;
    params.insert("tokenId", m_tokenId);
    QVariant response = injectAndWait("Users.RemoveToken", params);
    QCOMPARE(response.toMap().value("status").toString(), QString("success"));
    QCOMPARE(response.toMap().value("params").toMap().value("error").toString(), QString("UserErrorNoError"));
}

void TestUsermanager::changePassword()
{
    authenticate();

    QVariantMap params;
    params.insert("newPassword", "Blubb123");
    QVariant response = injectAndWait("Users.ChangePassword", params);
    QCOMPARE(response.toMap().value("status").toString(), QString("success"));
    QCOMPARE(response.toMap().value("params").toMap().value("error").toString(), QString("UserErrorNoError"));
}

void TestUsermanager::authenticateAfterPasswordChangeOK()
{
    changePassword();

    QVariantMap params;
    params.insert("username", "valid@user.test");
    params.insert("password", "Blubb123"); // New password, should be ok
    params.insert("deviceName", "autotests");
    QVariant response = injectAndWait("JSONRPC.Authenticate", params);

    m_apiToken = response.toMap().value("params").toMap().value("token").toByteArray();
    QVERIFY2(!m_apiToken.isEmpty(), "Token should not be empty");
    QVERIFY2(response.toMap().value("status").toString() == "success", "Error authenticating");
    QVERIFY2(response.toMap().value("params").toMap().value("success").toString() == "true", "Error authenticating");
}

void TestUsermanager::authenticateAfterPasswordChangeFail()
{
    changePassword();

    QSignalSpy disconnectedSpy(m_mockTcpServer, &MockTcpServer::clientDisconnected);

    QVariantMap params;
    params.insert("username", "valid@user.test");
    params.insert("password", "Bla1234*"); // Original password, should not be ok
    params.insert("deviceName", "autotests");
    QVariant response = injectAndWait("JSONRPC.Authenticate", params);

    m_apiToken = response.toMap().value("params").toMap().value("token").toByteArray();
    QVERIFY2(m_apiToken.isEmpty(), "Token should be empty");
    QVERIFY2(response.toMap().value("status").toString() == "success", "Error authenticating");
    QCOMPARE(response.toMap().value("params").toMap().value("success").toString(), QString("false"));

    // Connection should drop
    if (disconnectedSpy.count() == 0) disconnectedSpy.wait();
    QVERIFY2(disconnectedSpy.count() == 1, "Connection should have dropped");

    QTest::qWait(3200);
    emit m_mockTcpServer->clientConnected(m_clientId);
    injectAndWait("JSONRPC.Hello");
}

void TestUsermanager::getUserInfo()
{
    authenticate();

    QVariant response = injectAndWait("Users.GetUserInfo");
    QCOMPARE(response.toMap().value("status").toString(), QString("success"));
    QVariantMap userInfoMap = response.toMap().value("params").toMap().value("userInfo").toMap();
    QCOMPARE(userInfoMap.value("username").toString(), QString("valid@user.test"));
}

void TestUsermanager::unauthenticatedCallAfterTokenRemove()
{
    removeToken();

    QSignalSpy spy(m_mockTcpServer, &MockTcpServer::connectionTerminated);

    QVariant response = injectAndWait("Users.GetTokens");
    QCOMPARE(response.toMap().value("status").toString(), QString("unauthorized"));

    if (spy.count() == 0) {
        spy.wait();
    }
    QVERIFY2(spy.count() == 1, "Connection should be terminated!");

    QTest::qWait(3200);
    emit m_mockTcpServer->clientConnected(m_clientId);
    injectAndWait("JSONRPC.Hello");
}

void TestUsermanager::testScopeConsitancy_data()
{
    QTest::addColumn<QList<Types::PermissionScope>>("scopes");
    QTest::addColumn<QString>("error");

    QTest::newRow("valid: admin")
        << (QList<Types::PermissionScope>()
            << Types::PermissionScopeAdmin)
        << "UserErrorNoError";

    QTest::newRow("valid: none")
        << (QList<Types::PermissionScope>()
            << Types::PermissionScopeNone)
        << "UserErrorNoError";

    QTest::newRow("valid: only control, not all things")
        << (QList<Types::PermissionScope>()
            << Types::PermissionScopeControlThings
            << Types::PermissionScopeAccessAllThings)
        << "UserErrorNoError";

    QTest::newRow("valid: only control, not all things")
        << (QList<Types::PermissionScope>()
            << Types::PermissionScopeControlThings
            << Types::PermissionScopeConfigureThings
            << Types::PermissionScopeAccessAllThings)
        << "UserErrorNoError";

    QTest::newRow("valid: only control, all things")
        << (QList<Types::PermissionScope>()
            << Types::PermissionScopeControlThings
            << Types::PermissionScopeAccessAllThings)
        << "UserErrorNoError";

    QTest::newRow("valid: control things/rules, all things")
        << (QList<Types::PermissionScope>()
            << Types::PermissionScopeControlThings
            << Types::PermissionScopeAccessAllThings
            << Types::PermissionScopeExecuteRules)
        << "UserErrorNoError";

    QTest::newRow("valid: only execute rules")
        << (QList<Types::PermissionScope>()
            << Types::PermissionScopeAccessAllThings
            << Types::PermissionScopeExecuteRules)
        << "UserErrorNoError";


    QTest::newRow("invalid: missing control and all things")
        << (QList<Types::PermissionScope>()
            << Types::PermissionScopeConfigureThings)
        << "UserErrorInconsistantScopes";

    QTest::newRow("invalid: control/configure things. not all things")
        << (QList<Types::PermissionScope>()
            << Types::PermissionScopeControlThings
            << Types::PermissionScopeConfigureThings)
        << "UserErrorInconsistantScopes";

    QTest::newRow("invalid: only execute rules, not all things")
        << (QList<Types::PermissionScope>()
            << Types::PermissionScopeExecuteRules)
        << "UserErrorInconsistantScopes";

    QTest::newRow("invalid: only configure rules")
        << (QList<Types::PermissionScope>()
            << Types::PermissionScopeConfigureRules)
        << "UserErrorInconsistantScopes";

    QTest::newRow("invalid: configure and execute rules, not all things")
        << (QList<Types::PermissionScope>()
            << Types::PermissionScopeExecuteRules
            << Types::PermissionScopeConfigureRules)
        << "UserErrorInconsistantScopes";

    QTest::newRow("invalid: control things/rules, not all things")
        << (QList<Types::PermissionScope>()
            << Types::PermissionScopeControlThings
            << Types::PermissionScopeExecuteRules)
        << "UserErrorInconsistantScopes";
}

void TestUsermanager::testScopeConsitancy()
{
    QFETCH(QList<Types::PermissionScope>, scopes);
    QFETCH(QString, error);

    authenticate();

    QVariant response = injectAndWait("Users.GetUserInfo");
    QCOMPARE(response.toMap().value("status").toString(), QString("success"));
    QVariantMap userInfoMap = response.toMap().value("params").toMap().value("userInfo").toMap();
    QCOMPARE(userInfoMap.value("username").toString(), QString("valid@user.test"));

    QMetaEnum metaEnum = QMetaEnum::fromType<Types::PermissionScope>();
    QStringList scopesList;
    foreach (Types::PermissionScope scope, scopes)
        scopesList.append(metaEnum.valueToKey(scope));

    // Now try to edit with the given scopes
    QVariantMap params;
    params.insert("username", userInfoMap.value("username").toString());
    params.insert("scopes", scopesList);
    response = injectAndWait("Users.SetUserScopes", params);
    QCOMPARE(response.toMap().value("status").toString(), QString("success"));
    QCOMPARE(response.toMap().value("params").toMap().value("error").toString(), error);
}

void TestUsermanager::testRestrictedThingAccess()
{
    // Add 2 mock things
    ThingId thingIdOne;
    ThingId thingIdTwo;

    QString usernameAdmin = "admin";
    QString passwordAdmin = "Bla1234*";

    QString usernameGuest = "guest";
    QString passwordGuest = "Bla1234+";

    QVariant response;
    QVariantList thingParams;
    QVariantMap params;

    injectAndWait("JSONRPC.Hello");

    // Create admin user
    params.clear();
    params.insert("username", usernameAdmin);
    params.insert("password", passwordAdmin);
    response = injectAndWait("JSONRPC.CreateUser", params);
    QVERIFY2(response.toMap().value("status").toString() == "success", "Error creating user");
    QVERIFY2(response.toMap().value("params").toMap().value("error").toString() == "UserErrorNoError", "Error creating user");

    // Authenticate admin user
    params.clear();
    params.insert("username", usernameAdmin);
    params.insert("password", passwordAdmin);
    params.insert("deviceName", "autotests");
    response = injectAndWait("JSONRPC.Authenticate", params);
    QVERIFY2(response.toMap().value("status").toString() == "success", "Error authenticating");
    QVERIFY2(response.toMap().value("params").toMap().value("success").toString() == "true", "Error authenticating");

    m_adminToken = response.toMap().value("params").toMap().value("token").toByteArray();

    // Use the admin token for now
    m_apiToken = m_adminToken;

    // Add thing one
    QVariantMap httpportParamOne;
    httpportParamOne.insert("paramTypeId", mockThingHttpportParamTypeId.toString());
    httpportParamOne.insert("value", m_mockThing1Port - 1);
    thingParams << httpportParamOne;

    params.clear();
    params.insert("thingClassId", mockThingClassId);
    params.insert("name", "Test thing available for all users");
    params.insert("thingParams", thingParams);
    response = injectAndWait("Integrations.AddThing", params);
    verifyError(response, "thingError", enumValueName(Thing::ThingErrorNoError));
    thingIdOne = ThingId(response.toMap().value("params").toMap().value("thingId").toString());

    // Add thing two
    QVariantMap httpportParamTwo;
    httpportParamOne.insert("paramTypeId", mockThingHttpportParamTypeId.toString());
    httpportParamOne.insert("value", m_mockThing1Port - 2);
    thingParams.clear();
    thingParams << httpportParamOne;

    params.clear();
    params.insert("thingClassId", mockThingClassId);
    params.insert("name", "Test thing available for all users");
    params.insert("thingParams", thingParams);
    response = injectAndWait("Integrations.AddThing", params);
    verifyError(response, "thingError", enumValueName(Thing::ThingErrorNoError));
    thingIdTwo = ThingId(response.toMap().value("params").toMap().value("thingId").toString());


    // Create guest user
    QStringList scopes;
    scopes << "PermissionScopeControlThings";
    QVariantList allowedThingIds;
    allowedThingIds << thingIdTwo;

    params.clear();
    params.insert("username", usernameGuest);
    params.insert("password", passwordGuest);
    params.insert("scopes", scopes);
    params.insert("allowedThingIds", allowedThingIds);
    response = injectAndWait("Users.CreateUser", params);
    QVERIFY2(response.toMap().value("status").toString() == "success", "Error creating user");
    QVERIFY2(response.toMap().value("params").toMap().value("error").toString() == "UserErrorNoError", "Error creating user");

    response = injectAndWait("Integrations.GetThings");
    QVariantList things = response.toMap().value("params").toMap().value("things").toList();
    //qCDebug(dcTests()) << qUtf8Printable(QJsonDocument::fromVariant(things).toJson(QJsonDocument::Indented));
    QVERIFY2(things.count() >= 2, "Expected to get 2 or more things as admin");

    // Everything set up, now authenticate as guest

    // Authenticate guest user
    params.clear();
    params.insert("username", usernameGuest);
    params.insert("password", passwordGuest);
    params.insert("deviceName", "autotests");
    response = injectAndWait("JSONRPC.Authenticate", params);
    QVERIFY2(response.toMap().value("status").toString() == "success", "Error authenticating");
    QVERIFY2(response.toMap().value("params").toMap().value("success").toString() == "true", "Error authenticating");

    m_guestToken = response.toMap().value("params").toMap().value("token").toByteArray();

    // Use the admin token for now
    m_apiToken = m_guestToken;

    // Try to access restricted thing

    response = injectAndWait("Integrations.GetThings");
    verifyError(response, "thingError", enumValueName(Thing::ThingErrorNoError));
    things = response.toMap().value("params").toMap().value("things").toList();
    QVERIFY2(things.count() == 1, "Expected to get exactly 1 things as guest");

    // GetThings (access)
    params.clear();
    params.insert("thingId", thingIdTwo);
    response = injectAndWait("Integrations.GetThings", params);
    verifyError(response, "thingError", enumValueName(Thing::ThingErrorNoError));

    // GetThings (no access)
    params.clear();
    params.insert("thingId", thingIdOne);
    response = injectAndWait("Integrations.GetThings", params);
    verifyError(response, "thingError", enumValueName(Thing::ThingErrorThingNotFound));

    // GetStateValue
    params.clear();
    params.insert("thingId", thingIdOne);
    params.insert("stateTypeId", mockConnectedStateTypeId);
    response = injectAndWait("Integrations.GetStateValue", params);
    verifyError(response, "thingError", enumValueName(Thing::ThingErrorThingNotFound));

    // BrowseThing
    params.clear();
    params.insert("thingId", thingIdOne);
    response = injectAndWait("Integrations.BrowseThing", params);
    verifyError(response, "thingError", enumValueName(Thing::ThingErrorThingNotFound));

    // GetBrowserItem
    params.clear();
    params.insert("thingId", thingIdOne);
    response = injectAndWait("Integrations.GetBrowserItem", params);
    verifyError(response, "thingError", enumValueName(Thing::ThingErrorThingNotFound));

    // Make sure notification get received from allowed thing

    // Make sure no notification will be recived from restricted thing


    // Clean up


}

QTEST_MAIN(TestUsermanager)

