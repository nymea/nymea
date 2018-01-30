/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2017 Michael Zanetti <michael.zanetti@guh.io>            *
 *                                                                         *
 *  This file is part of guh.                                              *
 *                                                                         *
 *  Guh is free software: you can redistribute it and/or modify            *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  Guh is distributed in the hope that it will be useful,                 *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with guh. If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <QtTest>

#include "logging/logengine.h"

using namespace guhserver;

class TestLoggingDirect: public QObject
{
    Q_OBJECT
public:
    TestLoggingDirect(QObject* parent = nullptr);

private slots:
    void benchmarkDB_data();
    void benchmarkDB();

private:
    LogEngine engine;
};

TestLoggingDirect::TestLoggingDirect(QObject *parent): QObject(parent)
{
    // Setting timeout to 20 mins
    qputenv("QTEST_FUNCTION_TIMEOUT", "1200000");
    QCoreApplication::instance()->setOrganizationName("nymea-test");
}

void TestLoggingDirect::benchmarkDB_data() {
    QTest::addColumn<int>("prefill");
    QTest::addColumn<int>("maxSize");

    QTest::newRow("empty, no trim") << 0 << 20000;
    QTest::newRow("empty, trim") << 1 << 1;
    QTest::newRow("10000, no trim") << 10000 << 20000;
    QTest::newRow("10000, trim") << 10000 << 10000;
    QTest::newRow("20000, no trim") << 20000 << 30000;
//    QTest::newRow("20000, trim") << 20000 << 20000;
//    QTest::newRow("30000, no trim") << 30000 << 40000;
//    QTest::newRow("30000, trim") << 30000 << 30000;
//    QTest::newRow("40000, no trim") << 40000 << 50000;
//    QTest::newRow("40000, trim") << 40000 << 40000;
//    QTest::newRow("50000, no trim") << 50000 << 60000;
//    QTest::newRow("50000, trim") << 50000 << 50000;
//    QTest::newRow("60000, no trim") << 60000 << 70000;
//    QTest::newRow("60000, trim") << 60000 << 60000;

}

void TestLoggingDirect::benchmarkDB()
{
    QFETCH(int, prefill);
    QFETCH(int, maxSize);

    // setting max log entries to "prefill" to trim it down to what this test needs.
    int overflow = 10;
    engine.setMaxLogEntries(prefill, overflow);
    engine.setMaxLogEntries(maxSize, overflow);

    for (int i = engine.logEntries().count(); i < prefill; i++) {
        engine.logSystemEvent(QDateTime::currentDateTime(), true);
    }

    qDebug() << "Starting benchmark with" << engine.logEntries().count() << "entries in the db";
    QBENCHMARK {
        engine.logSystemEvent(QDateTime::currentDateTime(), true);
    }
    QDateTime now = QDateTime::currentDateTime();
    while (engine.logEntries().count() > maxSize + overflow) {
        qApp->processEvents();
        if (now.addSecs(5) < QDateTime::currentDateTime()) {
            QVERIFY2(false, QString("Housekeeping didn't work. Have %1 entries but expected to have max %2").arg(engine.logEntries().count()).arg(QString::number(maxSize)).toLocal8Bit());
        }
    }
    qDebug() << "Ended benchmark with" << engine.logEntries().count() << "entries in the db";
}

#include "testloggingdirect.moc"
QTEST_MAIN(TestLoggingDirect)
