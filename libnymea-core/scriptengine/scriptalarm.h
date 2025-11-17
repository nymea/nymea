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

#ifndef SCRIPTALARM_H
#define SCRIPTALARM_H

#include <QObject>
#include <QDateTime>
#include <QTimer>
namespace nymeaserver {
namespace scriptengine {

class ScriptAlarm : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QTime time READ time WRITE setTime NOTIFY timeChanged)
    Q_PROPERTY(QTime endTime READ endTime WRITE setEndTime NOTIFY endTimeChanged)
    Q_PROPERTY(WeekDays weekDays READ weekDays WRITE setWeekDays NOTIFY weekDaysChanged)

    Q_PROPERTY(bool active READ active NOTIFY activeChanged)

public:
    enum WeekDay {
        Monday    = 0x01,
        Tuesday   = 0x02,
        Wednesday = 0x04,
        Thursday  = 0x08,
        Friday    = 0x10,
        Saturday  = 0x20,
        Sunday    = 0x40,
        AllDays   = 0xFF
    };
    Q_ENUM(WeekDay)
    Q_DECLARE_FLAGS(WeekDays, WeekDay)
    Q_FLAG(WeekDays)

    explicit ScriptAlarm(QObject *parent = nullptr);

    QTime time() const;
    void setTime(const QTime &time);

    QTime endTime() const;
    void setEndTime(const QTime &endTime);

    WeekDays weekDays() const;
    void setWeekDays(const WeekDays &weekDays);

    bool active() const;

signals:
    void timeChanged();
    void endTimeChanged();
    void weekDaysChanged();

    void triggered();
    void activeChanged();

protected:
    void timerEvent(QTimerEvent *event) override;

private:
    WeekDay today() const;

    void updateActive();

private:
    QTime m_time;
    QTime m_endTime;
    WeekDays m_weekDays = AllDays;

    bool m_active = false;
    int m_timerId = 0;
};

}
}

#endif // SCRIPTALARM_H
