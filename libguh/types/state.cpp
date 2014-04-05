/****************************************************************************
 *                                                                          *
 *  This file is part of guh.                                               *
 *                                                                          *
 *  Guh is free software: you can redistribute it and/or modify             *
 *  it under the terms of the GNU General Public License as published by    *
 *  the Free Software Foundation, version 2 of the License.                 *
 *                                                                          *
 *  Guh is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *  GNU General Public License for more details.                            *
 *                                                                          *
 *  You should have received a copy of the GNU General Public License       *
 *  along with guh.  If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                          *
 ***************************************************************************/

/*!
  \class State
  \brief Holds the parameters of a State of a \l{Device}.

  \ingroup types
  \inmodule libguh

  States hold the state values for devices. A State is associated to a \l{Device} by
  the \l{State::deviceId()} and represents the value of a state described in a \l{StateType}

  \sa StateType
*/

#include "state.h"

/*! Constructs a State reflecting the \l{StateType} given by \a stateTypeId
    and associated with the \l{Device} given by \a deviceId */
State::State(const StateTypeId &stateTypeId, const DeviceId &deviceId):
    m_stateTypeId(stateTypeId),
    m_deviceId(deviceId)
{
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

/*! Returns the state's value.*/
QVariant State::value() const
{
    return m_value;
}

/*! Set the state's value to \a value.*/
void State::setValue(const QVariant &value)
{
    m_value = value;
}
