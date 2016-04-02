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

#include <QtTest/QtTest>
#include <QCoreApplication>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTimeZone>
#include <QDateTime>

using namespace guhserver;

class TestTimeManager: public GuhTestBase
{
    Q_OBJECT

private slots:
    void changeTimeZone_data();
    void changeTimeZone();

    void addTimeDescriptor_data();
    void addTimeDescriptor();

    void testCalendarItemHourly();

    void testCalendarItemDaily_data();
    void testCalendarItemDaily();

    void testCalendarItemWeekly_data();
    void testCalendarItemWeekly();

private:
    void initTimeManager();

    void verifyRuleExecuted(const ActionTypeId &actionTypeId);
    void verifyRuleNotExecuted();

    void cleanupMockHistory();

    QVariantMap createTimeEventItem(const QString &time = QString(), const QVariantMap &repeatingOption = QVariantMap()) const;
    QVariantMap createTimeEventItem(const int &dateTime, const QVariantMap &repeatingOption = QVariantMap()) const;
    QVariantMap createTimeDescriptorTimeEvent(const QVariantMap &timeEventItem) const;
    QVariantMap createTimeDescriptorTimeEvent(const QVariantList &timeEventItems) const;

    QVariantMap createCalendarItem(const QString &time = QString(), const uint &duration = 0, const QVariantMap &repeatingOption = QVariantMap()) const;
    QVariantMap createCalendarItem(const int &dateTime, const uint &duration = 0, const QVariantMap &repeatingOption = QVariantMap()) const;
    QVariantMap createTimeDescriptorCalendar(const QVariantMap &calendarItem) const;
    QVariantMap createTimeDescriptorCalendar(const QVariantList &calendarItems) const;
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

void TestTimeManager::addTimeDescriptor_data()
{
    // valid RepeatingOptions
    QVariantMap repeatingOptionDaily;
    repeatingOptionDaily.insert("mode", "RepeatingModeDaily");

    QVariantMap repeatingOptionNone;
    repeatingOptionNone.insert("mode", "RepeatingModeNone");

    QVariantMap repeatingOptionHourly;
    repeatingOptionHourly.insert("mode", "RepeatingModeHourly");

    QVariantMap repeatingOptionWeeklyMultiple;
    repeatingOptionWeeklyMultiple.insert("mode", "RepeatingModeWeekly");
    repeatingOptionWeeklyMultiple.insert("weekDays", QVariantList() << 2 << 4 << 5);

    QVariantMap repeatingOptionMonthlyMultiple;
    repeatingOptionMonthlyMultiple.insert("mode", "RepeatingModeMonthly");
    repeatingOptionMonthlyMultiple.insert("monthDays", QVariantList() << 20 << 14 << 5);

    // invalid RepeatingOptions
    QVariantMap repeatingOptionInvalidNone;
    repeatingOptionInvalidNone.insert("mode", "RepeatingModeNone");
    repeatingOptionInvalidNone.insert("monthDays", QVariantList() << 13 << 12 << 27);

    QVariantMap repeatingOptionInvalidWeekly;
    repeatingOptionInvalidWeekly.insert("mode", "RepeatingModeWeekly");
    repeatingOptionInvalidWeekly.insert("monthDays", QVariantList() << 12 << 2 << 7);

    QVariantMap repeatingOptionInvalidMonthly;
    repeatingOptionInvalidMonthly.insert("mode", "RepeatingModeMonthly");
    repeatingOptionInvalidMonthly.insert("weekDays", QVariantList() << 1 << 2 << 7);

    QVariantMap repeatingOptionInvalidWeekDays;
    repeatingOptionInvalidWeekDays.insert("mode", "RepeatingModeWeekly");
    repeatingOptionInvalidWeekDays.insert("weekDays", QVariantList() << -1);

    QVariantMap repeatingOptionInvalidWeekDays2;
    repeatingOptionInvalidWeekDays2.insert("mode", "RepeatingModeWeekly");
    repeatingOptionInvalidWeekDays2.insert("weekDays", QVariantList() << 8);

    QVariantMap repeatingOptionInvalidMonthDays;
    repeatingOptionInvalidMonthDays.insert("mode", "RepeatingModeMonthly");
    repeatingOptionInvalidMonthDays.insert("monthDays", QVariantList() << -1);

    QVariantMap repeatingOptionInvalidMonthDays2;
    repeatingOptionInvalidMonthDays2.insert("mode", "RepeatingModeMonthly");
    repeatingOptionInvalidMonthDays2.insert("monthDays", QVariantList() << 32);

    // Multiple calendar items
    QVariantList calendarItems;
    calendarItems.append(createCalendarItem("08:00", 5, repeatingOptionDaily));
    calendarItems.append(createCalendarItem("09:00", 5, repeatingOptionWeeklyMultiple));

    // Multiple timeEvent items
    QVariantList timeEventItems;
    timeEventItems.append(createTimeEventItem("08:00", repeatingOptionDaily));
    timeEventItems.append(createTimeEventItem("09:00", repeatingOptionWeeklyMultiple));

    QTest::addColumn<QVariantMap>("timeDescriptor");
    QTest::addColumn<RuleEngine::RuleError>("error");

    QTest::newRow("valid: calendarItem") << createTimeDescriptorCalendar(createCalendarItem("08:00", 5)) << RuleEngine::RuleErrorNoError;
    QTest::newRow("valid: calendarItem dateTime") << createTimeDescriptorCalendar(createCalendarItem(QDateTime::currentDateTime().toTime_t(), 5)) << RuleEngine::RuleErrorNoError;
    QTest::newRow("valid: calendarItem - daily") << createTimeDescriptorCalendar(createCalendarItem("08:00", 5, repeatingOptionDaily)) << RuleEngine::RuleErrorNoError;
    QTest::newRow("valid: calendarItem - none") << createTimeDescriptorCalendar(createCalendarItem("09:00", 30, repeatingOptionNone)) << RuleEngine::RuleErrorNoError;
    QTest::newRow("valid: calendarItem - hourly") << createTimeDescriptorCalendar(createCalendarItem("09:00", 30, repeatingOptionHourly)) << RuleEngine::RuleErrorNoError;
    QTest::newRow("valid: calendarItems - weekly - multiple days") << createTimeDescriptorCalendar(calendarItems) << RuleEngine::RuleErrorNoError;
    QTest::newRow("valid: calendarItem - monthly - multiple days") << createTimeDescriptorCalendar(createCalendarItem("23:00", 5, repeatingOptionMonthlyMultiple)) << RuleEngine::RuleErrorNoError;

    QTest::newRow("valid: timeEventItem") << createTimeDescriptorTimeEvent(createTimeEventItem("08:00")) << RuleEngine::RuleErrorNoError;
    QTest::newRow("valid: timeEventItem dateTime") << createTimeDescriptorTimeEvent(createTimeEventItem(QDateTime::currentDateTime().toTime_t())) << RuleEngine::RuleErrorNoError;
    QTest::newRow("valid: timeEventItem - daily") << createTimeDescriptorTimeEvent(createTimeEventItem("08:00", repeatingOptionDaily)) << RuleEngine::RuleErrorNoError;
    QTest::newRow("valid: timeEventItem - none") << createTimeDescriptorTimeEvent(createTimeEventItem("09:00", repeatingOptionNone)) << RuleEngine::RuleErrorNoError;
    QTest::newRow("valid: timeEventItem - hourly") << createTimeDescriptorTimeEvent(createTimeEventItem("09:00", repeatingOptionHourly)) << RuleEngine::RuleErrorNoError;
    QTest::newRow("valid: timeEventItem - weekly - multiple days") << createTimeDescriptorTimeEvent(timeEventItems) << RuleEngine::RuleErrorNoError;
    QTest::newRow("valid: timeEventItem - monthly - multiple days") << createTimeDescriptorTimeEvent(createTimeEventItem("23:00", repeatingOptionMonthlyMultiple)) << RuleEngine::RuleErrorNoError;

    QTest::newRow("invalid: calendarItem empty") << createTimeDescriptorCalendar(createCalendarItem()) << RuleEngine::RuleErrorInvalidCalendarItem;
    QTest::newRow("invalid: calendarItem none") << createTimeDescriptorCalendar(createCalendarItem("00:12", 12, repeatingOptionInvalidNone)) << RuleEngine::RuleErrorInvalidRepeatingOption;
    QTest::newRow("invalid: calendarItem - dateTime + repeatingOption") << createTimeDescriptorCalendar(createCalendarItem(QDateTime::currentDateTime().toTime_t(), 5, repeatingOptionDaily)) << RuleEngine::RuleErrorInvalidCalendarItem;
    QTest::newRow("invalid: calendarItem invalid time") << createTimeDescriptorCalendar(createCalendarItem("35:80", 5)) << RuleEngine::RuleErrorInvalidCalendarItem;
    QTest::newRow("invalid: calendarItem invalid duration") << createTimeDescriptorCalendar(createCalendarItem("12:00", 0)) << RuleEngine::RuleErrorInvalidCalendarItem;
    QTest::newRow("invalid: calendarItem - monthly - weekDays") << createTimeDescriptorCalendar(createCalendarItem("13:13", 5, repeatingOptionInvalidMonthly)) << RuleEngine::RuleErrorInvalidRepeatingOption;
    QTest::newRow("invalid: calendarItem - weekly - monthDays") << createTimeDescriptorCalendar(createCalendarItem("15:30", 20, repeatingOptionInvalidWeekly)) << RuleEngine::RuleErrorInvalidRepeatingOption;
    QTest::newRow("invalid: calendarItem - invalid weekdays  (negative)") << createTimeDescriptorCalendar(createCalendarItem("13:13", 5, repeatingOptionInvalidWeekDays)) << RuleEngine::RuleErrorInvalidRepeatingOption;
    QTest::newRow("invalid: calendarItem - invalid weekdays  (to big)") << createTimeDescriptorCalendar(createCalendarItem("13:13", 5, repeatingOptionInvalidWeekDays2)) << RuleEngine::RuleErrorInvalidRepeatingOption;
    QTest::newRow("invalid: calendarItem - invalid monthdays  (negative)") << createTimeDescriptorCalendar(createCalendarItem("13:13", 5, repeatingOptionInvalidMonthDays)) << RuleEngine::RuleErrorInvalidRepeatingOption;
    QTest::newRow("invalid: calendarItem - invalid monthdays  (to big)") << createTimeDescriptorCalendar(createCalendarItem("13:13", 5, repeatingOptionInvalidMonthDays2)) << RuleEngine::RuleErrorInvalidRepeatingOption;

    QTest::newRow("invalid: timeEventItem empty") << createTimeDescriptorTimeEvent(createTimeEventItem()) << RuleEngine::RuleErrorInvalidTimeEventItem;
    QTest::newRow("invalid: timeEventItem none") << createTimeDescriptorTimeEvent(createTimeEventItem("00:12", repeatingOptionInvalidNone)) << RuleEngine::RuleErrorInvalidRepeatingOption;
    QTest::newRow("invalid: timeEventItem - dateTime + repeatingOption") << createTimeDescriptorTimeEvent(createTimeEventItem(QDateTime::currentDateTime().toTime_t(), repeatingOptionDaily)) << RuleEngine::RuleErrorInvalidTimeEventItem;
    QTest::newRow("invalid: timeEventItem invalid time") << createTimeDescriptorTimeEvent(createTimeEventItem("35:80")) << RuleEngine::RuleErrorInvalidTimeEventItem;
    QTest::newRow("invalid: timeEventItem - monthly - weekDays") << createTimeDescriptorTimeEvent(createTimeEventItem("13:13", repeatingOptionInvalidMonthly)) << RuleEngine::RuleErrorInvalidRepeatingOption;
    QTest::newRow("invalid: timeEventItem - weekly - monthDays") << createTimeDescriptorTimeEvent(createTimeEventItem("15:30", repeatingOptionInvalidWeekly)) << RuleEngine::RuleErrorInvalidRepeatingOption;
    QTest::newRow("invalid: timeEventItem - invalid weekdays  (negative)") << createTimeDescriptorTimeEvent(createTimeEventItem("13:13", repeatingOptionInvalidWeekDays)) << RuleEngine::RuleErrorInvalidRepeatingOption;
    QTest::newRow("invalid: timeEventItem - invalid weekdays  (to big)") << createTimeDescriptorTimeEvent(createTimeEventItem("13:13", repeatingOptionInvalidWeekDays2)) << RuleEngine::RuleErrorInvalidRepeatingOption;
    QTest::newRow("invalid: timeEventItem - invalid monthdays  (negative)") << createTimeDescriptorTimeEvent(createTimeEventItem("13:13", repeatingOptionInvalidMonthDays)) << RuleEngine::RuleErrorInvalidRepeatingOption;
    QTest::newRow("invalid: timeEventItem - invalid monthdays  (to big)") << createTimeDescriptorTimeEvent(createTimeEventItem("13:13", repeatingOptionInvalidMonthDays2)) << RuleEngine::RuleErrorInvalidRepeatingOption;

}

void TestTimeManager::addTimeDescriptor()
{
    QFETCH(QVariantMap, timeDescriptor);
    QFETCH(RuleEngine::RuleError, error);

    // ADD the rule
    QVariantMap ruleMap; QVariantMap action;
    action.insert("actionTypeId", mockActionIdNoParams);
    action.insert("deviceId", m_mockDeviceId);
    action.insert("ruleActionParams", QVariantList());
    ruleMap.insert("name", "TimeBased rule");
    ruleMap.insert("timeDescriptor", timeDescriptor);
    ruleMap.insert("actions", QVariantList() << action);

    QVariant response = injectAndWait("Rules.AddRule", ruleMap);
    verifyRuleError(response, error);
    if (error != RuleEngine::RuleErrorNoError)
        return;

    // Print rule
    RuleId newRuleId = RuleId(response.toMap().value("params").toMap().value("ruleId").toString());

    // REMOVE rule
    QVariantMap removeParams;
    removeParams.insert("ruleId", newRuleId);
    response = injectAndWait("Rules.RemoveRule", removeParams);
    verifyRuleError(response);
}

void TestTimeManager::testCalendarItemHourly()
{
    initTimeManager();

    QVariantMap ruleMap; QVariantMap action; QVariantMap exitAction; QVariantMap repeatingOptionHourly;
    repeatingOptionHourly.insert("mode", "RepeatingModeHourly");
    action.insert("actionTypeId", mockActionIdNoParams);
    action.insert("deviceId", m_mockDeviceId);
    action.insert("ruleActionParams", QVariantList());
    exitAction.insert("actionTypeId", mockActionIdWithParams);
    QVariantList actionParams;
    QVariantMap param1;
    param1.insert("name", "mockActionParam1");
    param1.insert("value", 7);
    actionParams.append(param1);
    QVariantMap param2;
    param2.insert("name", "mockActionParam2");
    param2.insert("value", true);
    actionParams.append(param2);
    exitAction.insert("deviceId", m_mockDeviceId);
    exitAction.insert("ruleActionParams", actionParams);
    ruleMap.insert("name", "Time based hourly calendar rule");
    ruleMap.insert("timeDescriptor", createTimeDescriptorCalendar(createCalendarItem("08:05", 5, repeatingOptionHourly)));
    ruleMap.insert("actions", QVariantList() << action);
    ruleMap.insert("exitActions", QVariantList() << exitAction);

    QVariant response = injectAndWait("Rules.AddRule", ruleMap);
    verifyRuleError(response);
    RuleId ruleId = RuleId(response.toMap().value("params").toMap().value("ruleId").toString());

    QVariantMap params;
    params.insert("ruleId", ruleId);
    response = injectAndWait("Rules.GetRuleDetails", params);

    QDateTime currentDateTime = GuhCore::instance()->timeManager()->currentDateTime();

    // check the next 24 hours
    QDateTime future = QDateTime(currentDateTime.date(), QTime(8, 4));
    for (int i = 0; i < 24; i+=3) {
        // inactive
        GuhCore::instance()->timeManager()->setTime(future);
        verifyRuleNotExecuted();
        // active
        GuhCore::instance()->timeManager()->setTime(QDateTime(currentDateTime.date(), QTime(future.time().hour(), 5)));
        verifyRuleExecuted(mockActionIdNoParams);
        cleanupMockHistory();
        // active unchanged
        GuhCore::instance()->timeManager()->setTime(QDateTime(currentDateTime.date(), QTime(future.time().hour(), 7)));
        verifyRuleNotExecuted();
        // inactive
        GuhCore::instance()->timeManager()->setTime(QDateTime(currentDateTime.date(), QTime(future.time().hour(), 10)));
        verifyRuleExecuted(mockActionIdWithParams);
        cleanupMockHistory();
        // inactive unchanged
        GuhCore::instance()->timeManager()->setTime(QDateTime(currentDateTime.date(), QTime(future.time().hour(), 11)));
        verifyRuleNotExecuted();

        // One hour "Back to the future"
        future = future.addSecs(3*60*60);
    }

    // REMOVE rule
    QVariantMap removeParams;
    removeParams.insert("ruleId", ruleId);
    response = injectAndWait("Rules.RemoveRule", removeParams);
    verifyRuleError(response);
}

void TestTimeManager::testCalendarItemDaily_data()
{
    QVariantMap repeatingOptionDaily;
    repeatingOptionDaily.insert("mode", "RepeatingModeDaily");

    QTest::addColumn<QVariantMap>("timeDescriptor");
    QTest::addColumn<QString>("time");
    QTest::addColumn<int>("duration");

    QTest::newRow("daily") << createTimeDescriptorCalendar(createCalendarItem("06:55", 10, repeatingOptionDaily)) << "06:55" << 10;
    QTest::newRow("daily jump date") << createTimeDescriptorCalendar(createCalendarItem("23:55", 10, repeatingOptionDaily)) << "23:55" << 10;
}

void TestTimeManager::testCalendarItemDaily()
{
    QFETCH(QVariantMap, timeDescriptor);
    QFETCH(QString, time);

    initTimeManager();

    QVariantMap ruleMap; QVariantMap action; QVariantMap exitAction;
    action.insert("actionTypeId", mockActionIdNoParams);
    action.insert("deviceId", m_mockDeviceId);
    action.insert("ruleActionParams", QVariantList());
    exitAction.insert("actionTypeId", mockActionIdWithParams);
    QVariantList actionParams;
    QVariantMap param1;
    param1.insert("name", "mockActionParam1");
    param1.insert("value", 12);
    actionParams.append(param1);
    QVariantMap param2;
    param2.insert("name", "mockActionParam2");
    param2.insert("value", true);
    actionParams.append(param2);
    exitAction.insert("deviceId", m_mockDeviceId);
    exitAction.insert("ruleActionParams", actionParams);
    ruleMap.insert("name", "Time based hourly calendar rule");
    ruleMap.insert("timeDescriptor", timeDescriptor);
    ruleMap.insert("actions", QVariantList() << action);
    ruleMap.insert("exitActions", QVariantList() << exitAction);

    QVariant response = injectAndWait("Rules.AddRule", ruleMap);
    verifyRuleError(response);
    RuleId ruleId = RuleId(response.toMap().value("params").toMap().value("ruleId").toString());

    QVariantMap params;
    params.insert("ruleId", ruleId);
    response = injectAndWait("Rules.GetRuleDetails", params);

    qDebug() << QJsonDocument::fromVariant(response.toMap().value("params").toMap()).toJson();

    QDateTime currentDateTime = GuhCore::instance()->timeManager()->currentDateTime();

    // start with one minute before starttime today
    QDateTime future = QDateTime(currentDateTime.date(), QTime::fromString(time, "hh:mm").addSecs(-60));

    // check the next 7 days
    for (int i = 0; i < 7; i++) {
        // inactive
        GuhCore::instance()->timeManager()->setTime(future);
        verifyRuleNotExecuted();
        // active
        GuhCore::instance()->timeManager()->setTime(future.addSecs(60));
        verifyRuleExecuted(mockActionIdNoParams);
        cleanupMockHistory();
        // active unchanged
        GuhCore::instance()->timeManager()->setTime(future.addSecs(6* 60));
        verifyRuleNotExecuted();
        // inactive
        GuhCore::instance()->timeManager()->setTime(future.addSecs(11 * 60));
        verifyRuleExecuted(mockActionIdWithParams);
        cleanupMockHistory();
        // inactive unchanged
        GuhCore::instance()->timeManager()->setTime(future.addSecs(12 * 60));
        verifyRuleNotExecuted();
        // One day "Back to the future"
        future = future.addDays(1);
    }
    // REMOVE rule
    QVariantMap removeParams;
    removeParams.insert("ruleId", ruleId);
    response = injectAndWait("Rules.RemoveRule", removeParams);
    verifyRuleError(response);
}

void TestTimeManager::testCalendarItemWeekly_data()
{
    QVariantMap repeatingOptionWeekly;
    repeatingOptionWeekly.insert("mode", "RepeatingModeWeekly");
    repeatingOptionWeekly.insert("weekDays", QVariantList() << 2 << 4 << 6);

    QVariantMap repeatingOptionWeeklyOverlapping;
    repeatingOptionWeeklyOverlapping.insert("mode", "RepeatingModeWeekly");
    repeatingOptionWeeklyOverlapping.insert("weekDays", QVariantList() << 2 << 4 << 7);

    QTest::addColumn<QVariantMap>("timeDescriptor");
    QTest::addColumn<QVariantMap>("repeatingOption");
    QTest::addColumn<QString>("time");

    QTest::newRow("weekly") << createTimeDescriptorCalendar(createCalendarItem("06:55", 10, repeatingOptionWeekly)) << repeatingOptionWeekly << "06:55";
    //QTest::newRow("weekly overlapping") << createTimeDescriptorCalendar(createCalendarItem("23:55", 10, repeatingOptionWeeklyOverlapping)) << repeatingOptionWeeklyOverlapping << "23:55";
}

void TestTimeManager::testCalendarItemWeekly()
{
    QFETCH(QVariantMap, timeDescriptor);
    QFETCH(QVariantMap, repeatingOption);
    QFETCH(QString, time);

    initTimeManager();

    QVariantMap ruleMap; QVariantMap action; QVariantMap exitAction;
    action.insert("actionTypeId", mockActionIdNoParams);
    action.insert("deviceId", m_mockDeviceId);
    action.insert("ruleActionParams", QVariantList());
    exitAction.insert("actionTypeId", mockActionIdWithParams);
    QVariantList actionParams;
    QVariantMap param1;
    param1.insert("name", "mockActionParam1");
    param1.insert("value", 12);
    actionParams.append(param1);
    QVariantMap param2;
    param2.insert("name", "mockActionParam2");
    param2.insert("value", true);
    actionParams.append(param2);
    exitAction.insert("deviceId", m_mockDeviceId);
    exitAction.insert("ruleActionParams", actionParams);
    ruleMap.insert("name", "Time based hourly calendar rule");
    ruleMap.insert("timeDescriptor", timeDescriptor);
    ruleMap.insert("actions", QVariantList() << action);
    ruleMap.insert("exitActions", QVariantList() << exitAction);

    QVariant response = injectAndWait("Rules.AddRule", ruleMap);
    verifyRuleError(response);
    RuleId ruleId = RuleId(response.toMap().value("params").toMap().value("ruleId").toString());

    QVariantMap params;
    params.insert("ruleId", ruleId);
    response = injectAndWait("Rules.GetRuleDetails", params);

    qDebug() << QJsonDocument::fromVariant(response.toMap().value("params").toMap()).toJson();

    QDateTime currentDateTime = GuhCore::instance()->timeManager()->currentDateTime();

    // start with one minute before starttime today
    QDateTime future = QDateTime(currentDateTime.date(), QTime::fromString(time, "hh:mm").addSecs(-60));
    QVariantList weekDaysVariant = repeatingOption.value("weekDays").toList();

    QList<int> weekDays;
    foreach (const QVariant &variant, weekDaysVariant) {
        weekDays.append(variant.toInt());
    }

    // check the next 7 days
    for (int i = 0; i < 7; i++) {
        // inactive
        GuhCore::instance()->timeManager()->setTime(future);
        verifyRuleNotExecuted();

        // Check if today is a weekday
        qDebug() << "Day of week:" << future.date().dayOfWeek();
        if (weekDays.contains(future.date().dayOfWeek())) {
            // should trigger today
            // active
            GuhCore::instance()->timeManager()->setTime(future.addSecs(60));
            verifyRuleExecuted(mockActionIdNoParams);
            cleanupMockHistory();
            // active unchanged
            GuhCore::instance()->timeManager()->setTime(future.addSecs(6* 60));
            verifyRuleNotExecuted();
            // inactive
            GuhCore::instance()->timeManager()->setTime(future.addSecs(11 * 60));
            verifyRuleExecuted(mockActionIdWithParams);
            cleanupMockHistory();
            // inactive unchanged
            GuhCore::instance()->timeManager()->setTime(future.addSecs(12 * 60));
            verifyRuleNotExecuted();

            // One day "Back to the future"
            future = future.addDays(1);
        } else {
            // should not trigger today
            // inactive
            GuhCore::instance()->timeManager()->setTime(future);
            verifyRuleNotExecuted();
            // inactive
            GuhCore::instance()->timeManager()->setTime(future.addSecs(6* 60));
            verifyRuleNotExecuted();
            // inactive
            GuhCore::instance()->timeManager()->setTime(future.addSecs(11 * 60));
            verifyRuleNotExecuted();
            // inactive
            GuhCore::instance()->timeManager()->setTime(future.addSecs(12 * 60));
            verifyRuleNotExecuted();

            // One day "Back to the future"
            future = future.addDays(1);
        }
    }

    // REMOVE rule
    QVariantMap removeParams;
    removeParams.insert("ruleId", ruleId);
    response = injectAndWait("Rules.RemoveRule", removeParams);
    verifyRuleError(response);
}

void TestTimeManager::initTimeManager()
{
    GuhCore::instance()->timeManager()->stopTimer();
    qDebug() << GuhCore::instance()->timeManager()->currentTime().toString();
    qDebug() << GuhCore::instance()->timeManager()->currentDate().toString();
    qDebug() << GuhCore::instance()->timeManager()->currentDateTime().toString();
}

void TestTimeManager::verifyRuleExecuted(const ActionTypeId &actionTypeId)
{
    // Verify rule got executed
    QNetworkAccessManager nam;
    QSignalSpy spy(&nam, SIGNAL(finished(QNetworkReply*)));
    QNetworkRequest request(QUrl(QString("http://localhost:%1/actionhistory").arg(QString::number(m_mockDevice1Port))));
    QNetworkReply *reply = nam.get(request);
    spy.wait();
    QCOMPARE(spy.count(), 1);

    QByteArray actionHistory = reply->readAll();
    QVERIFY2(actionTypeId == ActionTypeId(actionHistory), "Action not triggered");
    reply->deleteLater();
}

void TestTimeManager::verifyRuleNotExecuted()
{
    QNetworkAccessManager nam;
    QSignalSpy spy(&nam, SIGNAL(finished(QNetworkReply*)));
    QNetworkRequest request(QUrl(QString("http://localhost:%1/actionhistory").arg(QString::number(m_mockDevice1Port))));
    QNetworkReply *reply = nam.get(request);
    spy.wait();
    QCOMPARE(spy.count(), 1);

    QByteArray actionHistory = reply->readAll();
    QVERIFY2(actionHistory.isEmpty(), "Action is triggered while it should not have been.");
    reply->deleteLater();
}

void TestTimeManager::cleanupMockHistory() {
    QNetworkAccessManager nam;
    QSignalSpy spy(&nam, SIGNAL(finished(QNetworkReply*)));
    QNetworkRequest request(QUrl(QString("http://localhost:%1/clearactionhistory").arg(QString::number(m_mockDevice1Port))));
    QNetworkReply *reply = nam.get(request);
    spy.wait();
    QCOMPARE(spy.count(), 1);
    reply->deleteLater();
}

QVariantMap TestTimeManager::createTimeEventItem(const QString &time, const QVariantMap &repeatingOption) const
{
    QVariantMap calendarItem;
    calendarItem.insert("time", time);
    if (!repeatingOption.isEmpty())
        calendarItem.insert("repeating", repeatingOption);

    return calendarItem;
}

QVariantMap TestTimeManager::createTimeEventItem(const int &dateTime, const QVariantMap &repeatingOption) const
{
    QVariantMap calendarItem;
    calendarItem.insert("datetime", dateTime);
    if (!repeatingOption.isEmpty())
        calendarItem.insert("repeating", repeatingOption);

    return calendarItem;
}

QVariantMap TestTimeManager::createTimeDescriptorTimeEvent(const QVariantMap &timeEventItem) const
{
    QVariantMap timeDescriptorTimeEvent;
    timeDescriptorTimeEvent.insert("timeEventItems", QVariantList() << timeEventItem);
    return timeDescriptorTimeEvent;
}

QVariantMap TestTimeManager::createTimeDescriptorTimeEvent(const QVariantList &timeEventItems) const
{
    QVariantMap timeDescriptorTimeEvent;
    timeDescriptorTimeEvent.insert("timeEventItems", timeEventItems);
    return timeDescriptorTimeEvent;
}

QVariantMap TestTimeManager::createCalendarItem(const QString &time, const uint &duration, const QVariantMap &repeatingOption) const
{
    QVariantMap calendarItem;
    calendarItem.insert("startTime", time);
    calendarItem.insert("duration", duration);
    if (!repeatingOption.isEmpty())
        calendarItem.insert("repeating", repeatingOption);

    return calendarItem;
}

QVariantMap TestTimeManager::createCalendarItem(const int &dateTime, const uint &duration, const QVariantMap &repeatingOption) const
{
    QVariantMap calendarItem;
    calendarItem.insert("datetime", dateTime);
    calendarItem.insert("duration", duration);
    if (!repeatingOption.isEmpty())
        calendarItem.insert("repeating", repeatingOption);

    return calendarItem;
}

QVariantMap TestTimeManager::createTimeDescriptorCalendar(const QVariantMap &calendarItem) const
{
    QVariantMap timeDescriptorCalendar;
    timeDescriptorCalendar.insert("calendarItems", QVariantList() << calendarItem);
    return timeDescriptorCalendar;
}

QVariantMap TestTimeManager::createTimeDescriptorCalendar(const QVariantList &calendarItems) const
{
    QVariantMap timeDescriptorCalendar;
    timeDescriptorCalendar.insert("calendarItems", calendarItems);
    return timeDescriptorCalendar;
}

#include "testtimemanager.moc"
QTEST_MAIN(TestTimeManager)
