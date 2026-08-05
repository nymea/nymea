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
    void testFreshDatabaseIsCreatedAtVersion4();
    void testMigrationV3ToV4PreservesExistingData();
    void testMigrationV3ToV4FailureRollsBackAndPreservesVersion();
    void testExpiredTokenIsRejectedByResolver();
    void testFutureExpiryTokenIsStillValid();
    void testAlreadyExpiredTokenIsPurgedOnStartup();
    void testSchedulerPurgesTokenWhenItExpires();

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

void TestUserLoading::testFreshDatabaseIsCreatedAtVersion4()
{
    QString dbName = "/tmp/nymea-test/user-db-fresh-v4.sqlite";
    if (QFile::exists(dbName))
        QVERIFY(QFile(dbName).remove());

    UserManager *userManager = new UserManager(dbName, this);
    QVERIFY(!userManager->initializationFailed());
    delete userManager;

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "test-fixture-verify-fresh");
    db.setDatabaseName(dbName);
    QVERIFY(db.open());

    QSqlQuery columns(db);
    QVERIFY(columns.exec("PRAGMA table_info(tokens);"));
    QStringList tokenColumns;
    while (columns.next())
        tokenColumns << columns.value("name").toString();
    QVERIFY(tokenColumns.contains("expirydate"));
    QVERIFY(tokenColumns.contains("lastseen"));

    QSqlQuery version(db);
    QVERIFY(version.exec("SELECT data FROM metadata WHERE key = 'version';"));
    QVERIFY(version.next());
    QCOMPARE(version.value("data").toString(), QString("4"));

    db.close();
    QSqlDatabase::removeDatabase("test-fixture-verify-fresh");
    QVERIFY(QFile(dbName).remove());
}

namespace {
// Writes a schema-v3 fixture (pre-token-expiry) with one real user and one real token,
// so migration tests exercise the same v3 -> v4 step production databases will go through.
// If tokensTableAlreadyHasExpirydate is true, the fixture pre-empts the migration's first
// ALTER TABLE to force it to fail with a duplicate-column error.
void writeV3Fixture(const QString &dbName, bool tokensTableAlreadyHasExpirydate = false)
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "test-fixture-writer");
    db.setDatabaseName(dbName);
    QVERIFY(db.open());
    QSqlQuery query(db);
    QVERIFY(query.exec("CREATE TABLE users (username VARCHAR(40) UNIQUE PRIMARY KEY, email VARCHAR(40), "
                        "displayName VARCHAR(40), password VARCHAR(100), salt VARCHAR(100), scopes TEXT, "
                        "allowedThingIds TEXT);"));
    QVERIFY(query.exec("INSERT INTO users (username, email, displayName, password, salt, scopes, allowedThingIds) "
                        "VALUES ('admin', 'admin@example.com', 'Admin', 'somehash', 'somesalt', 'Admin', '');"));
    QVERIFY(query.exec("CREATE TABLE userInventory (id VARCHAR(40) UNIQUE PRIMARY KEY, username VARCHAR(40), "
                        "type VARCHAR(40), displayName VARCHAR(100), enabled INTEGER, payload TEXT);"));
    QString tokensColumns = "id VARCHAR(40) UNIQUE, username VARCHAR(40), token VARCHAR(100) UNIQUE, "
                            "creationdate DATETIME, devicename VARCHAR(40)";
    if (tokensTableAlreadyHasExpirydate)
        tokensColumns += ", expirydate DATETIME";
    QVERIFY(query.exec(QString("CREATE TABLE tokens (%1);").arg(tokensColumns)));
    QVERIFY(query.exec("INSERT INTO tokens (id, username, token, creationdate, devicename) VALUES "
                        "('11111111-1111-1111-1111-111111111111', 'admin', 'sometoken', "
                        "'2026-01-01 00:00:00', 'somedevice');"));
    QVERIFY(query.exec("CREATE TABLE metadata (key VARCHAR(10), data VARCHAR(40));"));
    QVERIFY(query.exec("INSERT INTO metadata (key, data) VALUES ('version', '3');"));
    db.close();
    QSqlDatabase::removeDatabase("test-fixture-writer");
}
}

void TestUserLoading::testMigrationV3ToV4PreservesExistingData()
{
    QString dbName = "/tmp/nymea-test/user-db-v3-to-v4.sqlite";
    QString rotatedDbName = "/tmp/nymea-test/user-db-v3-to-v4.sqlite.1";
    if (QFile::exists(dbName))
        QVERIFY(QFile(dbName).remove());
    if (QFile::exists(rotatedDbName))
        QVERIFY(QFile(rotatedDbName).remove());

    writeV3Fixture(dbName);

    UserManager *userManager = new UserManager(dbName, this);
    QVERIFY(!userManager->initializationFailed());
    QVERIFY(!QFile::exists(rotatedDbName));
    delete userManager;

    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "test-fixture-verify-migrated");
        db.setDatabaseName(dbName);
        QVERIFY(db.open());

        QSqlQuery columns(db);
        QVERIFY(columns.exec("PRAGMA table_info(tokens);"));
        QStringList tokenColumns;
        while (columns.next())
            tokenColumns << columns.value("name").toString();
        QVERIFY(tokenColumns.contains("expirydate"));
        QVERIFY(tokenColumns.contains("lastseen"));

        QSqlQuery version(db);
        QVERIFY(version.exec("SELECT data FROM metadata WHERE key = 'version';"));
        QVERIFY(version.next());
        QCOMPARE(version.value("data").toString(), QString("4"));

        QSqlQuery token(db);
        QVERIFY(token.exec("SELECT token, expirydate, lastseen FROM tokens WHERE id = "
                            "'11111111-1111-1111-1111-111111111111';"));
        QVERIFY(token.next());
        QCOMPARE(token.value("token").toString(), QString("sometoken"));
        QVERIFY(token.value("expirydate").isNull());
        QVERIFY(token.value("lastseen").isNull());

        QSqlQuery user(db);
        QVERIFY(user.exec("SELECT username FROM users WHERE username = 'admin';"));
        QVERIFY(user.next());

        db.close();
    }
    QSqlDatabase::removeDatabase("test-fixture-verify-migrated");

    // Reopening an already-migrated (v4) database must succeed without touching it again.
    UserManager *reopened = new UserManager(dbName, this);
    QVERIFY(!reopened->initializationFailed());
    delete reopened;

    QVERIFY(QFile(dbName).remove());
}

void TestUserLoading::testMigrationV3ToV4FailureRollsBackAndPreservesVersion()
{
    QString dbName = "/tmp/nymea-test/user-db-v3-to-v4-failure.sqlite";
    QString rotatedDbName = "/tmp/nymea-test/user-db-v3-to-v4-failure.sqlite.1";
    if (QFile::exists(dbName))
        QVERIFY(QFile(dbName).remove());
    if (QFile::exists(rotatedDbName))
        QVERIFY(QFile(rotatedDbName).remove());

    // Pre-empt the migration's first ALTER TABLE so it fails with a duplicate-column error.
    writeV3Fixture(dbName, /*tokensTableAlreadyHasExpirydate=*/true);

    UserManager *userManager = new UserManager(dbName, this);
    QVERIFY(userManager->initializationFailed());
    QVERIFY(!QFile::exists(rotatedDbName));
    delete userManager;

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "test-fixture-verify-failed");
    db.setDatabaseName(dbName);
    QVERIFY(db.open());

    // Version must not have been bumped, and the second column must never have been added.
    QSqlQuery version(db);
    QVERIFY(version.exec("SELECT data FROM metadata WHERE key = 'version';"));
    QVERIFY(version.next());
    QCOMPARE(version.value("data").toString(), QString("3"));

    QSqlQuery columns(db);
    QVERIFY(columns.exec("PRAGMA table_info(tokens);"));
    QStringList tokenColumns;
    while (columns.next())
        tokenColumns << columns.value("name").toString();
    QVERIFY(!tokenColumns.contains("lastseen"));

    QSqlQuery token(db);
    QVERIFY(token.exec("SELECT token FROM tokens WHERE id = '11111111-1111-1111-1111-111111111111';"));
    QVERIFY(token.next());
    QCOMPARE(token.value("token").toString(), QString("sometoken"));

    db.close();
    QSqlDatabase::removeDatabase("test-fixture-verify-failed");

    QVERIFY(QFile(dbName).remove());
}

namespace {
// Writes an already-v4 fixture (tokens table already has expirydate/lastseen) with one
// real user and one real token carrying the given expirydate (empty = never expires).
void writeV4FixtureWithToken(const QString &dbName, const QString &tokenValue, const QString &expiryDateIso)
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "test-fixture-writer-v4");
    db.setDatabaseName(dbName);
    QVERIFY(db.open());
    QSqlQuery query(db);
    QVERIFY(query.exec("CREATE TABLE users (username VARCHAR(40) UNIQUE PRIMARY KEY, email VARCHAR(40), "
                        "displayName VARCHAR(40), password VARCHAR(100), salt VARCHAR(100), scopes TEXT, "
                        "allowedThingIds TEXT);"));
    QVERIFY(query.exec("INSERT INTO users (username, email, displayName, password, salt, scopes, allowedThingIds) "
                        "VALUES ('admin', 'admin@example.com', 'Admin', 'somehash', 'somesalt', 'Admin', '');"));
    QVERIFY(query.exec("CREATE TABLE userInventory (id VARCHAR(40) UNIQUE PRIMARY KEY, username VARCHAR(40), "
                        "type VARCHAR(40), displayName VARCHAR(100), enabled INTEGER, payload TEXT);"));
    QVERIFY(query.exec("CREATE TABLE tokens (id VARCHAR(40) UNIQUE, username VARCHAR(40), token VARCHAR(100) UNIQUE, "
                        "creationdate DATETIME, devicename VARCHAR(40), expirydate DATETIME, lastseen DATETIME);"));
    // Braced, matching QUuid::createUuid().toString()'s default format used in production.
    query.prepare("INSERT INTO tokens (id, username, token, creationdate, devicename, expirydate, lastseen) "
                  "VALUES ('{22222222-2222-2222-2222-222222222222}', 'admin', :token, '2026-01-01T00:00:00', "
                  "'somedevice', :expirydate, NULL);");
    query.bindValue(":token", tokenValue);
    query.bindValue(":expirydate", expiryDateIso.isEmpty() ? QVariant() : QVariant(expiryDateIso));
    QVERIFY(query.exec());
    QVERIFY(query.exec("CREATE TABLE metadata (key VARCHAR(10), data VARCHAR(40));"));
    QVERIFY(query.exec("INSERT INTO metadata (key, data) VALUES ('version', '4');"));
    db.close();
    QSqlDatabase::removeDatabase("test-fixture-writer-v4");
}
}

void TestUserLoading::testExpiredTokenIsRejectedByResolver()
{
    QString dbName = "/tmp/nymea-test/user-db-expired-token.sqlite";
    if (QFile::exists(dbName))
        QVERIFY(QFile(dbName).remove());

    // A token whose SHA-256/base64-shaped stand-in value passes validateToken()'s charset
    // check and expired well in the past.
    QString tokenValue = "sGVzdCB0b2tlbiB2YWx1ZSBmb3IgdGVzdGluZyBwdXJwb3Nlcw==";
    writeV4FixtureWithToken(dbName, tokenValue, "2020-01-01T00:00:00");

    UserManager *userManager = new UserManager(dbName, this);
    QVERIFY(!userManager->initializationFailed());

    QByteArray tokenBytes = tokenValue.toUtf8();
    QVERIFY(!userManager->verifyToken(tokenBytes));
    QVERIFY(userManager->tokenInfo(tokenBytes).id().isNull());
    QVERIFY(userManager->tokenInfo(QUuid("22222222-2222-2222-2222-222222222222")).id().isNull());
    QCOMPARE(userManager->tokens("admin").count(), 0);

    delete userManager;
    QVERIFY(QFile(dbName).remove());
}

void TestUserLoading::testFutureExpiryTokenIsStillValid()
{
    QString dbName = "/tmp/nymea-test/user-db-future-expiry-token.sqlite";
    if (QFile::exists(dbName))
        QVERIFY(QFile(dbName).remove());

    QString tokenValue = "sGVzdCB0b2tlbiB2YWx1ZSBmb3IgdGVzdGluZyBwdXJwb3Nlcw==";
    writeV4FixtureWithToken(dbName, tokenValue, "2099-01-01T00:00:00");

    UserManager *userManager = new UserManager(dbName, this);
    QVERIFY(!userManager->initializationFailed());

    QByteArray tokenBytes = tokenValue.toUtf8();
    QVERIFY(userManager->verifyToken(tokenBytes));

    TokenInfo info = userManager->tokenInfo(tokenBytes);
    QVERIFY(!info.id().isNull());
    QVERIFY(info.expiryTime().isValid());
    QDateTime expected = QDateTime::fromString("2099-01-01T00:00:00", Qt::ISODate);
    expected.setTimeSpec(Qt::UTC);
    QCOMPARE(info.expiryTime(), expected);

    QVERIFY(!userManager->tokenInfo(QUuid("{22222222-2222-2222-2222-222222222222}")).id().isNull());

    QCOMPARE(userManager->tokens("admin").count(), 1);

    delete userManager;
    QVERIFY(QFile(dbName).remove());
}

void TestUserLoading::testAlreadyExpiredTokenIsPurgedOnStartup()
{
    QString dbName = "/tmp/nymea-test/user-db-purge-on-startup.sqlite";
    if (QFile::exists(dbName))
        QVERIFY(QFile(dbName).remove());

    QString tokenValue = "sGVzdCB0b2tlbiB2YWx1ZSBmb3IgdGVzdGluZyBwdXJwb3Nlcw==";
    writeV4FixtureWithToken(dbName, tokenValue, "2020-01-01T00:00:00");

    UserManager *userManager = new UserManager(dbName, this);
    QVERIFY(!userManager->initializationFailed());

    // The constructor's initial rearmExpiryTimer() call purges synchronously; the row
    // and its notification must both already be gone by the time construction returns.
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "test-fixture-verify-purged");
    db.setDatabaseName(dbName);
    QVERIFY(db.open());
    QSqlQuery query(db);
    QVERIFY(query.exec("SELECT COUNT(*) AS c FROM tokens;"));
    QVERIFY(query.next());
    QCOMPARE(query.value("c").toInt(), 0);
    db.close();
    QSqlDatabase::removeDatabase("test-fixture-verify-purged");

    delete userManager;
    QVERIFY(QFile(dbName).remove());
}

void TestUserLoading::testSchedulerPurgesTokenWhenItExpires()
{
    QString dbName = "/tmp/nymea-test/user-db-scheduler-purge.sqlite";
    if (QFile::exists(dbName))
        QVERIFY(QFile(dbName).remove());

    QString tokenValue = "sGVzdCB0b2tlbiB2YWx1ZSBmb3IgdGVzdGluZyBwdXJwb3Nlcw==";
    // Expires shortly after construction: exercises the timer actually firing and
    // re-arming for the nearest deadline, not just the constructor's initial purge.
    QString soonExpiry = QDateTime::currentDateTimeUtc().addSecs(2).toString(Qt::ISODate);
    writeV4FixtureWithToken(dbName, tokenValue, soonExpiry);

    UserManager *userManager = new UserManager(dbName, this);
    QVERIFY(!userManager->initializationFailed());
    // Not expired yet at construction time.
    QVERIFY(userManager->verifyToken(tokenValue.toUtf8()));

    QSignalSpy invalidatedSpy(userManager, &UserManager::tokenInvalidated);
    QVERIFY(invalidatedSpy.wait(5000));
    QCOMPARE(invalidatedSpy.count(), 1);
    QCOMPARE(invalidatedSpy.first().first().toByteArray(), tokenValue.toUtf8());

    QVERIFY(!userManager->verifyToken(tokenValue.toUtf8()));

    delete userManager;
    QVERIFY(QFile(dbName).remove());
}

#include "testuserloading.moc"
QTEST_MAIN(TestUserLoading)
