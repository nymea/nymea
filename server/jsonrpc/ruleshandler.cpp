/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stuerz <simon.stuerz@guh.guru>                *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
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
#include "loggingcategories.h"

#include <QDebug>

namespace guhserver {

RulesHandler::RulesHandler(QObject *parent) :
    JsonHandler(parent)
{
    QVariantMap params;
    QVariantMap returns;

    params.clear(); returns.clear();
    setDescription("GetRules", "Get the descriptions of all configured rules. If you need more information about a specific rule use the "
                   "method Rules.GetRuleDetails.");
    setParams("GetRules", params);
    returns.insert("ruleDescriptions", QVariantList() << JsonTypes::ruleDescriptionRef());
    setReturns("GetRules", returns);

    params.clear(); returns.clear();
    setDescription("GetRuleDetails", "Get details for the rule identified by ruleId");
    params.insert("ruleId", JsonTypes::basicTypeToString(JsonTypes::Uuid));
    setParams("GetRuleDetails", params);
    returns.insert("o:rule", JsonTypes::ruleRef());
    returns.insert("ruleError", JsonTypes::ruleErrorRef());
    setReturns("GetRuleDetails", returns);

    params.clear(); returns.clear();
    setDescription("AddRule", "Add a rule. You can describe rules by one or many EventDesciptors and a StateEvaluator. Note that only "
                   "one of either eventDescriptor or eventDescriptorList may be passed at a time. A rule can be created but left disabled, "
                   "meaning it won't actually be executed until set to enabled. If not given, enabled defaults to true.");
    params.insert("o:eventDescriptor", JsonTypes::eventDescriptorRef());
    params.insert("o:eventDescriptorList", QVariantList() << JsonTypes::eventDescriptorRef());
    params.insert("o:stateEvaluator", JsonTypes::stateEvaluatorRef());
    params.insert("o:exitActions", QVariantList() << JsonTypes::ruleActionRef());
    params.insert("o:enabled", JsonTypes::basicTypeToString(JsonTypes::Bool));
    params.insert("name", JsonTypes::basicTypeToString(JsonTypes::String));
    QVariantList actions;
    actions.append(JsonTypes::ruleActionRef());
    params.insert("actions", actions);
    setParams("AddRule", params);
    returns.insert("ruleError", JsonTypes::ruleErrorRef());
    returns.insert("o:ruleId", JsonTypes::basicTypeToString(JsonTypes::Uuid));
    setReturns("AddRule", returns);

    params.clear(); returns.clear(); actions.clear();
    setDescription("EditRule", "Edit the parameters of a rule. The configuration of the rule with the given ruleId "
                   "will be replaced with the new given configuration. In ordert to enable or disable a Rule, please use the "
                   "methods \"Rules.EnableRule\" and \"Rules.DisableRule\". If successfull, the notification \"Rule.RuleConfigurationChanged\" "
                   "will be emitted.");
    params.insert("ruleId", JsonTypes::basicTypeToString(JsonTypes::Uuid));
    params.insert("name", JsonTypes::basicTypeToString(JsonTypes::String));
    params.insert("o:eventDescriptor", JsonTypes::eventDescriptorRef());
    params.insert("o:eventDescriptorList", QVariantList() << JsonTypes::eventDescriptorRef());
    params.insert("o:stateEvaluator", JsonTypes::stateEvaluatorRef());
    params.insert("o:exitActions", QVariantList() << JsonTypes::ruleActionRef());
    params.insert("o:enabled", JsonTypes::basicTypeToString(JsonTypes::Bool));
    actions.append(JsonTypes::ruleActionRef());
    params.insert("actions", actions);
    setParams("EditRule", params);
    returns.insert("ruleError", JsonTypes::ruleErrorRef());
    returns.insert("o:rule", JsonTypes::ruleRef());
    setReturns("EditRule", returns);

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
    setDescription("EnableRule", "Enabled a rule that has previously been disabled."
                   "If successfull, the notification \"Rule.RuleConfigurationChanged\" will be emitted.");
    params.insert("ruleId", JsonTypes::basicTypeToString(JsonTypes::Uuid));
    setParams("EnableRule", params);
    returns.insert("ruleError", JsonTypes::ruleErrorRef());
    setReturns("EnableRule", returns);

    params.clear(); returns.clear();
    setDescription("DisableRule", "Disable a rule. The rule won't be triggered by it's events or state changes while it is disabled. "
                   "If successfull, the notification \"Rule.RuleConfigurationChanged\" will be emitted.");
    params.insert("ruleId", JsonTypes::basicTypeToString(JsonTypes::Uuid));
    setParams("DisableRule", params);
    returns.insert("ruleError", JsonTypes::ruleErrorRef());
    setReturns("DisableRule", returns);

    // Notifications
    params.clear(); returns.clear();
    setDescription("RuleRemoved", "Emitted whenever a Rule was removed.");
    params.insert("ruleId", JsonTypes::basicTypeToString(JsonTypes::Uuid));
    setParams("RuleRemoved", params);

    params.clear(); returns.clear();
    setDescription("RuleAdded", "Emitted whenever a Rule was added.");
    params.insert("rule", JsonTypes::ruleRef());
    setParams("RuleAdded", params);

    params.clear(); returns.clear();
    setDescription("RuleActiveChanged", "Emitted whenever the active state of a Rule changed.");
    params.insert("ruleId", JsonTypes::basicTypeToString(JsonTypes::Uuid));
    params.insert("active", JsonTypes::basicTypeToString(JsonTypes::Bool));
    setParams("RuleActiveChanged", params);

    params.clear(); returns.clear();
    setDescription("RuleConfigurationChanged", "Emitted whenever the configuration of a Rule changed.");
    params.insert("rule", JsonTypes::ruleRef());
    setParams("RuleConfigurationChanged", params);

    connect(GuhCore::instance(), &GuhCore::ruleAdded, this, &RulesHandler::ruleAddedNotification);
    connect(GuhCore::instance(), &GuhCore::ruleRemoved, this, &RulesHandler::ruleRemovedNotification);
    connect(GuhCore::instance(), &GuhCore::ruleActiveChanged, this, &RulesHandler::ruleActiveChangedNotification);
    connect(GuhCore::instance(), &GuhCore::ruleConfigurationChanged, this, &RulesHandler::ruleConfigurationChangedNotification);
}

QString RulesHandler::name() const
{
    return "Rules";
}

JsonReply* RulesHandler::GetRules(const QVariantMap &params)
{
    Q_UNUSED(params)

    QVariantMap returns;
    returns.insert("ruleDescriptions", JsonTypes::packRuleDescriptions());

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
    // check rule consistency
    RuleEngine::RuleError ruleConsistencyError = verifyRuleConsistency(params);
    if (ruleConsistencyError !=  RuleEngine::RuleErrorNoError) {
        QVariantMap returns;
        returns.insert("ruleError", JsonTypes::ruleErrorToString(ruleConsistencyError));
        return createReply(returns);
    }

    // Check and upack eventDescriptorList
    QPair<QList<EventDescriptor>, RuleEngine::RuleError> eventDescriptorVerification = verifyEventDescriptors(params);
    QList<EventDescriptor> eventDescriptorList = eventDescriptorVerification.first;
    if (eventDescriptorVerification.second != RuleEngine::RuleErrorNoError) {
        QVariantMap returns;
        returns.insert("ruleError", JsonTypes::ruleErrorToString(eventDescriptorVerification.second));
        return createReply(returns);
    }


    // Check and unpack stateEvaluator
    qCDebug(dcJsonRpc) << "unpacking stateEvaluator:" << params.value("stateEvaluator").toMap();
    StateEvaluator stateEvaluator = JsonTypes::unpackStateEvaluator(params.value("stateEvaluator").toMap());

    // Check and unpack actions
    QPair<QList<RuleAction>, RuleEngine::RuleError> actionsVerification = verifyActions(params, eventDescriptorList);
    QList<RuleAction> actions = actionsVerification.first;
    if (actionsVerification.second != RuleEngine::RuleErrorNoError) {
        QVariantMap returns;
        returns.insert("ruleError", JsonTypes::ruleErrorToString(actionsVerification.second));
        return createReply(returns);
    }

    // Check and unpack exitActions
    QPair<QList<RuleAction>, RuleEngine::RuleError> exitActionsVerification = verifyExitActions(params);
    QList<RuleAction> exitActions = exitActionsVerification.first;
    if (exitActionsVerification.second != RuleEngine::RuleErrorNoError) {
        QVariantMap returns;
        returns.insert("ruleError", JsonTypes::ruleErrorToString(exitActionsVerification.second));
        return createReply(returns);
    }

    QString name = params.value("name", QString()).toString();
    bool enabled = params.value("enabled", true).toBool();

    RuleId newRuleId = RuleId::createRuleId();
    RuleEngine::RuleError status = GuhCore::instance()->addRule(newRuleId, name, eventDescriptorList, stateEvaluator, actions, exitActions, enabled);
    QVariantMap returns;
    if (status ==  RuleEngine::RuleErrorNoError) {
        returns.insert("ruleId", newRuleId.toString());
    }
    returns.insert("ruleError", JsonTypes::ruleErrorToString(status));
    return createReply(returns);
}

JsonReply *RulesHandler::EditRule(const QVariantMap &params)
{
    // check rule consistency
    RuleEngine::RuleError ruleConsistencyError = verifyRuleConsistency(params);
    if (ruleConsistencyError !=  RuleEngine::RuleErrorNoError) {
        QVariantMap returns;
        returns.insert("ruleError", JsonTypes::ruleErrorToString(ruleConsistencyError));
        return createReply(returns);
    }

    // Check and upack eventDescriptorList
    QPair<QList<EventDescriptor>, RuleEngine::RuleError> eventDescriptorVerification = verifyEventDescriptors(params);
    QList<EventDescriptor> eventDescriptorList = eventDescriptorVerification.first;
    if (eventDescriptorVerification.second != RuleEngine::RuleErrorNoError) {
        QVariantMap returns;
        returns.insert("ruleError", JsonTypes::ruleErrorToString(eventDescriptorVerification.second));
        return createReply(returns);
    }

    // Check and unpack stateEvaluator
    qCDebug(dcJsonRpc) << "unpacking stateEvaluator:" << params.value("stateEvaluator").toMap();
    StateEvaluator stateEvaluator = JsonTypes::unpackStateEvaluator(params.value("stateEvaluator").toMap());

    // Check and unpack actions
    QPair<QList<RuleAction>, RuleEngine::RuleError> actionsVerification = verifyActions(params, eventDescriptorList);
    QList<RuleAction> actions = actionsVerification.first;
    if (actionsVerification.second != RuleEngine::RuleErrorNoError) {
        QVariantMap returns;
        returns.insert("ruleError", JsonTypes::ruleErrorToString(actionsVerification.second));
        return createReply(returns);
    }

    // Check and unpack exitActions
    QPair<QList<RuleAction>, RuleEngine::RuleError> exitActionsVerification = verifyExitActions(params);
    QList<RuleAction> exitActions = exitActionsVerification.first;
    if (exitActionsVerification.second != RuleEngine::RuleErrorNoError) {
        QVariantMap returns;
        returns.insert("ruleError", JsonTypes::ruleErrorToString(exitActionsVerification.second));
        return createReply(returns);
    }

    QString name = params.value("name", QString()).toString();
    bool enabled = params.value("enabled", true).toBool();

    RuleId ruleId = RuleId(params.value("ruleId").toString());
    RuleEngine::RuleError status = GuhCore::instance()->editRule(ruleId, name, eventDescriptorList, stateEvaluator, actions, exitActions, enabled);
    QVariantMap returns;
    if (status ==  RuleEngine::RuleErrorNoError) {
        returns.insert("rule", JsonTypes::packRule(GuhCore::instance()->findRule(ruleId)));
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

QVariant::Type RulesHandler::getActionParamType(const ActionTypeId &actionTypeId, const QString &paramName)
{
    foreach (const DeviceClass &deviceClass, GuhCore::instance()->supportedDevices()) {
        foreach (const ActionType &actionType, deviceClass.actionTypes()) {
            if (actionType.id() == actionTypeId) {
                foreach (const ParamType &paramType, actionType.paramTypes()) {
                    if (paramType.name() == paramName) {
                        return paramType.type();
                    }
                }
            }
        }
    }
    return QVariant::Invalid;
}

QVariant::Type RulesHandler::getEventParamType(const EventTypeId &eventTypeId, const QString &paramName)
{
    foreach (const DeviceClass &deviceClass, GuhCore::instance()->supportedDevices()) {
        foreach (const EventType &eventType, deviceClass.eventTypes()) {
            if (eventType.id() == eventTypeId) {
                foreach (const ParamType &paramType, eventType.paramTypes()) {
                    // get ParamType of Event
                    if (paramType.name() == paramName) {
                        return paramType.type();
                    }
                }
            }
        }
    }
    return QVariant::Invalid;
}

bool RulesHandler::checkEventDescriptors(const QList<EventDescriptor> eventDescriptors, const EventTypeId &eventTypeId)
{
    foreach (const EventDescriptor eventDescriptor, eventDescriptors) {
        if (eventDescriptor.eventTypeId() == eventTypeId) {
            return true;
        }
    }
    return false;
}

RuleEngine::RuleError RulesHandler::verifyRuleConsistency(const QVariantMap &params)
{
    // check if there are an eventDescriptor and an eventDescriptorList
    if (params.contains("eventDescriptor") && params.contains("eventDescriptorList")) {
        qCWarning(dcJsonRpc) << "Only one of eventDesciptor or eventDescriptorList may be used.";
        return RuleEngine::RuleErrorInvalidParameter;
    }

    // check if this rules is based on any event and contains exit actions
    if (params.contains("eventDescriptor") || params.contains("eventDescriptorList")) {
        if (params.contains("exitActions")) {
            qCWarning(dcJsonRpc) << "The exitActions will never be executed if the rule contains an eventDescriptor.";
            return RuleEngine::RuleErrorInvalidRuleFormat;
        }
    }

    // check if there are any actions
    if (params.value("actions").toList().isEmpty()) {
        qCWarning(dcJsonRpc) << "Rule actions missing. A rule without actions has no effect.";
        return RuleEngine::RuleErrorMissingParameter;
    }

    // TODO: check if events and stateEvaluators are missing

    return RuleEngine::RuleErrorNoError;
}

QPair<QList<EventDescriptor>, RuleEngine::RuleError> RulesHandler::verifyEventDescriptors(const QVariantMap &params)
{
    // Check and unpack eventDescriptors
    QList<EventDescriptor> eventDescriptorList = QList<EventDescriptor>();
    if (params.contains("eventDescriptor")) {
        QVariantMap eventMap = params.value("eventDescriptor").toMap();
        qCDebug(dcJsonRpc) << "unpacking eventDescriptor" << eventMap;
        eventDescriptorList.append(JsonTypes::unpackEventDescriptor(eventMap));
    } else if (params.contains("eventDescriptorList")) {
        QVariantList eventDescriptors = params.value("eventDescriptorList").toList();
        qCDebug(dcJsonRpc) << "unpacking eventDescriptorList:" << eventDescriptors;
        foreach (const QVariant &eventVariant, eventDescriptors) {
            QVariantMap eventMap = eventVariant.toMap();
            eventDescriptorList.append(JsonTypes::unpackEventDescriptor(eventMap));
        }
    }
    return QPair<QList<EventDescriptor>, RuleEngine::RuleError>(eventDescriptorList, RuleEngine::RuleErrorNoError);
}

QPair<QList<RuleAction>, RuleEngine::RuleError> RulesHandler::verifyActions(const QVariantMap &params, const QList<EventDescriptor> &eventDescriptorList)
{
    QList<RuleAction> actions;
    QVariantList actionList = params.value("actions").toList();
    qCDebug(dcJsonRpc) << "unpacking actions:" << actionList;
    foreach (const QVariant &actionVariant, actionList) {
        QVariantMap actionMap = actionVariant.toMap();
        RuleAction action(ActionTypeId(actionMap.value("actionTypeId").toString()), DeviceId(actionMap.value("deviceId").toString()));
        RuleActionParamList actionParamList = JsonTypes::unpackRuleActionParams(actionMap.value("ruleActionParams").toList());
        foreach (const RuleActionParam &ruleActionParam, actionParamList) {
            if (!ruleActionParam.isValid()) {
                qCWarning(dcJsonRpc) << "got an actionParam with value AND eventTypeId!";
                return QPair<QList<RuleAction>, RuleEngine::RuleError>(actions, RuleEngine::RuleErrorInvalidRuleActionParameter);
            }
        }
        qCDebug(dcJsonRpc) << "params in exitAction" << action.ruleActionParams();
        action.setRuleActionParams(actionParamList);
        actions.append(action);
    }

    // check possible eventTypeIds in params
    foreach (const RuleAction &ruleAction, actions) {
        if (ruleAction.isEventBased()) {
            foreach (const RuleActionParam &ruleActionParam, ruleAction.ruleActionParams()) {
                if (ruleActionParam.eventTypeId() != EventTypeId()) {
                    // We have an eventTypeId
                    if (eventDescriptorList.isEmpty()) {
                        qCWarning(dcJsonRpc) << "RuleAction" << ruleAction.actionTypeId() << "contains an eventTypeId, but there are no eventDescriptors.";
                        return QPair<QList<RuleAction>, RuleEngine::RuleError>(actions, RuleEngine::RuleErrorInvalidRuleActionParameter);
                    }
                    // now check if this eventType is in the eventDescriptorList of this rule
                    if (!checkEventDescriptors(eventDescriptorList, ruleActionParam.eventTypeId())) {
                        qCWarning(dcJsonRpc) << "eventTypeId from RuleAction" << ruleAction.actionTypeId() << "missing in eventDescriptors.";
                        return QPair<QList<RuleAction>, RuleEngine::RuleError>(actions, RuleEngine::RuleErrorInvalidRuleActionParameter);
                    }

                    // check if the param type of the event and the action match
                    QVariant::Type eventParamType = getEventParamType(ruleActionParam.eventTypeId(), ruleActionParam.eventParamName());
                    QVariant::Type actionParamType = getActionParamType(ruleAction.actionTypeId(), ruleActionParam.name());
                    if (eventParamType != actionParamType) {
                        qCWarning(dcJsonRpc) << "RuleActionParam" << ruleActionParam.name() << " and given event param " << ruleActionParam.eventParamName() << "have not the same type:";
                        qCWarning(dcJsonRpc) << "        -> actionParamType:" << actionParamType;
                        qCWarning(dcJsonRpc) << "        ->  eventParamType:" << eventParamType;
                        return QPair<QList<RuleAction>, RuleEngine::RuleError>(actions, RuleEngine::RuleErrorTypesNotMatching);
                    }
                }
            }
        }
    }
    return QPair<QList<RuleAction>, RuleEngine::RuleError>(actions, RuleEngine::RuleErrorNoError);
}

QPair<QList<RuleAction>, RuleEngine::RuleError> RulesHandler::verifyExitActions(const QVariantMap &params)
{
    QList<RuleAction> exitActions;
    if (params.contains("exitActions")) {
        QVariantList exitActionList = params.value("exitActions").toList();
        qCDebug(dcJsonRpc) << "unpacking exitActions:" << exitActionList;
        foreach (const QVariant &actionVariant, exitActionList) {
            QVariantMap actionMap = actionVariant.toMap();
            RuleAction action(ActionTypeId(actionMap.value("actionTypeId").toString()), DeviceId(actionMap.value("deviceId").toString()));
            if (action.isEventBased()) {
                qCWarning(dcJsonRpc) << "got exitAction with a param value containing an eventTypeId!";
                return QPair<QList<RuleAction>, RuleEngine::RuleError>(exitActions, RuleEngine::RuleErrorInvalidRuleActionParameter);
            }
            qCDebug(dcJsonRpc) << "params in exitAction" << action.ruleActionParams();
            action.setRuleActionParams(JsonTypes::unpackRuleActionParams(actionMap.value("ruleActionParams").toList()));
            exitActions.append(action);
        }
    }
    return QPair<QList<RuleAction>, RuleEngine::RuleError>(exitActions, RuleEngine::RuleErrorNoError);
}

void RulesHandler::ruleRemovedNotification(const RuleId &ruleId)
{
    QVariantMap params;
    params.insert("ruleId", ruleId);

    emit RuleRemoved(params);
}

void RulesHandler::ruleAddedNotification(const Rule &rule)
{
    QVariantMap params;
    params.insert("rule", JsonTypes::packRule(rule));

    emit RuleAdded(params);
}

void RulesHandler::ruleActiveChangedNotification(const Rule &rule)
{
    QVariantMap params;
    params.insert("ruleId", rule.id());
    params.insert("active", rule.active());

    emit RuleActiveChanged(params);
}

void RulesHandler::ruleConfigurationChangedNotification(const Rule &rule)
{
    QVariantMap params;
    params.insert("rule", JsonTypes::packRule(rule));

    emit RuleConfigurationChanged(params);
}

}
