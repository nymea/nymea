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

#include "scriptalarm.h"

#include <QTimer>
#include <QLoggingCategory>
Q_DECLARE_LOGGING_CATEGORY(dcScriptEngine)

namespace nymeaserver {
namespace scriptengine {

ScriptAlarm::ScriptAlarm(QObject *parent) : QObject(parent)
{
}

QTime ScriptAlarm::time() const
{
    return m_time;
}

void ScriptAlarm::setTime(const QTime &time)
{
    if (m_time != time) {
        m_time = time;
        emit timeChanged();

        if (!time.isValid()) {
            qCWarning(dcScriptEngine()) << "Invalid time:" << time;
        }

        if (time.isValid() && m_timerId == 0) {
            m_timerId = startTimer(1000, Qt::VeryCoarseTimer);
        } else if (!time.isValid() && m_timerId != 0) {
            killTimer(m_timerId);
        }

        updateActive();
    }
}

QTime ScriptAlarm::endTime() const
{
    return m_endTime;
}

void ScriptAlarm::setEndTime(const QTime &endTime)
{
    if (m_endTime != endTime) {
        m_endTime = endTime;
        emit endTimeChanged();

        updateActive();
    }
}

ScriptAlarm::WeekDays ScriptAlarm::weekDays() const
{
    return m_weekDays;
}

void ScriptAlarm::setWeekDays(const WeekDays &weekDays)
{
    if (m_weekDays != weekDays) {
        m_weekDays = weekDays;
        emit weekDaysChanged();

        updateActive();
    }
}

bool ScriptAlarm::active() const
{
    return m_active;
}

void ScriptAlarm::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event)

    QTime now = QTime::currentTime();


    updateActive();

    if (!m_weekDays.testFlag(today())) {
        return;
    }
    if (!m_weekDays.testFlag(today())) {
        return;
    }
    if (m_time.hour() != now.hour()) {
        return;
    }
    if (m_time.minute() != now.minute()) {
        return;
    }
    if (m_time.second() != now.second()) {
        return;
    }

    emit triggered();
}

ScriptAlarm::WeekDay ScriptAlarm::today() const
{
    QList<WeekDay> allDays = {Monday, Tuesday, Wednesday, Thursday, Friday, Saturday, Sunday};
    return allDays.at(QDateTime::currentDateTime().date().dayOfWeek() - 1);
}

void ScriptAlarm::updateActive()
{
    QTime now = QTime::currentTime();

    bool active = m_endTime.isValid() && m_weekDays.testFlag(today());

    if (active) {
        bool beforeStart = now < m_time;
        bool afterEnd = now > m_endTime;
        if (m_time < m_endTime) {
            active = !beforeStart && !afterEnd;
        } else {
            active = beforeStart || afterEnd;
        }
    }
    if (active != m_active) {
        m_active = active;
        emit activeChanged();
    }
}

}
}
