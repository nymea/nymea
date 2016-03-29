/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2016 Simon Stuerz <simon.stuerz@guh.guru>                *
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

#include "repeatingoption.h"

namespace guhserver {

RepeatingOption::RepeatingOption() :
    m_mode(RepeatingModeNone)
{

}

RepeatingOption::RepeatingOption(const RepeatingMode &mode, const QList<int> &weekDays, const QList<int> &monthDays) :
    m_mode(mode),
    m_weekDays(weekDays),
    m_monthDays(monthDays)
{

}

RepeatingOption::RepeatingMode RepeatingOption::mode() const
{
    return m_mode;
}

QList<int> RepeatingOption::weekDays() const
{
    return m_weekDays;
}

QList<int> RepeatingOption::monthDays() const
{
    return m_monthDays;
}

bool RepeatingOption::isEmtpy() const
{
    return m_mode == RepeatingModeNone && m_weekDays.isEmpty() && m_monthDays.isEmpty();
}

}
