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

#include "ruleshandler.h"

#include "guhcore.h"
#include "ruleengine.h"

#include <QDebug>

RulesHandler::RulesHandler(QObject *parent) :
    JsonHandler(parent)
{
    QVariantMap params;
    QVariantMap returns;

    params.clear(); returns.clear();
    setDescription("GetRules", "Get all configured rules");
    setParams("GetRules", params);
    returns.insert("ruleIds", QVariantList() << JsonTypes::basicTypeToString(JsonTypes::Uuid));
    setReturns("GetRules", returns);

    params.clear(); returns.clear();
    setDescription("GetRuleDetails", "Get details for the rule identified by ruleId");
    params.insert("ruleId", JsonTypes::basicTypeToString(JsonTypes::Uuid));
    setParams("GetRuleDetails", params);
    returns.insert("o:rule", JsonTypes::ruleRef());
    returns.insert("ruleError", JsonTypes::ruleErrorRef());
    setReturns("GetRuleDetails", returns);

    params.clear(); returns.clear();
    setDescription("AddRule", "Add a rule. You can describe rules by one or many EventDesciptors and a StateEvaluator. Note that only"
                   "one of either eventDescriptor or eventDescriptorList may be passed at a time. A rule can be created but left disabled,"
                   "meaning it won't actually be executed until set to enabled. If not given, enabled defaults to true.");
    params.insert("o:eventDescriptor", JsonTypes::eventDescriptorRef());
    params.insert("o:eventDescriptorList", QVariantList() << JsonTypes::eventDescriptorRef());
    params.insert("o:stateEvaluator", JsonTypes::stateEvaluatorRef());
    params.insert("o:exitActions", QVariantList() << JsonTypes::actionRef());
    params.insert("o:enabled", JsonTypes::basicTypeToString(JsonTypes::Bool));
    params.insert("name", JsonTypes::basicTypeToString(JsonTypes::String));
    QVariantList actions;
    actions.append(JsonTypes::actionRef());
    params.insert("actions", actions);
    setParams("AddRule", params);
    returns.insert("ruleError", JsonTypes::ruleErrorRef());
    returns.insert("o:ruleId", JsonTypes::basicTypeToString(JsonTypes::Uuid));
    setReturns("AddRule", returns);

    params.clear(); returns.clear();
    setDescription("RemoveRule", "Remove a rule");
    params.insert("ruleId", JsonTypes::basicTypeToString(JsonTypes::Uuid));
    setParams("RemoveRule", params);
    returns.insert("ruleError", JsonTypes::ruleErrorRef());
    setReturns("RemoveRule", returns);

    params.clear(); returns.clear();
    setDescription("FindRules", "Find a list of rules containing any of the given parameters.");
    params.insert("deviceId", JsonTypes::basicTypeToString(JsonTypes::Uuid));
    setParams("FindRules", params);
    returns.insert("ruleIds", QVariantList() << JsonTypes::basicTypeToString(JsonTypes::Uuid));
    setReturns("FindRules", returns);

    params.clear(); returns.clear();
    setDescription("EnableRule", "Enabled a rule that has previously been disabled.");
    params.insert("ruleId", JsonTypes::basicTypeToString(JsonTypes::Uuid));
    setParams("EnableRule", params);
    returns.insert("ruleError", JsonTypes::ruleErrorRef());
    setReturns("EnableRule", returns);

    params.clear(); returns.clear();
    setDescription("DisableRule", "Disable a rule. The rule won't be triggered by it's events or state changes while it is disabled.");
    params.insert("ruleId", JsonTypes::basicTypeToString(JsonTypes::Uuid));
    setParams("DisableRule", params);
    returns.insert("ruleError", JsonTypes::ruleErrorRef());
    setReturns("DisableRule", returns);
}

QString RulesHandler::name() const
{
    return "Rules";
}

JsonReply* RulesHandler::GetRules(const QVariantMap &params)
{
    Q_UNUSED(params)

    QVariantList rulesList;
    foreach (const RuleId &ruleId, GuhCore::instance()->ruleIds()) {
        rulesList.append(ruleId);
    }
    QVariantMap returns;
    returns.insert("ruleIds", rulesList);

    return createReply(returns);
}

JsonReply *RulesHandler::GetRuleDetails(const QVariantMap &params)
{
    RuleId ruleId = RuleId(params.value("ruleId").toString());
    Rule rule = GuhCore::instance()->findRule(ruleId);
    if (rule.id().isNull()) {
        return createReply(statusToReply(RuleEngine::RuleErrorRuleNotFound));
    }
    QVariantMap returns = statusToReply(RuleEngine::RuleErrorNoError);
    returns.insert("rule", JsonTypes::packRule(rule));
    return createReply(returns);
}

JsonReply* RulesHandler::AddRule(const QVariantMap &params)
{
    if (params.contains("eventDescriptor") && params.contains("eventDescriptorList")) {
        QVariantMap returns;
        qWarning() << "Only one of eventDesciptor or eventDescriptorList may be used.";
        returns.insert("ruleError", JsonTypes::ruleErrorToString(RuleEngine::RuleErrorInvalidParameter));
        return createReply(returns);
    }

    if (params.contains("eventDescriptor") || params.contains("eventDescriptorList")) {
        if (params.contains("exitActions")) {
            QVariantMap returns;
            qWarning() << "The exitActions will never executed if this rule contains any eventDescriptor.";
            returns.insert("ruleError", JsonTypes::ruleErrorToString(RuleEngine::RuleErrorInvalidRuleFormat));
            return createReply(returns);
        }
    }


    QList<EventDescriptor> eventDescriptorList;
    if (params.contains("eventDescriptor")) {
        QVariantMap eventMap = params.value("eventDescriptor").toMap();
        eventDescriptorList.append(JsonTypes::unpackEventDescriptor(eventMap));
    } else if (params.contains("eventDescriptorList")) {
        foreach (const QVariant &eventVariant, params.value("eventDescriptorList").toList()) {
            QVariantMap eventMap = eventVariant.toMap();
            eventDescriptorList.append(JsonTypes::unpackEventDescriptor(eventMap));
        }
    }

    qDebug() << "unpacking:" << params.value("stateEvaluator").toMap();
    StateEvaluator stateEvaluator = JsonTypes::unpackStateEvaluator(params.value("stateEvaluator").toMap());

    QList<RuleAction> actions;
    QVariantList actionList = params.value("actions").toList();
    foreach (const QVariant &actionVariant, actionList) {
        QVariantMap actionMap = actionVariant.toMap();
        RuleAction action(ActionTypeId(actionMap.value("actionTypeId").toString()), DeviceId(actionMap.value("deviceId").toString()));
        qDebug() << "params from json" << actionMap.value("ruleActionParams");
        action.setRuleActionParams(JsonTypes::unpackRuleActionParams(actionMap.value("ruleActionParams").toList()));
        qDebug() << "params in action" << action.ruleActionParams();
        actions.append(action);
    }

    // check possible eventTypeIds in params
    foreach (const RuleAction &ruleAction, actions) {
        if (ruleAction.isEventBased()) {
            foreach (const RuleActionParam &ruleActionParam, ruleAction.ruleActionParams()) {
                if (ruleActionParam.eventTypeId() != EventTypeId()) {
                    // We have an eventTypeId
                    if (eventDescriptorList.isEmpty()) {
                        QVariantMap returns;
                        qWarning() << "RuleAction" << ruleAction.actionTypeId() << "contains an eventTypeId, but there areno eventDescriptors.";
                        returns.insert("ruleErorr", JsonTypes::ruleErrorToString(RuleEngine::RuleErrorInvalidRuleActionPatameter));
                        return createReply(returns);
                    }
                    // now check if this eventType is in the eventDescriptorList of this rule
                    foreach (const EventDescriptor eventDescriptor, eventDescriptorList) {
                        if (eventDescriptor.eventTypeId() == ruleActionParam.eventTypeId()) {
                            continue;
                        } else {
                            // the given eventTypeId is not in the eventDescriptorList
                            QVariantMap returns;
                            qWarning() << "eventTypeId from RuleAction" << ruleAction.actionTypeId() << "missing in eventDescriptors.";
                            returns.insert("ruleErorr", JsonTypes::ruleErrorToString(RuleEngine::RuleErrorInvalidRuleActionPatameter));
                            return createReply(returns);
                        }
                    }
                }
            }
        }
    }


    QVariantMap returns;
    if (actions.count() == 0) {
        returns.insert("ruleErorr", JsonTypes::ruleErrorToString(RuleEngine::RuleErrorMissingParameter));
        return createReply(returns);
    }

    QList<RuleAction> exitActions;
    if (params.contains("exitActions")) {
        QVariantList exitActionList = params.value("exitActions").toList();
        foreach (const QVariant &actionVariant, exitActionList) {
            QVariantMap actionMap = actionVariant.toMap();
            RuleAction action(ActionTypeId(actionMap.value("actionTypeId").toString()), DeviceId(actionMap.value("deviceId").toString()));
            qDebug() << "params from json" << actionMap.value("ruleActionParams");
            action.setRuleActionParams(JsonTypes::unpackRuleActionParams(actionMap.value("ruleActionParams").toList()));
            qDebug() << "params in action" << action.ruleActionParams();
            exitActions.append(action);
        }
    }


    QString name = params.value("name", QString()).toString();
    bool enabled = params.value("enabled", true).toBool();

    RuleId newRuleId = RuleId::createRuleId();
    RuleEngine::RuleError status = GuhCore::instance()->addRule(newRuleId, name, eventDescriptorList, stateEvaluator, actions, exitActions, enabled);
    if (status ==  RuleEngine::RuleErrorNoError) {
        returns.insert("ruleId", newRuleId.toString());
    }
    returns.insert("ruleError", JsonTypes::ruleErrorToString(status));
    return createReply(returns);
}

JsonReply* RulesHandler::RemoveRule(const QVariantMap &params)
{
    QVariantMap returns;
    RuleId ruleId(params.value("ruleId").toString());
    RuleEngine::RuleError status = GuhCore::instance()->removeRule(ruleId);
    returns.insert("ruleError", JsonTypes::ruleErrorToString(status));
    return createReply(returns);
}

JsonReply *RulesHandler::FindRules(const QVariantMap &params)
{
    DeviceId deviceId = DeviceId(params.value("deviceId").toString());
    QList<RuleId> rules = GuhCore::instance()->findRules(deviceId);

    QVariantList rulesList;
    foreach (const RuleId &ruleId, rules) {
        rulesList.append(ruleId);
    }

    QVariantMap returns;
    returns.insert("ruleIds", rulesList);
    return createReply(returns);
}

JsonReply *RulesHandler::EnableRule(const QVariantMap &params)
{
    return createReply(statusToReply(GuhCore::instance()->enableRule(RuleId(params.value("ruleId").toString()))));
}

JsonReply *RulesHandler::DisableRule(const QVariantMap &params)
{
    return createReply(statusToReply(GuhCore::instance()->disableRule(RuleId(params.value("ruleId").toString()))));
}
