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
    \class StateType
    \brief Describes the Type of a \l{State} from \l{Device}.

    \ingroup types
    \inmodule libguh

    \sa State, StateDescriptor
*/

#include "statetype.h"

/*! Constructs a StateType with the given \a id.
 *  When creating a \l{DevicePlugin} generate a new uuid for each StateType you define and
 *  hardcode it into the plugin. */
StateType::StateType(const StateTypeId &id):
    m_id(id),
    m_unit(Types::UnitNone)
{

}

/*! Returns the id of the StateType. */
StateTypeId StateType::id() const
{
    return m_id;
}

/*! Returns the name of the StateType. This is visible to the user (e.g. "Temperature"). */
QString StateType::name() const
{
    return m_name;
}

/*! Set the name of the StateType to \a name. This is visible to the user (e.g. "Temperature"). */
void StateType::setName(const QString &name)
{
    m_name = name;
}

/*! Returns the Type of the StateType (e.g. QVariant::Real). */
QVariant::Type StateType::type() const
{
    return m_type;
}

/*! Set the type fo the StateType to \a type (e.g. QVariant::Real). */
void StateType::setType(const QVariant::Type &type)
{
    m_type = type;
}

/*! Returns the default value of this StateType (e.g. 21.5). */
QVariant StateType::defaultValue() const
{
    return m_defaultValue;
}

/*! Set the default value of this StateType to \a defaultValue (e.g. 21.5). */
void StateType::setDefaultValue(const QVariant &defaultValue)
{
    m_defaultValue = defaultValue;
}

/*! Returns the unit of this StateType. */
Types::Unit StateType::unit() const
{
    return m_unit;
}

/*! Sets the unit of this StateType to the given \a unit. */
void StateType::setUnit(const Types::Unit &unit)
{
    m_unit = unit;
}
