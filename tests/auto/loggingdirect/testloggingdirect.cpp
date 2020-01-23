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

using namespace nymeaserver;

class TestLoggingDirect: public QObject
{
    Q_OBJECT
public:
    TestLoggingDirect(QObject* parent = nullptr);

private slots:
    void benchmarkDB_data();
    void benchmarkDB();

private:
    LogEngine *engine;
};

TestLoggingDirect::TestLoggingDirect(QObject *parent): QObject(parent)
{
    engine = new LogEngine("QSQLITE", "/tmp/nymea-test/nymea.sqlite");
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
//    QTest::newRow("20000, no trim") << 20000 << 30000;
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
    if (qgetenv("WITH_BENCHMARK").isEmpty()) {
        QSKIP("Skipping benchmark tests: export WITH_BENCHMARK=1 to enable it.");
    }

    QFETCH(int, prefill);
    QFETCH(int, maxSize);

    // setting max log entries to "prefill" to trim it down to what this test needs.
    int overflow = 10;
    qDebug() << "Flushing DB for test";
    engine->setMaxLogEntries(prefill, overflow);
    engine->setMaxLogEntries(maxSize, overflow);

    LogEntriesFetchJob *job = engine->fetchLogEntries();
    QSignalSpy fetchSpy(job, &LogEntriesFetchJob::finished);
    fetchSpy.wait();
    QList<LogEntry> entries = job->results();

    qDebug() << "DB has" << entries.count() << "entries";
    qDebug() << "Prefilling DB for test";
    for (int i = entries.count(); i < prefill; i++) {
        engine->logSystemEvent(QDateTime::currentDateTime(), true);
    }

    job = engine->fetchLogEntries();
    QSignalSpy fetchSpy2(job, &LogEntriesFetchJob::finished);
    fetchSpy2.wait();
    entries = job->results();

    qDebug() << "DB has" << entries.count() << "entries";

    qDebug() << "Starting benchmark with" << entries.count() << "entries in the db";
    QBENCHMARK {
        engine->logSystemEvent(QDateTime::currentDateTime(), true);
    }
    QDateTime now = QDateTime::currentDateTime();

    job = engine->fetchLogEntries();
    QSignalSpy fetchSpy3(job, &LogEntriesFetchJob::finished);
    fetchSpy3.wait();
    entries = job->results();

    while (entries.count() > maxSize + overflow) {
        qApp->processEvents();
        if (now.addSecs(5) < QDateTime::currentDateTime()) {
            QVERIFY2(false, QString("Housekeeping didn't work. Have %1 entries but expected to have max %2").arg(entries.count()).arg(QString::number(maxSize)).toLocal8Bit());
        }
    }
    qDebug() << "Ended benchmark with" << entries.count() << "entries in the db";
}

#include "testloggingdirect.moc"
QTEST_MAIN(TestLoggingDirect)
