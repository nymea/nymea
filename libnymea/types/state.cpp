/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
 *                                                                         *
 *  This file is part of guh.                                              *
 *                                                                         *
 *  This library is free software; you can redistribute it and/or          *
 *  modify it under the terms of the GNU Lesser General Public             *
 *  License as published by the Free Software Foundation; either           *
 *  version 2.1 of the License, or (at your option) any later version.     *
 *                                                                         *
 *  This library is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
 *  Lesser General Public License for more details.                        *
 *                                                                         *
 *  You should have received a copy of the GNU Lesser General Public       *
 *  License along with this library; If not, see                           *
 *  <http://www.gnu.org/licenses/>.                                        *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*!
  \class State
  \brief Holds the parameters of a State of a \l{Device}.

  \ingroup guh-types
  \inmodule libnymea

  States hold the state values for devices. A State is associated to a \l{Device} by
  the \l{State::deviceId()} and represents the value of a state described in a \l{StateType}

  \sa StateType, StateDescriptor
*/

#include "state.h"

/*! Constructs a State reflecting the \l{StateType} given by \a stateTypeId
 *  and associated with the \l{Device} given by \a deviceId */
State::State(const StateTypeId &stateTypeId, const DeviceId &deviceId):
    m_id(StateId::createStateId()),
    m_stateTypeId(stateTypeId),
    m_deviceId(deviceId)
{
}

/*! Returns the id of this State. */
StateId State::id() const
{
    return m_id;
}

/*! Returns the id of the StateType describing this State. */
StateTypeId State::stateTypeId() const
{
    return m_stateTypeId;
}

/*! Returns the id of the StateType describing this State. */
DeviceId State::deviceId() const
{
    return m_deviceId;
}

/*! Returns the state's value. */
QVariant State::value() const
{
    return m_value;
}

/*! Set the state's value to \a value. */
void State::setValue(const QVariant &value)
{
    m_value = value;
}

/*! Writes the stateTypeId, the deviceId and the value of the given \a state to \a dbg. */
QDebug operator<<(QDebug dbg, const State &state)
{
    dbg.nospace() << "State(StateTypeId: " << state.stateTypeId().toString() << ", DeviceId:" << state.deviceId() << ", value:" << state.value() << ")";
    return dbg.space();
}

/*! Writes each stateTypeId, deviceId and value of the given \a states to \a dbg. */
QDebug operator<<(QDebug dbg, const QList<State> &states)
{
    dbg.nospace() << "StateList (count:" << states.count() << ")";
    for (int i = 0; i < states.count(); i++ ) {
        dbg.nospace() << "     " << i << ": " << states.at(i);
    }

    return dbg.space();
}
