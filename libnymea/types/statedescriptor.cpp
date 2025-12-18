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

/*! \enum StateDescriptor::Type
    \value TypeDevice
        Describes a state by deviceId and stateTypeId
    \value TypeInterface
        Describes a state by interface name and interfaceState name
*/

#include "statedescriptor.h"

/*! Constructs an StateDescriptor describing an \l{State}.*/
StateDescriptor::StateDescriptor()
    : m_operatorType(Types::ValueOperatorEquals)
{}

/*! Constructs an StateDescriptor describing an \l{State} with the given \a stateTypeId, \a deviceId, \a stateValue and \a operatorType.*/
StateDescriptor::StateDescriptor(const StateTypeId &stateTypeId, const ThingId &thingId, const QVariant &stateValue, Types::ValueOperator operatorType)
    : m_stateTypeId(stateTypeId)
    , m_thingId(thingId)
    , m_stateValue(stateValue)
    , m_operatorType(operatorType)
{}

/*! Constructs an StateDescriptor describing an \l{State} with the given \a interface, \a interfaceState, \a stateValue and \a operatorType.*/
StateDescriptor::StateDescriptor(const QString &interface, const QString &interfaceState, const QVariant &stateValue, Types::ValueOperator operatorType)
    : m_interface(interface)
    , m_interfaceState(interfaceState)
    , m_stateValue(stateValue)
    , m_operatorType(operatorType)
{}

/*! Returns true \l{StateDescriptor::Type}{Type} of this descriptor. */
StateDescriptor::Type StateDescriptor::type() const
{
    return (!m_thingId.isNull() && !m_stateTypeId.isNull()) ? TypeThing : TypeInterface;
}

/*! Returns the StateTypeId of this \l{State}.*/
StateTypeId StateDescriptor::stateTypeId() const
{
    return m_stateTypeId;
}

void StateDescriptor::setStateTypeId(const StateTypeId &stateTypeId)
{
    m_stateTypeId = stateTypeId;
}

/*! Returns the \l{ThingId} of this \l{State}.*/
ThingId StateDescriptor::thingId() const
{
    return m_thingId;
}

void StateDescriptor::setThingId(const ThingId &thingId)
{
    m_thingId = thingId;
}

/*! Returns the interface for this \{StateDescriptor}.*/
QString StateDescriptor::interface() const
{
    return m_interface;
}

void StateDescriptor::setInterface(const QString &interface)
{
    m_interface = interface;
}

/*! Returns the interface state's name for this \{StateDescriptor}.*/
QString StateDescriptor::interfaceState() const
{
    return m_interfaceState;
}

void StateDescriptor::setInterfaceState(const QString &interfaceState)
{
    m_interfaceState = interfaceState;
}

/*! Returns the Value of this \l{State}.*/
QVariant StateDescriptor::stateValue() const
{
    return m_stateValue;
}

void StateDescriptor::setStateValue(const QVariant &value)
{
    m_stateValue = value;
}

ThingId StateDescriptor::valueThingId() const
{
    return m_valueThingId;
}

void StateDescriptor::setValueThingId(const ThingId &valueThingId)
{
    m_valueThingId = valueThingId;
}

StateTypeId StateDescriptor::valueStateTypeId() const
{
    return m_valueStateTypeId;
}

void StateDescriptor::setValueStateTypeId(const StateTypeId &valueStateTypeId)
{
    m_valueStateTypeId = valueStateTypeId;
}

/*! Returns the ValueOperator of this \l{State}.*/
Types::ValueOperator StateDescriptor::operatorType() const
{
    return m_operatorType;
}

void StateDescriptor::setOperatorType(Types::ValueOperator opertatorType)
{
    m_operatorType = opertatorType;
}

/*! Compare this StateDescriptor to \a other.
 *  StateDescriptors are equal (returns true) if stateTypeId, stateValue and operatorType match. */
bool StateDescriptor::operator==(const StateDescriptor &other) const
{
    return m_stateTypeId == other.stateTypeId() && m_thingId == other.thingId() && m_interface == other.interface() && m_interfaceState == other.interfaceState()
           && m_stateValue == other.stateValue() && m_operatorType == other.operatorType();
}

/*! Returns the true if this \l{StateDescriptor} is valid. A valid \l{StateDescriptor} must
 *  have a valid stateValue along with either a DeviceId/StateTypeId pair or an interface/interfaceState pair.
 * \sa StateDescriptor(), deviceId(), stateValue()
 */
bool StateDescriptor::isValid() const
{
    return ((!m_thingId.isNull() && !m_stateTypeId.isNull()) || (!m_interface.isNull() && !m_interfaceState.isNull()))
           && (m_stateValue.isValid() || (!m_valueThingId.isNull() && !m_valueStateTypeId.isNull()));
}

/*! Print a StateDescriptor with all its contents to QDebug. */
QDebug operator<<(QDebug dbg, const StateDescriptor &stateDescriptor)
{
    QDebugStateSaver saver(dbg);
    dbg.nospace() << "StateDescriptor(ThingId:" << stateDescriptor.thingId().toString() << ", StateTypeId:" << stateDescriptor.stateTypeId().toString()
                  << ", Interface:" << stateDescriptor.interface() << ", InterfaceState:" << stateDescriptor.interfaceState() << ", Operator:" << stateDescriptor.operatorType()
                  << ", Value:" << stateDescriptor.stateValue() << ", ValueThing:" << stateDescriptor.valueThingId().toString()
                  << ", ValueStateTypeId:" << stateDescriptor.valueStateTypeId().toString();
    return dbg;
}
