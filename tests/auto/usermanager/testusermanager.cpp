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

void TestUsermanager::tokenInfoExpiryAndLastSeenDefaultToInvalid()
{
    // Nothing in this repo mints a token with an expiry yet, so that always stays absent.
    // lastSeen, however, is marked by JSONRPC.Authenticate itself as of this commit.
    authenticate();

    TokenInfo byToken = NymeaCore::instance()->userManager()->tokenInfo(m_apiToken);
    QVERIFY(!byToken.id().isNull());
    QVERIFY(!byToken.expiryTime().isValid());
    QVERIFY(byToken.lastSeen().isValid());

    TokenInfo byId = NymeaCore::instance()->userManager()->tokenInfo(byToken.id());
    QVERIFY(!byId.expiryTime().isValid());
    QVERIFY(byId.lastSeen().isValid());

    QList<TokenInfo> tokens = NymeaCore::instance()->userManager()->tokens("valid@user.test");
    QCOMPARE(tokens.count(), 1);
    QVERIFY(!tokens.first().expiryTime().isValid());
    QVERIFY(tokens.first().lastSeen().isValid());
}

void TestUsermanager::lastSeenMarkedOnceOnAuthenticateAndNotAgainOnRepeatedHello()
{
    authenticate();

    TokenInfo afterAuth = NymeaCore::instance()->userManager()->tokenInfo(m_apiToken);
    QVERIFY(!afterAuth.id().isNull());
    QVERIFY(afterAuth.lastSeen().isValid());
    QDateTime firstSeen = afterAuth.lastSeen();

    // Advance the simulated clock and make another authenticated request on the same
    // connection with the same token: lastSeen must not move again.
    NymeaCore::instance()->timeManager()->setTime(QDateTime::currentDateTime().addSecs(3600));

    QVariant response = injectAndWait("JSONRPC.Hello");
    QVERIFY2(response.toMap().value("status").toString() == "success", "Hello failed");

    TokenInfo afterSecondHello = NymeaCore::instance()->userManager()->tokenInfo(m_apiToken);
    QCOMPARE(afterSecondHello.lastSeen(), firstSeen);

    // Restore real time so later tests in this binary are unaffected.
    NymeaCore::instance()->timeManager()->setTime(QDateTime::currentDateTime());
}

void TestUsermanager::activeDisconnectOnRemoveToken()
{
    // Regression test for the pre-existing gap: sendNotification() performs no token
    // re-check, so without active disconnect a revoked client's stream would keep
    // flowing until it disconnected on its own.
    getTokens();

    QSignalSpy disconnectedSpy(m_mockTcpServer, &MockTcpServer::clientDisconnected);

    QVariantMap params;
    params.insert("tokenId", m_tokenId);
    QVariant response = injectAndWait("Users.RemoveToken", params);
    // The reply for this very request, made with the token being removed, must still
    // arrive successfully: the disconnect must not race ahead of it.
    QCOMPARE(response.toMap().value("status").toString(), QString("success"));
    QCOMPARE(response.toMap().value("params").toMap().value("error").toString(), QString("UserErrorNoError"));

    if (disconnectedSpy.count() == 0)
        disconnectedSpy.wait();
    QCOMPARE(disconnectedSpy.count(), 1);
    QCOMPARE(disconnectedSpy.first().first().toUuid(), m_clientId);
}

void TestUsermanager::activeDisconnectOnRemoveUser()
{
    authenticate();
    QByteArray adminToken = m_apiToken;

    QVariantMap createParams;
    createParams.insert("username", "bob@user.test");
    createParams.insert("password", "Bla1234*");
    QVariant createResponse = injectAndWait("Users.CreateUser", createParams, m_clientId, adminToken);
    QCOMPARE(createResponse.toMap().value("params").toMap().value("error").toString(), QString("UserErrorNoError"));

    QUuid bobId = QUuid::createUuid();
    emit m_mockTcpServer->clientConnected(bobId);
    injectAndWait("JSONRPC.Hello", QVariantMap(), bobId, "");

    QVariantMap authParams;
    authParams.insert("username", "bob@user.test");
    authParams.insert("password", "Bla1234*");
    authParams.insert("deviceName", "bobdevice");
    QVariant bobAuthResponse = injectAndWait("JSONRPC.Authenticate", authParams, bobId, "");
    QByteArray bobToken = bobAuthResponse.toMap().value("params").toMap().value("token").toByteArray();
    QVERIFY2(!bobToken.isEmpty(), "Bob should have authenticated successfully");

    QSignalSpy disconnectedSpy(m_mockTcpServer, &MockTcpServer::clientDisconnected);

    QVariantMap removeParams;
    removeParams.insert("username", "bob@user.test");
    QVariant response = injectAndWait("Users.RemoveUser", removeParams, m_clientId, adminToken);
    QCOMPARE(response.toMap().value("params").toMap().value("error").toString(), QString("UserErrorNoError"));

    if (disconnectedSpy.count() == 0)
        disconnectedSpy.wait();
    QCOMPARE(disconnectedSpy.count(), 1);
    QCOMPARE(disconnectedSpy.first().first().toUuid(), bobId);
}

void TestUsermanager::getTokensExposesLastSeenAsEpochSecondsAndOmitsUnsetExpiryTime()
{
    authenticate();

    QVariant response = injectAndWait("Users.GetTokens");
    QCOMPARE(response.toMap().value("status").toString(), QString("success"));
    QVariantList tokenInfoList = response.toMap().value("params").toMap().value("tokenInfoList").toList();
    QCOMPARE(tokenInfoList.count(), 1);

    QVariantMap tokenInfo = tokenInfoList.first().toMap();
    QVERIFY2(tokenInfo.contains("lastSeen"), "lastSeen should be present as soon as the token has authenticated");
    qint64 lastSeenEpoch = tokenInfo.value("lastSeen").toLongLong();
    QVERIFY(lastSeenEpoch > 0);

    // Nothing in this repo sets an expiry yet, so this optional field must be entirely
    // absent from the wire response rather than serialized as null/0.
    QVERIFY2(!tokenInfo.contains("expiryTime"), "expiryTime should be absent when never set");
}

void TestUsermanager::createInvitationHappyPath()
{
    authenticate();

    QByteArray oneTimeToken;
    InvitationInfo info;
    UserManager::UserError error = NymeaCore::instance()->userManager()->createInvitation(
                "valid@user.test", 3600, false, 0, oneTimeToken, info);
    QCOMPARE(error, UserManager::UserErrorNoError);
    QVERIFY(!oneTimeToken.isEmpty());
    QVERIFY(!info.id().isNull());
    QCOMPARE(info.username(), QString("valid@user.test"));
    QVERIFY(info.expiryTime().isValid());
    QVERIFY(qAbs(info.creationTime().secsTo(info.expiryTime()) - 3600) <= 2);
    QVERIFY(!info.tokenValidityDuration().isValid());

    // Two invitations must never share a one-time token.
    QByteArray secondToken;
    InvitationInfo secondInfo;
    QCOMPARE(NymeaCore::instance()->userManager()->createInvitation(
                 "valid@user.test", 3600, true, 1800, secondToken, secondInfo),
             UserManager::UserErrorNoError);
    QVERIFY(secondToken != oneTimeToken);
    QVERIFY(secondInfo.tokenValidityDuration().isValid());
    QCOMPARE(secondInfo.tokenValidityDuration().toUInt(), 1800u);
}

void TestUsermanager::createInvitationForUnknownUserFails()
{
    authenticate();

    QByteArray oneTimeToken;
    InvitationInfo info;
    UserManager::UserError error = NymeaCore::instance()->userManager()->createInvitation(
                "nobody@user.test", 3600, false, 0, oneTimeToken, info);
    QCOMPARE(error, UserManager::UserErrorInvalidUserId);
    QVERIFY(oneTimeToken.isEmpty());
}

void TestUsermanager::createInvitationDurationValidation()
{
    authenticate();

    QByteArray oneTimeToken;
    InvitationInfo info;

    QCOMPARE(NymeaCore::instance()->userManager()->createInvitation(
                 "valid@user.test", 0, false, 0, oneTimeToken, info),
             UserManager::UserErrorInvalidInvitationDuration);

    QCOMPARE(NymeaCore::instance()->userManager()->createInvitation(
                 "valid@user.test", 2592001, false, 0, oneTimeToken, info),
             UserManager::UserErrorInvalidInvitationDuration);

    QCOMPARE(NymeaCore::instance()->userManager()->createInvitation(
                 "valid@user.test", 3600, true, 0, oneTimeToken, info),
             UserManager::UserErrorInvalidInvitationDuration);

    QCOMPARE(NymeaCore::instance()->userManager()->createInvitation(
                 "valid@user.test", 3600, true, 2592001, oneTimeToken, info),
             UserManager::UserErrorInvalidInvitationDuration);

    // Boundaries are accepted.
    QCOMPARE(NymeaCore::instance()->userManager()->createInvitation(
                 "valid@user.test", 1, true, 1, oneTimeToken, info),
             UserManager::UserErrorNoError);
    QCOMPARE(NymeaCore::instance()->userManager()->createInvitation(
                 "valid@user.test", 2592000, true, 2592000, oneTimeToken, info),
             UserManager::UserErrorNoError);
}

void TestUsermanager::getInvitationsListsAndFilters()
{
    authenticate();

    QVariantMap createParams;
    createParams.insert("username", "bob@user.test");
    createParams.insert("password", "Bla1234*");
    QVariant createResponse = injectAndWait("Users.CreateUser", createParams);
    QCOMPARE(createResponse.toMap().value("params").toMap().value("error").toString(), QString("UserErrorNoError"));

    UserManager *userManager = NymeaCore::instance()->userManager();
    QByteArray token;
    InvitationInfo info;
    QCOMPARE(userManager->createInvitation("valid@user.test", 3600, false, 0, token, info), UserManager::UserErrorNoError);
    QCOMPARE(userManager->createInvitation("valid@user.test", 3600, false, 0, token, info), UserManager::UserErrorNoError);
    QCOMPARE(userManager->createInvitation("bob@user.test", 3600, false, 0, token, info), UserManager::UserErrorNoError);

    QList<InvitationInfo> all;
    QCOMPARE(userManager->invitations(all), UserManager::UserErrorNoError);
    QCOMPARE(all.count(), 3);

    QList<InvitationInfo> validUserOnly;
    QCOMPARE(userManager->invitations(validUserOnly, "valid@user.test"), UserManager::UserErrorNoError);
    QCOMPARE(validUserOnly.count(), 2);
    foreach (const InvitationInfo &invitation, validUserOnly)
        QCOMPARE(invitation.username(), QString("valid@user.test"));
}

void TestUsermanager::removeInvitationRemovesRow()
{
    authenticate();

    UserManager *userManager = NymeaCore::instance()->userManager();
    QByteArray token;
    InvitationInfo info;
    QCOMPARE(userManager->createInvitation("valid@user.test", 3600, false, 0, token, info), UserManager::UserErrorNoError);

    QCOMPARE(userManager->removeInvitation(info.id()), UserManager::UserErrorNoError);

    QList<InvitationInfo> remaining;
    QCOMPARE(userManager->invitations(remaining), UserManager::UserErrorNoError);
    QCOMPARE(remaining.count(), 0);
}

void TestUsermanager::removeInvitationNotFoundFails()
{
    authenticate();

    QCOMPARE(NymeaCore::instance()->userManager()->removeInvitation(QUuid::createUuid()), UserManager::UserErrorInvitationNotFound);
}

void TestUsermanager::invitationsPurgesExpiredEntries()
{
    // Defensive: make sure no earlier test left a simulated time offset behind, since
    // this test relies on real elapsed wall-clock time via QTest::qWait().
    NymeaCore::instance()->timeManager()->setTime(QDateTime::currentDateTime());

    authenticate();

    UserManager *userManager = NymeaCore::instance()->userManager();
    QByteArray token;
    InvitationInfo info;
    QCOMPARE(userManager->createInvitation("valid@user.test", 2, false, 0, token, info), UserManager::UserErrorNoError);

    QSignalSpy removedSpy(userManager, &UserManager::invitationRemoved);
    QTest::qWait(2500);

    QList<InvitationInfo> remaining;
    QCOMPARE(userManager->invitations(remaining), UserManager::UserErrorNoError);
    QCOMPARE(remaining.count(), 0);
    QCOMPARE(removedSpy.count(), 1);
    QCOMPARE(removedSpy.first().first().toUuid(), info.id());
}

void TestUsermanager::redeemInvitationHappyPath()
{
    authenticate();

    UserManager *userManager = NymeaCore::instance()->userManager();
    QByteArray oneTimeToken;
    InvitationInfo info;
    QCOMPARE(userManager->createInvitation("valid@user.test", 3600, false, 0, oneTimeToken, info), UserManager::UserErrorNoError);

    QByteArray clientToken = userManager->redeemInvitation(oneTimeToken, "guest-phone");
    QVERIFY(!clientToken.isEmpty());
    QVERIFY(clientToken != oneTimeToken);
    QVERIFY(userManager->verifyToken(clientToken));

    TokenInfo tokenInfo = userManager->tokenInfo(clientToken);
    QVERIFY(!tokenInfo.id().isNull());
    QCOMPARE(tokenInfo.username(), QString("valid@user.test"));
    QCOMPARE(tokenInfo.deviceName(), QString("guest-phone"));
    QVERIFY(!tokenInfo.expiryTime().isValid());

    // The invitation is one-time: it must be gone from the list now.
    QList<InvitationInfo> remaining;
    QCOMPARE(userManager->invitations(remaining), UserManager::UserErrorNoError);
    QCOMPARE(remaining.count(), 0);
}

void TestUsermanager::redeemInvitationSecondAttemptFails()
{
    authenticate();

    UserManager *userManager = NymeaCore::instance()->userManager();
    QByteArray oneTimeToken;
    InvitationInfo info;
    QCOMPARE(userManager->createInvitation("valid@user.test", 3600, false, 0, oneTimeToken, info), UserManager::UserErrorNoError);

    QByteArray firstResult = userManager->redeemInvitation(oneTimeToken, "guest-phone");
    QVERIFY(!firstResult.isEmpty());

    QByteArray secondResult = userManager->redeemInvitation(oneTimeToken, "guest-phone-2");
    QVERIFY(secondResult.isEmpty());
}

void TestUsermanager::redeemExpiredInvitationFailsAndRemovesRow()
{
    // Defensive: this test relies on real elapsed wall-clock time.
    NymeaCore::instance()->timeManager()->setTime(QDateTime::currentDateTime());

    authenticate();

    UserManager *userManager = NymeaCore::instance()->userManager();
    QByteArray oneTimeToken;
    InvitationInfo info;
    QCOMPARE(userManager->createInvitation("valid@user.test", 1, false, 0, oneTimeToken, info), UserManager::UserErrorNoError);

    QTest::qWait(1500);

    QByteArray result = userManager->redeemInvitation(oneTimeToken, "guest-phone");
    QVERIFY(result.isEmpty());

    QList<InvitationInfo> remaining;
    QCOMPARE(userManager->invitations(remaining), UserManager::UserErrorNoError);
    QCOMPARE(remaining.count(), 0);
}

void TestUsermanager::redeemInvitationRejectsMalformedTokenAndDeviceName()
{
    authenticate();

    UserManager *userManager = NymeaCore::instance()->userManager();
    QByteArray oneTimeToken;
    InvitationInfo info;
    QCOMPARE(userManager->createInvitation("valid@user.test", 3600, false, 0, oneTimeToken, info), UserManager::UserErrorNoError);

    // Wrong-length/garbage tokens fail without ever consuming the real invitation.
    QVERIFY(userManager->redeemInvitation("garbage", "guest-phone").isEmpty());
    QVERIFY(userManager->redeemInvitation(oneTimeToken.left(43), "guest-phone").isEmpty());
    QVERIFY(userManager->redeemInvitation(oneTimeToken + "A", "guest-phone").isEmpty());

    // Device name outside the 1..40 UTF-8 byte contract, or containing a control character.
    QVERIFY(userManager->redeemInvitation(oneTimeToken, "").isEmpty());
    QVERIFY(userManager->redeemInvitation(oneTimeToken, QString(41, QChar('a'))).isEmpty());
    QVERIFY(userManager->redeemInvitation(oneTimeToken, QString("bad") + QChar(0x0007)).isEmpty());

    // The real invitation must still be redeemable after all of the above.
    QByteArray clientToken = userManager->redeemInvitation(oneTimeToken, "guest-phone");
    QVERIFY(!clientToken.isEmpty());
}

void TestUsermanager::redeemedTokenExpiryMeasuredFromRedemptionNotCreation()
{
    // Defensive: this test relies on real elapsed wall-clock time.
    NymeaCore::instance()->timeManager()->setTime(QDateTime::currentDateTime());

    authenticate();

    UserManager *userManager = NymeaCore::instance()->userManager();
    QByteArray oneTimeToken;
    InvitationInfo info;
    // Invitation itself stays valid for a long time; the redeemed token should only
    // live for tokenValiditySeconds starting when it is actually redeemed.
    QCOMPARE(userManager->createInvitation("valid@user.test", 3600, true, 5, oneTimeToken, info), UserManager::UserErrorNoError);

    // Let a couple of real seconds pass between invitation creation and redemption.
    QTest::qWait(2000);

    QDateTime beforeRedeem = QDateTime::currentDateTimeUtc();
    QByteArray clientToken = userManager->redeemInvitation(oneTimeToken, "guest-phone");
    QVERIFY(!clientToken.isEmpty());

    TokenInfo tokenInfo = userManager->tokenInfo(clientToken);
    QVERIFY(tokenInfo.expiryTime().isValid());
    // Expiry must be ~5s after redemption time, not 5s after (the earlier) creation time.
    qint64 secondsFromRedeemToExpiry = beforeRedeem.secsTo(tokenInfo.expiryTime());
    QVERIFY2(secondsFromRedeemToExpiry >= 3 && secondsFromRedeemToExpiry <= 7,
             QString("expiry %1s after redemption, expected ~5s").arg(secondsFromRedeemToExpiry).toUtf8());

    QTest::qWait(3500);
    QVERIFY(userManager->verifyToken(clientToken));
    QTest::qWait(3000);
    QVERIFY(!userManager->verifyToken(clientToken));
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

void TestUsermanager::userInventory()
{
    UserManager *userManager = NymeaCore::instance()->userManager();

    QCOMPARE(userManager->createUser("alice", "Bla1234*", "", "Alice", Types::PermissionScopeAdmin), UserManager::UserErrorNoError);
    QCOMPARE(userManager->createUser("bob", "Bla1234*", "", "Bob", Types::PermissionScopeAdmin), UserManager::UserErrorNoError);

    QVariantMap ecoProfile;
    ecoProfile.insert("mode", "Eco");

    QVariantMap payload;
    payload.insert("tagHash", "sha256:alice");
    payload.insert("profile", ecoProfile);

    QCOMPARE(userManager->addUserInventoryItem("alice", "rfidTag", "Alice tag", payload, true), UserManager::UserErrorNoError);

    UserInventoryItems aliceTags = userManager->userInventoryItems("alice", "rfidTag");
    QCOMPARE(aliceTags.count(), 1);
    QCOMPARE(aliceTags.first().username(), QString("alice"));
    QCOMPARE(aliceTags.first().type(), QString("rfidTag"));
    QCOMPARE(aliceTags.first().displayName(), QString("Alice tag"));
    QCOMPARE(aliceTags.first().enabled(), true);
    QCOMPARE(aliceTags.first().payload().value("tagHash").toString(), QString("sha256:alice"));
    QVERIFY(!aliceTags.first().payload().contains("code"));

    UserInventoryItem resolved = userManager->findEnabledUserInventoryItem("rfidTag", "tagHash", "sha256:alice");
    QVERIFY(resolved.isValid());
    QCOMPARE(resolved.username(), QString("alice"));

    QCOMPARE(userManager->addUserInventoryItem("bob", "rfidTag", "Bob duplicate", payload, true), UserManager::UserErrorDuplicateInventoryItem);
    QCOMPARE(userManager->addUserInventoryItem("bob", "rfidTag", "Bob disabled", payload, false), UserManager::UserErrorNoError);
    QCOMPARE(userManager->findEnabledUserInventoryItem("rfidTag", "tagHash", "sha256:alice").username(), QString("alice"));

    QVariantMap invalidPayload = payload;
    invalidPayload.insert("code", "raw-reader-code");
    QCOMPARE(userManager->addUserInventoryItem("alice", "rfidTag", "Invalid raw code", invalidPayload, true), UserManager::UserErrorInvalidInventoryItem);

    const QUuid aliceInventoryItemId = aliceTags.first().inventoryItemId();
    QCOMPARE(userManager->updateUserInventoryItem(aliceInventoryItemId, "Alice disabled", payload, false), UserManager::UserErrorNoError);
    QVERIFY(!userManager->findEnabledUserInventoryItem("rfidTag", "tagHash", "sha256:alice").isValid());

    UserInventoryItems bobTags = userManager->userInventoryItems("bob", "rfidTag");
    QCOMPARE(bobTags.count(), 1);
    QCOMPARE(userManager->updateUserInventoryItem(bobTags.first().inventoryItemId(), "Bob enabled", payload, true), UserManager::UserErrorNoError);
    QCOMPARE(userManager->findEnabledUserInventoryItem("rfidTag", "tagHash", "sha256:alice").username(), QString("bob"));

    QCOMPARE(userManager->removeUserInventoryItem(bobTags.first().inventoryItemId()), UserManager::UserErrorNoError);
    QVERIFY(!userManager->findEnabledUserInventoryItem("rfidTag", "tagHash", "sha256:alice").isValid());

    // A mixed-case username must resolve to the existing (lower-cased) user
    QVariantMap mixedCasePayload;
    mixedCasePayload.insert("tagHash", "sha256:alice-2");
    QCOMPARE(userManager->addUserInventoryItem("Alice", "rfidTag", "Alice tag 2", mixedCasePayload, true), UserManager::UserErrorNoError);
    QCOMPARE(userManager->userInventoryItems("alice", "rfidTag").count(), 2);

    QCOMPARE(userManager->removeUser("alice"), UserManager::UserErrorNoError);
    QCOMPARE(userManager->userInventoryItems("alice", "rfidTag").count(), 0);
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
    httpportParamTwo.insert("paramTypeId", mockThingHttpportParamTypeId.toString());
    httpportParamTwo.insert("value", m_mockThing1Port - 2);
    thingParams.clear();
    thingParams << httpportParamTwo;

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

    // GetStateValue (no access)
    params.clear();
    params.insert("thingId", thingIdOne);
    params.insert("stateTypeId", mockConnectedStateTypeId);
    response = injectAndWait("Integrations.GetStateValue", params);
    verifyError(response, "thingError", enumValueName(Thing::ThingErrorThingNotFound));

    // BrowseThing (no access)
    params.clear();
    params.insert("thingId", thingIdOne);
    response = injectAndWait("Integrations.BrowseThing", params);
    verifyError(response, "thingError", enumValueName(Thing::ThingErrorThingNotFound));

    // GetBrowserItem (no access)
    params.clear();
    params.insert("thingId", thingIdOne);
    response = injectAndWait("Integrations.GetBrowserItem", params);
    verifyError(response, "thingError", enumValueName(Thing::ThingErrorThingNotFound));

    // Clean up
    UserManager *userManager = NymeaCore::instance()->userManager();
    foreach (const UserInfo &userInfo, userManager->users()) {
        qCDebug(dcTests()) << "Removing user" << userInfo.username();
        userManager->removeUser(userInfo.username());
    }
    userManager->removeUser("");
}

QTEST_MAIN(TestUsermanager)
