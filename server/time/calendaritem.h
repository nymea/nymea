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

#ifndef CALENDARITEM_H
#define CALENDARITEM_H

#include <QTime>

#include "repeatingoption.h"

namespace guhserver {

class CalendarItem
{
public:
    CalendarItem();
    CalendarItem(const QTime &startTime, const QTime &duration, const RepeatingOption &repeatingOption);

    QTime startTime() const;
    QTime duration() const;

    RepeatingOption repeatingOption() const;

    bool isValid() const;

    bool evaluate(const QDateTime &dateTime) const;

private:
    QTime m_startTime;
    QTime m_duration;

    RepeatingOption m_repeatingOption;
};

}

#endif // CALENDARITEM_H
