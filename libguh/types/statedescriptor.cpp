/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stuerz <simon.stuerz@guh.guru>                *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
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

/*!
    \class StateDescriptor
    \brief Describes a certain \l{State}.

    \ingroup guh-types
    \ingroup rules
    \inmodule libguh

    An StateDescriptor describes a \l{State} in order to match it with a \l{guhserver::Rule}.

    \sa State, guhserver::Rule
*/


#include "statedescriptor.h"

/*! Constructs an StateDescriptor describing an \l{State}.*/
StateDescriptor::StateDescriptor():
    m_operatorType(Types::ValueOperatorEquals)
{

}

/*! Constructs an StateDescriptor describing an \l{State} with the given \a stateTypeId, \a deviceId, \a stateValue and \a operatorType.*/
StateDescriptor::StateDescriptor(const StateTypeId &stateTypeId, const DeviceId &deviceId, const QVariant &stateValue, Types::ValueOperator operatorType):
    m_stateTypeId(stateTypeId),
    m_deviceId(deviceId),
    m_stateValue(stateValue),
    m_operatorType(operatorType)
{

}

/*! Returns the StateTypeId of this \l{State}.*/
StateTypeId StateDescriptor::stateTypeId() const
{
    return m_stateTypeId;
}

/*! Returns the DeviceId of this \l{State}.*/
DeviceId StateDescriptor::deviceId() const
{
    return m_deviceId;
}

/*! Returns the Value of this \l{State}.*/
QVariant StateDescriptor::stateValue() const
{
    return m_stateValue;
}

/*! Returns the ValueOperator of this \l{State}.*/
Types::ValueOperator StateDescriptor::operatorType() const
{
    return m_operatorType;
}

/*! Compare this StateDescriptor to \a other.
 *  StateDescriptors are equal (returns true) if stateTypeId, stateValue and operatorType match. */
bool StateDescriptor::operator ==(const StateDescriptor &other) const
{
    return m_stateTypeId == other.stateTypeId() &&
            m_deviceId == other.deviceId() &&
            m_stateValue == other.stateValue() &&
            m_operatorType == other.operatorType();
}

/*! Compare this StateDescriptor to the \l{State} given by \a state.
 *  Returns true if the given \a state matches the definition of the StateDescriptor */
bool StateDescriptor::operator ==(const State &state) const
{
    if ((m_stateTypeId != state.stateTypeId()) || (m_deviceId != state.deviceId())) {
        return false;
    }
    QVariant convertedValue = state.value();
    convertedValue.convert(m_stateValue.type());
    switch (m_operatorType) {
    case Types::ValueOperatorEquals:
        return m_stateValue == convertedValue;
    case Types::ValueOperatorGreater:
        return convertedValue > m_stateValue;
    case Types::ValueOperatorGreaterOrEqual:
        return convertedValue >= m_stateValue;
    case Types::ValueOperatorLess:
        return convertedValue < m_stateValue;
    case Types::ValueOperatorLessOrEqual:
        return convertedValue <= m_stateValue;
    case Types::ValueOperatorNotEquals:
        return m_stateValue != convertedValue;
    }
    return false;
}

/*! Compare this StateDescriptor to the \l{State} given by \a state.
 *  Returns true if the given \a state does not match the definition of the StateDescriptor */
bool StateDescriptor::operator !=(const State &state) const
{
    return !(operator==(state));
}

/*! Returns the true if this \l{StateDescriptor} is valid. A \l{StateDescriptor} is valid
 *  if the DeviceId and the StateTypeId are set and the state value of this \l{StateDescriptor} is valid.
 * \sa StateDescriptor(), deviceId(), stateValue()
 */
bool StateDescriptor::isValid() const
{
    return !m_deviceId.isNull() && !m_stateTypeId.isNull() && m_stateValue.isValid();
}
