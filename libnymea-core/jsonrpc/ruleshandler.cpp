/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
 *                                                                         *
 *  This file is part of nymea.                                            *
 *                                                                         *
 *  nymea is free software: you can redistribute it and/or modify          *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  nymea is distributed in the hope that it will be useful,               *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with nymea. If not, see <http://www.gnu.org/licenses/>.          *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*!
    \class nymeaserver::RulesHandler
    \brief This subclass of \l{JsonHandler} processes the JSON requests for the \tt Rules namespace of the JSON-RPC API.

    \ingroup json
    \inmodule core

    This \l{JsonHandler} will be created in the \l{JsonRPCServer} and used to handle JSON-RPC requests
    for the \tt {Rules} namespace of the API.

    \sa RuleEngine, JsonHandler, JsonRPCServer
*/

/*! \fn void nymeaserver::RulesHandler::RuleRemoved(const QVariantMap &params);
    This signal is emitted to the API notifications when a \l{Rule} was removed.
    The \a params contain the map for the notification.
*/

/*! \fn void nymeaserver::RulesHandler::RuleAdded(const QVariantMap &params);
    This signal is emitted to the API notifications when a \l{Rule} was added.
    The \a params contain the map for the notification.
*/

/*! \fn void nymeaserver::RulesHandler::RuleActiveChanged(const QVariantMap &params);
    This signal is emitted to the API notifications when a \l{Rule} has changed the active status.
    The \a params contain the map for the notification.
*/

/*! \fn void nymeaserver::RulesHandler::RuleConfigurationChanged(const QVariantMap &params);
    This signal is emitted to the API notifications when a \l{Rule} has changed the configuration.
    The \a params contain the map for the notification.
*/

#include "ruleshandler.h"
#include "nymeacore.h"
#include "ruleengine.h"
#include "loggingcategories.h"

#include <QDebug>

namespace nymeaserver {

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
    setDescription("AddRule", "Add a rule. You can describe rules by one or many EventDesciptors and a StateEvaluator. "
                              "Note that only one of either eventDescriptor or eventDescriptorList may be passed at a time. "
                              "A rule can be created but left disabled, meaning it won't actually be executed until set to enabled. "
                              "If not given, enabled defaults to true. A rule can have a list of actions and exitActions. "
                              "It must have at least one Action. For state based rules, actions will be executed when the system "
                              "enters a state matching the stateDescriptor. The exitActions will be executed when the system leaves "
                              "the described state again. For event based rules, actions will be executed when a matching event "
                              "happens and if the stateEvaluator matches the system's state. ExitActions for such rules will be "
                              "executed when a matching event happens and the stateEvaluator is not matching the system's state. "
                              "A rule marked as executable can be executed via the API using Rules.ExecuteRule, that means, its "
                              "actions will be executed regardless of the the eventDescriptor and stateEvaluators.");
    params.insert("name", JsonTypes::basicTypeToString(JsonTypes::String));
    params.insert("actions", QVariantList() << JsonTypes::ruleActionRef());
    params.insert("o:timeDescriptor", JsonTypes::timeDescriptorRef());
    params.insert("o:stateEvaluator", JsonTypes::stateEvaluatorRef());
    params.insert("o:eventDescriptors", QVariantList() << JsonTypes::eventDescriptorRef());
    params.insert("o:exitActions", QVariantList() << JsonTypes::ruleActionRef());
    params.insert("o:enabled", JsonTypes::basicTypeToString(JsonTypes::Bool));
    params.insert("o:executable", JsonTypes::basicTypeToString(JsonTypes::Bool));
    setParams("AddRule", params);
    returns.insert("ruleError", JsonTypes::ruleErrorRef());
    returns.insert("o:ruleId", JsonTypes::basicTypeToString(JsonTypes::Uuid));
    setReturns("AddRule", returns);

    params.clear(); returns.clear();
    setDescription("EditRule", "Edit the parameters of a rule. The configuration of the rule with the given ruleId "
                   "will be replaced with the new given configuration. In ordert to enable or disable a Rule, please use the "
                   "methods \"Rules.EnableRule\" and \"Rules.DisableRule\". If successful, the notification \"Rule.RuleConfigurationChanged\" "
                   "will be emitted.");
    params.insert("ruleId", JsonTypes::basicTypeToString(JsonTypes::Uuid));
    params.insert("name", JsonTypes::basicTypeToString(JsonTypes::String));
    params.insert("actions", QVariantList() << JsonTypes::ruleActionRef());
    params.insert("o:timeDescriptor", JsonTypes::timeDescriptorRef());
    params.insert("o:stateEvaluator", JsonTypes::stateEvaluatorRef());
    params.insert("o:eventDescriptors", QVariantList() << JsonTypes::eventDescriptorRef());
    params.insert("o:exitActions", QVariantList() << JsonTypes::ruleActionRef());
    params.insert("o:enabled", JsonTypes::basicTypeToString(JsonTypes::Bool));
    params.insert("o:executable", JsonTypes::basicTypeToString(JsonTypes::Bool));
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
                   "If successful, the notification \"Rule.RuleConfigurationChanged\" will be emitted.");
    params.insert("ruleId", JsonTypes::basicTypeToString(JsonTypes::Uuid));
    setParams("EnableRule", params);
    returns.insert("ruleError", JsonTypes::ruleErrorRef());
    setReturns("EnableRule", returns);

    params.clear(); returns.clear();
    setDescription("DisableRule", "Disable a rule. The rule won't be triggered by it's events or state changes while it is disabled. "
                   "If successful, the notification \"Rule.RuleConfigurationChanged\" will be emitted.");
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

    connect(NymeaCore::instance(), &NymeaCore::ruleAdded, this, &RulesHandler::ruleAddedNotification);
    connect(NymeaCore::instance(), &NymeaCore::ruleRemoved, this, &RulesHandler::ruleRemovedNotification);
    connect(NymeaCore::instance(), &NymeaCore::ruleActiveChanged, this, &RulesHandler::ruleActiveChangedNotification);
    connect(NymeaCore::instance(), &NymeaCore::ruleConfigurationChanged, this, &RulesHandler::ruleConfigurationChangedNotification);
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
    Rule rule = NymeaCore::instance()->ruleEngine()->findRule(ruleId);
    if (rule.id().isNull()) {
        return createReply(statusToReply(RuleEngine::RuleErrorRuleNotFound));
    }
    QVariantMap returns = statusToReply(RuleEngine::RuleErrorNoError);
    returns.insert("rule", JsonTypes::packRule(rule));
    return createReply(returns);
}

JsonReply* RulesHandler::AddRule(const QVariantMap &params)
{
    Rule rule = JsonTypes::unpackRule(params);
    rule.setId(RuleId::createRuleId());

    RuleEngine::RuleError status = NymeaCore::instance()->ruleEngine()->addRule(rule);
    QVariantMap returns;
    if (status ==  RuleEngine::RuleErrorNoError) {
        returns.insert("ruleId", rule.id().toString());
    }
    returns.insert("ruleError", JsonTypes::ruleErrorToString(status));
    return createReply(returns);
}

JsonReply *RulesHandler::EditRule(const QVariantMap &params)
{
    Rule rule = JsonTypes::unpackRule(params);
    RuleEngine::RuleError status = NymeaCore::instance()->ruleEngine()->editRule(rule);
    QVariantMap returns;
    if (status ==  RuleEngine::RuleErrorNoError) {
        returns.insert("rule", JsonTypes::packRule(NymeaCore::instance()->ruleEngine()->findRule(rule.id())));
    }
    returns.insert("ruleError", JsonTypes::ruleErrorToString(status));
    return createReply(returns);
}

JsonReply* RulesHandler::RemoveRule(const QVariantMap &params)
{
    QVariantMap returns;
    RuleId ruleId(params.value("ruleId").toString());
    RuleEngine::RuleError status = NymeaCore::instance()->removeRule(ruleId);
    returns.insert("ruleError", JsonTypes::ruleErrorToString(status));
    return createReply(returns);
}

JsonReply *RulesHandler::FindRules(const QVariantMap &params)
{
    DeviceId deviceId = DeviceId(params.value("deviceId").toString());
    QList<RuleId> rules = NymeaCore::instance()->ruleEngine()->findRules(deviceId);

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
    return createReply(statusToReply(NymeaCore::instance()->ruleEngine()->enableRule(RuleId(params.value("ruleId").toString()))));
}

JsonReply *RulesHandler::DisableRule(const QVariantMap &params)
{
    return createReply(statusToReply(NymeaCore::instance()->ruleEngine()->disableRule(RuleId(params.value("ruleId").toString()))));
}

JsonReply *RulesHandler::ExecuteActions(const QVariantMap &params)
{
    QVariantMap returns;
    RuleId ruleId(params.value("ruleId").toString());
    RuleEngine::RuleError status = NymeaCore::instance()->ruleEngine()->executeActions(ruleId);
    returns.insert("ruleError", JsonTypes::ruleErrorToString(status));
    return createReply(returns);
}

JsonReply *RulesHandler::ExecuteExitActions(const QVariantMap &params)
{
    QVariantMap returns;
    RuleId ruleId(params.value("ruleId").toString());
    RuleEngine::RuleError status = NymeaCore::instance()->ruleEngine()->executeExitActions(ruleId);
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
