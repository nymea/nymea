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
    \class RuleAction
    \brief Describes an action for a \l{nymeaserver::Rule}.

    \ingroup guh-types
    \ingroup rules
    \inmodule libnymea

    A RuleAction describes a special form of an \l{Action} for a \l{nymeaserver::Rule}. The main difference is
    the \l{RuleActionParam}, which allows to use an EventTypeId within a \l{nymeaserver::Rule} to execute this \l{RuleAction}.

    \sa nymeaserver::Rule, RuleActionParam,
*/

#include "ruleaction.h"

/*! Constructs a RuleAction with the given by \a actionTypeId and \a deviceId. */
RuleAction::RuleAction(const ActionTypeId &actionTypeId, const DeviceId &deviceId, const RuleActionParamList &params):
    m_id(ActionId::createActionId()),
    m_actionTypeId(actionTypeId),
    m_deviceId(deviceId),
    m_ruleActionParams(params)
{

}

/*! Constructs a RuleAction with the given by \a interface and \a interfaceAction. */
RuleAction::RuleAction(const QString &interface, const QString &interfaceAction, const RuleActionParamList &params) :
    m_interface(interface),
    m_interfaceAction(interfaceAction),
    m_ruleActionParams(params)
{

}

/*! Constructs a copy of the given \a other RuleAction. */
RuleAction::RuleAction(const RuleAction &other) :
    m_id(other.id()),
    m_actionTypeId(other.actionTypeId()),
    m_deviceId(other.deviceId()),
    m_interface(other.interface()),
    m_interfaceAction(other.interfaceAction()),
    m_ruleActionParams(other.ruleActionParams())
{

}

/*! Return the ActionId of this RuleAction.*/
ActionId RuleAction::id() const
{
    return m_id;
}

/*! Return true, if the actionTypeId and the deviceId of this RuleAction are valid (set).*/
bool RuleAction::isValid() const
{
    return (!m_actionTypeId.isNull() && !m_deviceId.isNull()) || (!m_interface.isEmpty() && !m_interfaceAction.isEmpty());
}

/*! Returns whether this RuleAction is targetting a specific device or rather an interface. */
RuleAction::Type RuleAction::type() const
{
    return (!m_actionTypeId.isNull() && !m_deviceId.isNull()) ? TypeDevice : TypeInterface;
}

/*! Return true, if this RuleAction contains a \l{RuleActionParam} which is based on an EventTypeId.*/
bool RuleAction::isEventBased() const
{
    foreach (const RuleActionParam &param, m_ruleActionParams) {
        if (param.eventTypeId() != EventTypeId()) {
            return true;
        }
    }
    return false;
}

/*! Converts this \l{RuleAction} to a normal \l{Action}.
 *  \sa Action, */
Action RuleAction::toAction() const
{
    Action action(m_actionTypeId, m_deviceId);
    ParamList params;
    foreach (const RuleActionParam &ruleActionParam, m_ruleActionParams) {
        params.append(Param(ruleActionParam.paramTypeId(), ruleActionParam.value()));
    }
    action.setParams(params);
    return action;
}

/*! Returns the actionTypeId of this RuleAction. */
ActionTypeId RuleAction::actionTypeId() const
{
    return m_actionTypeId;
}

/*! Returns the deviceId of this RuleAction. */
DeviceId RuleAction::deviceId() const
{
    return m_deviceId;
}

/*! Returns the name of the interface associated with this RuleAction. */
QString RuleAction::interface() const
{
    return m_interface;
}

/*! Returns the name of the action of the associated interface. */
QString RuleAction::interfaceAction() const
{
    return m_interfaceAction;
}

/*! Returns the \l{RuleActionParamList} of this RuleAction.
 *  \sa RuleActionParam, */
RuleActionParamList RuleAction::ruleActionParams() const
{
    return m_ruleActionParams;
}

/*! Set the \l{RuleActionParamList} of this RuleAction to the given \a ruleActionParams.
 *  \sa RuleActionParam, */
void RuleAction::setRuleActionParams(const RuleActionParamList &ruleActionParams)
{
    m_ruleActionParams = ruleActionParams;
}

/*! Returns the \l{RuleActionParam} of this RuleAction with the given \a ruleActionParamTypeId.
 *  If there is no \l{RuleActionParam} with th given id an invalid \l{RuleActionParam} will be returnend.
 *  \sa RuleActionParam, */
RuleActionParam RuleAction::ruleActionParam(const ParamTypeId &ruleActionParamTypeId) const
{
    foreach (const RuleActionParam &ruleActionParam, m_ruleActionParams) {
        if (ruleActionParam.paramTypeId() == ruleActionParamTypeId) {
            return ruleActionParam;
        }
    }
    return RuleActionParam(QString());
}

/*! Returns the \l{RuleActionParam} of this RuleAction with the given \a ruleActionParamName.
 *  If there is no \l{RuleActionParam} with th given name an invalid \l{RuleActionParam} will be returnend.
 *  \sa RuleActionParam, */
RuleActionParam RuleAction::ruleActionParam(const QString &ruleActionParamName) const
{
    foreach (const RuleActionParam &ruleActionParam, m_ruleActionParams) {
        if (ruleActionParam.paramName() == ruleActionParamName) {
            return ruleActionParam;
        }
    }
    return RuleActionParam(QString());
}

/*! Copy the data to a \l{RuleAction} from an \a other rule action. */
void RuleAction::operator=(const RuleAction &other)
{
    m_id = other.id();
    m_actionTypeId = other.actionTypeId();
    m_ruleActionParams = other.ruleActionParams();
}
