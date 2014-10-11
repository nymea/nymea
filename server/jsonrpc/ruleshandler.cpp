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
    returns.insert("rule", JsonTypes::ruleRef());
    setReturns("GetRuleDetails", returns);

    params.clear(); returns.clear();
    setDescription("AddRule", "Add a rule. You can describe rules by one or many EventDesciptors and a StateEvaluator. Note that only"
                   "one of either eventDescriptor or eventDescriptorList may be passed at a time.");
    params.insert("o:eventDescriptor", JsonTypes::eventDescriptorRef());
    params.insert("o:eventDescriptorList", QVariantList() << JsonTypes::eventDescriptorRef());
    params.insert("o:stateEvaluator", JsonTypes::stateEvaluatorRef());
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
    QVariantMap ruleData;
    if (!rule.id().isNull()) {
        ruleData.insert("rule", JsonTypes::packRule(rule));
    }
    return createReply(ruleData);
}

JsonReply* RulesHandler::AddRule(const QVariantMap &params)
{
    if (params.contains("eventDescriptor") && params.contains("eventDescriptorList")) {
        QVariantMap returns;
        qWarning() << "Only one of eventDesciptor or eventDescriptorList may be used.";
        returns.insert("ruleError", JsonTypes::ruleErrorToString(RuleEngine::RuleErrorInvalidParameter));
        return createReply(returns);
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

    QList<Action> actions;
    QVariantList actionList = params.value("actions").toList();
    foreach (const QVariant &actionVariant, actionList) {
        QVariantMap actionMap = actionVariant.toMap();
        Action action(ActionTypeId(actionMap.value("actionTypeId").toString()), DeviceId(actionMap.value("deviceId").toString()));
        action.setParams(JsonTypes::unpackParams(actionMap.value("params").toList()));
        actions.append(action);
    }

    QVariantMap returns;
    if (actions.count() == 0) {
        returns.insert("ruleErorr", JsonTypes::ruleErrorToString(RuleEngine::RuleErrorMissingParameter));
        return createReply(returns);
    }

    RuleId newRuleId = RuleId::createRuleId();
    RuleEngine::RuleError status = GuhCore::instance()->addRule(newRuleId, eventDescriptorList, actions);
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
