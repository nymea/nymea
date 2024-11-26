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

#include "nymeatestbase.h"
#include "nymeacore.h"
#include "servers/mocktcpserver.h"

#include "platform/platform.h"
#include "platform/platformsystemcontroller.h"

#include "../plugins/mock/extern-plugininfo.h"

using namespace nymeaserver;

class TestTimeManager: public NymeaTestBase
{
    Q_OBJECT

private:
    inline void verifyRuleError(const QVariant &response, RuleEngine::RuleError error = RuleEngine::RuleErrorNoError) {
        verifyError(response, "ruleError", enumValueName(error));
    }

private slots:
    void initTestCase();

    void loadSaveTimeDescriptor_data();
    void loadSaveTimeDescriptor();

    void addTimeDescriptor_data();
    void addTimeDescriptor();

    void addTimeDescriptorInvalidTimes_data();
    void addTimeDescriptorInvalidTimes();

    // CalendarItems
    void testCalendarDateTime_data();
    void testCalendarDateTime();

    void testCalendarItemHourly_data();
    void testCalendarItemHourly();

    void testCalendarItemDaily_data();
    void testCalendarItemDaily();

    void testCalendarItemWeekly_data();
    void testCalendarItemWeekly();

    void testCalendarItemMonthly_data();
    void testCalendarItemMonthly();

    void testCalendarYearlyDateTime_data();
    void testCalendarYearlyDateTime();

    void testCalendarItemStates_data();
    void testCalendarItemStates();

    void testCalendarItemEvent_data();
    void testCalendarItemEvent();

    void testCalendarItemStatesEvent_data();
    void testCalendarItemStatesEvent();

    void testCalendarItemCrossesMidnight();

    void testEventBasedWithCalendarItemCrossingMidnight();

    // TimeEventItems
    void testEventItemDateTime_data();
    void testEventItemDateTime();

    void testEventItemHourly_data();
    void testEventItemHourly();

    void testEventItemDaily_data();
    void testEventItemDaily();

    void testEventItemWeekly_data();
    void testEventItemWeekly();

    void testEventItemMonthly_data();
    void testEventItemMonthly();

    void testEventItemYearly_data();
    void testEventItemYearly();

    void testEventItemStates_data();
    void testEventItemStates();

    void testEnableDisableTimeRule();

private:
    void initTimeManager();

    void verifyRuleExecuted(const ActionTypeId &actionTypeId);
    void verifyRuleNotExecuted();

    void cleanupMockHistory();

    void removeAllRules();

    void setIntState(const int &value);
    void setBoolState(const bool &value);
    void triggerMockEvent1();

    QVariantMap createTimeEventItem(const QString &time = QString(), const QVariantMap &repeatingOption = QVariantMap()) const;
    QVariantMap createTimeEventItem(const int &dateTime, const QVariantMap &repeatingOption = QVariantMap()) const;
    QVariantMap createTimeDescriptorTimeEvent(const QVariantMap &timeEventItem) const;
    QVariantMap createTimeDescriptorTimeEvent(const QVariantList &timeEventItems) const;

    QVariantMap createCalendarItem(const QString &time = QString(), const uint &duration = 0, const QVariantMap &repeatingOption = QVariantMap()) const;
    QVariantMap createCalendarItem(const int &dateTime, const uint &duration = 0, const QVariantMap &repeatingOption = QVariantMap()) const;
    QVariantMap createTimeDescriptorCalendar(const QVariantMap &calendarItem) const;
    QVariantMap createTimeDescriptorCalendar(const QVariantList &calendarItems) const;
};

void TestTimeManager::initTestCase()
{
    NymeaTestBase::initTestCase("*.debug=false\n"
                                 "Tests.debug=true\n"
                                 "RuleEngine.debug=true\n"
//                                 "RuleEngineDebug.debug=true\n"
                                 "Mock.debug=true\n"
                                 "JsonRpc.debug=true\n"
                                 "TimeManager.debug=true");
}

void TestTimeManager::loadSaveTimeDescriptor_data()
{
    // Repeating options
    QVariantMap repeatingOptionWeekly;
    repeatingOptionWeekly.insert("mode", "RepeatingModeWeekly");
    repeatingOptionWeekly.insert("weekDays", QVariantList() << 2 << 4 << 5);

    QVariantMap repeatingOptionMonthly;
    repeatingOptionMonthly.insert("mode", "RepeatingModeMonthly");
    repeatingOptionMonthly.insert("monthDays", QVariantList() << 20 << 14 << 5);

    QVariantMap repeatingOptionYearly;
    repeatingOptionYearly.insert("mode", "RepeatingModeYearly");

    QVariantList calendarItems;
    calendarItems.append(createCalendarItem("12:10", 20, repeatingOptionWeekly));
    calendarItems.append(createCalendarItem("23:33", 11, repeatingOptionMonthly));
    calendarItems.append(createCalendarItem(QDateTime::currentDateTime().toSecsSinceEpoch(), 50, repeatingOptionYearly));

    QVariantList timeEventItems;
    timeEventItems.append(createTimeEventItem(QDateTime::currentDateTime().toSecsSinceEpoch(), repeatingOptionYearly));
    timeEventItems.append(createTimeEventItem("13:12", repeatingOptionWeekly));
    timeEventItems.append(createTimeEventItem("18:45", repeatingOptionMonthly));

    QTest::addColumn<QVariantMap>("timeDescriptor");

    QTest::newRow("calendarItems") << createTimeDescriptorCalendar(calendarItems);
//    QTest::newRow("timeEventItems") << createTimeDescriptorTimeEvent(timeEventItems);
}

void TestTimeManager::loadSaveTimeDescriptor()
{
    QFETCH(QVariantMap, timeDescriptor);

    initTimeManager();

    // Action (without params)
    QVariantMap ruleMap; QVariantMap action;
    action.insert("actionTypeId", mockWithoutParamsActionTypeId);
    action.insert("thingId", m_mockThingId);
    action.insert("ruleActionParams", QVariantList());

    // Create the rule map
    ruleMap.insert("name", "Time based weekly calendar rule");
    ruleMap.insert("timeDescriptor", timeDescriptor);
    ruleMap.insert("actions", QVariantList() << action);

    // Add the rule
    qCDebug(dcTests()) << "Adding rule:" << qUtf8Printable(QJsonDocument::fromVariant(ruleMap).toJson());
    QVariant response = injectAndWait("Rules.AddRule", ruleMap);
    verifyRuleError(response);

    RuleId ruleId = RuleId(response.toMap().value("params").toMap().value("ruleId").toString());

    QVariantMap params;
    params.insert("ruleId", ruleId);
    response = injectAndWait("Rules.GetRuleDetails", params);
    verifyRuleError(response);

    QVariantMap timeDescriptorMap = response.toMap().value("params").toMap().value("rule").toMap().value("timeDescriptor").toMap();

    QVERIFY2(timeDescriptorMap == timeDescriptor,
             QString("TimeDescriptor not matching:\nExpected: %1\nGot: %2")
             .arg(QString(QJsonDocument::fromVariant(timeDescriptor).toJson()))
             .arg(QString(QJsonDocument::fromVariant(timeDescriptorMap).toJson()))
             .toUtf8());

    // Restart the server
    restartServer();

    // Get the loaded rule
    response = injectAndWait("Rules.GetRuleDetails", params);
    verifyRuleError(response);

    QVariantMap timeDescriptorMapLoaded = response.toMap().value("params").toMap().value("rule").toMap().value("timeDescriptor").toMap();

    QVERIFY2(timeDescriptorMap == timeDescriptorMapLoaded,
             QString("TimeDescriptor not matching:\nExpected: %1\nGot: %2")
             .arg(QString(QJsonDocument::fromVariant(timeDescriptorMap).toJson()))
             .arg(QString(QJsonDocument::fromVariant(timeDescriptorMapLoaded).toJson()))
             .toUtf8());

    // REMOVE rule
    QVariantMap removeParams;
    removeParams.insert("ruleId", ruleId);
    response = injectAndWait("Rules.RemoveRule", removeParams);
    verifyRuleError(response);
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

    QVariantMap repeatingOptionYearly;
    repeatingOptionYearly.insert("mode", "RepeatingModeYearly");

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
    QTest::newRow("valid: calendarItem dateTime") << createTimeDescriptorCalendar(createCalendarItem(QDateTime::currentDateTime().toSecsSinceEpoch(), 5)) << RuleEngine::RuleErrorNoError;
    QTest::newRow("valid: calendarItem dateTime - yearly") << createTimeDescriptorCalendar(createCalendarItem(QDateTime::currentDateTime().toSecsSinceEpoch(), 5, repeatingOptionYearly)) << RuleEngine::RuleErrorNoError;
    QTest::newRow("valid: calendarItem - daily") << createTimeDescriptorCalendar(createCalendarItem("08:00", 5, repeatingOptionDaily)) << RuleEngine::RuleErrorNoError;
    QTest::newRow("valid: calendarItem - none") << createTimeDescriptorCalendar(createCalendarItem("09:00", 30, repeatingOptionNone)) << RuleEngine::RuleErrorNoError;
    QTest::newRow("valid: calendarItem - hourly") << createTimeDescriptorCalendar(createCalendarItem("09:00", 30, repeatingOptionHourly)) << RuleEngine::RuleErrorNoError;
    QTest::newRow("valid: calendarItems - weekly - multiple days") << createTimeDescriptorCalendar(calendarItems) << RuleEngine::RuleErrorNoError;
    QTest::newRow("valid: calendarItem - monthly - multiple days") << createTimeDescriptorCalendar(createCalendarItem("23:00", 5, repeatingOptionMonthlyMultiple)) << RuleEngine::RuleErrorNoError;

    QTest::newRow("valid: timeEventItem") << createTimeDescriptorTimeEvent(createTimeEventItem("08:00")) << RuleEngine::RuleErrorNoError;
    QTest::newRow("valid: timeEventItem dateTime") << createTimeDescriptorTimeEvent(createTimeEventItem(QDateTime::currentDateTime().toSecsSinceEpoch())) << RuleEngine::RuleErrorNoError;
    QTest::newRow("valid: timeEventItem dateTime - yearly") << createTimeDescriptorTimeEvent(createTimeEventItem(QDateTime::currentDateTime().toSecsSinceEpoch(), repeatingOptionYearly)) << RuleEngine::RuleErrorNoError;
    QTest::newRow("valid: timeEventItem - daily") << createTimeDescriptorTimeEvent(createTimeEventItem("08:00", repeatingOptionDaily)) << RuleEngine::RuleErrorNoError;
    QTest::newRow("valid: timeEventItem - none") << createTimeDescriptorTimeEvent(createTimeEventItem("09:00", repeatingOptionNone)) << RuleEngine::RuleErrorNoError;
    QTest::newRow("valid: timeEventItem - hourly") << createTimeDescriptorTimeEvent(createTimeEventItem("09:00", repeatingOptionHourly)) << RuleEngine::RuleErrorNoError;
    QTest::newRow("valid: timeEventItem - weekly - multiple days") << createTimeDescriptorTimeEvent(timeEventItems) << RuleEngine::RuleErrorNoError;
    QTest::newRow("valid: timeEventItem - monthly - multiple days") << createTimeDescriptorTimeEvent(createTimeEventItem("23:00", repeatingOptionMonthlyMultiple)) << RuleEngine::RuleErrorNoError;

    QTest::newRow("invalid: calendarItem none") << createTimeDescriptorCalendar(createCalendarItem("00:12", 12, repeatingOptionInvalidNone)) << RuleEngine::RuleErrorInvalidRepeatingOption;
    QTest::newRow("invalid: calendarItem dateTime - daily") << createTimeDescriptorCalendar(createCalendarItem(QDateTime::currentDateTime().toSecsSinceEpoch(), 5, repeatingOptionDaily)) << RuleEngine::RuleErrorInvalidCalendarItem;
    QTest::newRow("invalid: calendarItem invalid duration") << createTimeDescriptorCalendar(createCalendarItem("12:00", 0)) << RuleEngine::RuleErrorInvalidCalendarItem;
    QTest::newRow("invalid: calendarItem - monthly - weekDays") << createTimeDescriptorCalendar(createCalendarItem("13:13", 5, repeatingOptionInvalidMonthly)) << RuleEngine::RuleErrorInvalidRepeatingOption;
    QTest::newRow("invalid: calendarItem - weekly - monthDays") << createTimeDescriptorCalendar(createCalendarItem("15:30", 20, repeatingOptionInvalidWeekly)) << RuleEngine::RuleErrorInvalidRepeatingOption;
    QTest::newRow("invalid: calendarItem - invalid weekdays  (negative)") << createTimeDescriptorCalendar(createCalendarItem("13:13", 5, repeatingOptionInvalidWeekDays)) << RuleEngine::RuleErrorInvalidRepeatingOption;
    QTest::newRow("invalid: calendarItem - invalid weekdays  (to big)") << createTimeDescriptorCalendar(createCalendarItem("13:13", 5, repeatingOptionInvalidWeekDays2)) << RuleEngine::RuleErrorInvalidRepeatingOption;
    QTest::newRow("invalid: calendarItem - invalid monthdays  (negative)") << createTimeDescriptorCalendar(createCalendarItem("13:13", 5, repeatingOptionInvalidMonthDays)) << RuleEngine::RuleErrorInvalidRepeatingOption;
    QTest::newRow("invalid: calendarItem - invalid monthdays  (to big)") << createTimeDescriptorCalendar(createCalendarItem("13:13", 5, repeatingOptionInvalidMonthDays2)) << RuleEngine::RuleErrorInvalidRepeatingOption;

    QTest::newRow("invalid: timeEventItem none") << createTimeDescriptorTimeEvent(createTimeEventItem("00:12", repeatingOptionInvalidNone)) << RuleEngine::RuleErrorInvalidRepeatingOption;
    QTest::newRow("invalid: timeEventItem - dateTime + repeatingOption") << createTimeDescriptorTimeEvent(createTimeEventItem(QDateTime::currentDateTime().toSecsSinceEpoch(), repeatingOptionDaily)) << RuleEngine::RuleErrorInvalidTimeEventItem;
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
    action.insert("actionTypeId", mockWithoutParamsActionTypeId);
    action.insert("thingId", m_mockThingId);
    action.insert("ruleActionParams", QVariantList());
    ruleMap.insert("name", "TimeBased rule");
    ruleMap.insert("timeDescriptor", timeDescriptor);
    ruleMap.insert("actions", QVariantList() << action);

    QVariant response = injectAndWait("Rules.AddRule", ruleMap);
    verifyRuleError(response, error);
    if (error != RuleEngine::RuleErrorNoError)
        return;

    RuleId newRuleId = RuleId(response.toMap().value("params").toMap().value("ruleId").toString());

    // REMOVE rule
    QVariantMap removeParams;
    removeParams.insert("ruleId", newRuleId);
    response = injectAndWait("Rules.RemoveRule", removeParams);
    verifyRuleError(response);
}

void TestTimeManager::addTimeDescriptorInvalidTimes_data()
{
    QTest::addColumn<QVariantMap>("timeDescriptor");

    QTest::newRow("invalid: calendarItem empty") << createTimeDescriptorCalendar(createCalendarItem());
    QTest::newRow("invalid: calendarItem invalid time") << createTimeDescriptorCalendar(createCalendarItem("35:80", 5));

    QTest::newRow("invalid: timeEventItem empty") << createTimeDescriptorTimeEvent(createTimeEventItem());
    QTest::newRow("invalid: timeEventItem invalid time") << createTimeDescriptorTimeEvent(createTimeEventItem("35:80"));
}

void TestTimeManager::addTimeDescriptorInvalidTimes()
{
    QFETCH(QVariantMap, timeDescriptor);

    // ADD the rule
    QVariantMap ruleMap; QVariantMap action;
    action.insert("actionTypeId", mockWithoutParamsActionTypeId);
    action.insert("thingId", m_mockThingId);
    action.insert("ruleActionParams", QVariantList());
    ruleMap.insert("name", "TimeBased rule");
    ruleMap.insert("timeDescriptor", timeDescriptor);
    ruleMap.insert("actions", QVariantList() << action);

    QVariant response = injectAndWait("Rules.AddRule", ruleMap);
    QVERIFY2(response.toMap().value("status").toString() == "error", "Invalid time must fail JSON verification.");
}

void TestTimeManager::testCalendarDateTime_data()
{
    QTest::addColumn<QDateTime>("dateTime");
    QTest::addColumn<int>("duration");

    QTest::newRow("dateTime - christmas") << QDateTime::fromString("24.12.2017 20:00", "dd.MM.yyyy hh:mm") << 60;
    QTest::newRow("dateTime - new year") << QDateTime::fromString("31.12.2017 23:00", "dd.MM.yyyy hh:mm") << 120;
    QTest::newRow("dateTime - valentines day") << QDateTime::fromString("14.02.2017 08:00", "dd.MM.yyyy hh:mm") << 120;
}

void TestTimeManager::testCalendarDateTime()
{
    QFETCH(QDateTime, dateTime);
    QFETCH(int, duration);

    initTimeManager();

    // Action (without params)
    QVariantMap ruleMap; QVariantMap action; QVariantMap exitAction;
    action.insert("actionTypeId", mockWithoutParamsActionTypeId);
    action.insert("thingId", m_mockThingId);
    action.insert("ruleActionParams", QVariantList());

    // Exit action (with params)
    QVariantList actionParams;
    QVariantMap param1;
    param1.insert("paramTypeId", mockWithParamsActionParam1ParamTypeId);
    param1.insert("value", 12);
    actionParams.append(param1);
    QVariantMap param2;
    param2.insert("paramTypeId", mockWithParamsActionParam2ParamTypeId);
    param2.insert("value", true);
    actionParams.append(param2);
    exitAction.insert("actionTypeId", mockWithParamsActionTypeId);
    exitAction.insert("thingId", m_mockThingId);
    exitAction.insert("ruleActionParams", actionParams);

    // CalendarItem
    QVariantMap calendarItem;
    calendarItem.insert("datetime", QVariant(dateTime.toSecsSinceEpoch()));
    calendarItem.insert("duration", QVariant(duration));

    // Create the rule map
    ruleMap.insert("name", "Time based hourly calendar rule");
    ruleMap.insert("timeDescriptor", createTimeDescriptorCalendar(calendarItem));
    ruleMap.insert("actions", QVariantList() << action);
    ruleMap.insert("exitActions", QVariantList() << exitAction);

    // Add the rule
    QVariant response = injectAndWait("Rules.AddRule", ruleMap);
    verifyRuleError(response);
    RuleId ruleId = RuleId(response.toMap().value("params").toMap().value("ruleId").toString());

    QDateTime oneMinuteBeforeEvent = dateTime.addSecs(-60);

    NymeaCore::instance()->timeManager()->setTime(oneMinuteBeforeEvent);
    verifyRuleNotExecuted();
    // active
    NymeaCore::instance()->timeManager()->setTime(dateTime);
    verifyRuleExecuted(mockWithoutParamsActionTypeId);
    cleanupMockHistory();
    // active unchanged
    NymeaCore::instance()->timeManager()->setTime(dateTime.addSecs(duration * 30));
    verifyRuleNotExecuted();
    // inactive
    NymeaCore::instance()->timeManager()->setTime(dateTime.addSecs(duration * 60));
    verifyRuleExecuted(mockWithParamsActionTypeId);
    cleanupMockHistory();
    // inactive unchanged
    NymeaCore::instance()->timeManager()->setTime(dateTime.addSecs((duration + 1) * 60));
    verifyRuleNotExecuted();

    // REMOVE rule
    QVariantMap removeParams;
    removeParams.insert("ruleId", ruleId);
    response = injectAndWait("Rules.RemoveRule", removeParams);
    verifyRuleError(response);
}

void TestTimeManager::testCalendarItemHourly_data()
{
    QTest::addColumn<int>("duration");

    //QTest::newRow("hourly - for 60 minutes") << 60;
    QTest::newRow("hourly - for 5 minutes") << 5;
}

void TestTimeManager::testCalendarItemHourly()
{
    QFETCH(int, duration);

    initTimeManager();

    QVariantMap ruleMap; QVariantMap action; QVariantMap exitAction; QVariantMap repeatingOptionHourly;
    repeatingOptionHourly.insert("mode", "RepeatingModeHourly");
    action.insert("actionTypeId", mockWithoutParamsActionTypeId);
    action.insert("thingId", m_mockThingId);
    action.insert("ruleActionParams", QVariantList());
    exitAction.insert("actionTypeId", mockWithParamsActionTypeId);
    QVariantList actionParams;
    QVariantMap param1;
    param1.insert("paramTypeId", mockWithParamsActionParam1ParamTypeId);
    param1.insert("value", 7);
    actionParams.append(param1);
    QVariantMap param2;
    param2.insert("paramTypeId", mockWithParamsActionParam2ParamTypeId);
    param2.insert("value", true);
    actionParams.append(param2);
    exitAction.insert("thingId", m_mockThingId);
    exitAction.insert("ruleActionParams", actionParams);
    ruleMap.insert("name", "Time based hourly calendar rule");
    ruleMap.insert("timeDescriptor", createTimeDescriptorCalendar(createCalendarItem("08:05", duration, repeatingOptionHourly)));
    ruleMap.insert("actions", QVariantList() << action);
    ruleMap.insert("exitActions", QVariantList() << exitAction);

    QVariant response = injectAndWait("Rules.AddRule", ruleMap);
    verifyRuleError(response);
    RuleId ruleId = RuleId(response.toMap().value("params").toMap().value("ruleId").toString());

    QVariantMap params;
    params.insert("ruleId", ruleId);
    response = injectAndWait("Rules.GetRuleDetails", params);

    QDateTime currentDateTime = NymeaCore::instance()->timeManager()->currentDateTime();

    QDateTime future = QDateTime(currentDateTime.date(), QTime(8, 4));

    // Check if should be enabled always
    if (duration == 60) {
        NymeaCore::instance()->timeManager()->setTime(future);
        // Should be active since adding
        verifyRuleExecuted(mockWithoutParamsActionTypeId);
    } else {
        // check the next 24 hours in 8h steps
        for (int i = 0; i < 24; i+=8) {
            // inactive
            NymeaCore::instance()->timeManager()->setTime(future);
            verifyRuleNotExecuted();
            // active
            NymeaCore::instance()->timeManager()->setTime(QDateTime(currentDateTime.date(), QTime(future.time().hour(), 5)));
            verifyRuleExecuted(mockWithoutParamsActionTypeId);
            cleanupMockHistory();
            // active unchanged
            NymeaCore::instance()->timeManager()->setTime(QDateTime(currentDateTime.date(), QTime(future.time().hour(), 7)));
            verifyRuleNotExecuted();
            // inactive
            NymeaCore::instance()->timeManager()->setTime(QDateTime(currentDateTime.date(), QTime(future.time().hour(), 10)));
            verifyRuleExecuted(mockWithParamsActionTypeId);
            cleanupMockHistory();
            // inactive unchanged
            NymeaCore::instance()->timeManager()->setTime(QDateTime(currentDateTime.date(), QTime(future.time().hour(), 11)));
            verifyRuleNotExecuted();

            // 'i' hours "Back to the future"
            future = future.addSecs(i*60*60);
        }
    }

    cleanupMockHistory();

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
    QTest::newRow("daily - jump date") << createTimeDescriptorCalendar(createCalendarItem("23:55", 10, repeatingOptionDaily)) << "23:55" << 10;
}

void TestTimeManager::testCalendarItemDaily()
{
    QFETCH(QVariantMap, timeDescriptor);
    QFETCH(QString, time);

    initTimeManager();

    QVariantMap ruleMap; QVariantMap action; QVariantMap exitAction;
    action.insert("actionTypeId", mockWithoutParamsActionTypeId);
    action.insert("thingId", m_mockThingId);
    action.insert("ruleActionParams", QVariantList());
    exitAction.insert("actionTypeId", mockWithParamsActionTypeId);
    QVariantList actionParams;
    QVariantMap param1;
    param1.insert("paramTypeId", mockWithParamsActionParam1ParamTypeId);
    param1.insert("value", 12);
    actionParams.append(param1);
    QVariantMap param2;
    param2.insert("paramTypeId", mockWithParamsActionParam2ParamTypeId);
    param2.insert("value", true);
    actionParams.append(param2);
    exitAction.insert("thingId", m_mockThingId);
    exitAction.insert("ruleActionParams", actionParams);
    ruleMap.insert("name", "Time based hourly calendar rule");
    ruleMap.insert("timeDescriptor", timeDescriptor);
    ruleMap.insert("actions", QVariantList() << action);
    ruleMap.insert("exitActions", QVariantList() << exitAction);

    QVariant response = injectAndWait("Rules.AddRule", ruleMap);
    verifyRuleError(response);
    RuleId ruleId = RuleId(response.toMap().value("params").toMap().value("ruleId").toString());

    QDateTime currentDateTime = NymeaCore::instance()->timeManager()->currentDateTime();

    // start with one minute before starttime today
    QDateTime future = QDateTime(currentDateTime.date(), QTime::fromString(time, "hh:mm").addSecs(-60));

    // if always true
    if (time == "08:00") {
        NymeaCore::instance()->timeManager()->setTime(future);
        // Should be active since adding
        verifyRuleExecuted(mockWithoutParamsActionTypeId);
    } else {
        // check the next 7 days
        for (int i = 0; i < 7; i++) {
            // inactive
            NymeaCore::instance()->timeManager()->setTime(future);
            verifyRuleNotExecuted();
            // active
            NymeaCore::instance()->timeManager()->setTime(future.addSecs(60));
            verifyRuleExecuted(mockWithoutParamsActionTypeId);
            cleanupMockHistory();
            // active unchanged
            NymeaCore::instance()->timeManager()->setTime(future.addSecs(6* 60));
            verifyRuleNotExecuted();
            // inactive
            NymeaCore::instance()->timeManager()->setTime(future.addSecs(11 * 60));
            verifyRuleExecuted(mockWithParamsActionTypeId);
            cleanupMockHistory();
            // inactive unchanged
            NymeaCore::instance()->timeManager()->setTime(future.addSecs(12 * 60));
            verifyRuleNotExecuted();
            // One day "Back to the future"
            future = future.addDays(1);
        }
    }

    cleanupMockHistory();

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
    repeatingOptionWeeklyOverlapping.insert("weekDays", QVariantList() << 6);

    QTest::addColumn<QVariantMap>("timeDescriptor");
    QTest::addColumn<QVariantMap>("repeatingOption");
    QTest::addColumn<QString>("time");
    QTest::addColumn<bool>("overlapping");

    QTest::newRow("weekly") << createTimeDescriptorCalendar(createCalendarItem("06:55", 10, repeatingOptionWeekly)) << repeatingOptionWeekly << "06:55" << false;
    //QTest::newRow("weekly - always") << createTimeDescriptorCalendar(createCalendarItem("22:34", 10080)) << QVariantMap() << "22:34" << false;
    //QTest::newRow("weekly - overlapping") << createTimeDescriptorCalendar(createCalendarItem("08:00", 2880, repeatingOptionWeeklyOverlapping)) << repeatingOptionWeeklyOverlapping << "08:00" << true;
}

void TestTimeManager::testCalendarItemWeekly()
{
    QFETCH(QVariantMap, timeDescriptor);
    QFETCH(QVariantMap, repeatingOption);
    QFETCH(QString, time);
    QFETCH(bool, overlapping);

    initTimeManager();

    // Action (without params)
    QVariantMap ruleMap; QVariantMap action; QVariantMap exitAction;
    action.insert("actionTypeId", mockWithoutParamsActionTypeId);
    action.insert("thingId", m_mockThingId);
    action.insert("ruleActionParams", QVariantList());

    // Exit action (with params)
    QVariantList actionParams;
    QVariantMap param1;
    param1.insert("paramTypeId", mockWithParamsActionParam1ParamTypeId);
    param1.insert("value", 12);
    actionParams.append(param1);
    QVariantMap param2;
    param2.insert("paramTypeId", mockWithParamsActionParam2ParamTypeId);
    param2.insert("value", true);
    actionParams.append(param2);
    exitAction.insert("actionTypeId", mockWithParamsActionTypeId);
    exitAction.insert("thingId", m_mockThingId);
    exitAction.insert("ruleActionParams", actionParams);

    // Create the rule map
    ruleMap.insert("name", "Time based weekly calendar rule");
    ruleMap.insert("timeDescriptor", timeDescriptor);
    ruleMap.insert("actions", QVariantList() << action);
    ruleMap.insert("exitActions", QVariantList() << exitAction);

    // Add the rule
    QVariant response = injectAndWait("Rules.AddRule", ruleMap);
    verifyRuleError(response);
    RuleId ruleId = RuleId(response.toMap().value("params").toMap().value("ruleId").toString());

    QDateTime currentDateTime = NymeaCore::instance()->timeManager()->currentDateTime();

    // start with one minute before starttime today
    QDateTime future = QDateTime(currentDateTime.date(), QTime::fromString(time, "hh:mm").addSecs(-60));
    QVariantList weekDaysVariant = repeatingOption.value("weekDays").toList();

    QList<int> weekDays;
    foreach (const QVariant &variant, weekDaysVariant) {
        weekDays.append(variant.toInt());
    }

    // the whole week active (always)
    if (repeatingOption.isEmpty()) {
        NymeaCore::instance()->timeManager()->setTime(future);
        // Should be active since adding
        verifyRuleExecuted(mockWithoutParamsActionTypeId);
    } else {
        if (!overlapping) {
            // check the next 7 days (because not overlapping the week)
            for (int i = 0; i < 7; i++) {

                // inactive
                NymeaCore::instance()->timeManager()->setTime(future);
                verifyRuleNotExecuted();

                // Check if today is a weekday
                if (weekDays.contains(future.date().dayOfWeek())) {
                    // should trigger today
                    // active
                    NymeaCore::instance()->timeManager()->setTime(future.addSecs(60));
                    verifyRuleExecuted(mockWithoutParamsActionTypeId);
                    cleanupMockHistory();
                    // active unchanged
                    NymeaCore::instance()->timeManager()->setTime(future.addSecs(6* 60));
                    verifyRuleNotExecuted();
                    // inactive
                    NymeaCore::instance()->timeManager()->setTime(future.addSecs(11 * 60));
                    verifyRuleExecuted(mockWithParamsActionTypeId);
                    cleanupMockHistory();
                    // inactive unchanged
                    NymeaCore::instance()->timeManager()->setTime(future.addSecs(12 * 60));
                    verifyRuleNotExecuted();

                    // One day "Back to the future"
                    future = future.addDays(1);
                } else {
                    // should not trigger today
                    NymeaCore::instance()->timeManager()->setTime(future.addSecs(6* 60));
                    verifyRuleNotExecuted();

                    // One day "Back to the future"
                    future = future.addDays(1);
                }
            }
        } else {
            // Overlapping the week
            int weekDay = weekDays.first();

            // go to the next start day (weeksaturday)
            QDateTime startDate = future;
            while (startDate.date().dayOfWeek() < weekDay) {
                startDate = startDate.addDays(1);
            }

            // inactive
            NymeaCore::instance()->timeManager()->setTime(startDate);
            verifyRuleNotExecuted();

            // active
            NymeaCore::instance()->timeManager()->setTime(startDate.addSecs(60));
            verifyRuleExecuted(mockWithoutParamsActionTypeId);
            cleanupMockHistory();

            // still active
            NymeaCore::instance()->timeManager()->setTime(startDate.addDays(1));
            verifyRuleNotExecuted();

            // still active
            NymeaCore::instance()->timeManager()->setTime(startDate.addDays(2));
            verifyRuleNotExecuted();

            // inactive
            NymeaCore::instance()->timeManager()->setTime(startDate.addDays(2).addSecs(60));
            verifyRuleExecuted(mockWithParamsActionTypeId);
        }
    }

    cleanupMockHistory();

    // REMOVE rule
    QVariantMap removeParams;
    removeParams.insert("ruleId", ruleId);
    response = injectAndWait("Rules.RemoveRule", removeParams);
    verifyRuleError(response);
}

void TestTimeManager::testCalendarItemMonthly_data()
{
    QVariantMap repeatingOptionMonthly;
    repeatingOptionMonthly.insert("mode", "RepeatingModeMonthly");
    repeatingOptionMonthly.insert("monthDays", QVariantList() << 1 << 10 << 16 << 20 << 25);

    QVariantMap repeatingOptionMonthlyOverlapping;
    repeatingOptionMonthlyOverlapping.insert("mode", "RepeatingModeMonthly");
    repeatingOptionMonthlyOverlapping.insert("monthDays", QVariantList() << 28);

    QTest::addColumn<QVariantMap>("timeDescriptor");
    QTest::addColumn<QVariantMap>("repeatingOption");
    QTest::addColumn<QString>("time");
    QTest::addColumn<bool>("overlapping");

    QTest::newRow("monthly") << createTimeDescriptorCalendar(createCalendarItem("06:55", 10, repeatingOptionMonthly)) << repeatingOptionMonthly << "06:55" << false;
    //QTest::newRow("monthly - overlapping") << createTimeDescriptorCalendar(createCalendarItem("08:00", 4320, repeatingOptionMonthlyOverlapping)) << repeatingOptionMonthlyOverlapping << "08:00" << true;
}

void TestTimeManager::testCalendarItemMonthly()
{
    QFETCH(QVariantMap, timeDescriptor);
    QFETCH(QVariantMap, repeatingOption);
    QFETCH(QString, time);
    QFETCH(bool, overlapping);

    initTimeManager();

    // Action (without params)
    QVariantMap ruleMap; QVariantMap action; QVariantMap exitAction;
    action.insert("actionTypeId", mockWithoutParamsActionTypeId);
    action.insert("thingId", m_mockThingId);
    action.insert("ruleActionParams", QVariantList());

    // Exit action (with params)
    QVariantList actionParams;
    QVariantMap param1;
    param1.insert("paramTypeId", mockWithParamsActionParam1ParamTypeId);
    param1.insert("value", 12);
    actionParams.append(param1);
    QVariantMap param2;
    param2.insert("paramTypeId", mockWithParamsActionParam2ParamTypeId);
    param2.insert("value", true);
    actionParams.append(param2);
    exitAction.insert("actionTypeId", mockWithParamsActionTypeId);
    exitAction.insert("thingId", m_mockThingId);
    exitAction.insert("ruleActionParams", actionParams);

    // Create the rule map
    ruleMap.insert("name", "Time based monthly calendar rule");
    ruleMap.insert("timeDescriptor", timeDescriptor);
    ruleMap.insert("actions", QVariantList() << action);
    ruleMap.insert("exitActions", QVariantList() << exitAction);

    // Add the rule
    QVariant response = injectAndWait("Rules.AddRule", ruleMap);
    verifyRuleError(response);
    RuleId ruleId = RuleId(response.toMap().value("params").toMap().value("ruleId").toString());

    QDateTime currentDateTime = NymeaCore::instance()->timeManager()->currentDateTime();

    QVariantList monthDaysVariant = repeatingOption.value("monthDays").toList();
    QList<int> monthDays;
    foreach (const QVariant &variant, monthDaysVariant) {
        monthDays.append(variant.toInt());
    }

    // start with one minute before starttime today
    QDateTime future = QDateTime(currentDateTime.date(), QTime::fromString(time, "hh:mm").addSecs(-60));

    if (!overlapping) {

        // run one month to the future
        for (int i = 0; i < 31; i++) {
            QDateTime dateTime = future.addDays(i);
            // Check if today is a weekday
            if (monthDays.contains(dateTime.date().day())) {
                // should trigger today
                // not active yet
                NymeaCore::instance()->timeManager()->setTime(dateTime);
                verifyRuleNotExecuted();
                // active
                NymeaCore::instance()->timeManager()->setTime(dateTime.addSecs(60));
                verifyRuleExecuted(mockWithoutParamsActionTypeId);
                cleanupMockHistory();
                // active unchanged
                NymeaCore::instance()->timeManager()->setTime(dateTime.addSecs(6* 60));
                verifyRuleNotExecuted();
                // inactive
                NymeaCore::instance()->timeManager()->setTime(dateTime.addSecs(11 * 60));
                verifyRuleExecuted(mockWithParamsActionTypeId);
                cleanupMockHistory();
                // inactive unchanged
                NymeaCore::instance()->timeManager()->setTime(dateTime.addSecs(12 * 60));
                verifyRuleNotExecuted();
            } else {
                // should not trigger today
                NymeaCore::instance()->timeManager()->setTime(dateTime.addSecs(60));
                verifyRuleNotExecuted();
            }
        }
    } else {
        // Overlapping the month
        int monthDay = monthDays.first();
        QCOMPARE(monthDay, 28);

        // go to the next start day (28.MM.yyyy)
        QDateTime startDate = future;
        while (startDate.date().day() < monthDay) {
            startDate = startDate.addDays(1);
        }

        // inactive
        NymeaCore::instance()->timeManager()->setTime(startDate);
        verifyRuleNotExecuted();

        // active
        NymeaCore::instance()->timeManager()->setTime(startDate.addSecs(60));
        verifyRuleExecuted(mockWithoutParamsActionTypeId);
        cleanupMockHistory();

        // still active
        NymeaCore::instance()->timeManager()->setTime(startDate.addDays(3));
        verifyRuleNotExecuted();

        // inactive
        NymeaCore::instance()->timeManager()->setTime(startDate.addDays(3).addSecs(60));
        verifyRuleExecuted(mockWithParamsActionTypeId);
    }

    cleanupMockHistory();

    // REMOVE rule
    QVariantMap removeParams;
    removeParams.insert("ruleId", ruleId);
    response = injectAndWait("Rules.RemoveRule", removeParams);
    verifyRuleError(response);
}

void TestTimeManager::testCalendarYearlyDateTime_data()
{
    QTest::addColumn<QDateTime>("dateTime");
    QTest::addColumn<int>("duration");

    QTest::newRow("dateTime - yearly - christmas") << QDateTime::fromString(QString("24.12.%1 20:00").arg(QDateTime::currentDateTime().date().year() + 1), "dd.MM.yyyy hh:mm") << 60;
    QTest::newRow("dateTime - yearly - new year") << QDateTime::fromString(QString("31.12.%1 23:00").arg(QDateTime::currentDateTime().date().year() + 1), "dd.MM.yyyy hh:mm") << 120;
    QTest::newRow("dateTime - yearly - valentines day") << QDateTime::fromString("14.02.2017 08:00", "dd.MM.yyyy hh:mm") << 120;
}

void TestTimeManager::testCalendarYearlyDateTime()
{
    QFETCH(QDateTime, dateTime);
    QFETCH(int, duration);

    initTimeManager();

    // Action (without params)
    QVariantMap ruleMap; QVariantMap action; QVariantMap exitAction;
    action.insert("actionTypeId", mockWithoutParamsActionTypeId);
    action.insert("thingId", m_mockThingId);
    action.insert("ruleActionParams", QVariantList());

    // Exit action (with params)
    QVariantList actionParams;
    QVariantMap param1;
    param1.insert("paramTypeId", mockWithParamsActionParam1ParamTypeId);
    param1.insert("value", 12);
    actionParams.append(param1);
    QVariantMap param2;
    param2.insert("paramTypeId", mockWithParamsActionParam2ParamTypeId);
    param2.insert("value", true);
    actionParams.append(param2);
    exitAction.insert("actionTypeId", mockWithParamsActionTypeId);
    exitAction.insert("thingId", m_mockThingId);
    exitAction.insert("ruleActionParams", actionParams);

    // RepeatingOption
    QVariantMap repeatingOption;
    repeatingOption.insert("mode", "RepeatingModeYearly");

    // CalendarItem
    QVariantMap calendarItem;
    calendarItem.insert("datetime", QVariant(dateTime.toSecsSinceEpoch()));
    calendarItem.insert("duration", QVariant(duration));
    calendarItem.insert("repeating", repeatingOption);

    // Create the rule map
    ruleMap.insert("name", "Time based yearly calendar rule");
    ruleMap.insert("timeDescriptor", createTimeDescriptorCalendar(calendarItem));
    ruleMap.insert("actions", QVariantList() << action);
    ruleMap.insert("exitActions", QVariantList() << exitAction);

    // Add the rule
    QVariant response = injectAndWait("Rules.AddRule", ruleMap);
    verifyRuleError(response);
    RuleId ruleId = RuleId(response.toMap().value("params").toMap().value("ruleId").toString());

    QDateTime oneMinuteBeforeEvent = dateTime.addSecs(-60);

    NymeaCore::instance()->timeManager()->setTime(oneMinuteBeforeEvent);
    verifyRuleNotExecuted();
    // active
    NymeaCore::instance()->timeManager()->setTime(dateTime);
    verifyRuleExecuted(mockWithoutParamsActionTypeId);
    cleanupMockHistory();
    cleanupMockHistory();
    // active unchanged
    NymeaCore::instance()->timeManager()->setTime(dateTime.addSecs(duration * 30));
    verifyRuleNotExecuted();
    // inactive
    NymeaCore::instance()->timeManager()->setTime(dateTime.addSecs(duration * 60));
    verifyRuleExecuted(mockWithParamsActionTypeId);
    cleanupMockHistory();
    cleanupMockHistory();
    // inactive unchanged
    NymeaCore::instance()->timeManager()->setTime(dateTime.addSecs((duration + 1) * 60));
    verifyRuleNotExecuted();


    // One year "Back to the future"
    oneMinuteBeforeEvent = oneMinuteBeforeEvent.addYears(1);
    dateTime = dateTime.addYears(1);

    NymeaCore::instance()->timeManager()->setTime(oneMinuteBeforeEvent);
    verifyRuleNotExecuted();
    // active
    NymeaCore::instance()->timeManager()->setTime(dateTime);
    verifyRuleExecuted(mockWithoutParamsActionTypeId);
    cleanupMockHistory();
    cleanupMockHistory();
    // active unchanged
    NymeaCore::instance()->timeManager()->setTime(dateTime.addSecs(duration * 30));
    verifyRuleNotExecuted();
    // inactive
    NymeaCore::instance()->timeManager()->setTime(dateTime.addSecs(duration * 60));
    verifyRuleExecuted(mockWithParamsActionTypeId);
    cleanupMockHistory();

    // inactive unchanged
    NymeaCore::instance()->timeManager()->setTime(dateTime.addSecs((duration + 1) * 60));
    verifyRuleNotExecuted();

    cleanupMockHistory();

    // REMOVE rule
    QVariantMap removeParams;
    removeParams.insert("ruleId", ruleId);
    response = injectAndWait("Rules.RemoveRule", removeParams);
    verifyRuleError(response);
}

void TestTimeManager::testCalendarItemStates_data()
{
    initTimeManager();

    // Repeating option
    QVariantMap repeatingOptionDaily;
    repeatingOptionDaily.insert("mode", "RepeatingModeDaily");

    // Action
    QVariantMap action;
    action.insert("actionTypeId", mockWithoutParamsActionTypeId);
    action.insert("thingId", m_mockThingId);
    action.insert("ruleActionParams", QVariantList());

    // Exit action (with params)
    QVariantMap exitAction;
    QVariantList actionParams;
    QVariantMap param1;
    param1.insert("paramTypeId", mockWithParamsActionParam1ParamTypeId);
    param1.insert("value", 12);
    actionParams.append(param1);
    QVariantMap param2;
    param2.insert("paramTypeId", mockWithParamsActionParam2ParamTypeId);
    param2.insert("value", true);
    actionParams.append(param2);
    exitAction.insert("actionTypeId", mockWithParamsActionTypeId);
    exitAction.insert("thingId", m_mockThingId);
    exitAction.insert("ruleActionParams", actionParams);

    // Stateevaluators
    QVariantMap stateEvaluator;
    QVariantMap stateDescriptorInt;
    stateDescriptorInt.insert("thingId", m_mockThingId);
    stateDescriptorInt.insert("operator", enumValueName(Types::ValueOperatorGreaterOrEqual));
    stateDescriptorInt.insert("stateTypeId", mockIntStateTypeId);
    stateDescriptorInt.insert("value", 65);
    QVariantMap stateDescriptorBool;
    stateDescriptorBool.insert("thingId", m_mockThingId);
    stateDescriptorBool.insert("operator", enumValueName(Types::ValueOperatorEquals));
    stateDescriptorBool.insert("stateTypeId", mockBoolStateTypeId);
    stateDescriptorBool.insert("value", true);
    QVariantMap stateEvaluatorInt;
    stateEvaluatorInt.insert("stateDescriptor", stateDescriptorInt);
    stateEvaluatorInt.insert("operator", enumValueName(Types::StateOperatorAnd));
    QVariantMap stateEvaluatorBool;
    stateEvaluatorBool.insert("stateDescriptor", stateDescriptorBool);
    stateEvaluatorBool.insert("operator", enumValueName(Types::StateOperatorAnd));
    QVariantList childEvaluators;
    childEvaluators.append(stateEvaluatorInt);
    childEvaluators.append(stateEvaluatorBool);
    stateEvaluator.insert("childEvaluators", childEvaluators);
    stateEvaluator.insert("operator", enumValueName(Types::StateOperatorAnd));


    // The rule
    QVariantMap ruleMap;
    ruleMap.insert("name", "Time and state based daily calendar rule");
    ruleMap.insert("stateEvaluator", stateEvaluator);
    ruleMap.insert("actions", QVariantList() << action);
    ruleMap.insert("exitActions", QVariantList() << exitAction);
    ruleMap.insert("timeDescriptor", createTimeDescriptorCalendar(createCalendarItem("08:00", 10, repeatingOptionDaily)));

    NymeaCore::instance()->timeManager()->setTime(QDateTime(QDate::currentDate(), QTime(07,59)));

    QVariant response = injectAndWait("Rules.AddRule", ruleMap);
    verifyRuleError(response);

    QTest::addColumn<QDateTime>("dateTime");
    QTest::addColumn<bool>("boolValue");
    QTest::addColumn<int>("intValue");
    QTest::addColumn<bool>("trigger");
    QTest::addColumn<bool>("active");

    QTest::newRow("07:59 | true - 66") << QDateTime(QDate::currentDate(), QTime(07,59)) << true << 66 << false << false;
    QTest::newRow("08:00 | true - 66") << QDateTime(QDate::currentDate(), QTime(8,0)) << true << 66 << true << true;
    QTest::newRow("08:01 | true - 66") << QDateTime(QDate::currentDate(), QTime(8,1)) << true << 66 << false << false;
    QTest::newRow("08:02 | false - 66") << QDateTime(QDate::currentDate(), QTime(8,2)) << false << 66 << true << false;
    QTest::newRow("08:03 | true - 65") << QDateTime(QDate::currentDate(), QTime(8,3)) << true << 65 << true << true;
    QTest::newRow("08:06 | true - 64") << QDateTime(QDate::currentDate(), QTime(8,6)) << true << 64 << true << false;
    QTest::newRow("08:07 | false - 64") << QDateTime(QDate::currentDate(), QTime(8,7)) << false << 64 << false << false;
    QTest::newRow("08:07 | false - 65") << QDateTime(QDate::currentDate(), QTime(8,7)) << false << 65 << false << false;
    QTest::newRow("08:08 | true - 65") << QDateTime(QDate::currentDate(), QTime(8,8)) << true << 65 << true << true;
    QTest::newRow("08:09 | true - 65") << QDateTime(QDate::currentDate(), QTime(8,9)) << true << 65 << false << false;
    QTest::newRow("08:10 | true - 65") << QDateTime(QDate::currentDate(), QTime(8,10)) << true << 65 << true << false;
    QTest::newRow("08:11 | true - 65") << QDateTime(QDate::currentDate(), QTime(8,11)) << true << 65 << false << false;

}

void TestTimeManager::testCalendarItemStates()
{
    QFETCH(QDateTime, dateTime);
    QFETCH(bool, boolValue);
    QFETCH(int, intValue);
    QFETCH(bool, trigger);
    QFETCH(bool, active);

    NymeaCore::instance()->timeManager()->setTime(dateTime);
    setBoolState(boolValue);
    setIntState(intValue);

    // Actions
    if (trigger && active) {
        verifyRuleExecuted(mockWithoutParamsActionTypeId);
        cleanupMockHistory();
    }

    // Exit actions
    if (trigger && !active) {
        verifyRuleExecuted(mockWithParamsActionTypeId);
        cleanupMockHistory();
    }

    // Nothing triggert
    if (!trigger) {
        verifyRuleNotExecuted();
    }

}

void TestTimeManager::testCalendarItemEvent_data()
{
    initTimeManager();

    // Action (without params)
    QVariantMap action;
    action.insert("actionTypeId", mockWithoutParamsActionTypeId);
    action.insert("thingId", m_mockThingId);
    action.insert("ruleActionParams", QVariantList());

    QVariantMap eventDescriptor;
    eventDescriptor.insert("eventTypeId", mockEvent1EventTypeId);
    eventDescriptor.insert("thingId", m_mockThingId);

    // The rule
    QVariantMap ruleMap;
    ruleMap.insert("name", "Time and state based daily calendar rule");
    ruleMap.insert("actions", QVariantList() << action);
    ruleMap.insert("eventDescriptors", QVariantList() << eventDescriptor);
    ruleMap.insert("timeDescriptor", createTimeDescriptorCalendar(createCalendarItem("08:00", 10)));

    NymeaCore::instance()->timeManager()->setTime(QDateTime(QDate::currentDate(), QTime(7,59)));

    QVariant response = injectAndWait("Rules.AddRule", ruleMap);
    verifyRuleError(response);
    RuleId ruleId = RuleId(response.toMap().value("params").toMap().value("ruleId").toString());

    QVariantMap params;
    params.insert("ruleId", ruleId);
    response = injectAndWait("Rules.GetRuleDetails", params);
    printJson(response);


    QTest::addColumn<QDateTime>("dateTime");
    QTest::addColumn<bool>("trigger");

    QTest::newRow("07:59 | not trigger") << QDateTime(QDate::currentDate(), QTime(07,59)) << false;
    QTest::newRow("08:00 | trigger") << QDateTime(QDate::currentDate(), QTime(8,0)) << true;
    QTest::newRow("08:05 | trigger") << QDateTime(QDate::currentDate(), QTime(8,5)) << true;
    QTest::newRow("08:10 | not trigger") << QDateTime(QDate::currentDate(), QTime(8,10)) << false;

}

void TestTimeManager::testCalendarItemEvent()
{
    QFETCH(QDateTime, dateTime);
    QFETCH(bool, trigger);

    cleanupMockHistory();

    NymeaCore::instance()->timeManager()->setTime(dateTime);

    // Trigger event
    triggerMockEvent1();

    if (trigger) {
        verifyRuleExecuted(mockWithoutParamsActionTypeId);
    } else {
        verifyRuleNotExecuted();
    }
}

void TestTimeManager::testCalendarItemStatesEvent_data()
{
    initTimeManager();

    NymeaCore::instance()->timeManager()->setTime(QDateTime(QDate::currentDate(), QTime(7,59)));

    // Action (without params)
    QVariantMap action;
    action.insert("actionTypeId", mockWithoutParamsActionTypeId);
    action.insert("thingId", m_mockThingId);
    action.insert("ruleActionParams", QVariantList());

    // Event descriptor
    QVariantMap eventDescriptor;
    eventDescriptor.insert("eventTypeId", mockEvent1EventTypeId);
    eventDescriptor.insert("thingId", m_mockThingId);

    // State evaluator
    QVariantMap stateDescriptorBool;
    stateDescriptorBool.insert("thingId", m_mockThingId);
    stateDescriptorBool.insert("operator", enumValueName(Types::ValueOperatorEquals));
    stateDescriptorBool.insert("stateTypeId", mockBoolStateTypeId);
    stateDescriptorBool.insert("value", true);

    QVariantMap stateEvaluator;
    stateEvaluator.insert("stateDescriptor", stateDescriptorBool);

    // The rule
    QVariantMap ruleMap;
    ruleMap.insert("name", "Time and state and event based daily calendar rule");
    ruleMap.insert("actions", QVariantList() << action);
    ruleMap.insert("eventDescriptors", QVariantList() << eventDescriptor);
    ruleMap.insert("stateEvaluator", stateEvaluator);
    ruleMap.insert("timeDescriptor", createTimeDescriptorCalendar(createCalendarItem("08:00", 10)));

    QVariant response = injectAndWait("Rules.AddRule", ruleMap);
    verifyRuleError(response);
    RuleId ruleId = RuleId(response.toMap().value("params").toMap().value("ruleId").toString());

    QVariantMap params;
    params.insert("ruleId", ruleId);
    response = injectAndWait("Rules.GetRuleDetails", params);

    QTest::addColumn<QDateTime>("dateTime");
    QTest::addColumn<bool>("boolValue");
    QTest::addColumn<bool>("trigger");

    QTest::newRow("07:59 | state true | not trigger") << QDateTime(QDate::currentDate(), QTime(07,59)) << true << false;
    QTest::newRow("08:00 | state true | trigger") << QDateTime(QDate::currentDate(), QTime(8,0)) << true << true;
    QTest::newRow("08:01 | state false | not trigger") << QDateTime(QDate::currentDate(), QTime(8,1)) << false << false;
    QTest::newRow("08:02 | state true | trigger") << QDateTime(QDate::currentDate(), QTime(8,2)) << true << true;
    QTest::newRow("08:10 | state true | not trigger") << QDateTime(QDate::currentDate(), QTime(8,10)) << true << false;
}

void TestTimeManager::testCalendarItemStatesEvent()
{
    QFETCH(QDateTime, dateTime);
    QFETCH(bool, boolValue);
    QFETCH(bool, trigger);

    NymeaCore::instance()->timeManager()->setTime(dateTime);
    setBoolState(boolValue);

    // Trigger event
    triggerMockEvent1();

    if (trigger) {
        verifyRuleExecuted(mockWithoutParamsActionTypeId);
        cleanupMockHistory();
    } else {
        verifyRuleNotExecuted();
    }
}

void TestTimeManager::testCalendarItemCrossesMidnight()
{
    initTimeManager();

    NymeaCore::instance()->timeManager()->setTime(QDateTime(QDate::currentDate(), QTime(20,00)));

    // Action
    QVariantMap action;
    action.insert("actionTypeId", mockWithoutParamsActionTypeId);
    action.insert("thingId", m_mockThingId);
    action.insert("ruleActionParams", QVariantList());

    QVariantMap repeatingOptionDaily;
    repeatingOptionDaily.insert("mode", "RepeatingModeDaily");

    // The rule
    QVariantMap ruleMap;
    ruleMap.insert("name", "Time based from 23:00 to 01:00");
    ruleMap.insert("actions", QVariantList() << action);
    ruleMap.insert("timeDescriptor", createTimeDescriptorCalendar(createCalendarItem("23:00", 120, repeatingOptionDaily)));

    QVariant response = injectAndWait("Rules.AddRule", ruleMap);
    verifyRuleError(response);
    RuleId ruleId = RuleId(response.toMap().value("params").toMap().value("ruleId").toString());

    QVariantMap params;
    params.insert("ruleId", ruleId);

    response = injectAndWait("Rules.GetRuleDetails", params);
    QVERIFY2(response.toMap().value("params").toMap().value("rule").toMap().value("active").toBool() == false, "Rule is active while it should not be (20:00)");

    NymeaCore::instance()->timeManager()->setTime(QDateTime(QDate::currentDate(), QTime(22,59)));

    response = injectAndWait("Rules.GetRuleDetails", params);
    QVERIFY2(response.toMap().value("params").toMap().value("rule").toMap().value("active").toBool() == false, "Rule is active while it should not be (22:59)");

    NymeaCore::instance()->timeManager()->setTime(QDateTime(QDate::currentDate(), QTime(23,00)));

    response = injectAndWait("Rules.GetRuleDetails", params);
    QVERIFY2(response.toMap().value("params").toMap().value("rule").toMap().value("active").toBool() == true, "Rule is not active while it should be (23:00)");

    NymeaCore::instance()->timeManager()->setTime(QDateTime(QDate::currentDate(), QTime(23,10)));

    response = injectAndWait("Rules.GetRuleDetails", params);
    QVERIFY2(response.toMap().value("params").toMap().value("rule").toMap().value("active").toBool() == true, "Rule is not active while it should be (23:10)");

    NymeaCore::instance()->timeManager()->setTime(QDateTime(QDate::currentDate(), QTime(00,00)));

    response = injectAndWait("Rules.GetRuleDetails", params);
    QVERIFY2(response.toMap().value("params").toMap().value("rule").toMap().value("active").toBool() == true, "Rule is not active while it should be (00:00)");

    NymeaCore::instance()->timeManager()->setTime(QDateTime(QDate::currentDate(), QTime(00,30)));

    response = injectAndWait("Rules.GetRuleDetails", params);
    QVERIFY2(response.toMap().value("params").toMap().value("rule").toMap().value("active").toBool() == true, "Rule is not active while it should be (00:30)");

    NymeaCore::instance()->timeManager()->setTime(QDateTime(QDate::currentDate(), QTime(01,00)));

    response = injectAndWait("Rules.GetRuleDetails", params);
    QVERIFY2(response.toMap().value("params").toMap().value("rule").toMap().value("active").toBool() == false, "Rule is active while it should not be (01:00)");

    NymeaCore::instance()->timeManager()->setTime(QDateTime(QDate::currentDate(), QTime(02,00)));

    response = injectAndWait("Rules.GetRuleDetails", params);
    QVERIFY2(response.toMap().value("params").toMap().value("rule").toMap().value("active").toBool() == false, "Rule is active while it should not be (02:00)");

}

void TestTimeManager::testEventBasedWithCalendarItemCrossingMidnight()
{
    initTimeManager();

    NymeaCore::instance()->timeManager()->setTime(QDateTime(QDate::currentDate(), QTime(20,00)));

    // Event descriptor
    QVariantMap eventDescriptor;
    eventDescriptor.insert("eventTypeId", mockEvent1EventTypeId);
    eventDescriptor.insert("thingId", m_mockThingId);

    // Action
    QVariantMap action;
    action.insert("actionTypeId", mockWithoutParamsActionTypeId);
    action.insert("thingId", m_mockThingId);
    action.insert("ruleActionParams", QVariantList());

    QVariantMap repeatingOptionDaily;
    repeatingOptionDaily.insert("mode", "RepeatingModeDaily");

    // The rule
    QVariantMap ruleMap;
    ruleMap.insert("name", "Time based from 23:00 to 01:00");
    ruleMap.insert("actions", QVariantList() << action);
    ruleMap.insert("eventDescriptors", QVariantList() << eventDescriptor);
    ruleMap.insert("timeDescriptor", createTimeDescriptorCalendar(createCalendarItem("23:00", 120, repeatingOptionDaily)));

    QVariant response = injectAndWait("Rules.AddRule", ruleMap);
    verifyRuleError(response);
    RuleId ruleId = RuleId(response.toMap().value("params").toMap().value("ruleId").toString());

    QVariantMap params;
    params.insert("ruleId", ruleId);

    response = injectAndWait("Rules.GetRuleDetails", params);
    QVERIFY2(response.toMap().value("params").toMap().value("rule").toMap().value("id").toUuid() == ruleId, "Rule not found in GetRules");

    cleanupMockHistory();
    NymeaCore::instance()->timeManager()->setTime(QDateTime(QDate::currentDate(), QTime(20, 00)));
    triggerMockEvent1();
    verifyRuleNotExecuted();

    cleanupMockHistory();
    NymeaCore::instance()->timeManager()->setTime(QDateTime(QDate::currentDate(), QTime(23, 00)));
    triggerMockEvent1();
    verifyRuleExecuted(mockWithoutParamsActionTypeId);

    cleanupMockHistory();
    NymeaCore::instance()->timeManager()->setTime(QDateTime(QDate::currentDate(), QTime(23, 50)));
    triggerMockEvent1();
    verifyRuleExecuted(mockWithoutParamsActionTypeId);

    cleanupMockHistory();
    NymeaCore::instance()->timeManager()->setTime(QDateTime(QDate::currentDate(), QTime(00, 00)));
    triggerMockEvent1();
    verifyRuleExecuted(mockWithoutParamsActionTypeId);

    cleanupMockHistory();
    NymeaCore::instance()->timeManager()->setTime(QDateTime(QDate::currentDate(), QTime(01, 00)));
    triggerMockEvent1();
    verifyRuleNotExecuted();
}

void TestTimeManager::testEventItemDateTime_data()
{
    QTest::addColumn<QDateTime>("dateTime");

    QTest::newRow("dateTime - christmas") << QDateTime::fromString("24.12.2016 20:00", "dd.MM.yyyy hh:mm");
    QTest::newRow("dateTime - new year") << QDateTime::fromString("31.12.2016 00:00", "dd.MM.yyyy hh:mm");
    QTest::newRow("dateTime - valentines day") << QDateTime::fromString("14.02.2016 08:00", "dd.MM.yyyy hh:mm");
}

void TestTimeManager::testEventItemDateTime()
{
    QFETCH(QDateTime, dateTime);

    initTimeManager();

    // Action (without params)
    QVariantMap ruleMap; QVariantMap action;
    action.insert("actionTypeId", mockWithoutParamsActionTypeId);
    action.insert("thingId", m_mockThingId);
    action.insert("ruleActionParams", QVariantList());

    // Create the rule map
    ruleMap.insert("name", "Time based hourly calendar rule");
    ruleMap.insert("timeDescriptor", createTimeDescriptorTimeEvent(createTimeEventItem(dateTime.toSecsSinceEpoch())));
    ruleMap.insert("actions", QVariantList() << action);

    // Add the rule
    QVariant response = injectAndWait("Rules.AddRule", ruleMap);
    verifyRuleError(response);
    RuleId ruleId = RuleId(response.toMap().value("params").toMap().value("ruleId").toString());

    // not triggering
    NymeaCore::instance()->timeManager()->setTime(dateTime.addSecs(-120));
    NymeaCore::instance()->timeManager()->setTime(dateTime.addSecs(-60));
    verifyRuleNotExecuted();

    // trigger
    NymeaCore::instance()->timeManager()->setTime(dateTime);
    verifyRuleExecuted(mockWithoutParamsActionTypeId);
    cleanupMockHistory();

    // not triggering
    NymeaCore::instance()->timeManager()->setTime(dateTime.addSecs(60));
    verifyRuleNotExecuted();

    cleanupMockHistory();

    // REMOVE rule
    QVariantMap removeParams;
    removeParams.insert("ruleId", ruleId);
    response = injectAndWait("Rules.RemoveRule", removeParams);
    verifyRuleError(response);
}

void TestTimeManager::testEventItemHourly_data()
{
    QTest::addColumn<QTime>("time");

    QTest::newRow("timeEvent - houly 08:10") << QTime(8,10);
    QTest::newRow("timeEvent - houly 12:33") << QTime(12,33);
}

void TestTimeManager::testEventItemHourly()
{
    QFETCH(QTime, time);

    initTimeManager();

    // Repeating option
    QVariantMap repeatingOptionHourly;
    repeatingOptionHourly.insert("mode", "RepeatingModeHourly");

    // Action
    QVariantMap action;
    action.insert("actionTypeId", mockWithoutParamsActionTypeId);
    action.insert("thingId", m_mockThingId);
    action.insert("ruleActionParams", QVariantList());

    QVariantMap ruleMap;
    ruleMap.insert("name", "Time based hourly event rule");
    ruleMap.insert("timeDescriptor", createTimeDescriptorTimeEvent(createTimeEventItem(time.toString("hh:mm"), repeatingOptionHourly)));
    ruleMap.insert("actions", QVariantList() << action);

    QVariant response = injectAndWait("Rules.AddRule", ruleMap);
    verifyRuleError(response);
    RuleId ruleId = RuleId(response.toMap().value("params").toMap().value("ruleId").toString());

    QVariantMap params;
    params.insert("ruleId", ruleId);
    response = injectAndWait("Rules.GetRuleDetails", params);

    QDateTime currentDateTime = NymeaCore::instance()->timeManager()->currentDateTime();
    QDateTime beforeEventDateTime = QDateTime(currentDateTime.date(), time.addSecs(-60));

    // check the next 24 hours in 8h steps
    for (int i = 0; i < 24; i+=8) {
        // Back to the future (8h)
        beforeEventDateTime = beforeEventDateTime.addSecs(i * 60 * 60);

        // not triggering
        NymeaCore::instance()->timeManager()->setTime(beforeEventDateTime);
        verifyRuleNotExecuted();
        // trigger
        NymeaCore::instance()->timeManager()->setTime(beforeEventDateTime.addSecs(60));
        verifyRuleExecuted(mockWithoutParamsActionTypeId);
        cleanupMockHistory();
        // not triggering
        NymeaCore::instance()->timeManager()->setTime(beforeEventDateTime.addSecs(120));
        verifyRuleNotExecuted();
        cleanupMockHistory();
    }

    cleanupMockHistory();

    // REMOVE rule
    QVariantMap removeParams;
    removeParams.insert("ruleId", ruleId);
    response = injectAndWait("Rules.RemoveRule", removeParams);
    verifyRuleError(response);
}

void TestTimeManager::testEventItemDaily_data()
{
    QTest::addColumn<QTime>("time");
    QTest::addColumn<bool>("withRepeatingOption");

    QTest::newRow("timeEvent - daily 10:01") << QTime(10,1) << false;
    QTest::newRow("timeEvent - daily 22:22") << QTime(22,22) << true;
}

void TestTimeManager::testEventItemDaily()
{
    QFETCH(QTime, time);
    QFETCH(bool, withRepeatingOption);

    initTimeManager();

    // Repeating option
    QVariantMap repeatingOptionDaily;
    repeatingOptionDaily.insert("mode", "RepeatingModeDaily");

    // Action
    QVariantMap action;
    action.insert("actionTypeId", mockWithoutParamsActionTypeId);
    action.insert("thingId", m_mockThingId);
    action.insert("ruleActionParams", QVariantList());

    QVariantMap ruleMap;
    ruleMap.insert("name", "Time based daily event rule");
    ruleMap.insert("actions", QVariantList() << action);
    if (withRepeatingOption) {
        ruleMap.insert("timeDescriptor", createTimeDescriptorTimeEvent(createTimeEventItem(time.toString("hh:mm"), repeatingOptionDaily)));
    } else {
        ruleMap.insert("timeDescriptor", createTimeDescriptorTimeEvent(createTimeEventItem(time.toString("hh:mm"))));
    }

    QVariant response = injectAndWait("Rules.AddRule", ruleMap);
    verifyRuleError(response);
    RuleId ruleId = RuleId(response.toMap().value("params").toMap().value("ruleId").toString());

    QVariantMap params;
    params.insert("ruleId", ruleId);
    response = injectAndWait("Rules.GetRuleDetails", params);

    QDateTime currentDateTime = NymeaCore::instance()->timeManager()->currentDateTime();
    QDateTime beforeEventDateTime = QDateTime(currentDateTime.date(), time.addSecs(-60));

    // check the next 2 days
    for (int i = 0; i < 2; i++) {
        // not triggering
        NymeaCore::instance()->timeManager()->setTime(beforeEventDateTime);
        verifyRuleNotExecuted();
        // trigger
        NymeaCore::instance()->timeManager()->setTime(beforeEventDateTime.addSecs(60));
        verifyRuleExecuted(mockWithoutParamsActionTypeId);
        cleanupMockHistory();
        // not triggering
        NymeaCore::instance()->timeManager()->setTime(beforeEventDateTime.addSecs(120));
        verifyRuleNotExecuted();

        // Back to the future (1 day)
        beforeEventDateTime = beforeEventDateTime.addDays(1);
    }

    cleanupMockHistory();

    // REMOVE rule
    QVariantMap removeParams;
    removeParams.insert("ruleId", ruleId);
    response = injectAndWait("Rules.RemoveRule", removeParams);
    verifyRuleError(response);
}

void TestTimeManager::testEventItemWeekly_data()
{
    QTest::addColumn<QTime>("time");
    QTest::addColumn<QVariantList>("weekDays");

    QTest::newRow("timeEvent - houly 08:10") << QTime(8,10) << (QVariantList() << 6 << 7);
    QTest::newRow("timeEvent - houly 12:33") << QTime(12,33) << (QVariantList() << 2 << 4 << 7);
}

void TestTimeManager::testEventItemWeekly()
{
    QFETCH(QTime, time);
    QFETCH(QVariantList, weekDays);

    initTimeManager();

    // Repeating option
    QVariantMap repeatingOptionWeekly;
    repeatingOptionWeekly.insert("mode", "RepeatingModeWeekly");
    repeatingOptionWeekly.insert("weekDays", weekDays);

    // Action
    QVariantMap action;
    action.insert("actionTypeId", mockWithoutParamsActionTypeId);
    action.insert("thingId", m_mockThingId);
    action.insert("ruleActionParams", QVariantList());

    QVariantMap ruleMap;
    ruleMap.insert("name", "Time based daily event rule");
    ruleMap.insert("actions", QVariantList() << action);
    ruleMap.insert("timeDescriptor", createTimeDescriptorTimeEvent(createTimeEventItem(time.toString("hh:mm"), repeatingOptionWeekly)));

    QVariant response = injectAndWait("Rules.AddRule", ruleMap);
    verifyRuleError(response);
    RuleId ruleId = RuleId(response.toMap().value("params").toMap().value("ruleId").toString());

    QVariantMap params;
    params.insert("ruleId", ruleId);
    response = injectAndWait("Rules.GetRuleDetails", params);

    QDateTime currentDateTime = NymeaCore::instance()->timeManager()->currentDateTime();
    QDateTime beforeEventDateTime = QDateTime(currentDateTime.date(), time.addSecs(-60));

    QList<int> allowedDays;
    foreach (const QVariant &weekDayVariant, weekDays) {
        allowedDays.append(weekDayVariant.toInt());
    }

    // check the next 7 days
    for (int i = 0; i < 7; i++) {
        // check if today is one of the weekdays
        if (allowedDays.contains(beforeEventDateTime.date().dayOfWeek())) {
            // not triggering
            NymeaCore::instance()->timeManager()->setTime(beforeEventDateTime);
            verifyRuleNotExecuted();
            // trigger
            NymeaCore::instance()->timeManager()->setTime(beforeEventDateTime.addSecs(60));
            verifyRuleExecuted(mockWithoutParamsActionTypeId);
            cleanupMockHistory();
            // not triggering
            NymeaCore::instance()->timeManager()->setTime(beforeEventDateTime.addSecs(120));
            verifyRuleNotExecuted();

        } else {
            // not triggering on this weekday
            NymeaCore::instance()->timeManager()->setTime(beforeEventDateTime.addSecs(60));
            verifyRuleNotExecuted();
        }

        // Back to the future (1 day)
        beforeEventDateTime = beforeEventDateTime.addDays(1);
    }

    cleanupMockHistory();

    // REMOVE rule
    QVariantMap removeParams;
    removeParams.insert("ruleId", ruleId);
    response = injectAndWait("Rules.RemoveRule", removeParams);
    verifyRuleError(response);
}

void TestTimeManager::testEventItemMonthly_data()
{
    QTest::addColumn<QTime>("time");
    QTest::addColumn<QVariantList>("monthDays");

    QTest::newRow("timeEvent - houly 08:10") << QTime(8,10) << (QVariantList() << 1 << 12 << 17 << 19 << 31);
    QTest::newRow("timeEvent - houly 12:33") << QTime(12,33) << (QVariantList() << 2 << 4 << 7 << 30);
}

void TestTimeManager::testEventItemMonthly()
{
    QFETCH(QTime, time);
    QFETCH(QVariantList, monthDays);

    initTimeManager();

    // Repeating option
    QVariantMap repeatingOptionMonthly;
    repeatingOptionMonthly.insert("mode", "RepeatingModeMonthly");
    repeatingOptionMonthly.insert("monthDays", monthDays);

    // Action
    QVariantMap action;
    action.insert("actionTypeId", mockWithoutParamsActionTypeId);
    action.insert("thingId", m_mockThingId);
    action.insert("ruleActionParams", QVariantList());

    QVariantMap ruleMap;
    ruleMap.insert("name", "Time based daily event rule");
    ruleMap.insert("actions", QVariantList() << action);
    ruleMap.insert("timeDescriptor", createTimeDescriptorTimeEvent(createTimeEventItem(time.toString("hh:mm"), repeatingOptionMonthly)));

    QVariant response = injectAndWait("Rules.AddRule", ruleMap);
    verifyRuleError(response);
    RuleId ruleId = RuleId(response.toMap().value("params").toMap().value("ruleId").toString());

    QVariantMap params;
    params.insert("ruleId", ruleId);
    response = injectAndWait("Rules.GetRuleDetails", params);

    QDateTime currentDateTime = NymeaCore::instance()->timeManager()->currentDateTime();
    QDateTime beforeEventDateTime = QDateTime(currentDateTime.date(), time.addSecs(-60));

    QList<int> allowedDays;
    foreach (const QVariant &monthDayVariant, monthDays) {
        allowedDays.append(monthDayVariant.toInt());
    }

    // check the next 7 days
    for (int i = 0; i < 31; i++) {
        // check if today is one of the month days
        if (allowedDays.contains(beforeEventDateTime.date().day())) {
            // not triggering
            NymeaCore::instance()->timeManager()->setTime(beforeEventDateTime);
            verifyRuleNotExecuted();
            // trigger
            NymeaCore::instance()->timeManager()->setTime(beforeEventDateTime.addSecs(60));
            verifyRuleExecuted(mockWithoutParamsActionTypeId);
            cleanupMockHistory();
            // not triggering
            NymeaCore::instance()->timeManager()->setTime(beforeEventDateTime.addSecs(120));
            verifyRuleNotExecuted();
        } else {
            // not triggering on this weekday
            NymeaCore::instance()->timeManager()->setTime(beforeEventDateTime.addSecs(60));
            verifyRuleNotExecuted();
        }

        // Back to the future (1 day)
        beforeEventDateTime = beforeEventDateTime.addDays(1);
    }

    cleanupMockHistory();

    // REMOVE rule
    QVariantMap removeParams;
    removeParams.insert("ruleId", ruleId);
    response = injectAndWait("Rules.RemoveRule", removeParams);
    verifyRuleError(response);
}

void TestTimeManager::testEventItemYearly_data()
{
    QTest::addColumn<QDateTime>("dateTime");

    QTest::newRow("dateTime - christmas") << QDateTime::fromString("24.12.2016 20:00", "dd.MM.yyyy hh:mm");
    QTest::newRow("dateTime - new year") << QDateTime::fromString("31.12.2016 00:00", "dd.MM.yyyy hh:mm");
    QTest::newRow("dateTime - valentines day") << QDateTime::fromString("14.02.2016 08:00", "dd.MM.yyyy hh:mm");
}

void TestTimeManager::testEventItemYearly()
{
    QFETCH(QDateTime, dateTime);

    initTimeManager();

    // Repeating option
    QVariantMap repeatingOptionYearly;
    repeatingOptionYearly.insert("mode", "RepeatingModeYearly");

    // Action (without params)
    QVariantMap ruleMap; QVariantMap action;
    action.insert("actionTypeId", mockWithoutParamsActionTypeId);
    action.insert("thingId", m_mockThingId);
    action.insert("ruleActionParams", QVariantList());

    // Create the rule map
    ruleMap.insert("name", "Time based hourly calendar rule");
    ruleMap.insert("timeDescriptor", createTimeDescriptorTimeEvent(createTimeEventItem(dateTime.toSecsSinceEpoch(), repeatingOptionYearly)));
    ruleMap.insert("actions", QVariantList() << action);

    // Add the rule
    QVariant response = injectAndWait("Rules.AddRule", ruleMap);
    verifyRuleError(response);
    RuleId ruleId = RuleId(response.toMap().value("params").toMap().value("ruleId").toString());

    // Tick now, one minute before, on time, one minute after

    // not triggering
    NymeaCore::instance()->timeManager()->setTime(dateTime.addSecs(-60));
    verifyRuleNotExecuted();
    // trigger
    NymeaCore::instance()->timeManager()->setTime(dateTime);
    verifyRuleExecuted(mockWithoutParamsActionTypeId);
    cleanupMockHistory();
    // not triggering
    NymeaCore::instance()->timeManager()->setTime(dateTime.addSecs(60));
    verifyRuleNotExecuted();

    // Tick next year, one minute before, on time, one minute after
    QDateTime nextYear = dateTime.addYears(1);

    // not triggering
    NymeaCore::instance()->timeManager()->setTime(nextYear.addSecs(-60));
    verifyRuleNotExecuted();
    // trigger
    NymeaCore::instance()->timeManager()->setTime(nextYear);
    verifyRuleExecuted(mockWithoutParamsActionTypeId);
    cleanupMockHistory();
    // not triggering
    NymeaCore::instance()->timeManager()->setTime(nextYear.addSecs(60));
    verifyRuleNotExecuted();

    cleanupMockHistory();

    // REMOVE rule
    QVariantMap removeParams;
    removeParams.insert("ruleId", ruleId);
    response = injectAndWait("Rules.RemoveRule", removeParams);
    verifyRuleError(response);
}


void TestTimeManager::testEventItemStates_data()
{
    initTimeManager();

    NymeaCore::instance()->timeManager()->setTime(QDateTime(QDate::currentDate(), QTime(7,59)));

    // Action (without params)
    QVariantMap action;
    action.insert("actionTypeId", mockWithoutParamsActionTypeId);
    action.insert("thingId", m_mockThingId);
    action.insert("ruleActionParams", QVariantList());

    // Time descriptor
    QVariantMap timeEventItem1 = createTimeEventItem("08:00");
    QVariantMap timeEventItem2 = createTimeEventItem("09:00");
    QVariantMap timeDescriptor;
    timeDescriptor.insert("timeEventItems", QVariantList() << timeEventItem1 << timeEventItem2);

    // State evaluator
    QVariantMap stateDescriptorBool;
    stateDescriptorBool.insert("thingId", m_mockThingId);
    stateDescriptorBool.insert("operator", enumValueName(Types::ValueOperatorEquals));
    stateDescriptorBool.insert("stateTypeId", mockBoolStateTypeId);
    stateDescriptorBool.insert("value", true);

    QVariantMap stateEvaluator;
    stateEvaluator.insert("stateDescriptor", stateDescriptorBool);

    // The rule
    QVariantMap ruleMap;
    ruleMap.insert("name", "Time and state and event based daily calendar rule");
    ruleMap.insert("actions", QVariantList() << action);
    ruleMap.insert("stateEvaluator", stateEvaluator);
    ruleMap.insert("timeDescriptor", timeDescriptor);

    QVariant response = injectAndWait("Rules.AddRule", ruleMap);
    verifyRuleError(response);
    RuleId ruleId = RuleId(response.toMap().value("params").toMap().value("ruleId").toString());

    QVariantMap params;
    params.insert("ruleId", ruleId);
    response = injectAndWait("Rules.GetRuleDetails", params);

    QTest::addColumn<QDateTime>("dateTime");
    QTest::addColumn<bool>("boolValue");
    QTest::addColumn<bool>("trigger");

    QTest::newRow("TimeEvent 07:59 | state false | not trigger") << QDateTime(QDate::currentDate(), QTime(7,59)) << false << false;
    QTest::newRow("TimeEvent 07:59 | state true | not trigger") << QDateTime(QDate::currentDate(), QTime(7,59)) << true << false;
    QTest::newRow("TimeEvent 08:00 | state false | not trigger") << QDateTime(QDate::currentDate(), QTime(8,0)) << false << false;
    QTest::newRow("TimeEvent 07:59 | state true | not trigger") << QDateTime(QDate::currentDate(), QTime(7,59)) << true << false;
    QTest::newRow("TimeEvent 08:00 | state true | trigger") << QDateTime(QDate::currentDate(), QTime(8,0)) << true << true;
    QTest::newRow("TimeEvent 08:01 | state true | not trigger") << QDateTime(QDate::currentDate(), QTime(8,1)) << true << false;
    QTest::newRow("TimeEvent 08:01 | state false | not trigger") << QDateTime(QDate::currentDate(), QTime(8,1)) << true << false;
    QTest::newRow("TimeEvent 08:30 | state true | not trigger") << QDateTime(QDate::currentDate(), QTime(8,30)) << true << false;
    QTest::newRow("TimeEvent 09:00 | state true | trigger") << QDateTime(QDate::currentDate(), QTime(9,0)) << true << true;
    QTest::newRow("TimeEvent 09:01 | state true | not trigger") << QDateTime(QDate::currentDate(), QTime(9,1)) << true << false;
}

void TestTimeManager::testEventItemStates()
{
    QFETCH(QDateTime, dateTime);
    QFETCH(bool, boolValue);
    QFETCH(bool, trigger);

    // Set state
    setBoolState(boolValue);

    // Set time
    NymeaCore::instance()->timeManager()->setTime(dateTime);

    if (trigger) {
        verifyRuleExecuted(mockWithoutParamsActionTypeId);
        cleanupMockHistory();
    } else {
        verifyRuleNotExecuted();
    }
}


void TestTimeManager::testEnableDisableTimeRule()
{
    initTimeManager();
    QDateTime dateTime(QDate::currentDate(), QTime(10,15));

    // Repeating option
    QVariantMap repeatingOptionDaily;
    repeatingOptionDaily.insert("mode", "RepeatingModeDaily");

    // Action
    QVariantMap action;
    action.insert("actionTypeId", mockWithoutParamsActionTypeId);
    action.insert("thingId", m_mockThingId);
    action.insert("ruleActionParams", QVariantList());

    QVariantMap ruleMap;
    ruleMap.insert("name", "Time based daily event rule");
    ruleMap.insert("actions", QVariantList() << action);
    ruleMap.insert("timeDescriptor", createTimeDescriptorTimeEvent(createTimeEventItem(dateTime.time().toString("hh:mm"), repeatingOptionDaily)));


    QVariant response = injectAndWait("Rules.AddRule", ruleMap);
    verifyRuleError(response);
    RuleId ruleId = RuleId(response.toMap().value("params").toMap().value("ruleId").toString());

    // not triggering
    NymeaCore::instance()->timeManager()->setTime(dateTime.addSecs(-2));
    NymeaCore::instance()->timeManager()->setTime(dateTime.addSecs(-1));
    verifyRuleNotExecuted();
    // trigger
    NymeaCore::instance()->timeManager()->setTime(dateTime);
    verifyRuleExecuted(mockWithoutParamsActionTypeId);
    cleanupMockHistory();
    // not triggering
    NymeaCore::instance()->timeManager()->setTime(dateTime.addSecs(1));
    NymeaCore::instance()->timeManager()->setTime(dateTime.addSecs(2));
    verifyRuleNotExecuted();


    // Now DISABLE the rule
    QVariantMap enableDisableParams;
    enableDisableParams.insert("ruleId", ruleId.toString());
    response = injectAndWait("Rules.DisableRule", enableDisableParams);
    verifyRuleError(response);

    // trigger
    NymeaCore::instance()->timeManager()->setTime(dateTime.addSecs(-1));
    NymeaCore::instance()->timeManager()->setTime(dateTime);
    verifyRuleNotExecuted();

    // Now ENABLE the rule again
    response.clear();
    response = injectAndWait("Rules.EnableRule", enableDisableParams);
    verifyRuleError(response);

    // trigger
    NymeaCore::instance()->timeManager()->setTime(dateTime.addSecs(-1));
    NymeaCore::instance()->timeManager()->setTime(dateTime);
    verifyRuleExecuted(mockWithoutParamsActionTypeId);
    cleanupMockHistory();

    // REMOVE rule
    QVariantMap removeParams;
    removeParams.insert("ruleId", ruleId);
    response = injectAndWait("Rules.RemoveRule", removeParams);
    verifyRuleError(response);
}

void TestTimeManager::initTimeManager()
{
    cleanupMockHistory();
    removeAllRules();
    enableNotifications({"Rules", "Integrations"});
    NymeaCore::instance()->timeManager()->stopTimer();
    qDebug() << NymeaCore::instance()->timeManager()->currentDateTime().toString();
}

void TestTimeManager::verifyRuleExecuted(const ActionTypeId &actionTypeId)
{
    // Verify rule got executed
    QNetworkAccessManager nam;
    QSignalSpy spy(&nam, SIGNAL(finished(QNetworkReply*)));
    QNetworkRequest request(QUrl(QString("http://localhost:%1/actionhistory").arg(QString::number(m_mockThing1Port))));
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
    QNetworkRequest request(QUrl(QString("http://localhost:%1/actionhistory").arg(QString::number(m_mockThing1Port))));
    QNetworkReply *reply = nam.get(request);
    spy.wait();
    QCOMPARE(spy.count(), 1);

    QByteArray actionHistory = reply->readAll();
    QVERIFY2(actionHistory.isEmpty(), "Actfdsfadsion is triggered while it should not have been.");
    reply->deleteLater();
}

void TestTimeManager::cleanupMockHistory() {
    QNetworkAccessManager nam;
    QSignalSpy spy(&nam, SIGNAL(finished(QNetworkReply*)));
    QNetworkRequest request(QUrl(QString("http://localhost:%1/clearactionhistory").arg(QString::number(m_mockThing1Port))));
    QNetworkReply *reply = nam.get(request);
    spy.wait();
    QCOMPARE(spy.count(), 1);
    reply->deleteLater();
}

void TestTimeManager::removeAllRules()
{
    qDebug() << "Remove all rules";
    QVariant response = injectAndWait("Rules.GetRules");
    QVariantList ruleDescriptions = response.toMap().value("params").toMap().value("ruleDescriptions").toList();
    QVariantMap removeParams;
    foreach (const QVariant &ruleDescription, ruleDescriptions) {
        removeParams.insert("ruleId", ruleDescription.toMap().value("id"));
        response = injectAndWait("Rules.RemoveRule", removeParams);
        verifyRuleError(response);
    }
}

void TestTimeManager::setIntState(const int &value)
{
    qDebug() << "Setting mock int state to" << value;

    QVariantMap params;
    params.insert("thingId", m_mockThingId);
    params.insert("stateTypeId", mockIntStateTypeId);
    QVariant response = injectAndWait("Integrations.GetStateValue", params);
    verifyError(response, "thingError", "ThingErrorNoError");

    int currentStateValue = response.toMap().value("params").toMap().value("value").toInt();
    bool shouldGetNotification = currentStateValue != value;

    QNetworkAccessManager nam;
    QSignalSpy spy(&nam, SIGNAL(finished(QNetworkReply*)));
    QSignalSpy stateSpy(m_mockTcpServer, SIGNAL(outgoingData(QUuid,QByteArray)));

    spy.clear();
    QNetworkRequest request(QUrl(QString("http://localhost:%1/setstate?%2=%3").arg(m_mockThing1Port).arg(mockIntStateTypeId.toString()).arg(value)));
    QNetworkReply *reply = nam.get(request);
    spy.wait();
    QCOMPARE(spy.count(), 1);
    reply->deleteLater();

    if (shouldGetNotification) {
        stateSpy.wait(100);
        // Wait for state changed notification
        QVariantList stateChangedVariants = checkNotifications(stateSpy, "Integrations.StateChanged");
        QVERIFY2(stateChangedVariants.count() == 1, "Did not get Integrations.StateChanged notification.");

        QVariantMap notification = stateChangedVariants.first().toMap().value("params").toMap();
        QVERIFY2(notification.contains("thingId"), "Integrations.StateChanged notification does not contain thingId");
        QVERIFY2(ThingId(notification.value("thingId").toString()) == m_mockThingId, "Integrations.StateChanged notification does not contain the correct thingId");
        QVERIFY2(notification.contains("stateTypeId"), "Integrations.StateChanged notification does not contain stateTypeId");
        QVERIFY2(StateTypeId(notification.value("stateTypeId").toString()) == mockIntStateTypeId, "Integrations.StateChanged notification does not contain the correct stateTypeId");
        QVERIFY2(notification.contains("value"), "Integrations.StateChanged notification does not contain new state value");
        QVERIFY2(notification.value("value").toInt() == value, "Integrations.StateChanged notification does not contain the new value");
    }
}

void TestTimeManager::setBoolState(const bool &value)
{
    qCDebug(dcTests()) << "Setting mock bool state to" << value;

    // Get the current state value to check if we have to wait for state changed notfication
    QVariantMap params;
    params.insert("thingId", m_mockThingId);
    params.insert("stateTypeId", mockBoolStateTypeId);
    QVariant response = injectAndWait("Integrations.GetStateValue", params);
    verifyError(response, "thingError", "ThingErrorNoError");

    bool currentStateValue = response.toMap().value("params").toMap().value("value").toBool();
    bool shouldGetNotification = currentStateValue != value;

    QNetworkAccessManager nam;
    QSignalSpy spy(&nam, SIGNAL(finished(QNetworkReply*)));
    QSignalSpy stateSpy(m_mockTcpServer, SIGNAL(outgoingData(QUuid,QByteArray)));

    QNetworkRequest request(QUrl(QString("http://localhost:%1/setstate?%2=%3").arg(m_mockThing1Port).arg(mockBoolStateTypeId.toString()).arg(value)));
    QNetworkReply *reply = nam.get(request);
    spy.wait();
    QCOMPARE(spy.count(), 1);
    reply->deleteLater();

    if (shouldGetNotification) {
        stateSpy.wait(100);
        // Wait for state changed notification
        QVariantList stateChangedSignals = checkNotifications(stateSpy, "Integrations.StateChanged");
        QCOMPARE(stateChangedSignals.count(), 1);

        QVariantMap notification = stateChangedSignals.first().toMap().value("params").toMap();
        QVERIFY2(notification.contains("thingId"), "Integrations.StateChanged notification does not contain thingId");
        QVERIFY2(ThingId(notification.value("thingId").toString()) == m_mockThingId, "Integrations.StateChanged notification does not contain the correct thingId");
        QVERIFY2(notification.contains("stateTypeId"), "Integrations.StateChanged notification does not contain stateTypeId");
        QVERIFY2(StateTypeId(notification.value("stateTypeId").toString()) == mockBoolStateTypeId, "Integrations.StateChanged notification does not contain the correct stateTypeId");
        QVERIFY2(notification.contains("value"), "Integrations.StateChanged notification does not contain new state value");
        QVERIFY2(notification.value("value").toBool() == value, "Integrations.StateChanged notification does not contain the new value");
    }
}

void TestTimeManager::triggerMockEvent1()
{
    qDebug() << "Trigger mock event 1";

    QNetworkAccessManager nam;
    QSignalSpy spy(&nam, SIGNAL(finished(QNetworkReply*)));

    QSignalSpy eventSpy(m_mockTcpServer, SIGNAL(outgoingData(QUuid,QByteArray)));

    QNetworkRequest request(QUrl(QString("http://localhost:%1/generateevent?eventtypeid=%2").arg(m_mockThing1Port).arg(mockEvent1EventTypeId.toString())));
    QNetworkReply *reply = nam.get(request);
    spy.wait();
    QCOMPARE(spy.count(), 1);
    reply->deleteLater();

    if (eventSpy.isEmpty()) {
        eventSpy.wait();
    }
    QVariantList eventTriggerVariants = checkNotifications(eventSpy, "Integrations.EventTriggered");
    QVERIFY2(eventTriggerVariants.count() == 1, "Did not get Events.EventTriggered notification.");
    QVERIFY2(eventTriggerVariants.first().toMap().value("params").toMap().contains("event"), "Notification Events.EventTriggered does not contain event.");
    QVariantMap eventMap = eventTriggerVariants.first().toMap().value("params").toMap().value("event").toMap();

    QVERIFY2(eventMap.contains("thingId"), "Integrations.EventTriggered notification does not contain thingId");
    QVERIFY2(ThingId(eventMap.value("thingId").toString()) == m_mockThingId, "Integrations.EventTriggered notification does not contain the correct thingId");
    QVERIFY2(eventMap.contains("eventTypeId"), "Integrations.EventTriggered notification does not contain eventTypeId");
    QVERIFY2(EventTypeId(eventMap.value("eventTypeId").toString()) == mockEvent1EventTypeId, "Events.EventTriggered notification does not contain the correct eventTypeId");
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
