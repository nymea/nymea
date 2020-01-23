/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2020, nymea GmbH
* Contact: contact@nymea.io
*
* This file is part of nymea.
* This project including source code and documentation is protected by
* copyright law, and remains the property of nymea GmbH. All rights, including
* reproduction, publication, editing and translation, are reserved. The use of
* this project is subject to the terms of a license agreement to be concluded
* with nymea GmbH in accordance with the terms of use of nymea GmbH, available
* under https://nymea.io/license
*
* GNU Lesser General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU Lesser General Public License as published by the Free
* Software Foundation; version 3. This project is distributed in the hope that
* it will be useful, but WITHOUT ANY WARRANTY; without even the implied
* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

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
    Q_INVOKABLE QVariant get(int index) const;
    Q_INVOKABLE void put(const QVariant &variant);
};
Q_DECLARE_METATYPE(CalendarItems)

QDebug operator<<(QDebug dbg, const CalendarItem &calendarItem);

#endif // CALENDARITEM_H
