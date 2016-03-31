/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stuerz <simon.stuerz@guh.guru>                *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
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

#include "guhtestbase.h"
#include "guhcore.h"
#include "time/timemanager.h"
#include "devicemanager.h"
#include "mocktcpserver.h"

#include <QTimeZone>
#include <QDateTime>
#include <QtTest/QtTest>
#include <QCoreApplication>

using namespace guhserver;

class TestTimeManager: public GuhTestBase
{
    Q_OBJECT

private slots:
    void changeTimeZone_data();
    void changeTimeZone();

};




void TestTimeManager::changeTimeZone_data()
{
    QTest::addColumn<QByteArray>("timeZoneId");
    QTest::addColumn<bool>("valid");

    QTest::newRow("valid timezone: Asia/Tokyo") << QByteArray("Asia/Tokyo") << true;
    QTest::newRow("valid timezone: America/Lima") << QByteArray("America/Lima") << true;
    QTest::newRow("valid timezone: Africa/Harare") << QByteArray("Africa/Harare") << true;
    QTest::newRow("invalid timezone: Mars/Diacria") << QByteArray("Mars/Diacria") << false;
    QTest::newRow("invalid timezone: Moon/Kepler") << QByteArray("Moon/Kepler") << false;
}

void TestTimeManager::changeTimeZone()
{
    QFETCH(QByteArray, timeZoneId);
    QFETCH(bool, valid);

    QTimeZone currentTimeZone(GuhCore::instance()->timeManager()->timeZone());
    QTimeZone newTimeZone(timeZoneId);


    QDateTime currentDateTime = GuhCore::instance()->timeManager()->currentDateTime();
    qDebug() << currentDateTime.toString();

    GuhCore::instance()->timeManager()->setTimeZone(timeZoneId);

    QDateTime newDateTime = GuhCore::instance()->timeManager()->currentDateTime();
    qDebug() << newDateTime.toString();

    int offsetOriginal = currentTimeZone.offsetFromUtc(currentDateTime);
    int offsetNew = newTimeZone.offsetFromUtc(newDateTime);

    if (valid)
        QVERIFY(offsetOriginal != offsetNew);

}

#include "testtimemanager.moc"
QTEST_MAIN(TestTimeManager)
