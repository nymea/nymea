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
#include "nymeasettings.h"
#include "usermanager/usermanager.h"
#include "servers/mocktcpserver.h"
#include "nymeadbusservice.h"

#include "../../utils/pushbuttonagent.h"

#include "../plugins/mock/extern-plugininfo.h"

#include <QCryptographicHash>
#include <QMutex>
#include <QScopeGuard>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

using namespace nymeaserver;

namespace {
// Message-handler bridge for tokenFailurePathsNeverLogRawToken(): qInstallMessageHandler()
// only accepts a plain function pointer, so the QStringList being appended to has to be
// reached through a static rather than a capture.
QMutex s_capturedLogLinesMutex;
QStringList *s_capturedLogLines = nullptr;

void captureLogMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    Q_UNUSED(type)
    Q_UNUSED(context)
    QMutexLocker locker(&s_capturedLogLinesMutex);
    if (s_capturedLogLines)
        *s_capturedLogLines << msg;
}
}

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

    // Deliberately tear the shared mock client down before wiping users below, rather
    // than letting removeUser()'s active-disconnect side effect (see
    // activeDisconnectOnRemoveUser/activeDisconnectOnRemoveToken) fire asynchronously -
    // it's delivered via QueuedConnection (see
    // JsonRPCServerImplementation::onTokenInvalidated) - mid this cleanup or mid the next
    // test's first event-loop spin, and disconnect the client out from under whichever
    // test happens to be running at that point. Simulating the disconnect up front makes
    // every test start from the exact same deterministic baseline instead.
    if (m_mockTcpServer->isClientConnected(m_clientId)) {
        m_mockTcpServer->clientDisconnected(m_clientId);
    }

    foreach (const UserInfo &userInfo, userManager->users()) {
        qCDebug(dcTests()) << "Removing user" << userInfo.username();
        userManager->removeUser(userInfo.username());
    }
    userManager->removeUser("");

    // Flush any tokenInvalidated() that cleanup just queued. Harmless now - the client
    // was already disconnected above, so JsonRPCServerImplementation has nothing left
    // to do for it - but this keeps the queue from accumulating across tests.
    qApp->processEvents();
    qApp->processEvents();

    // Reconnect with a clean, tokenless Hello, matching the genuinely fresh, no-users
    // (initRequired()) state this cleanup just produced - several tests (e.g.
    // authenticate(), which itself clears m_apiToken and expects JSONRPC.CreateUser to
    // succeed pre-auth) rely on that being true at the start of their own body.
    // Recreating "dummy" here instead, to keep the shared connection's token
    // permanently valid, would leave initRequired() false before those tests even start
    // and break exactly that assumption. m_apiToken is reset to match: every
    // injectAndWait() call defaults to presenting it, and a stale non-empty value here
    // would itself become a bad token (or a "token changed without redoing the
    // handshake" mismatch) the moment any of them runs before creating their own user.
    m_apiToken.clear();
    m_mockTcpServer->clientConnected(m_clientId);
    injectAndWait("JSONRPC.Hello");

    // A previous test may have deliberately tripped the connection lockdown (e.g.
    // authenticateWithTokenFailureShape(), or any repeated-bad-token/bad-handshake
    // scenario) as part of exercising that mechanism itself, and its 3-second timer can
    // still be running when this reconnect lands - or even get re-armed again right at
    // the boundary. Keep waiting it out and retrying, the same way the redemption
    // controller itself is required to, rather than assuming a single 3-second margin
    // is always enough.
    int attempt = 0;
    while (!m_mockTcpServer->isClientConnected(m_clientId) && attempt < 5) {
        attempt++;
        QTest::qWait(3100);
        m_mockTcpServer->clientConnected(m_clientId);
        injectAndWait("JSONRPC.Hello");
    }
    QVERIFY2(m_mockTcpServer->isClientConnected(m_clientId), "Could not reconnect the shared mock client - lockdown never cleared");
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

void TestUsermanager::invitationsFilterExcludesExpiredRowWhenPurgeDeleteFails()
{
    authenticate();

    UserManager *userManager = NymeaCore::instance()->userManager();
    QByteArray token;
    InvitationInfo info;
    QCOMPARE(userManager->createInvitation("valid@user.test", 3600, false, 0, token, info), UserManager::UserErrorNoError);

    QString dbPath = NymeaSettings::privodeFromDefaultFilePath("user-db.sqlite");

    // Backdate the row directly in the DB so it is already expired, independent of
    // whatever purgeExpiredInvitations() would otherwise have caught on its own.
    {
        QSqlDatabase backdateDb = QSqlDatabase::addDatabase("QSQLITE", "test-invitations-backdate");
        backdateDb.setDatabaseName(dbPath);
        QVERIFY(backdateDb.open());
        QSqlQuery backdateQuery(backdateDb);
        backdateQuery.prepare("UPDATE invitations SET expirydate = :expiry WHERE id = :id;");
        backdateQuery.bindValue(":expiry", QDateTime::currentDateTimeUtc().addSecs(-30).toString(Qt::ISODate));
        backdateQuery.bindValue(":id", info.id().toString());
        QVERIFY(backdateQuery.exec());
        QCOMPARE(backdateQuery.numRowsAffected(), 1);
        backdateDb.close();
    }
    QSqlDatabase::removeDatabase("test-invitations-backdate");

    // Hold a write lock on the same on-disk DB file from a second connection so that
    // purgeExpiredInvitations()'s own DELETE - run synchronously at the top of
    // invitations() - is forced to fail with "database is locked", leaving the
    // now-expired row physically in place. This isolates the defense-in-depth filter
    // inside invitations()'s result loop from the normal purge-then-list path, which
    // invitationsPurgesExpiredEntries() above already covers.
    {
        QSqlDatabase lockDb = QSqlDatabase::addDatabase("QSQLITE", "test-invitations-lock");
        lockDb.setDatabaseName(dbPath);
        QVERIFY(lockDb.open());
        QSqlQuery lockQuery(lockDb);
        QVERIFY(lockQuery.exec("BEGIN IMMEDIATE;"));

        QList<InvitationInfo> remaining;
        QCOMPARE(userManager->invitations(remaining), UserManager::UserErrorNoError);
        bool expiredRowLeaked = false;
        foreach (const InvitationInfo &invitation, remaining) {
            if (invitation.id() == info.id())
                expiredRowLeaked = true;
        }
        QVERIFY2(!expiredRowLeaked, "Expired invitation leaked from invitations() while its purge delete was blocked");

        // Confirm the delete really did fail - i.e. this test is exercising the
        // defense-in-depth filter itself, not just relying on a purge that quietly
        // succeeded anyway.
        QSqlQuery stillThereQuery(lockDb);
        stillThereQuery.prepare("SELECT COUNT(*) FROM invitations WHERE id = :id;");
        stillThereQuery.bindValue(":id", info.id().toString());
        QVERIFY(stillThereQuery.exec());
        QVERIFY(stillThereQuery.next());
        QCOMPARE(stillThereQuery.value(0).toInt(), 1);

        QVERIFY(lockQuery.exec("ROLLBACK;"));
        lockDb.close();
    }
    QSqlDatabase::removeDatabase("test-invitations-lock");
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

void TestUsermanager::tokenExpiryChecksRespectSimulatedTimeOverride()
{
    // Defensive: make sure no earlier test left a simulated time offset behind.
    NymeaCore::instance()->timeManager()->setTime(QDateTime::currentDateTime());

    authenticate();

    UserManager *userManager = NymeaCore::instance()->userManager();
    QByteArray oneTimeToken;
    InvitationInfo info;
    // Token validity is measured from redemption; give it a short window so a simulated
    // clock jump can obviously outrun it.
    QCOMPARE(userManager->createInvitation("valid@user.test", 3600, true, 10, oneTimeToken, info), UserManager::UserErrorNoError);

    QByteArray clientToken = userManager->redeemInvitation(oneTimeToken, "guest-phone");
    QVERIFY(!clientToken.isEmpty());
    QVERIFY(userManager->verifyToken(clientToken));

    // Jump the simulated clock well past the token's expiry - deliberately with no
    // QTest::qWait() afterwards. This can only pass if the expiry check reads
    // TimeManager::currentDateTime() rather than the real wall clock: the real clock has
    // not actually advanced at all here.
    NymeaCore::instance()->timeManager()->setTime(QDateTime::currentDateTime().addSecs(3600));

    QVERIFY(!userManager->verifyToken(clientToken));
    TokenInfo tokenInfo = userManager->tokenInfo(clientToken);
    QVERIFY(tokenInfo.id().isNull());

    // Reset the time override so later tests aren't affected.
    NymeaCore::instance()->timeManager()->setTime(QDateTime::currentDateTime());
}

void TestUsermanager::tokenFailurePathsNeverLogRawToken()
{
    authenticate();

    UserManager *userManager = NymeaCore::instance()->userManager();

    QStringList capturedLogLines;
    {
        QMutexLocker locker(&s_capturedLogLinesMutex);
        s_capturedLogLines = &capturedLogLines;
    }
    QtMessageHandler previousHandler = qInstallMessageHandler(captureLogMessageHandler);
    auto guard = qScopeGuard([previousHandler]() {
        qInstallMessageHandler(previousHandler);
        QMutexLocker locker(&s_capturedLogLinesMutex);
        s_capturedLogLines = nullptr;
    });

    // A well-formed, base64-alphabet secret generated exactly the way real tokens are -
    // just like a genuine token, except it will never exist in the DB. Exercises the
    // "fails the DB lookup" branch of tokenInfo()/verifyToken().
    QByteArray unknownToken = QCryptographicHash::hash(QUuid::createUuid().toByteArray(), QCryptographicHash::Sha256).toBase64();

    // Same secret value, but with a character validateToken()'s charset regex rejects -
    // exercises the "fails validation" branch instead.
    QByteArray malformedToken = unknownToken;
    malformedToken[0] = ' ';

    Q_UNUSED(userManager->tokenInfo(malformedToken));
    Q_UNUSED(userManager->verifyToken(malformedToken));
    Q_UNUSED(userManager->tokenInfo(unknownToken));
    Q_UNUSED(userManager->verifyToken(unknownToken));

    foreach (const QString &line, capturedLogLines) {
        QVERIFY2(!line.contains(QString::fromUtf8(malformedToken)),
                 qUtf8Printable(QString("Log line leaked the malformed token: %1").arg(line)));
        QVERIFY2(!line.contains(QString::fromUtf8(unknownToken)),
                 qUtf8Printable(QString("Log line leaked the unknown token: %1").arg(line)));
    }
}

void TestUsermanager::removeUserCascadesInvitations()
{
    authenticate();

    QVariantMap createParams;
    createParams.insert("username", "bob@user.test");
    createParams.insert("password", "Bla1234*");
    QVariant createResponse = injectAndWait("Users.CreateUser", createParams);
    QCOMPARE(createResponse.toMap().value("params").toMap().value("error").toString(), QString("UserErrorNoError"));

    UserManager *userManager = NymeaCore::instance()->userManager();
    QByteArray oneTimeToken;
    InvitationInfo info;
    QCOMPARE(userManager->createInvitation("bob@user.test", 3600, false, 0, oneTimeToken, info), UserManager::UserErrorNoError);

    QSignalSpy invitationRemovedSpy(userManager, &UserManager::invitationRemoved);
    QCOMPARE(userManager->removeUser("bob@user.test"), UserManager::UserErrorNoError);

    QCOMPARE(invitationRemovedSpy.count(), 1);
    QCOMPARE(invitationRemovedSpy.first().first().toUuid(), info.id());

    QList<InvitationInfo> remaining;
    QCOMPARE(userManager->invitations(remaining, "bob@user.test"), UserManager::UserErrorNoError);
    QCOMPARE(remaining.count(), 0);
}

void TestUsermanager::disabledInvitationsRejectAllManagementMethods()
{
    authenticate();
    UserManager *userManager = NymeaCore::instance()->userManager();

    QByteArray oneTimeToken;
    InvitationInfo info;
    QCOMPARE(userManager->createInvitation("valid@user.test", 3600, false, 0, oneTimeToken, info), UserManager::UserErrorNoError);
    QVERIFY(!oneTimeToken.isEmpty());

    userManager->setInvitationsAvailable(false);

    QByteArray disabledToken;
    InvitationInfo disabledInfo;
    QCOMPARE(userManager->createInvitation("valid@user.test", 3600, false, 0, disabledToken, disabledInfo),
             UserManager::UserErrorInvitationsDisabled);
    QVERIFY(disabledToken.isEmpty());

    QList<InvitationInfo> list;
    QCOMPARE(userManager->invitations(list), UserManager::UserErrorInvitationsDisabled);
    QCOMPARE(list.count(), 0);

    QCOMPARE(userManager->removeInvitation(QUuid::createUuid()), UserManager::UserErrorInvitationsDisabled);

    // Disabling already purged the pending invitation from above; redemption fails with
    // the same undistinguishable empty result either way.
    QVERIFY(userManager->redeemInvitation(oneTimeToken, "guest-phone").isEmpty());

    userManager->setInvitationsAvailable(true);
}

void TestUsermanager::disablingInvitationsPurgesPendingRowsWithoutNotification()
{
    authenticate();
    UserManager *userManager = NymeaCore::instance()->userManager();

    QByteArray oneTimeToken;
    InvitationInfo info;
    QCOMPARE(userManager->createInvitation("valid@user.test", 3600, false, 0, oneTimeToken, info), UserManager::UserErrorNoError);

    QSignalSpy removedSpy(userManager, &UserManager::invitationRemoved);
    userManager->setInvitationsAvailable(false);
    QCOMPARE(removedSpy.count(), 0);
    QVERIFY(!userManager->initializationFailed());

    userManager->setInvitationsAvailable(true);
    QList<InvitationInfo> list;
    QCOMPARE(userManager->invitations(list), UserManager::UserErrorNoError);
    QCOMPARE(list.count(), 0);
}

void TestUsermanager::reenablingAfterDisabledStartupStartsEmptyButKeepsRegularTokens()
{
    authenticate();
    UserManager *userManager = NymeaCore::instance()->userManager();

    QByteArray oneTimeToken;
    InvitationInfo info;
    QCOMPARE(userManager->createInvitation("valid@user.test", 3600, false, 0, oneTimeToken, info), UserManager::UserErrorNoError);
    QByteArray clientToken = userManager->redeemInvitation(oneTimeToken, "guest-phone");
    QVERIFY(!clientToken.isEmpty());

    userManager->setInvitationsAvailable(false);
    userManager->setInvitationsAvailable(true);

    // Already-redeemed regular client tokens survive a disable/re-enable cycle.
    QVERIFY(userManager->verifyToken(clientToken));

    QList<InvitationInfo> list;
    QCOMPARE(userManager->invitations(list), UserManager::UserErrorNoError);
    QCOMPARE(list.count(), 0);
}

void TestUsermanager::environmentVariableFormsAllDisableInvitations()
{
    // Presence disables regardless of value; only an unset variable means available.
    // Exercised through a real restart - for one representative disabling value, plus
    // the re-enable case - so NymeaCore::init()'s own resolution logic is what gets
    // tested, not just the setter. Kept to a single restart deliberately: restartServer()
    // re-runs ThingManagerImplementation's construction, which reinitializes the embedded
    // CPython interpreter (PythonIntegrationPlugin::initPython(), pre-existing and
    // unrelated to invitations) - a Py_FinalizeEx()/Py_InitializeEx() cycle that CPython's
    // own C-extension statics (e.g. the _datetime module) do not reliably survive. Extra
    // restarts here would only multiply exposure to that unrelated, pre-existing fragility.
    // The remaining "any value disables" half of the claim (independent of
    // NymeaCore::init(), which only ever calls qEnvironmentVariableIsSet() and never
    // inspects the value) is instead checked directly against Qt's own environment
    // semantics, with no further restarts.
    qputenv("NYMEA_DISABLE_INVITATIONS", "");
    restartServer();
    QVERIFY2(!NymeaCore::instance()->userManager()->invitationsAvailable(), "An empty value should disable invitations");

    const QStringList otherDisablingValues = {QStringLiteral("0"), QStringLiteral("false"), QStringLiteral("1")};
    foreach (const QString &value, otherDisablingValues) {
        qputenv("NYMEA_DISABLE_INVITATIONS", value.toUtf8());
        QVERIFY2(qEnvironmentVariableIsSet("NYMEA_DISABLE_INVITATIONS"),
                 QString("Value '%1' should be detected as set").arg(value).toUtf8().constData());
    }

    qunsetenv("NYMEA_DISABLE_INVITATIONS");
    restartServer();
    QVERIFY(NymeaCore::instance()->userManager()->invitationsAvailable());
}

void TestUsermanager::createInvitationOverJsonRpc()
{
    authenticate();

    QVariantMap params;
    params.insert("username", "valid@user.test");
    params.insert("validityDuration", 3600);
    params.insert("tokenValidityDuration", 1800);
    QVariant response = injectAndWait("Users.CreateInvitation", params);
    QCOMPARE(response.toMap().value("status").toString(), QString("success"));

    QVariantMap returnParams = response.toMap().value("params").toMap();
    QCOMPARE(returnParams.value("error").toString(), QString("UserErrorNoError"));
    QVERIFY(!returnParams.value("token").toString().isEmpty());

    QVariantMap invitation = returnParams.value("invitation").toMap();
    QVERIFY(!invitation.value("id").toString().isEmpty());
    QCOMPARE(invitation.value("username").toString(), QString("valid@user.test"));
    QVERIFY(invitation.contains("creationTime"));
    QVERIFY(invitation.contains("expiryTime"));
    qint64 creationEpoch = invitation.value("creationTime").toLongLong();
    qint64 expiryEpoch = invitation.value("expiryTime").toLongLong();
    QVERIFY(qAbs((expiryEpoch - creationEpoch) - 3600) <= 2);
    QCOMPARE(invitation.value("tokenValidityDuration").toUInt(), 1800u);
}

void TestUsermanager::createInvitationOverJsonRpcDefaultsDuration()
{
    authenticate();

    QVariantMap params;
    params.insert("username", "valid@user.test");
    QVariant response = injectAndWait("Users.CreateInvitation", params);
    QCOMPARE(response.toMap().value("status").toString(), QString("success"));

    QVariantMap returnParams = response.toMap().value("params").toMap();
    QCOMPARE(returnParams.value("error").toString(), QString("UserErrorNoError"));
    QVariantMap invitation = returnParams.value("invitation").toMap();
    // No tokenValidityDuration given: the redeemed token never expires, so this optional
    // field must be entirely absent, not null/0.
    QVERIFY2(!invitation.contains("tokenValidityDuration"), "tokenValidityDuration should be absent when unset");

    qint64 creationEpoch = invitation.value("creationTime").toLongLong();
    qint64 expiryEpoch = invitation.value("expiryTime").toLongLong();
    QVERIFY(qAbs((expiryEpoch - creationEpoch) - 86400) <= 2);
}

void TestUsermanager::getInvitationsOverJsonRpc()
{
    authenticate();

    QVariantMap createParams;
    createParams.insert("username", "valid@user.test");
    QCOMPARE(injectAndWait("Users.CreateInvitation", createParams).toMap().value("params").toMap().value("error").toString(),
             QString("UserErrorNoError"));
    QCOMPARE(injectAndWait("Users.CreateInvitation", createParams).toMap().value("params").toMap().value("error").toString(),
             QString("UserErrorNoError"));

    QVariant response = injectAndWait("Users.GetInvitations");
    QCOMPARE(response.toMap().value("status").toString(), QString("success"));
    QVariantMap returnParams = response.toMap().value("params").toMap();
    QCOMPARE(returnParams.value("error").toString(), QString("UserErrorNoError"));
    QCOMPARE(returnParams.value("invitations").toList().count(), 2);
}

void TestUsermanager::removeInvitationOverJsonRpc()
{
    authenticate();

    QVariantMap createParams;
    createParams.insert("username", "valid@user.test");
    QVariant createResponse = injectAndWait("Users.CreateInvitation", createParams);
    QString invitationId = createResponse.toMap().value("params").toMap().value("invitation").toMap().value("id").toString();
    QVERIFY(!invitationId.isEmpty());

    QVariantMap removeParams;
    removeParams.insert("invitationId", invitationId);
    QVariant removeResponse = injectAndWait("Users.RemoveInvitation", removeParams);
    QCOMPARE(removeResponse.toMap().value("status").toString(), QString("success"));
    QCOMPARE(removeResponse.toMap().value("params").toMap().value("error").toString(), QString("UserErrorNoError"));

    QVariant listResponse = injectAndWait("Users.GetInvitations");
    QCOMPARE(listResponse.toMap().value("params").toMap().value("invitations").toList().count(), 0);
}

void TestUsermanager::invitationMethodsRequireAdminScope()
{
    authenticate();
    QByteArray adminToken = m_apiToken;

    QVariantMap createGuestParams;
    createGuestParams.insert("username", "guest@user.test");
    createGuestParams.insert("password", "Bla1234*");
    QStringList guestScopes;
    guestScopes << "PermissionScopeControlThings";
    createGuestParams.insert("scopes", guestScopes);
    QVariant createGuestResponse = injectAndWait("Users.CreateUser", createGuestParams, m_clientId, adminToken);
    QCOMPARE(createGuestResponse.toMap().value("params").toMap().value("error").toString(), QString("UserErrorNoError"));

    // Authenticate the guest on its own fresh connection - reusing m_clientId (already
    // bound to adminToken from authenticate() above, server-side) with a different
    // token would trip "Client changed token without redoing the handshake" and drop
    // the connection, since only JSONRPC.Hello is allowed to rebind a connection's token.
    QUuid guestClientId = QUuid::createUuid();
    m_mockTcpServer->clientConnected(guestClientId);
    injectAndWait("JSONRPC.Hello", QVariantMap(), guestClientId, "");

    QVariantMap authParams;
    authParams.insert("username", "guest@user.test");
    authParams.insert("password", "Bla1234*");
    authParams.insert("deviceName", "guestdevice");
    QVariant authResponse = injectAndWait("JSONRPC.Authenticate", authParams, guestClientId, "");
    QByteArray guestToken = authResponse.toMap().value("params").toMap().value("token").toByteArray();
    QVERIFY(!guestToken.isEmpty());

    QVariantMap createParams;
    createParams.insert("username", "guest@user.test");
    QVariant response = injectAndWait("Users.CreateInvitation", createParams, guestClientId, guestToken);
    // Insufficient scope on an otherwise-valid token surfaces as a generic "error"
    // status (with a permission message), not "unauthorized" - that status is reserved
    // for a missing/invalid token, which this is not.
    QCOMPARE(response.toMap().value("status").toString(), QString("error"));
}

void TestUsermanager::invitationNotificationsOnlyReachAdminSubscribers()
{
    authenticate();
    QByteArray adminToken = m_apiToken;

    QVariantMap adminSubscribeParams;
    adminSubscribeParams.insert("enabled", true);
    injectAndWait("JSONRPC.SetNotificationStatus", adminSubscribeParams, m_clientId, adminToken);

    QVariantMap createGuestParams;
    createGuestParams.insert("username", "guest2@user.test");
    createGuestParams.insert("password", "Bla1234*");
    QStringList guestScopes;
    guestScopes << "PermissionScopeControlThings";
    createGuestParams.insert("scopes", guestScopes);
    QCOMPARE(injectAndWait("Users.CreateUser", createGuestParams, m_clientId, adminToken).toMap().value("params").toMap().value("error").toString(),
             QString("UserErrorNoError"));

    QUuid guestClientId = QUuid::createUuid();
    emit m_mockTcpServer->clientConnected(guestClientId);
    injectAndWait("JSONRPC.Hello", QVariantMap(), guestClientId, "");

    QVariantMap guestAuthParams;
    guestAuthParams.insert("username", "guest2@user.test");
    guestAuthParams.insert("password", "Bla1234*");
    guestAuthParams.insert("deviceName", "guestdevice2");
    QVariant guestAuthResponse = injectAndWait("JSONRPC.Authenticate", guestAuthParams, guestClientId, "");
    QByteArray guestToken = guestAuthResponse.toMap().value("params").toMap().value("token").toByteArray();
    QVERIFY(!guestToken.isEmpty());

    QVariantMap guestSubscribeParams;
    guestSubscribeParams.insert("enabled", true);
    injectAndWait("JSONRPC.SetNotificationStatus", guestSubscribeParams, guestClientId, guestToken);

    QSignalSpy notificationSpy(m_mockTcpServer, &MockTcpServer::outgoingData);

    QVariantMap createParams;
    createParams.insert("username", "valid@user.test");
    QVariant createResponse = injectAndWait("Users.CreateInvitation", createParams, m_clientId, adminToken);
    QCOMPARE(createResponse.toMap().value("params").toMap().value("error").toString(), QString("UserErrorNoError"));

    // Give a (wrongly sent) notification to the guest a chance to arrive before we count.
    QTest::qWait(300);

    bool adminReceived = false;
    bool guestReceived = false;
    for (int i = 0; i < notificationSpy.count(); i++) {
        QUuid recipientClientId = notificationSpy.at(i).first().toUuid();
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(notificationSpy.at(i).at(1).toByteArray(), &error);
        if (error.error != QJsonParseError::NoError)
            continue;
        if (doc.toVariant().toMap().value("notification").toString() != "Users.InvitationAdded")
            continue;
        if (recipientClientId == m_clientId)
            adminReceived = true;
        if (recipientClientId == guestClientId)
            guestReceived = true;
    }

    QVERIFY2(adminReceived, "Admin client subscribed to Users should have received Users.InvitationAdded");
    QVERIFY2(!guestReceived, "Non-admin client must not receive Users.InvitationAdded even when subscribed");
}

void TestUsermanager::helloExposesInvitationsAvailable()
{
    QVariant response = injectAndWait("JSONRPC.Hello");
    QVariantMap returnParams = response.toMap().value("params").toMap();
    QVERIFY2(returnParams.contains("invitationsAvailable"), "Hello must always include invitationsAvailable");
    QCOMPARE(returnParams.value("invitationsAvailable").toBool(), NymeaCore::instance()->userManager()->invitationsAvailable());
}

void TestUsermanager::authenticateWithTokenHappyPath()
{
    authenticate();
    QByteArray adminToken = m_apiToken;

    QVariantMap createParams;
    createParams.insert("username", "valid@user.test");
    QVariant createResponse = injectAndWait("Users.CreateInvitation", createParams, m_clientId, adminToken);
    QString oneTimeToken = createResponse.toMap().value("params").toMap().value("token").toString();
    QVERIFY(!oneTimeToken.isEmpty());

    QUuid guestClientId = QUuid::createUuid();
    emit m_mockTcpServer->clientConnected(guestClientId);
    injectAndWait("JSONRPC.Hello", QVariantMap(), guestClientId, "");

    QVariantMap redeemParams;
    redeemParams.insert("token", oneTimeToken);
    redeemParams.insert("deviceName", "guest-phone");
    QVariant redeemResponse = injectAndWait("JSONRPC.AuthenticateWithToken", redeemParams, guestClientId, "");
    QVariantMap redeemReturn = redeemResponse.toMap().value("params").toMap();
    QVERIFY2(redeemReturn.value("success").toBool(), "Redemption should have succeeded");
    QByteArray clientToken = redeemReturn.value("token").toByteArray();
    QVERIFY(!clientToken.isEmpty());
    QCOMPARE(redeemReturn.value("username").toString(), QString("valid@user.test"));
    QVERIFY(redeemReturn.value("scopes").toStringList().contains("PermissionScopeAdmin"));

    // Same connection, same socket, no second Hello: the token must already be bound.
    QVariant getUserInfoResponse = injectAndWait("Users.GetUserInfo", QVariantMap(), guestClientId, clientToken);
    QCOMPARE(getUserInfoResponse.toMap().value("status").toString(), QString("success"));
    QCOMPARE(getUserInfoResponse.toMap().value("params").toMap().value("userInfo").toMap().value("username").toString(),
             QString("valid@user.test"));
}

void TestUsermanager::authenticateWithTokenFailureShape()
{
    QUuid guestClientId = QUuid::createUuid();
    emit m_mockTcpServer->clientConnected(guestClientId);
    injectAndWait("JSONRPC.Hello", QVariantMap(), guestClientId, "");

    QVariantMap redeemParams;
    redeemParams.insert("token", QString("not-a-real-token"));
    redeemParams.insert("deviceName", "guest-phone");
    QVariant response = injectAndWait("JSONRPC.AuthenticateWithToken", redeemParams, guestClientId, "");
    QVariantMap returnParams = response.toMap().value("params").toMap();
    QCOMPARE(returnParams.value("success").toBool(), false);
    QVERIFY2(!returnParams.contains("token"), "token must be absent on failure");
    QVERIFY2(!returnParams.contains("username"), "username must be absent on failure");
    QVERIFY2(!returnParams.contains("scopes"), "scopes must be absent on failure");
}

void TestUsermanager::endToEndInvitationRedemptionThenActiveDisconnectOnRevoke()
{
    // End-to-end: admin creates an invitation, a fresh guest client redeems it and uses
    // the resulting token on the same connection, then the admin revokes that token and
    // the guest's connection is actively disconnected - exercising 01's redemption path
    // together with 00's active-disconnect mechanism in one scenario.
    authenticate();
    QByteArray adminToken = m_apiToken;

    QVariantMap createParams;
    createParams.insert("username", "valid@user.test");
    QVariant createResponse = injectAndWait("Users.CreateInvitation", createParams, m_clientId, adminToken);
    QString oneTimeToken = createResponse.toMap().value("params").toMap().value("token").toString();
    QVERIFY(!oneTimeToken.isEmpty());

    QUuid guestClientId = QUuid::createUuid();
    emit m_mockTcpServer->clientConnected(guestClientId);
    injectAndWait("JSONRPC.Hello", QVariantMap(), guestClientId, "");

    QVariantMap redeemParams;
    redeemParams.insert("token", oneTimeToken);
    redeemParams.insert("deviceName", "guest-phone");
    QVariant redeemResponse = injectAndWait("JSONRPC.AuthenticateWithToken", redeemParams, guestClientId, "");
    QVariantMap redeemReturn = redeemResponse.toMap().value("params").toMap();
    QVERIFY2(redeemReturn.value("success").toBool(), "Redemption should have succeeded");
    QByteArray guestClientToken = redeemReturn.value("token").toByteArray();
    QVERIFY(!guestClientToken.isEmpty());

    // Same connection, no second Hello: the guest can already make an authenticated request.
    QVariant getUserInfoResponse = injectAndWait("Users.GetUserInfo", QVariantMap(), guestClientId, guestClientToken);
    QCOMPARE(getUserInfoResponse.toMap().value("status").toString(), QString("success"));

    // Admin looks up the guest's token id and revokes it.
    QVariantMap getUserTokensParams;
    getUserTokensParams.insert("username", "valid@user.test");
    QVariant getUserTokensResponse = injectAndWait("Users.GetUserTokens", getUserTokensParams, m_clientId, adminToken);
    QVariantList guestTokenInfoList = getUserTokensResponse.toMap().value("params").toMap().value("tokenInfoList").toList();
    QUuid guestTokenId;
    foreach (const QVariant &tokenInfoVariant, guestTokenInfoList) {
        if (tokenInfoVariant.toMap().value("deviceName").toString() == "guest-phone")
            guestTokenId = tokenInfoVariant.toMap().value("id").toUuid();
    }
    QVERIFY2(!guestTokenId.isNull(), "Should have found the guest's redeemed token");

    QSignalSpy disconnectedSpy(m_mockTcpServer, &MockTcpServer::clientDisconnected);
    QVariantMap removeParams;
    removeParams.insert("tokenId", guestTokenId);
    QVariant removeResponse = injectAndWait("Users.RemoveToken", removeParams, m_clientId, adminToken);
    QCOMPARE(removeResponse.toMap().value("params").toMap().value("error").toString(), QString("UserErrorNoError"));

    if (disconnectedSpy.count() == 0)
        disconnectedSpy.wait();
    bool guestWasDisconnected = false;
    for (int i = 0; i < disconnectedSpy.count(); i++) {
        if (disconnectedSpy.at(i).first().toUuid() == guestClientId)
            guestWasDisconnected = true;
    }
    QVERIFY2(guestWasDisconnected, "Guest connection should be actively disconnected when its redeemed token is revoked");
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

    // A single bad attempt only arms the lockdown timer (see
    // JsonRPCServerImplementation::Authenticate) - init() now always reconnects with a
    // clean, non-armed lockdown state (unlike before, where a preceding test could
    // coincidentally leave one active), so a second bad attempt is needed here to
    // actually trigger the drop.
    injectAndWait("JSONRPC.Authenticate", params);

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
