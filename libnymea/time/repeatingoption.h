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

#ifndef REPEATINGOPTION_H
#define REPEATINGOPTION_H

#include <QList>
#include <QMetaType>
#include <QVariant>

class QDateTime;

class RepeatingOption
{
    Q_GADGET
    Q_PROPERTY(RepeatingMode mode READ mode WRITE setMode)
    Q_PROPERTY(QList<int> weekDays READ weekDays WRITE setWeekDays USER true)
    Q_PROPERTY(QList<int> monthDays READ monthDays WRITE setMonthDays USER true)
public:
    enum RepeatingMode { RepeatingModeNone, RepeatingModeHourly, RepeatingModeDaily, RepeatingModeWeekly, RepeatingModeMonthly, RepeatingModeYearly };
    Q_ENUM(RepeatingMode)

    RepeatingOption();
    RepeatingOption(const RepeatingMode &mode, const QList<int> &weekDays = QList<int>(), const QList<int> &monthDays = QList<int>());

    RepeatingMode mode() const;
    void setMode(RepeatingMode mode);

    QList<int> weekDays() const;
    void setWeekDays(const QList<int> &weekDays);

    QList<int> monthDays() const;
    void setMonthDays(const QList<int> &monthDays);

    bool isEmtpy() const;
    bool isValid() const;

    bool evaluateWeekDay(const QDateTime &dateTime) const;
    bool evaluateMonthDay(const QDateTime &dateTime) const;

private:
    RepeatingMode m_mode;

    QList<int> m_weekDays;
    QList<int> m_monthDays;
};

QDebug operator<<(QDebug dbg, const RepeatingOption &RepeatingOption);

#endif // REPEATINGOPTION_H
