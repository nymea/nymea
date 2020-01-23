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
* GNU General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU General Public License as published by the Free Software
* Foundation, GNU version 3. This project is distributed in the hope that it
* will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
* of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
* Public License for more details.
*
* You should have received a copy of the GNU General Public License along with
* this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*!
    \class RuleAction
    \brief Describes an action for a \l{nymeaserver::Rule}.

    \ingroup nymea-types
    \ingroup rules
    \inmodule libnymea

    A RuleAction describes a special form of an \l{Action} for a \l{nymeaserver::Rule}. The main difference is
    the \l{RuleActionParam}, which allows to use an EventTypeId within a \l{nymeaserver::Rule} to execute this \l{RuleAction}.

    \sa nymeaserver::Rule, RuleActionParam,
*/

/*! \enum RuleAction::Type

    \value TypeDevice
        The RuleAction describes a device Action.
    \value TypeInterface
        The RuleAction describes an interface based Action.
*/


#include "ruleaction.h"

/*! Constructs a RuleAction with the given by \a actionTypeId, \a deviceId and \a params.
 *  Use this to create a RuleAction for regular actions, that is, identifying the Action by deviceId and actionTypeId.
 */
RuleAction::RuleAction(const ActionTypeId &actionTypeId, const DeviceId &deviceId, const RuleActionParams &params):
    m_id(ActionId::createActionId()),
    m_deviceId(deviceId),
    m_actionTypeId(actionTypeId),
    m_ruleActionParams(params)
{

}

/*! Constructs a RuleAction with the given by \a interface and \a interfaceAction.
 *  This will create an interface based RuleAction. Meaning, the Action is idenfified by an interface and and interfaceAction.
 */
RuleAction::RuleAction(const QString &interface, const QString &interfaceAction, const RuleActionParams &params) :
    m_interface(interface),
    m_interfaceAction(interfaceAction),
    m_ruleActionParams(params)
{

}

/*! Constructs a RuleAction with the given by \a interface and \a interfaceAction.
 *  Use this to create a RuleAction for executing browser items.
 */
RuleAction::RuleAction(const DeviceId &deviceId, const QString &browserItemId):
    m_deviceId(deviceId),
    m_browserItemId(browserItemId)
{

}

/*! Constructs a copy of the given \a other RuleAction. */
RuleAction::RuleAction(const RuleAction &other) :
    m_id(other.id()),
    m_deviceId(other.deviceId()),
    m_actionTypeId(other.actionTypeId()),
    m_browserItemId(other.browserItemId()),
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
    return (!m_actionTypeId.isNull() && !m_deviceId.isNull())
            || (!m_interface.isEmpty() && !m_interfaceAction.isEmpty())
            || (!m_deviceId.isNull() && !m_browserItemId.isEmpty());
}

/*! Returns whether this RuleAction is targetting a specific device or rather an interface. */
RuleAction::Type RuleAction::type() const
{
    if (!m_deviceId.isNull() && !m_actionTypeId.isNull()) {
        return TypeDevice;
    }
    if (!m_deviceId.isNull() && !m_browserItemId.isEmpty()) {
        return TypeBrowser;
    }
    if (!m_interface.isEmpty() && !m_interfaceAction.isEmpty()) {
        return TypeInterface;
    }
    // uhmm... invalid...
    return TypeDevice;
}

/*! Return true, if this RuleAction contains a \l{RuleActionParam} which is based on an EventTypeId.*/
bool RuleAction::isEventBased() const
{
    foreach (const RuleActionParam &param, m_ruleActionParams) {
        if (param.isEventBased()) {
            return true;
        }
    }
    return false;
}

bool RuleAction::isStateBased() const
{
    foreach (const RuleActionParam &param, m_ruleActionParams) {
        if (param.isStateBased()) {
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

/*! Converts this \l{RuleAction} to a \l{BrowserItemAction}.
 *  \sa BrowserItemAction, */
BrowserItemAction RuleAction::toBrowserItemAction() const
{
    return BrowserItemAction(m_deviceId, m_browserItemId);
}

/*! Returns the actionTypeId of this RuleAction. */
ActionTypeId RuleAction::actionTypeId() const
{
    return m_actionTypeId;
}

void RuleAction::setActionTypeId(const ActionTypeId &actionTypeId)
{
    m_actionTypeId = actionTypeId;
}

/*! Returns the browserItemId of this RuleAction. */
QString RuleAction::browserItemId() const
{
    return m_browserItemId;
}

void RuleAction::setBrowserItemId(const QString &browserItemId)
{
    m_browserItemId = browserItemId;
}

/*! Returns the deviceId of this RuleAction. */
DeviceId RuleAction::deviceId() const
{
    return m_deviceId;
}

void RuleAction::setDeviceId(const DeviceId &deviceId)
{
    m_deviceId = deviceId;
}

/*! Returns the name of the interface associated with this RuleAction. */
QString RuleAction::interface() const
{
    return m_interface;
}

void RuleAction::setInterface(const QString &interface)
{
    m_interface = interface;
}

/*! Returns the name of the action of the associated interface. */
QString RuleAction::interfaceAction() const
{
    return m_interfaceAction;
}

void RuleAction::setInterfaceAction(const QString &interfaceAction)
{
    m_interfaceAction = interfaceAction;
}

/*! Returns the \l{RuleActionParamList} of this RuleAction.
 *  \sa RuleActionParam, */
RuleActionParams RuleAction::ruleActionParams() const
{
    return m_ruleActionParams;
}

/*! Set the \l{RuleActionParamList} of this RuleAction to the given \a ruleActionParams.
 *  \sa RuleActionParam, */
void RuleAction::setRuleActionParams(const RuleActionParams &ruleActionParams)
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
    return RuleActionParam();
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
    return RuleActionParam();
}

/*! Copy the data to a \l{RuleAction} from an \a other rule action. */
void RuleAction::operator=(const RuleAction &other)
{
    m_id = other.id();
    m_actionTypeId = other.actionTypeId();
    m_ruleActionParams = other.ruleActionParams();
}

/*! Print a RuleAction including RuleActionParams to QDebug. */
QDebug operator<<(QDebug dbg, const RuleAction &ruleAction)
{
    dbg.nospace() << "RuleAction(ActionTypeId:" << ruleAction.actionTypeId().toString() << ", DeviceId:" << ruleAction.deviceId().toString() << ", Interface:" << ruleAction.interface() << ", InterfaceAction:" << ruleAction.interfaceAction() << ", BrowserItemId:" << ruleAction.browserItemId() << ")" << endl;
    for (int i = 0; i < ruleAction.ruleActionParams().count(); i++) {
        dbg.nospace() << "    " << i << ": " << ruleAction.ruleActionParams().at(i) << endl;
    }
    return dbg;
}

/*! Print a List of RuleActions with all their contents to QDebug. */
QDebug operator<<(QDebug dbg, const QList<RuleAction> &ruleActionList)
{
    dbg.nospace() << "RuleActionList (count:" << ruleActionList.count() << "):" << endl;
    for (int i = 0; i < ruleActionList.count(); i++ ) {
        dbg.nospace() << "  " << i << ": " << ruleActionList.at(i);
    }
    return dbg;
}

RuleActions::RuleActions()
{

}

RuleActions::RuleActions(const QList<RuleAction> &other): QList<RuleAction>(other)
{

}

QVariant RuleActions::get(int index) const
{
    return QVariant::fromValue(at(index));
}

void RuleActions::put(const QVariant &variant)
{
    append(variant.value<RuleAction>());
}
