/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2016 Simon Stürz <simon.stuerz@guh.io>                   *
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

#ifndef REPEATINGOPTION_H
#define REPEATINGOPTION_H

#include <QList>
#include <QMetaType>

class QDateTime;

namespace guhserver {

class RepeatingOption
{
    Q_GADGET
    Q_ENUMS(RepeatingMode)

public:
    enum RepeatingMode {
        RepeatingModeNone,
        RepeatingModeHourly,
        RepeatingModeDaily,
        RepeatingModeWeekly,
        RepeatingModeMonthly,
        RepeatingModeYearly
    };

    RepeatingOption();
    RepeatingOption(const RepeatingMode &mode, const QList<int> &weekDays = QList<int>(), const QList<int> &monthDays = QList<int>());

    RepeatingMode mode() const;

    QList<int> weekDays() const;
    QList<int> monthDays() const;

    bool isEmtpy() const;
    bool isValid() const;

    bool evaluateWeekDay(const QDateTime &dateTime) const;
    bool evaluateMonthDay(const QDateTime &dateTime) const;

private:
    RepeatingMode m_mode;

    QList<int> m_weekDays;
    QList<int> m_monthDays;

};

}

#endif // REPEATINGOPTION_H
