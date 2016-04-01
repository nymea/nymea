/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2016 Simon Stuerz <simon.stuerz@guh.guru>                *
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

/*!
    \class guhserver::RepeatingOption
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

/*! \enum guhserver::RepeatingOption::RepeatingMode

    This enum type specifies the mode of a \l{RepeatingOption}.

    \value RepeatingModeNone
        There is no special repeating mode. The \l{RuleEngine} will assume a daily repeating.
    \value RepeatingModeHourly
        The time item should be repeated hourly.
    \value RepeatingModeDaily
        The time item should be repeated daily.
    \value RepeatingModeWeekly
        The time item should be repeated weekly. A week starts at Monday. This mode needs a list of \l{weekDays()}.
        The \l{monthDays()} list will be ignored.
    \value RepeatingModeMonthly
        The time item should be repeated monthly. This mode needs a list of \l{monthDays()}.
        The \l{monthDays()} list will be ignored.

*/

#include "repeatingoption.h"
#include "loggingcategories.h"

#include <QDateTime>

namespace guhserver {

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

/*! Returns the list of week days on which this \l{RepeatingOption} should be valid. */
QList<int> RepeatingOption::weekDays() const
{
    return m_weekDays;
}

/*! Returns the list of month days on which this \l{RepeatingOption} should be valid. */
QList<int> RepeatingOption::monthDays() const
{
    return m_monthDays;
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
            qCWarning(dcRuleEngine()) << "Invalid week day value:" << weekDay << ". Value out of range [1,7].";
            return false;
        }
    }

    // Validate monthdays range
    foreach (const uint &monthDay, m_monthDays) {
        if (monthDay <= 0 || monthDay > 31) {
            qCWarning(dcRuleEngine()) << "Invalid month day value:" << monthDay << ". Value out of range [1,31].";
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

}
