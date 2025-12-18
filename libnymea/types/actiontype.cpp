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
    \class ActionType
    \brief Describes an \l{Action} for a \l{Device}.

    \ingroup nymea-types
    \inmodule libnymea

    ActionTypes are contained in \l{DeviceClass} templates returned
    by \l{DevicePlugin}{DevicePlugins} in order to describe the hardware supported
    by the plugin.

    All Actions must have valid a ActionType in order to be useful.

    \sa Action
*/

#include "actiontype.h"

/*! Constructs an \l{ActionType} with the given \a id. */
ActionType::ActionType(const ActionTypeId &id)
    : m_id(id)
    , m_index(0)
{}

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

/*! Set the \a name for this \l{ActionType}. */
void ActionType::setName(const QString &name)
{
    m_name = name;
}

/*! Returns the display name of this \l{ActionType}. */
QString ActionType::displayName() const
{
    return m_displayName;
}

/*! Set the \a displayName for this \l{ActionType}. This will be visible to the user. */
void ActionType::setDisplayName(const QString &displayName)
{
    m_displayName = displayName;
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
ParamTypes ActionType::paramTypes() const
{
    return m_paramTypes;
}

/*! Set the parameter description of this \l{ActionType}. \l{Action}{Actions} created
 *  from this \l{ActionType} must have their \a paramTypes matching to this template. */
void ActionType::setParamTypes(const ParamTypes &paramTypes)
{
    m_paramTypes = paramTypes;
}

ActionTypes::ActionTypes(const QList<ActionType> &other)
{
    foreach (const ActionType &at, other) {
        append(at);
    }
}

bool ActionTypes::contains(const ActionTypeId &id) const
{
    foreach (const ActionType &actionType, *this) {
        if (actionType.id() == id) {
            return true;
        }
    }
    return false;
}

bool ActionTypes::contains(const QString &name) const
{
    foreach (const ActionType &actionType, *this) {
        if (actionType.name() == name) {
            return true;
        }
    }
    return false;
}

QVariant ActionTypes::get(int index) const
{
    return QVariant::fromValue(at(index));
}

void ActionTypes::put(const QVariant &variant)
{
    append(variant.value<ActionType>());
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

ActionType &ActionTypes::operator[](const QString &name)
{
    int index = -1;
    for (int i = 0; i < count(); i++) {
        if (at(i).name() == name) {
            index = i;
            break;
        }
    }
    return QList::operator[](index);
}

QDebug operator<<(QDebug dbg, const ActionType &actionType)
{
    QDebugStateSaver saver(dbg);
    dbg.nospace().noquote() << "ActionType(" << actionType.name() << ", " << actionType.displayName() << ", " << actionType.id() << ") ";
    return dbg;
}
