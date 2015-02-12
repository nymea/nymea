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

/*!
    \class Action
    \brief Holds information required to execute an action described by a \l{ActionType}.

    \ingroup types
    \inmodule libguh

    It is bound to a \l{Device} and an \l{ActionType} and holds the parameters
    for the execution of the action.

    The params must match the template as described in \l{ActionType}.

    \sa Device, ActionType
*/

#include "action.h"

/*! Construct an Action with the given \a deviceId and \a actionTypeId. */
Action::Action(const ActionTypeId &actionTypeId, const DeviceId &deviceId) :
    m_id(ActionId::createActionId()),
    m_actionTypeId(actionTypeId),
    m_deviceId(deviceId)
{
}

Action::Action(const Action &other):
    m_id(other.id()),
    m_actionTypeId(other.actionTypeId()),
    m_deviceId(other.deviceId()),
    m_params(other.params())
{
}

/*! Returns the actionId for this Action. */
ActionId Action::id() const
{
    return m_id;
}

/*! An Action is valid if actionTypeId and deviceId are valid uuids. Returns true if valid, false if not. */
bool Action::isValid() const
{
    return !m_actionTypeId.isNull() && !m_deviceId.isNull();
}

/*! Returns the actionTypeId for this Action.*/
ActionTypeId Action::actionTypeId() const
{
    return m_actionTypeId;
}

/*! Returns the deviceId this Action is associated with. */
DeviceId Action::deviceId() const
{
    return m_deviceId;
}

/*! Returns the parameters for this Action. */
ParamList Action::params() const
{
    return m_params;
}

/*! Set the the parameters for this Action. \a params must match the template in the \l{ActionType}
 *  referred by \l{Action::actionTypeId()}. */
void Action::setParams(const ParamList &params)
{
    m_params = params;
}

/*! Returns the parameter of this Action with a cetrain \a paramName. */
Param Action::param(const QString &paramName) const
{
    foreach (const Param &param, m_params) {
        if (param.name() == paramName) {
            return param;
        }
    }
    return Param(QString());
}

void Action::operator =(const Action &other)
{
    m_id = other.id();
    m_actionTypeId = other.actionTypeId();
    m_params = other.params();
}
