// SPDX-License-Identifier: LGPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2025, chargebyte austria GmbH
*
* This file is part of nymea.
*
* nymea is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* as published by the Free Software Foundation, either version 3
* of the License, or (at your option) any later version.
*
* nymea is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with nymea. If not, see <https://www.gnu.org/licenses/>.
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef TIMEDESCRIPTOR_H
#define TIMEDESCRIPTOR_H

#include "calendaritem.h"
#include "timeeventitem.h"

class TimeDescriptor
{
    Q_GADGET
    Q_PROPERTY(TimeEventItems timeEventItems READ timeEventItems WRITE setTimeEventItems USER true)
    Q_PROPERTY(CalendarItems calendarItems READ calendarItems WRITE setCalendarItems USER true)
public:
    explicit TimeDescriptor();

    TimeEventItems timeEventItems() const;
    void setTimeEventItems(const TimeEventItems &timeEventItems);

    CalendarItems calendarItems() const;
    void setCalendarItems(const CalendarItems &calendarItems);

    Q_INVOKABLE bool isValid() const;
    bool isEmpty() const;

    bool evaluate(const QDateTime &lastEvaluationTime, const QDateTime &dateTime) const;

    //    void dumpToSettings(NymeaSettings &settings, const QString &groupName) const;
    //    static TimeDescriptor loadFromSettings(NymeaSettings &settings, const QString &groupPrefix);

private:
    TimeEventItems m_timeEventItems;
    CalendarItems m_calendarItems;
};

QDebug operator<<(QDebug dbg, const TimeDescriptor &timeDescriptor);

#endif // TIMEDESCRIPTOR_H
