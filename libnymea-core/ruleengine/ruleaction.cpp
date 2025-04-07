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

#include "ruleaction.h"

RuleAction::RuleAction(const ActionTypeId &actionTypeId, const ThingId &thingId, const RuleActionParams &params):
    m_thingId(thingId),
    m_actionTypeId(actionTypeId),
    m_ruleActionParams(params)
{

}

RuleAction::RuleAction(const QString &interface, const QString &interfaceAction, const RuleActionParams &params) :
    m_interface(interface),
    m_interfaceAction(interfaceAction),
    m_ruleActionParams(params)
{

}

RuleAction::RuleAction(const ThingId &thingId, const QString &browserItemId):
    m_thingId(thingId),
    m_browserItemId(browserItemId)
{

}

RuleAction::RuleAction(const RuleAction &other) :
    m_thingId(other.thingId()),
    m_actionTypeId(other.actionTypeId()),
    m_browserItemId(other.browserItemId()),
    m_interface(other.interface()),
    m_interfaceAction(other.interfaceAction()),
    m_ruleActionParams(other.ruleActionParams())
{

}

bool RuleAction::isValid() const
{
    return (!m_actionTypeId.isNull() && !m_thingId.isNull())
            || (!m_interface.isEmpty() && !m_interfaceAction.isEmpty())
            || (!m_thingId.isNull() && !m_browserItemId.isEmpty());
}

RuleAction::Type RuleAction::type() const
{
    if (!m_thingId.isNull() && !m_actionTypeId.isNull()) {
        return TypeThing;
    }
    if (!m_thingId.isNull() && !m_browserItemId.isEmpty()) {
        return TypeBrowser;
    }
    if (!m_interface.isEmpty() && !m_interfaceAction.isEmpty()) {
        return TypeInterface;
    }
    // uhmm... invalid...
    return TypeThing;
}

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

Action RuleAction::toAction() const
{
    Action action(m_actionTypeId, m_thingId, Action::TriggeredByRule);
    ParamList params;
    foreach (const RuleActionParam &ruleActionParam, m_ruleActionParams) {
        params.append(Param(ruleActionParam.paramTypeId(), ruleActionParam.value()));
    }
    action.setParams(params);
    return action;
}

BrowserItemAction RuleAction::toBrowserItemAction() const
{
    return BrowserItemAction(m_thingId, m_browserItemId);
}

ActionTypeId RuleAction::actionTypeId() const
{
    return m_actionTypeId;
}

void RuleAction::setActionTypeId(const ActionTypeId &actionTypeId)
{
    m_actionTypeId = actionTypeId;
}

QString RuleAction::browserItemId() const
{
    return m_browserItemId;
}

void RuleAction::setBrowserItemId(const QString &browserItemId)
{
    m_browserItemId = browserItemId;
}

ThingId RuleAction::thingId() const
{
    return m_thingId;
}

void RuleAction::setThingId(const ThingId &thingId)
{
    m_thingId = thingId;
}

QString RuleAction::interface() const
{
    return m_interface;
}

void RuleAction::setInterface(const QString &interface)
{
    m_interface = interface;
}

QString RuleAction::interfaceAction() const
{
    return m_interfaceAction;
}

void RuleAction::setInterfaceAction(const QString &interfaceAction)
{
    m_interfaceAction = interfaceAction;
}

RuleActionParams RuleAction::ruleActionParams() const
{
    return m_ruleActionParams;
}

void RuleAction::setRuleActionParams(const RuleActionParams &ruleActionParams)
{
    m_ruleActionParams = ruleActionParams;
}

RuleActionParam RuleAction::ruleActionParam(const ParamTypeId &ruleActionParamTypeId) const
{
    foreach (const RuleActionParam &ruleActionParam, m_ruleActionParams) {
        if (ruleActionParam.paramTypeId() == ruleActionParamTypeId) {
            return ruleActionParam;
        }
    }
    return RuleActionParam();
}

RuleActionParam RuleAction::ruleActionParam(const QString &ruleActionParamName) const
{
    foreach (const RuleActionParam &ruleActionParam, m_ruleActionParams) {
        if (ruleActionParam.paramName() == ruleActionParamName) {
            return ruleActionParam;
        }
    }
    return RuleActionParam();
}

void RuleAction::operator=(const RuleAction &other)
{
    m_actionTypeId = other.actionTypeId();
    m_ruleActionParams = other.ruleActionParams();
}

QDebug operator<<(QDebug dbg, const RuleAction &ruleAction)
{
    QDebugStateSaver saver(dbg);
    dbg.nospace() << "RuleAction(ActionTypeId:" << ruleAction.actionTypeId().toString() << ", ThingId:" << ruleAction.thingId().toString() << ", Interface:" << ruleAction.interface() << ", InterfaceAction:" << ruleAction.interfaceAction() << ", BrowserItemId:" << ruleAction.browserItemId() << ")" << Qt::endl;
    for (int i = 0; i < ruleAction.ruleActionParams().count(); i++) {
        dbg.nospace() << "    " << i << ": " << ruleAction.ruleActionParams().at(i) << Qt::endl;
    }
    return dbg;
}

QDebug operator<<(QDebug dbg, const QList<RuleAction> &ruleActionList)
{
    QDebugStateSaver saver(dbg);
    dbg.nospace() << "RuleActionList (count:" << ruleActionList.count() << "):" << Qt::endl;
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
