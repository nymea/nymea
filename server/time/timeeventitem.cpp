/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2016 Simon Stürz <simon.stuerz@guh.io>                   *
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
    \class guhserver::TimeEventItem
    \brief Describes a time event of a time based \l{guhserver::Rule}{Rule}.

    \ingroup rules
    \inmodule core


    \sa Rule, TimeDescriptor, CalendarItem
*/

#include "timeeventitem.h"

namespace guhserver {

/*! Constructs an invalid \l{TimeEventItem}. */
TimeEventItem::TimeEventItem()
{

}

/*! Returns the dateTime of this \l{TimeEventItem}. */
QDateTime TimeEventItem::dateTime() const
{
    return m_dateTime;
}

/*! Sets the dateTime of this \l{TimeEventItem} to the given \a timeStamp. */
void TimeEventItem::setDateTime(const int &timeStamp)
{
    m_dateTime = QDateTime::fromTime_t(timeStamp);
}

/*! Returns the time of this \l{TimeEventItem}. */
QTime TimeEventItem::time() const
{
    return m_time;
}

/*! Sets the \a time of this \l{TimeEventItem}. */
void TimeEventItem::setTime(const QTime &time)
{
    m_time = time;
}

/*! Returns the \l{RepeatingOption} of this \l{TimeEventItem}. */
RepeatingOption TimeEventItem::repeatingOption() const
{
    return m_repeatingOption;
}

/*! Sets the \a repeatingOption of this \l{TimeEventItem}. */
void TimeEventItem::setRepeatingOption(const RepeatingOption &repeatingOption)
{
    m_repeatingOption = repeatingOption;
}

/*! Returns true if this \l{TimeEventItem} is valid. A \l{TimeEventItem} is valid
    if either the \l{time()} or the \l{dateTime()} is set.
*/
bool TimeEventItem::isValid() const
{
    // If dateTime AND a repeating option definend...
    if (m_dateTime.isValid() && !repeatingOption().isEmtpy())
        // ...only repeating mode yearly is allowed for dateTime
        if (repeatingOption().mode() != RepeatingOption::RepeatingModeYearly)
            return false;

    return (!m_dateTime.isNull() != !m_time.isNull());
}

/*! Returns true, if the given \a dateTime matches this \l{TimeEventItem}. */
bool TimeEventItem::evaluate(const QDateTime &dateTime) const
{
    // Check time matches
    if (m_time.isValid()) {
        switch (m_repeatingOption.mode()) {
        case RepeatingOption::RepeatingModeNone:
            // If there is no repeating option, we assume it is meant daily.
            return m_time == dateTime.time();;
        case RepeatingOption::RepeatingModeHourly:
            return m_time.minute() == dateTime.time().minute();
        case RepeatingOption::RepeatingModeDaily:
            return m_time == dateTime.time();;
        case RepeatingOption::RepeatingModeWeekly:
            return m_repeatingOption.evaluateWeekDay(dateTime) && m_time == dateTime.time();
        case RepeatingOption::RepeatingModeMonthly:
            return m_repeatingOption.evaluateMonthDay(dateTime) && m_time == dateTime.time();
        case RepeatingOption::RepeatingModeYearly:
            return false;
        }
    }

    // Check dateTime and yearly repeating
    if (m_repeatingOption.mode() == RepeatingOption::RepeatingModeYearly)
        return m_dateTime.date().month() == dateTime.date().month() &&
                m_dateTime.date().day() == dateTime.date().day() &&
                m_dateTime.time() == dateTime.time();

    // Check dateTime matches
    return dateTime == m_dateTime;
}

}
