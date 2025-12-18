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
    \class Action
    \brief Holds information required to execute an action described by a \l{ActionType}.

    \ingroup nymea-types
    \inmodule libnymea

    It is bound to a \l{Device} and an \l{ActionType} and holds the parameters
    for the execution of the action.

    The params must match the template as described in \l{ActionType}.

    \sa Device, ActionType
*/

#include "action.h"

/*! Construct an Action with the given \a deviceId and \a actionTypeId. */
Action::Action(const ActionTypeId &actionTypeId, const ThingId &thingId, TriggeredBy triggeredBy)
    : m_actionTypeId(actionTypeId)
    , m_thingId(thingId)
    , m_triggeredBy(triggeredBy)
{}

/*! Construct a copy of an \a other Action. */
Action::Action(const Action &other)
    : m_actionTypeId(other.actionTypeId())
    , m_thingId(other.thingId())
    , m_params(other.params())
    , m_triggeredBy(other.triggeredBy())
{}

/*! An Action is valid if actionTypeId and deviceId are valid uuids. Returns true if valid, false if not. */
bool Action::isValid() const
{
    return !m_actionTypeId.isNull() && !m_thingId.isNull();
}

/*! Returns the actionTypeId for this Action.*/
ActionTypeId Action::actionTypeId() const
{
    return m_actionTypeId;
}

void Action::setActionTypeId(const ActionTypeId &actionTypeId)
{
    m_actionTypeId = actionTypeId;
}

/*! Returns the \l {ThingId} this Action is associated with. */
ThingId Action::thingId() const
{
    return m_thingId;
}

void Action::setThingId(const ThingId &thingId)
{
    m_thingId = thingId;
}

/*! Returns the parameters for this Action. */
ParamList Action::params() const
{
    return m_params;
}

/*! Set the parameters for this Action. \a params must match the template in the \l{ActionType}
 *  referred by \l{Action::actionTypeId()}. */
void Action::setParams(const ParamList &params)
{
    m_params = params;
}

/*! Returns the parameter for the given \a paramTypeId. The returned \l{Param} will be invalid if this Action does not have such a \l{Param}. */
Param Action::param(const ParamTypeId &paramTypeId) const
{
    foreach (const Param &param, m_params) {
        if (param.paramTypeId() == paramTypeId) {
            return param;
        }
    }
    return Param(ParamTypeId(), QString());
}

/*! Returns the parameter value for the given \a paramTypeId. The returned \l{QVariant} will be null if this Action does not have such a \l{Param}. */
QVariant Action::paramValue(const ParamTypeId &paramTypeId) const
{
    foreach (const Param &param, m_params) {
        if (param.paramTypeId() == paramTypeId) {
            return param.value();
        }
    }
    return QVariant();
}

/*! Gives an indication of the origin of this action.
    Normally a plugin should treat all actions the same. There might be
    rare exceptions tho:

    - This might be important to know for some devices, e.g. garage doors which are required to
      not close in some circumstances. I.e. if a garage door does not have a light sensor bar, it
      must never close automatically by a rule to prevent damage and fulfill legal requirements.

    - Other use cases might be to prioritize a single request by a user over a queue of automated
      ones. I.e. if a script animation animates a color light, the plugin might build up a queue of
      pending commands. To still react timely to a user interaction (e.g. power off), this
      information might be of use to a plugin.
*/
Action::TriggeredBy Action::triggeredBy() const
{
    return m_triggeredBy;
}

/*! Copy the data to an \l{Action} from an \a other action. */
void Action::operator=(const Action &other)
{
    m_actionTypeId = other.actionTypeId();
    m_params = other.params();
    m_thingId = other.thingId();
    m_triggeredBy = other.triggeredBy();
}
