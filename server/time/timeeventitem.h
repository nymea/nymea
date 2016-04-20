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

#ifndef TIMEEVENTITEM_H
#define TIMEEVENTITEM_H

#include <QDateTime>

#include "repeatingoption.h"

namespace guhserver {

class TimeEventItem
{
public:
    TimeEventItem();

    QDateTime dateTime() const;
    void setDateTime(const int &timeStamp);

    QTime time() const;
    void setTime(const QTime &time);

    RepeatingOption repeatingOption() const;
    void setRepeatingOption(const RepeatingOption &repeatingOption);

    // TODO spectioalDayTime

    bool isValid() const;

    bool evaluate(const QDateTime &dateTime) const;

private:
    QDateTime m_dateTime;
    QTime m_time;

    RepeatingOption m_repeatingOption;
};

}

#endif // TIMEEVENTITEM_H
