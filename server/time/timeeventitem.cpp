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

#include "timeeventitem.h"

namespace guhserver {

TimeEventItem::TimeEventItem()
{

}

QDateTime TimeEventItem::dateTime() const
{
    return m_dateTimer;
}

void TimeEventItem::setDateTime(const int &timeStamp)
{
    m_dateTimer = QDateTime::fromTime_t(timeStamp);
}

QTime TimeEventItem::time() const
{
    return m_time;
}

void TimeEventItem::setTime(const QTime &time)
{
    m_time = time;
}

RepeatingOption TimeEventItem::repatingOption() const
{
    return m_repeatingOption;
}

void TimeEventItem::setRepeatingOption(const RepeatingOption &repeatingOption)
{
    m_repeatingOption = repeatingOption;
}

bool TimeEventItem::isValid() const
{
    return !m_dateTimer.isNull() || !m_time.isNull();
}

bool TimeEventItem::evaluate(const QDateTime &dateTime) const
{
    Q_UNUSED(dateTime)

    // TODO: evaluate the calendar item, return true if the current time matches the calendar item, otherwise false

    return false;
}

}
