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
* GNU Lesser General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU Lesser General Public License as published by the Free
* Software Foundation; version 3. This project is distributed in the hope that
* it will be useful, but WITHOUT ANY WARRANTY; without even the implied
* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*!
    \class nymeaserver::RepeatingOption
    \brief Describes the repeating option of a time item.

    \ingroup rules
    \inmodule core

    The list of \l{weekDays()} can contain following values:
    \table
    \header
        \li Weekday
        \li int
    \row
        \li Monday
        \li 1
    \row
        \li Tuesday
        \li 2
    \row
        \li Wednesday
        \li 3
    \row
        \li Thursday
        \li 4
    \row
        \li Friday
        \li 5
    \row
        \li Saturday
        \li 6
    \row
        \li Sunday
        \li 7
    \endtable


    \sa Rule, TimeDescriptor
*/

/*! \enum nymeaserver::RepeatingOption::RepeatingMode

    This enum type specifies the mode of a \l{RepeatingOption}.

    \value RepeatingModeNone
        There is no special repeating mode. The \l{RuleEngine} will assume a daily repeating.
    \value RepeatingModeHourly
        The time item should be repeated hourly. The \l{monthDays()} and \l{weekDays()} list has to be empty.
    \value RepeatingModeDaily
        The time item should be repeated daily. The \l{monthDays()} and \l{weekDays()} list has to be empty.
    \value RepeatingModeWeekly
        The time item should be repeated weekly. A week starts at Monday. This mode needs a list of \l{weekDays()}.
        The \l{monthDays()} list has to empty.
    \value RepeatingModeMonthly
        The time item should be repeated every month. This mode needs a list of \l{monthDays()}.
        The \l{monthDays()} list has to be empty.
    \value RepeatingModeYearly
        The time item should be repeated every year. The \l{monthDays()} and \l{weekDays()} list has to be empty.

*/

#include "repeatingoption.h"
#include "loggingcategories.h"

#include <QDateTime>

/*! Constructs an empty \l{RepeatingOption}. */
RepeatingOption::RepeatingOption() :
    m_mode(RepeatingModeNone)
{

}

/*! Constructs a \l{RepeatingOption} with the given \a mode, \a weekDays list and \a monthDays list. */
RepeatingOption::RepeatingOption(const RepeatingMode &mode, const QList<int> &weekDays, const QList<int> &monthDays) :
    m_mode(mode),
    m_weekDays(weekDays),
    m_monthDays(monthDays)
{

}

/*! Returns the mode of this \l{RepeatingOption}. */
RepeatingOption::RepeatingMode RepeatingOption::mode() const
{
    return m_mode;
}

void RepeatingOption::setMode(RepeatingOption::RepeatingMode mode)
{
    m_mode = mode;
}

/*! Returns the list of week days on which this \l{RepeatingOption} should be valid. */
QList<int> RepeatingOption::weekDays() const
{
    return m_weekDays;
}

void RepeatingOption::setWeekDays(const QList<int> &weekDays)
{
    m_weekDays = weekDays;
}

/*! Returns the list of month days on which this \l{RepeatingOption} should be valid. */
QList<int> RepeatingOption::monthDays() const
{
    return m_monthDays;
}

void RepeatingOption::setMonthDays(const QList<int> &monthDays)
{
    m_monthDays = monthDays;
}

/*! Returns true if this \l{RepeatingOption} is empty. */
bool RepeatingOption::isEmtpy() const
{
    return m_mode == RepeatingModeNone && m_weekDays.isEmpty() && m_monthDays.isEmpty();
}

/*! Returns true if this \l{RepeatingOption} is valid. */
bool RepeatingOption::isValid() const
{
    // Validate weekdays range
    foreach (const uint &weekDay, m_weekDays) {
        if (weekDay <= 0 || weekDay > 7) {
            return false;
        }
    }

    // Validate monthdays range
    foreach (const uint &monthDay, m_monthDays) {
        if (monthDay <= 0 || monthDay > 31) {
            return false;
        }
    }

    // Validate the lists
    switch (m_mode) {
    case RepeatingModeNone:
        return m_weekDays.isEmpty() && m_monthDays.isEmpty();
    case RepeatingModeHourly:
        return m_weekDays.isEmpty() && m_monthDays.isEmpty();
    case RepeatingModeDaily:
        return m_weekDays.isEmpty() && m_monthDays.isEmpty();
    case RepeatingModeWeekly:
        return !m_weekDays.isEmpty() && m_monthDays.isEmpty();
    case RepeatingModeMonthly:
        return m_weekDays.isEmpty() && !m_monthDays.isEmpty();
    case RepeatingModeYearly:
        return m_weekDays.isEmpty() && m_monthDays.isEmpty();
    default:
        return false;
    }
}

/*! Returns true if the week day of the given \a dateTime matches this \l{RepeatingOption}. */
bool RepeatingOption::evaluateWeekDay(const QDateTime &dateTime) const
{
    // If there is no weekday specified it's always true
    if (m_weekDays.isEmpty())
        return true;

    // Check if dateTime week day matches one of the specified week days
    if (m_weekDays.contains(dateTime.date().dayOfWeek()))
        return true;

    return false;
}

/*! Returns true if the month day of the given \a dateTime matches this \l{RepeatingOption}. */
bool RepeatingOption::evaluateMonthDay(const QDateTime &dateTime) const
{
    // If there is no month days specified it's always true
    if (m_monthDays.isEmpty())
        return true;

    // Check if dateTime month day matches one of the specified month days
    if (m_monthDays.contains(dateTime.date().day()))
        return true;

    return false;
}

/*! Print a RepeatingOption to QDebug. */
QDebug operator<<(QDebug dbg, const RepeatingOption &repeatingOption)
{
    QDebugStateSaver saver(dbg);
    dbg.nospace() << "RepeatingOption(Mode:" << repeatingOption.mode() << ", Monthdays:" << repeatingOption.monthDays() << "Weekdays:" << repeatingOption.weekDays() << ")";
    return dbg;
}
