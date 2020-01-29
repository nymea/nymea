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
    enum RepeatingMode {
        RepeatingModeNone,
        RepeatingModeHourly,
        RepeatingModeDaily,
        RepeatingModeWeekly,
        RepeatingModeMonthly,
        RepeatingModeYearly
    };
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
