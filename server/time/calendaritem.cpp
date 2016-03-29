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

#include "calendaritem.h"

namespace guhserver {

CalendarItem::CalendarItem()
{

}

CalendarItem::CalendarItem(const QTime &startTime, const QTime &duration, const RepeatingOption &repeatingOption) :
    m_startTime(startTime),
    m_duration(duration),
    m_repeatingOption(repeatingOption)
{

}

QTime CalendarItem::startTime() const
{
    return m_startTime;
}

QTime CalendarItem::duration() const
{
    return m_duration;
}

bool CalendarItem::isValid() const
{
    return !m_startTime.isNull() && !m_duration.isNull();
}

bool CalendarItem::evaluate(const QDateTime &dateTime) const
{
    Q_UNUSED(dateTime)

    // TODO: evaluate the calendar item, return true if the current time matches the calendar item, otherwise false

    return false;
}

}
