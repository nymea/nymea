/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2018 Simon Stürz <simon.stuerz@guh.io>                   *
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
#include "logging/logvaluetool.h"

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

#include "testuserloading.moc"
QTEST_MAIN(TestUserLoading)
