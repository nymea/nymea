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

#include <QtTest>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

#include "logging/logengine.h"

#include "usermanager/usermanager.h"

using namespace nymeaserver;

class TestUserLoading: public QObject
{
    Q_OBJECT
public:
    TestUserLoading(QObject* parent = nullptr);

protected slots:
    void initTestCase();

private slots:
    void testLogfileRotation();
    void testMigrationFailureOnExistingUsersIsNotRotated();

};

TestUserLoading::TestUserLoading(QObject *parent): QObject(parent)
{

    Q_INIT_RESOURCE(userloading);
}

void TestUserLoading::initTestCase()
{
    // Important for settings
    QCoreApplication::instance()->setOrganizationName("nymea-test");
}

void TestUserLoading::testLogfileRotation()
{
    // Create UserManager with log db from resource file
    QString temporaryDbName = "/tmp/nymea-test/user-db-broken.sqlite";
    QString rotatedDbName = "/tmp/nymea-test/user-db-broken.sqlite.1";

    // Remove the files if there are some left
    if (QFile::exists(temporaryDbName))
        QVERIFY(QFile(temporaryDbName).remove());

    if (QFile::exists(rotatedDbName))
        QVERIFY(QFile(rotatedDbName).remove());

    // Copy broken user db from resources to default settings path and set permissions
    qDebug() << "Copy broken user db to" << temporaryDbName;
    QVERIFY(QFile::copy(":/user-db-broken.sqlite", temporaryDbName));
    QVERIFY(QFile::setPermissions(temporaryDbName, QFile::ReadOwner | QFile::WriteOwner | QFile::ReadGroup | QFile::ReadOther));

    QVERIFY(!QFile::exists(rotatedDbName));
    UserManager *userManager = new UserManager(temporaryDbName, this);
    QVERIFY(QFile::exists(rotatedDbName));

    delete userManager;

    QVERIFY(QFile(temporaryDbName).remove());
    QVERIFY(QFile(rotatedDbName).remove());
}

void TestUserLoading::testMigrationFailureOnExistingUsersIsNotRotated()
{
    // A users table that already has a real account, but pre-empts the very first
    // migration step (adding the "scopes" column) so it fails with "duplicate column
    // name". This simulates a migration breaking partway through on a database that
    // already holds real user data, as opposed to testLogfileRotation()'s fixture, which
    // has no readable users table at all and is therefore safe to rotate away.
    QString dbName = "/tmp/nymea-test/user-db-migration-failure.sqlite";
    QString rotatedDbName = "/tmp/nymea-test/user-db-migration-failure.sqlite.1";

    if (QFile::exists(dbName))
        QVERIFY(QFile(dbName).remove());
    if (QFile::exists(rotatedDbName))
        QVERIFY(QFile(rotatedDbName).remove());

    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "test-fixture-setup");
        db.setDatabaseName(dbName);
        QVERIFY(db.open());
        QSqlQuery query(db);
        QVERIFY(query.exec("CREATE TABLE users (username VARCHAR(40) UNIQUE PRIMARY KEY, "
                            "password VARCHAR(100), salt VARCHAR(100), scopes TEXT);"));
        QVERIFY(query.exec("INSERT INTO users (username, password, salt, scopes) "
                            "VALUES ('admin', 'somehash', 'somesalt', 'Admin');"));
        db.close();
    }
    QSqlDatabase::removeDatabase("test-fixture-setup");

    UserManager *userManager = new UserManager(dbName, this);
    QVERIFY(userManager->initializationFailed());

    // The existing file must be left exactly in place: no backup/rotation file appears...
    QVERIFY(!QFile::exists(rotatedDbName));

    delete userManager;

    // ...and the original data is still there, untouched, for a fresh connection to read.
    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "test-fixture-verify");
        db.setDatabaseName(dbName);
        QVERIFY(db.open());
        QSqlQuery query(db);
        QVERIFY(query.exec("SELECT username, password, salt, scopes FROM users;"));
        QVERIFY(query.next());
        QCOMPARE(query.value("username").toString(), QString("admin"));
        QCOMPARE(query.value("password").toString(), QString("somehash"));
        QCOMPARE(query.value("scopes").toString(), QString("Admin"));
        QVERIFY(!query.next());
        db.close();
    }
    QSqlDatabase::removeDatabase("test-fixture-verify");

    QVERIFY(QFile(dbName).remove());
}

#include "testuserloading.moc"
QTEST_MAIN(TestUserLoading)
