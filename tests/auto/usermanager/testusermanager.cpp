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

#include <QtTest>

#include "logging/logengine.h"
#include "nymeacore.h"
#include "nymeatestbase.h"
#include "usermanager/usermanager.h"
#include "servers/mocktcpserver.h"

using namespace nymeaserver;

class TestUsermanager: public NymeaTestBase
{
    Q_OBJECT
public:
    TestUsermanager(QObject* parent = nullptr);

private slots:
    void init();

    void loginValidation_data();
    void loginValidation();

    void createUser();
    void authenticate();

    void createDuplicateUser();

    void getTokens();

    void removeToken();

    void unauthenticatedCallAfterTokenRemove();

    void changePassword();

    void authenticateAfterPasswordChangeOK();

    void authenticateAfterPasswordChangeFail();

    void getUserInfo();

private:
    // m_apiToken is in testBase
    QUuid m_tokenId;
};

TestUsermanager::TestUsermanager(QObject *parent): NymeaTestBase(parent)
{
    QCoreApplication::instance()->setOrganizationName("nymea-test");
}

void TestUsermanager::init()
{
    UserManager *userManager = NymeaCore::instance()->userManager();
    foreach (const QString &user, userManager->users()) {
        qCDebug(dcTests()) << "Removing user" << user;
        userManager->removeUser(user);
    }
    userManager->removeUser("");

}

void TestUsermanager::loginValidation_data() {
    QTest::addColumn<QString>("username");
    QTest::addColumn<QString>("password");
    QTest::addColumn<UserManager::UserError>("expectedError");

    QTest::newRow("foo@bar.baz, Bla1234*, NoError")       << "foo@bar.baz" << "Bla1234*" << UserManager::UserErrorNoError;

    QTest::newRow("foo,         Bla1234*, InvalidUserId") << "foo"         << "Bla1234*" << UserManager::UserErrorInvalidUserId;
    QTest::newRow("@,           Bla1234*, InvalidUserId") << "@"           << "Bla1234*" << UserManager::UserErrorInvalidUserId;
    QTest::newRow("foo@,        Bla1234*, InvalidUserId") << "foo@"        << "Bla1234*" << UserManager::UserErrorInvalidUserId;
    QTest::newRow("foo@bar,     Bla1234*, InvalidUserId") << "foo@bar"     << "Bla1234*" << UserManager::UserErrorInvalidUserId;
    QTest::newRow("foo@bar.,    Bla1234*, InvalidUserId") << "foo@bar."    << "Bla1234*" << UserManager::UserErrorInvalidUserId;

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
    UserManager::UserError error = userManager->createUser(username, password);
    qDebug() << "Error:" << error << "Expected:" << expectedError;
    QCOMPARE(error, expectedError);

}

void TestUsermanager::createUser()
{
    QVariantMap params;
    params.insert("username", "valid@user.test");
    params.insert("password", "Bla1234*");
    QVariant response = injectAndWait("Users.CreateUser", params);

    QVERIFY2(response.toMap().value("status").toString() == "success", "Error creating user");
    QVERIFY2(response.toMap().value("params").toMap().value("error").toString() == "UserErrorNoError", "Error creating user");
}

void TestUsermanager::authenticate()
{
    m_apiToken.clear();
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

    QVariantMap params;
    params.insert("username", "valid@user.test");
    params.insert("password", "Bla1234*"); // Original password, should not be ok
    params.insert("deviceName", "autotests");
    QVariant response = injectAndWait("JSONRPC.Authenticate", params);

    m_apiToken = response.toMap().value("params").toMap().value("token").toByteArray();
    QVERIFY2(m_apiToken.isEmpty(), "Token should be empty");
    QVERIFY2(response.toMap().value("status").toString() == "success", "Error authenticating");
    QCOMPARE(response.toMap().value("params").toMap().value("success").toString(), QString("false"));
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

    // need to restart as our connection dies
    restartServer();
}

#include "testusermanager.moc"
QTEST_MAIN(TestUsermanager)
