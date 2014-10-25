/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
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

#ifndef STATE_H
#define STATE_H

#include "typeutils.h"

#include <QVariant>
#include <QDebug>

class State
{
public:
    State(const StateTypeId &stateTypeId, const DeviceId &deviceId);

    StateId id() const;

    StateTypeId stateTypeId() const;
    DeviceId deviceId() const;

    //QStringList stateNames() const;
    QVariant value() const;
    void setValue(const QVariant &value);

private:
    StateId m_id;
    StateTypeId m_stateTypeId;
    DeviceId m_deviceId;
    QVariant m_value;
};

QDebug operator<<(QDebug dbg, const State &event);
QDebug operator<<(QDebug dbg, const QList<State> &events);


#endif // STATE_H
