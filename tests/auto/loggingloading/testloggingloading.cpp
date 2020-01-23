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
#include "logging/logvaluetool.h"

using namespace nymeaserver;

class TestLoggingLoading: public QObject
{
    Q_OBJECT
public:
    TestLoggingLoading(QObject* parent = nullptr);

protected slots:
    void initTestCase();

private slots:
    void testLogMigration();
    void testLogfileRotation();

    void databaseSerializationTest_data();
    void databaseSerializationTest();
};

TestLoggingLoading::TestLoggingLoading(QObject *parent): QObject(parent)
{

    Q_INIT_RESOURCE(loggingloading);

}

void TestLoggingLoading::initTestCase()
{
    // Important for settings
    QCoreApplication::instance()->setOrganizationName("nymea-test");
}

void TestLoggingLoading::testLogMigration()
{
    // Create LogEngine with log db from resource file
    QString temporaryDbName = "/tmp/nymea-test/nymead-v2.sqlite";

    if (QFile::exists(temporaryDbName))
        QVERIFY(QFile(temporaryDbName).remove());

    // Copy v2 log db from resources to default settings path and set permissions
    qDebug() << "Copy logdb v2 to" << temporaryDbName;
    QVERIFY(QFile::copy(":/nymead-v2.sqlite", temporaryDbName));
    QVERIFY(QFile::setPermissions(temporaryDbName, QFile::ReadOwner | QFile::WriteOwner | QFile::ReadGroup | QFile::ReadOther));

    LogEngine *logEngine = new LogEngine("QSQLITE", temporaryDbName);
    // Check there is no rotated logfile
    QVERIFY(!QFile::exists(temporaryDbName + ".1"));

    delete logEngine;

    QVERIFY(QFile(temporaryDbName).remove());
}

void TestLoggingLoading::testLogfileRotation()
{
    // Create LogEngine with log db from resource file
    QString temporaryDbName = "/tmp/nymea-test/nymead-broken.sqlite";
    QString rotatedDbName = "/tmp/nymea-test/nymead-broken.sqlite.1";

    // Remove the files if there are some left
    if (QFile::exists(temporaryDbName))
        QVERIFY(QFile(temporaryDbName).remove());

    if (QFile::exists(rotatedDbName))
        QVERIFY(QFile(rotatedDbName).remove());

    // Copy broken log db from resources to default settings path and set permissions
    qDebug() << "Copy broken log db to" << temporaryDbName;
    QVERIFY(QFile::copy(":/nymead-broken.sqlite", temporaryDbName));
    QVERIFY(QFile::setPermissions(temporaryDbName, QFile::ReadOwner | QFile::WriteOwner | QFile::ReadGroup | QFile::ReadOther));

    QVERIFY(!QFile::exists(rotatedDbName));
    LogEngine *logEngine = new LogEngine("QSQLITE", temporaryDbName);
    QVERIFY(QFile::exists(rotatedDbName));

    delete logEngine;

    QVERIFY(QFile(temporaryDbName).remove());
    QVERIFY(QFile(rotatedDbName).remove());
}

void TestLoggingLoading::databaseSerializationTest_data()
{
    QUuid uuid = QUuid("3782732b-61b4-48e8-8d6d-b5205159d7cd");

    QVariantMap variantMap;
    variantMap.insert("string", "value");
    variantMap.insert("int", 5);
    variantMap.insert("double", 3.14);
    variantMap.insert("uuid", uuid);

    QVariantList variantList;
    variantList.append(variantMap);
    variantList.append("String");
    variantList.append(3.14);
    variantList.append(uuid);

    QTest::addColumn<QVariant>("value");

    QTest::newRow("QString") << QVariant(QString("Hello"));
    QTest::newRow("Integer") << QVariant((int)2);
    QTest::newRow("Double") << QVariant((double)2.34);
    QTest::newRow("Float") << QVariant((float)2.34);
    QTest::newRow("QColor") << QVariant(QColor(0,255,128));
    QTest::newRow("QByteArray") << QVariant(QByteArray::fromHex("01FF332F762A"));
    QTest::newRow("QUuid") << QVariant(uuid);
    QTest::newRow("QVariantMap") << QVariant(variantMap);
    QTest::newRow("QVariantList") << QVariant(variantList);
}

void TestLoggingLoading::databaseSerializationTest()
{
    QFETCH(QVariant, value);

    QString serializedValue = LogValueTool::serializeValue(value);
    QVariant deserializedValue = LogValueTool::deserializeValue(serializedValue);

    qDebug() << "Stored:" << value;
    qDebug() << "Loaded:" << deserializedValue;
    QCOMPARE(deserializedValue, value);
}

#include "testloggingloading.moc"
QTEST_MAIN(TestLoggingLoading)
