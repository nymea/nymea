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

/*!
    \class guhserver::RulesHandler
    \brief This subclass of \l{JsonHandler} processes the JSON requests for the \tt Rules namespace of the JSON-RPC API.

    \ingroup json
    \inmodule core

    This \l{JsonHandler} will be created in the \l{JsonRPCServer} and used to handle JSON-RPC requests
    for the \tt {Rules} namespace of the API.

    \sa RuleEngine, JsonHandler, JsonRPCServer
*/

/*! \fn void guhserver::RulesHandler::RuleRemoved(const QVariantMap &params);
    This signal is emitted to the API notifications when a \l{Rule} was removed.
    The \a params contain the map for the notification.
*/

/*! \fn void guhserver::RulesHandler::RuleAdded(const QVariantMap &params);
    This signal is emitted to the API notifications when a \l{Rule} was added.
    The \a params contain the map for the notification.
*/

/*! \fn void guhserver::RulesHandler::RuleActiveChanged(const QVariantMap &params);
    This signal is emitted to the API notifications when a \l{Rule} has changed the active status.
    The \a params contain the map for the notification.
*/

/*! \fn void guhserver::RulesHandler::RuleConfigurationChanged(const QVariantMap &params);
    This signal is emitted to the API notifications when a \l{Rule} has changed the configuration.
    The \a params contain the map for the notification.
*/

#include "ruleshandler.h"
#include "guhcore.h"
#include "ruleengine.h"
#include "loggingcategories.h"

#include <QDebug>

namespace guhserver {

/*! Constructs a new \l{RulesHandler} with the given \a parent. */
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
    params.insert("o:executable", JsonTypes::basicTypeToString(JsonTypes::Bool));
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
    params.insert("o:executable", JsonTypes::basicTypeToString(JsonTypes::Bool));
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

    params.clear(); returns.clear();
    setDescription("ExecuteActions", "Execute the action list of the rule with the given ruleId.");
    params.insert("ruleId", JsonTypes::basicTypeToString(JsonTypes::Uuid));
    setParams("ExecuteActions", params);
    returns.insert("ruleError", JsonTypes::ruleErrorRef());
    setReturns("ExecuteActions", returns);

    params.clear(); returns.clear();
    setDescription("ExecuteExitActions", "Execute the exit action list of the rule with the given ruleId.");
    params.insert("ruleId", JsonTypes::basicTypeToString(JsonTypes::Uuid));
    setParams("ExecuteExitActions", params);
    returns.insert("ruleError", JsonTypes::ruleErrorRef());
    setReturns("ExecuteExitActions", returns);

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

/*! Returns the name of the \l{RulesHandler}. In this case \b Rules.*/
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
    Rule rule = GuhCore::instance()->ruleEngine()->findRule(ruleId);
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
    RuleEngine::RuleError ruleConsistencyError = JsonTypes::verifyRuleConsistency(params);
    if (ruleConsistencyError !=  RuleEngine::RuleErrorNoError) {
        QVariantMap returns;
        returns.insert("ruleError", JsonTypes::ruleErrorToString(ruleConsistencyError));
        return createReply(returns);
    }

    // Check and upack eventDescriptorList
    QPair<QList<EventDescriptor>, RuleEngine::RuleError> eventDescriptorVerification = JsonTypes::verifyEventDescriptors(params);
    QList<EventDescriptor> eventDescriptorList = eventDescriptorVerification.first;
    if (eventDescriptorVerification.second != RuleEngine::RuleErrorNoError) {
        QVariantMap returns;
        returns.insert("ruleError", JsonTypes::ruleErrorToString(eventDescriptorVerification.second));
        return createReply(returns);
    }

    // Check and unpack stateEvaluator
    qCDebug(dcJsonRpc) << "unpacking stateEvaluator:" << params.value("stateEvaluator").toMap();
    StateEvaluator stateEvaluator = JsonTypes::unpackStateEvaluator(params.value("stateEvaluator").toMap());
    if (!stateEvaluator.isValid()) {
        QVariantMap returns;
        returns.insert("ruleError", JsonTypes::ruleErrorToString(RuleEngine::RuleErrorInvalidStateEvaluatorValue));
        return createReply(returns);
    }

    // Check and unpack actions
    QPair<QList<RuleAction>, RuleEngine::RuleError> actionsVerification = JsonTypes::verifyActions(params, eventDescriptorList);
    QList<RuleAction> actions = actionsVerification.first;
    if (actionsVerification.second != RuleEngine::RuleErrorNoError) {
        QVariantMap returns;
        returns.insert("ruleError", JsonTypes::ruleErrorToString(actionsVerification.second));
        return createReply(returns);
    }

    // Check and unpack exitActions
    QPair<QList<RuleAction>, RuleEngine::RuleError> exitActionsVerification = JsonTypes::verifyExitActions(params);
    QList<RuleAction> exitActions = exitActionsVerification.first;
    if (exitActionsVerification.second != RuleEngine::RuleErrorNoError) {
        QVariantMap returns;
        returns.insert("ruleError", JsonTypes::ruleErrorToString(exitActionsVerification.second));
        return createReply(returns);
    }

    QString name = params.value("name", QString()).toString();
    bool enabled = params.value("enabled", true).toBool();
    bool executable = params.value("executable", true).toBool();

    RuleId newRuleId = RuleId::createRuleId();
    RuleEngine::RuleError status = GuhCore::instance()->ruleEngine()->addRule(newRuleId, name, eventDescriptorList, stateEvaluator, actions, exitActions, enabled, executable);
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
    RuleEngine::RuleError ruleConsistencyError = JsonTypes::verifyRuleConsistency(params);
    if (ruleConsistencyError !=  RuleEngine::RuleErrorNoError) {
        QVariantMap returns;
        returns.insert("ruleError", JsonTypes::ruleErrorToString(ruleConsistencyError));
        return createReply(returns);
    }

    // Check and upack eventDescriptorList
    QPair<QList<EventDescriptor>, RuleEngine::RuleError> eventDescriptorVerification = JsonTypes::verifyEventDescriptors(params);
    QList<EventDescriptor> eventDescriptorList = eventDescriptorVerification.first;
    if (eventDescriptorVerification.second != RuleEngine::RuleErrorNoError) {
        QVariantMap returns;
        returns.insert("ruleError", JsonTypes::ruleErrorToString(eventDescriptorVerification.second));
        return createReply(returns);
    }

    // Check and unpack stateEvaluator
    qCDebug(dcJsonRpc) << "unpacking stateEvaluator:" << params.value("stateEvaluator").toMap();
    StateEvaluator stateEvaluator = JsonTypes::unpackStateEvaluator(params.value("stateEvaluator").toMap());
    if (!stateEvaluator.isValid()) {
        QVariantMap returns;
        returns.insert("ruleError", JsonTypes::ruleErrorToString(RuleEngine::RuleErrorInvalidStateEvaluatorValue));
        return createReply(returns);
    }

    // Check and unpack actions
    QPair<QList<RuleAction>, RuleEngine::RuleError> actionsVerification = JsonTypes::verifyActions(params, eventDescriptorList);
    QList<RuleAction> actions = actionsVerification.first;
    if (actionsVerification.second != RuleEngine::RuleErrorNoError) {
        QVariantMap returns;
        returns.insert("ruleError", JsonTypes::ruleErrorToString(actionsVerification.second));
        return createReply(returns);
    }

    // Check and unpack exitActions
    QPair<QList<RuleAction>, RuleEngine::RuleError> exitActionsVerification = JsonTypes::verifyExitActions(params);
    QList<RuleAction> exitActions = exitActionsVerification.first;
    if (exitActionsVerification.second != RuleEngine::RuleErrorNoError) {
        QVariantMap returns;
        returns.insert("ruleError", JsonTypes::ruleErrorToString(exitActionsVerification.second));
        return createReply(returns);
    }

    QString name = params.value("name", QString()).toString();
    bool enabled = params.value("enabled", true).toBool();
    bool executable = params.value("executable", true).toBool();

    RuleId ruleId = RuleId(params.value("ruleId").toString());
    RuleEngine::RuleError status = GuhCore::instance()->ruleEngine()->editRule(ruleId, name, eventDescriptorList, stateEvaluator, actions, exitActions, enabled, executable);
    QVariantMap returns;
    if (status ==  RuleEngine::RuleErrorNoError) {
        returns.insert("rule", JsonTypes::packRule(GuhCore::instance()->ruleEngine()->findRule(ruleId)));
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
    QList<RuleId> rules = GuhCore::instance()->ruleEngine()->findRules(deviceId);

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
    return createReply(statusToReply(GuhCore::instance()->ruleEngine()->enableRule(RuleId(params.value("ruleId").toString()))));
}

JsonReply *RulesHandler::DisableRule(const QVariantMap &params)
{
    return createReply(statusToReply(GuhCore::instance()->ruleEngine()->disableRule(RuleId(params.value("ruleId").toString()))));
}

JsonReply *RulesHandler::ExecuteActions(const QVariantMap &params)
{
    QVariantMap returns;
    RuleId ruleId(params.value("ruleId").toString());
    RuleEngine::RuleError status = GuhCore::instance()->ruleEngine()->executeActions(ruleId);
    returns.insert("ruleError", JsonTypes::ruleErrorToString(status));
    return createReply(returns);
}

JsonReply *RulesHandler::ExecuteExitActions(const QVariantMap &params)
{
    QVariantMap returns;
    RuleId ruleId(params.value("ruleId").toString());
    RuleEngine::RuleError status = GuhCore::instance()->ruleEngine()->executeExitActions(ruleId);
    returns.insert("ruleError", JsonTypes::ruleErrorToString(status));
    return createReply(returns);
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
