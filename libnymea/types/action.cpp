/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2020, nymea GmbH
* Contact: contact@nymea.io
*
* This file is part of nymea.
* This project including source code and documentation is protected by
* copyright law, and remains the property of nymea GmbH. All rights, including
* reproduction, publication, editing and translation, are reserved. The use of
* this project is subject to the terms of a license agreement to be concluded
* with nymea GmbH in accordance with the terms of use of nymea GmbH, available
* under https://nymea.io/license
*
* GNU Lesser General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU Lesser General Public License as published by the Free
* Software Foundation; version 3. This project is distributed in the hope that
* it will be useful, but WITHOUT ANY WARRANTY; without even the implied
* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
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
Action::Action(const ActionTypeId &actionTypeId, const DeviceId &deviceId) :
    m_id(ActionId::createActionId()),
    m_actionTypeId(actionTypeId),
    m_deviceId(deviceId)
{
}

/*! Construct a copy of an \a other Action. */
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

void Action::setActionTypeId(const ActionTypeId &actionTypeId)
{
    m_actionTypeId = actionTypeId;
}

/*! Returns the deviceId this Action is associated with. */
DeviceId Action::deviceId() const
{
    return m_deviceId;
}

void Action::setDeviceId(const DeviceId &deviceId)
{
    m_deviceId = deviceId;
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

/*! Returns the parameter of this Action with a cetrain \a paramTypeId. */
Param Action::param(const ParamTypeId &paramTypeId) const
{
    foreach (const Param &param, m_params) {
        if (param.paramTypeId() == paramTypeId) {
            return param;
        }
    }
    return Param(ParamTypeId(), QString());
}

/*! Copy the data to an \l{Action} from an \a other action. */
void Action::operator =(const Action &other)
{
    m_id = other.id();
    m_actionTypeId = other.actionTypeId();
    m_params = other.params();
}
