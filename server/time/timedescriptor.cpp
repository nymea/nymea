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

#include "timedescriptor.h"

namespace guhserver {

TimeDescriptor::TimeDescriptor()
{

}

QList<TimeEventItem> TimeDescriptor::timeEventItems() const
{
    return m_timeEventItems;
}

void TimeDescriptor::setTimeEventItems(const QList<TimeEventItem> &timeEventItems)
{
    m_timeEventItems = timeEventItems;
}

QList<CalendarItem> TimeDescriptor::calendarItems() const
{
    return m_calendarItems;
}

void TimeDescriptor::setCalendarItems(const QList<CalendarItem> &calendarItems)
{
    m_calendarItems = calendarItems;
}

bool TimeDescriptor::isValid() const
{
    return !m_timeEventItems.isEmpty() || !m_calendarItems.isEmpty();
}

}
