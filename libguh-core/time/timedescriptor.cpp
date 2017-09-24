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
    \class guhserver::TimeDescriptor
    \brief Describes the time elements of a time based \l{guhserver::Rule}{Rule}.

    \ingroup rules
    \inmodule core

    A time based rule can be described with a \l{TimeDescriptor}. The \l{TimeDescriptor}
    can have either a list of \l{TimeEventItem}{TimeEventItems} or a list of \l{CalendarItem}{CalendarItems},
    never both.

    \sa Rule, TimeEventItem, CalendarItem
*/

#include "timedescriptor.h"

namespace guhserver {

/*! Constructs an invalid \l{TimeDescriptor}.*/
TimeDescriptor::TimeDescriptor()
{

}

/*! Returns the list of \l{TimeEventItem}{TimeEventItems} of this \l{TimeDescriptor}.*/
QList<TimeEventItem> TimeDescriptor::timeEventItems() const
{
    return m_timeEventItems;
}

/*! Set the list of \l{TimeEventItem}{TimeEventItems} of this \l{TimeDescriptor} to the given \a timeEventItems.*/
void TimeDescriptor::setTimeEventItems(const QList<TimeEventItem> &timeEventItems)
{
    m_timeEventItems = timeEventItems;
}

/*! Returns the list of \l{CalendarItem}{CalendarItems} of this \l{TimeDescriptor}.*/
QList<CalendarItem> TimeDescriptor::calendarItems() const
{
    return m_calendarItems;
}

/*! Set the list of \l{CalendarItem}{CalendarItems} of this \l{TimeDescriptor} to the given \a calendarItems.*/
void TimeDescriptor::setCalendarItems(const QList<CalendarItem> &calendarItems)
{
    m_calendarItems = calendarItems;
}

/*! Returns true if either the calendarItems list is not empty or the timeEventItems list.*/
bool TimeDescriptor::isValid() const
{
    return !m_timeEventItems.isEmpty() != !m_calendarItems.isEmpty();
}

/*! Returns true if the calendarItems list and the timeEventItems list is empty.*/
bool TimeDescriptor::isEmpty() const
{
    return m_calendarItems.isEmpty() && m_timeEventItems.isEmpty();
}

/*! Returns true if this \l{TimeDescriptor} is valid for the given \a dateTime. A \l{TimeDescriptor} is
    valid if the \l{TimeEventItem}{TimeEventItems} or \l{CalendarItem}{CalendarItems} match
    the given \a dateTime.
*/
bool TimeDescriptor::evaluate(const QDateTime &lastEvaluationTime, const QDateTime &dateTime) const
{
    // If there are calendarItems (always OR connected)
    if (!m_calendarItems.isEmpty()) {
        foreach (const CalendarItem &calendarItem, m_calendarItems) {
            if (calendarItem.evaluate(dateTime)) {
                return true;
            }
        }
    }

    // If there are timeEventItems (always OR connected)
    if (!m_timeEventItems.isEmpty()) {
        foreach (const TimeEventItem &timeEventItem, m_timeEventItems) {
            if (timeEventItem.evaluate(lastEvaluationTime, dateTime)) {
                return true;
            }
        }
    }

    return false;
}

}
