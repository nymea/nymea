/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
 *                                                                         *
 *  This file is part of nymea.                                            *
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
    \class StateDescriptor
    \brief Describes a certain \l{State}.

    \ingroup nymea-types
    \ingroup rules
    \inmodule libnymea

    A StateDescriptor describes a \l{State} in order to match it with a \l{nymeaserver::Rule}.
    A StateDescriptor uses either a \l{DeviceId}/\l{StateTypeId} pair to describe a \l{State} or
    a pair of strings describing the interface and interface action for a \l{State}.

    \sa State, nymeaserver::Rule
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

/*! Constructs an StateDescriptor describing an \l{State} with the given \a interface, \a interfaceState, \a stateValue and \a operatorType.*/
StateDescriptor::StateDescriptor(const QString &interface, const QString &interfaceState, const QVariant &stateValue, Types::ValueOperator operatorType):
    m_interface(interface),
    m_interfaceState(interfaceState),
    m_stateValue(stateValue),
    m_operatorType(operatorType)
{

}

/*! Returns true \l{StateDescriptor::Type}{Type} of this descriptor. */
StateDescriptor::Type StateDescriptor::type() const
{
    return (!m_deviceId.isNull() && !m_stateTypeId.isNull()) ? TypeDevice : TypeInterface;
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

/*! Returns the interface for this \{StateDescriptor}.*/
QString StateDescriptor::interface() const
{
    return m_interface;
}

/*! Returns the interface state's name for this \{StateDescriptor}.*/
QString StateDescriptor::interfaceState() const
{
    return m_interfaceState;
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
            m_interface == other.interface() &&
            m_interfaceState == other.interfaceState() &&
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

/*! Returns the true if this \l{StateDescriptor} is valid. A valid \l{StateDescriptor} must
 *  have a valid stateValue along with either a DeviceId/StateTypeId pair or an interface/interfaceState pair.
 * \sa StateDescriptor(), deviceId(), stateValue()
 */
bool StateDescriptor::isValid() const
{
    return ((!m_deviceId.isNull() && !m_stateTypeId.isNull()) || (!m_interface.isNull() && !m_interfaceState.isNull())) && m_stateValue.isValid();
}

QDebug operator<<(QDebug dbg, const StateDescriptor &stateDescriptor)
{
    dbg.nospace() << "StateDescriptor(DeviceId:" << stateDescriptor.deviceId().toString() << ", StateTypeId:"
                  << stateDescriptor.stateTypeId().toString() << ", Interface:" << stateDescriptor.interface()
                  << ", InterfaceState:" << stateDescriptor.interfaceState() << ", Operator:" << stateDescriptor.operatorType() << ", Value:" << stateDescriptor.stateValue();
    return dbg;
}
