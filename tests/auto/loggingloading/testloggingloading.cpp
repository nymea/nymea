/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2017 Simon Stürz <simon.stuerz@guh.io>                   *
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
    QString temporaryDbName = NymeaSettings::settingsPath() + "/nymead-v2.sqlite";

    if (QFile::exists(temporaryDbName))
        QVERIFY(QFile(temporaryDbName).remove());

    // Copy v2 log db from resources to default settings path and set permissions
    qDebug() << "Copy logdb v2 to" << temporaryDbName;
    QVERIFY(QFile::copy(":/nymead-v2.sqlite", temporaryDbName));
    QVERIFY(QFile::setPermissions(temporaryDbName, QFile::ReadOwner | QFile::WriteOwner | QFile::ReadGroup | QFile::ReadOther));

    LogEngine *logEngine = new LogEngine(temporaryDbName, this);
    // Check there is no rotated logfile
    QVERIFY(!QFile::exists(temporaryDbName + ".1"));

    delete logEngine;

    QVERIFY(QFile(temporaryDbName).remove());
}

void TestLoggingLoading::testLogfileRotation()
{
    // Create LogEngine with log db from resource file
    QString temporaryDbName = NymeaSettings::settingsPath() + "/nymead-broken.sqlite";
    QString rotatedDbName = NymeaSettings::settingsPath() + "/nymead-broken.sqlite.1";

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
    LogEngine *logEngine = new LogEngine(temporaryDbName, this);
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
