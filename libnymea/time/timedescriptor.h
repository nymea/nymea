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

#ifndef TIMEDESCRIPTOR_H
#define TIMEDESCRIPTOR_H

#include "timeeventitem.h"
#include "calendaritem.h"

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
