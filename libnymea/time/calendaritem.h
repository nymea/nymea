/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2016 Simon Stürz <simon.stuerz@guh.io>                   *
 *                                                                         *
 *  This file is part of nymea.                                            *
 *                                                                         *
 *  nymea is free software: you can redistribute it and/or modify          *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  nymea is distributed in the hope that it will be useful,               *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with nymea. If not, see <http://www.gnu.org/licenses/>.          *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef CALENDARITEM_H
#define CALENDARITEM_H

#include <QTime>

#include "repeatingoption.h"
#include <QVariant>

class CalendarItem
{
    Q_GADGET
    Q_PROPERTY(uint duration READ duration WRITE setDuration)
    Q_PROPERTY(QDateTime datetime READ dateTime WRITE setDateTime USER true)
    Q_PROPERTY(QTime startTime READ startTime WRITE setStartTime USER true)
    Q_PROPERTY(RepeatingOption repeating READ repeatingOption WRITE setRepeatingOption USER true)
public:
    CalendarItem();

    QDateTime dateTime() const;
    void setDateTime(const QDateTime &dateTime);

    QTime startTime() const;
    void setStartTime(const QTime &startTime);

    uint duration() const;
    void setDuration(const uint &duration);

    RepeatingOption repeatingOption() const;
    void setRepeatingOption(const RepeatingOption &repeatingOption);

    bool isValid() const;
    bool evaluate(const QDateTime &dateTime) const;

private:
    QDateTime m_dateTime;
    QTime m_startTime;
    QTime m_endTime;
    uint m_duration;

    RepeatingOption m_repeatingOption;

    bool evaluateHourly(const QDateTime &dateTime) const;
    bool evaluateDaily(const QDateTime &dateTime) const;
    bool evaluateWeekly(const QDateTime &dateTime) const;
    bool evaluateMonthly(const QDateTime &dateTime) const;
    bool evaluateYearly(const QDateTime &dateTime) const;

};

class CalendarItems: public QList<CalendarItem>
{
    Q_GADGET
    Q_PROPERTY(int count READ count)
public:
    CalendarItems();
    CalendarItems(const QList<CalendarItem> &other);
    Q_INVOKABLE QVariant get(int index);
};
Q_DECLARE_METATYPE(CalendarItems)

QDebug operator<<(QDebug dbg, const CalendarItem &calendarItem);

#endif // CALENDARITEM_H
