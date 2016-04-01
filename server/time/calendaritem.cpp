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
    \class guhserver::CalendarItem
    \brief Describes a clendar item for a time based \l{guhserver::Rule}{Rule}.

    \ingroup rules
    \inmodule core

    \sa Rule, TimeDescriptor
*/

#include "calendaritem.h"

namespace guhserver {

/*! Construct a invalid \l{CalendarItem}. */
CalendarItem::CalendarItem():
    m_duration(0)
{

}

/*! Returns the date time of this \l{CalendarItem}. */
QDateTime CalendarItem::dateTime() const
{
    return m_dateTime;
}

/*! Sets the \a dateTime of this \l{CalendarItem}. */
void CalendarItem::setDateTime(const QDateTime &dateTime)
{
    m_dateTime = dateTime;
}

/*! Returns the start time of this \l{CalendarItem}. */
QTime CalendarItem::startTime() const
{
    return m_startTime;
}

/*! Sets the \a startTime of this \l{CalendarItem}. */
void CalendarItem::setStartTime(const QTime &startTime)
{
    m_startTime = startTime;
}

/*! Returns the duratiorn of this \l{CalendarItem}. */
uint CalendarItem::duration() const
{
    return m_duration;
}

/*! Sets the \a duration of this \l{CalendarItem}. */
void CalendarItem::setDuration(const uint &duration)
{
    m_duration = duration;
}

/*! Returns the \l{RepeatingOption} of this \l{CalendarItem}. */
RepeatingOption CalendarItem::repeatingOption() const
{
    return m_repeatingOption;
}

/*! Sets the \a repeatingOption of this \l{CalendarItem}. */
void CalendarItem::setRepeatingOption(const RepeatingOption &repeatingOption)
{
    m_repeatingOption = repeatingOption;
}

/*! Returns true if this \l{CalendarItem} is valid. A \l{CalendarItem} is invalid
    if start time and datetime are set or if the duration is 0.
*/
bool CalendarItem::isValid() const
{
    // A dateTime AND a repeating option would not make sense
    if (m_dateTime.isValid() && !repeatingOption().isEmtpy())
        return false;

    return (!m_startTime.isValid() != !m_dateTime.isValid()) && m_duration > 0;
}

/*! Returns true, if the given \a dateTime matches this \l{CalendarItem}. */
bool CalendarItem::evaluate(const QDateTime &dateTime) const
{
    if (!isValid())
        return false;

    if (!repeatingOption().isValid())
        return false;

    if (m_dateTime.isValid() && !repeatingOption().isEmtpy())
        return false;

    // Only check repeating option mode if this is not a timedate calendarItem,
    // which can only be valid once and is not repeatable.
    if (!m_dateTime.isValid()) {
        switch (repeatingOption().mode()) {
        case RepeatingOption::RepeatingModeNone:
            // If there is no repeating option, we assume it is meant daily.
            return evaluateDaily(dateTime);
        case RepeatingOption::RepeatingModeHourly:
            return evaluateHourly(dateTime);
        case RepeatingOption::RepeatingModeDaily:
            return evaluateDaily(dateTime);
        case RepeatingOption::RepeatingModeWeekly:
            return evaluateWeekly(dateTime);
        case RepeatingOption::RepeatingModeMonthly:
            return evaluateMonthly(dateTime);
        default:
            return false;
        }
    } else {
        return dateTime >= m_dateTime && dateTime < m_dateTime.addSecs(duration() * 60);
    }
    return false;
}

bool CalendarItem::evaluateHourly(const QDateTime &dateTime) const
{
    QDateTime startDateTime = QDateTime(dateTime.date(), QTime(dateTime.time().hour(), startTime().minute()));
    QDateTime endDateTime = startDateTime.addSecs(duration() * 60);

    bool timeValid = dateTime >= startDateTime && dateTime < endDateTime;
    bool weekdayValid = repeatingOption().evaluateWeekDay(dateTime);
    bool monthdayValid = repeatingOption().evaluateMonthDay(dateTime);

    return timeValid && weekdayValid && monthdayValid;
}

bool CalendarItem::evaluateDaily(const QDateTime &dateTime) const
{
    // If the duration is longer than a day, this calendar item is always true
    // 1 day has 1440 minutes
    if (duration() >= 1440)
        return true;

    QDateTime startDateTime = QDateTime(dateTime.date(), startTime());
    QDateTime endDateTime = startDateTime.addSecs(duration() * 60);

    bool timeValid = false;

    if (startDateTime.date() == endDateTime.date()) {
        timeValid = dateTime >= startDateTime && dateTime < endDateTime;
    } else {
        // If the time duration changes the date,
        // check only if the time is smaler than the overlapping time
        timeValid = dateTime.time() < endDateTime.time();
    }

    return timeValid;
}

bool CalendarItem::evaluateWeekly(const QDateTime &dateTime) const
{
    // If the duration is longer than a week, this calendar item is always true
    // 1 week has 10080 minutes
    if (duration() >= 10080)
        return true;

    // Get the first day of this week with the correct start time
    QDateTime weekStartDateTime = dateTime.addDays(-dateTime.date().dayOfWeek());
    weekStartDateTime.setTime(m_startTime);

    // Check each week day in the list
    foreach (const int &weekDay, repeatingOption().weekDays()) {
        QDateTime startDateTime = weekStartDateTime.addDays(weekDay -1);
        QDateTime endDateTime = startDateTime.addSecs(duration() * 60);

        bool overlapping = false;

        // Check if this calendar item overlaps a week
        if (startDateTime.date().weekNumber() != endDateTime.date().weekNumber())
            overlapping = true;

        if (overlapping) {
            // Jump one week into the past
            QDateTime startPreviouseDateTime = startDateTime.addDays(-7);
            QDateTime endPreviouseDateTime = startPreviouseDateTime.addSecs(duration() * 60);

            if (dateTime >= startPreviouseDateTime && dateTime < endPreviouseDateTime)
                // Return true if the current time is between start
                // and end of this calendar item from the previouse week
                return true;

        } else if (dateTime >= startDateTime && dateTime < endDateTime) {
            // Return true if the current time is between start
            // and end of this calendar item
            return true;
        }
    }

    return false;
}

bool CalendarItem::evaluateMonthly(const QDateTime &dateTime) const
{
    Q_UNUSED(dateTime)

    return false;
}

}

