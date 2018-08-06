/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2018 Michael Zanetti <michael.zanetti@guh.io>            *
 *                                                                         *
 *  This file is part of nymea.                                            *
 *                                                                         *
 *  nymea is free software: you can redistribute it and/or modify          *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 **
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with nymea. If not, see <http://www.gnu.org/licenses/>.          *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <QtTest>

#include "logging/logengine.h"
#include "nymeacore.h"
#include "nymeatestbase.h"

using namespace nymeaserver;

class TestUsermanager: public NymeaTestBase
{
    Q_OBJECT
public:
    TestUsermanager(QObject* parent = nullptr);

private slots:
    void createUser_data();
    void createUser();

private:
    LogEngine *engine;
};

TestUsermanager::TestUsermanager(QObject *parent): NymeaTestBase(parent)
{
    QCoreApplication::instance()->setOrganizationName("nymea-test");
}

void TestUsermanager::createUser_data() {
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

    QTest::newRow("foo@bar.baz, aaaaa!1A, BadPassword") << "foo@bar.baz" << "aaaaa!1A" << UserManager::UserErrorNoError;
    QTest::newRow("foo@bar.baz, aaaaaa!1, BadPassword") << "foo@bar.baz" << "aaaaaa!1" << UserManager::UserErrorNoError;

    QTest::newRow("foo@bar.baz, Bla1234*a, NoError")           << "foo@bar.baz" << "Bla1234*a" << UserManager::UserErrorNoError;
    QTest::newRow("foo@bar.baz, #1-Nymea-is-awesome, NoError") << "foo@bar.baz" << "#1-Nymea-is-awesome" << UserManager::UserErrorNoError;
    QTest::newRow("foo@bar.baz, Bla1234.a, NoError")           << "foo@bar.baz" << "Bla1234.a" << UserManager::UserErrorNoError;
    QTest::newRow("foo@bar.baz, Bla1234\\a, NoError")          << "foo@bar.baz" << "Bla1234\\a" << UserManager::UserErrorNoError;

}

void TestUsermanager::createUser()
{
    QFETCH(QString, username);
    QFETCH(QString, password);
    QFETCH(UserManager::UserError, expectedError);

    UserManager *userManager = NymeaCore::instance()->userManager();
    foreach (const QString &user, userManager->users()) {
        userManager->removeUser(user);
    }
    userManager->removeUser("");

    UserManager::UserError error = userManager->createUser(username, password);
    qDebug() << "Error:" << error << "Expected:" << expectedError;
    QCOMPARE(error, expectedError);

}

#include "testusermanager.moc"
QTEST_MAIN(TestUsermanager)
