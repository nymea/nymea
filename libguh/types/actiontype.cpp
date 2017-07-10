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
    \class ActionType
    \brief Describes an \l{Action} for a \l{Device}.

    \ingroup guh-types
    \inmodule libguh

    ActionTypes are contained in \l{DeviceClass} templates returned
    by \l{DevicePlugin}{DevicePlugins} in order to describe the hardware supported
    by the plugin.

    All Actions must have valid a ActionType in order to be useful.

    \sa Action
*/

#include "actiontype.h"

/*! Constructs an \l{ActionType} with the given \a id. */
ActionType::ActionType(const ActionTypeId &id):
    m_id(id),
    m_index(0)
{

}

/*! Returns the id of this \l{ActionType}. */
ActionTypeId ActionType::id() const
{
    return m_id;
}

/*! Returns the name of this \l{ActionType}. */
QString ActionType::name() const
{
    return m_name;
}

/*! Set the \a name for this \l{ActionType}. This will be visible to the user. */
void ActionType::setName(const QString &name)
{
    m_name = name;
}

/*! Returns the index of this \l{ActionType}. The index of an \l{ActionType} indicates the order in the \l{DeviceClass}.
 *  This guarantees that a \l{Device} will look always the same (\l{Action} order). */
int ActionType::index() const
{
    return m_index;
}

/*! Set the \a index of this \l{ActionType}. */
void ActionType::setIndex(const int &index)
{
    m_index = index;
}

/*! Returns the parameter description of this \l{ActionType}. \l{Action}{Actions} created
 *  from this \l{ActionType} must have their parameters matching to this template. */
QList<ParamType> ActionType::paramTypes() const
{
    return m_paramTypes;
}

/*! Set the parameter description of this \l{ActionType}. \l{Action}{Actions} created
 *  from this \l{ActionType} must have their \a paramTypes matching to this template. */
void ActionType::setParamTypes(const QList<ParamType> &paramTypes)
{
    m_paramTypes = paramTypes;
}

ActionTypes::ActionTypes(const QList<ActionType> &other)
{
    foreach (const ActionType &at, other) {
        append(at);
    }
}

ActionType ActionTypes::findByName(const QString &name)
{
    foreach (const ActionType &actionType, *this) {
        if (actionType.name() == name) {
            return actionType;
        }
    }
    return ActionType(ActionTypeId());
}

ActionType ActionTypes::findById(const ActionTypeId &id)
{
    foreach (const ActionType &actionType, *this) {
        if (actionType.id() == id) {
            return actionType;
        }
    }
    return ActionType(ActionTypeId());
}
