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


/*! Construct a invalid \l{CalendarItem}.*/
CalendarItem::CalendarItem()
{

}

/*! Construct a \l{CalendarItem} with the given \a startTime, \a duration and \a repeatingOption.*/
CalendarItem::CalendarItem(const QTime &startTime, const QTime &duration, const RepeatingOption &repeatingOption) :
    m_startTime(startTime),
    m_duration(duration),
    m_repeatingOption(repeatingOption)
{

}

/*! Returns the start time of this \l{CalendarItem}.*/
QTime CalendarItem::startTime() const
{
    return m_startTime;
}

/*! Returns the duratiorn of this \l{CalendarItem}.*/
QTime CalendarItem::duration() const
{
    return m_duration;
}

/*! Returns the \l{RepeatingOption} of this \l{CalendarItem}.*/
RepeatingOption CalendarItem::repeatingOption() const
{
    return m_repeatingOption;
}

/*! Returns true if this \l{CalendarItem} is valid. A \l{CalendarItem} is valid if the start time and the duration are set.*/
bool CalendarItem::isValid() const
{
    return !m_startTime.isNull() && !m_duration.isNull();
}

/*! Returns true, if the given \a dateTime matches this \l{CalendarItem}.*/
bool CalendarItem::evaluate(const QDateTime &dateTime) const
{
    Q_UNUSED(dateTime)

    // TODO: evaluate the calendar item, return true if the current time matches the calendar item, otherwise false

    return false;
}

}
