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
* GNU General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU General Public License as published by the Free Software
* Foundation, GNU version 3. This project is distributed in the hope that it
* will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
* of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
* Public License for more details.
*
* You should have received a copy of the GNU General Public License along with
* this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef SCRIPTALARM_H
#define SCRIPTALARM_H

#ifdef WITH_QML

#include <QObject>
#include <QDateTime>
#include <QTimer>

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
#endif // WITH_QML

#endif // SCRIPTALARM_H
