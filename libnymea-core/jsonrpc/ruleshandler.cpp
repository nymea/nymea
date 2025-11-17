// SPDX-License-Identifier: LGPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2025, chargebyte austria GmbH
*
* This file is part of nymea.
*
* nymea is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* as published by the Free Software Foundation, either version 3
* of the License, or (at your option) any later version.
*
* nymea is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with nymea. If not, see <https://www.gnu.org/licenses/>.
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

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
#include "ruleengine/ruleengine.h"
#include "loggingcategories.h"

#include <QDebug>
#include <QJsonDocument>

namespace nymeaserver {

/*! Constructs a new \l{RulesHandler} with the given \a parent. */
RulesHandler::RulesHandler(RuleEngine *ruleEngine, QObject *parent) :
    JsonHandler(parent),
    m_ruleEngine(ruleEngine)
{
    // Enums
    registerEnum<RuleEngine::RuleError>();
    registerEnum<Types::ValueOperator>();
    registerEnum<Types::StateOperator>();
    registerEnum<RepeatingOption::RepeatingMode>();

    // Objects
    QVariantMap ruleDescription;
    ruleDescription.insert("id", enumValueName(Uuid));
    ruleDescription.insert("name", enumValueName(String));
    ruleDescription.insert("enabled", enumValueName(Bool));
    ruleDescription.insert("active", enumValueName(Bool));
    ruleDescription.insert("executable", enumValueName(Bool));
    registerObject("RuleDescription", ruleDescription);

    registerObject<ParamDescriptor, ParamDescriptors>();
    registerObject<EventDescriptor, EventDescriptors>();
    registerObject<StateDescriptor>();
    registerObject<StateEvaluator, StateEvaluators>();
    registerObject<RepeatingOption>();
    registerObject<CalendarItem, CalendarItems>();
    registerObject<TimeEventItem, TimeEventItems>();
    registerObject<TimeDescriptor>();
    registerObject<RuleActionParam, RuleActionParams>();
    registerObject<RuleAction, RuleActions>();
    registerObject<Rule, Rules>();

    // Methods
    QString description; QVariantMap params; QVariantMap returns;
    description = "Get the descriptions of all configured rules. If you need more information about a specific rule use the "
                   "method Rules.GetRuleDetails.";
    returns.insert("ruleDescriptions", QVariantList() << objectRef("RuleDescription"));
    registerMethod("GetRules", description, params, returns, Types::PermissionScopeExecuteRules);

    params.clear(); returns.clear();
    description = "Get details for the rule identified by ruleId";
    params.insert("ruleId", enumValueName(Uuid));
    returns.insert("o:rule", objectRef("Rule"));
    returns.insert("ruleError", enumRef<RuleEngine::RuleError>());
    registerMethod("GetRuleDetails", description, params, returns, Types::PermissionScopeExecuteRules);

    params.clear(); returns.clear();
    description = "Add a rule. You can describe rules by one or many EventDesciptors and a StateEvaluator. "
                              "Note that only one of either eventDescriptor or eventDescriptorList may be passed at a time. "
                              "A rule can be created but left disabled, meaning it won't actually be executed until set to enabled. "
                              "If not given, enabled defaults to true. A rule can have a list of actions and exitActions. "
                              "It must have at least one Action. For state based rules, actions will be executed when the system "
                              "enters a state matching the stateDescriptor. The exitActions will be executed when the system leaves "
                              "the described state again. For event based rules, actions will be executed when a matching event "
                              "happens and if the stateEvaluator matches the system's state. ExitActions for such rules will be "
                              "executed when a matching event happens and the stateEvaluator is not matching the system's state. "
                              "A rule marked as executable can be executed via the API using Rules.ExecuteRule, that means, its "
                              "actions will be executed regardless of the eventDescriptor and stateEvaluators.";
    params.insert("name", enumValueName(String));
    params.insert("actions", QVariantList() << objectRef("RuleAction"));
    params.insert("o:timeDescriptor", objectRef("TimeDescriptor"));
    params.insert("o:stateEvaluator", objectRef("StateEvaluator"));
    params.insert("o:eventDescriptors", QVariantList() << objectRef("EventDescriptor"));
    params.insert("o:exitActions", QVariantList() << objectRef("RuleAction"));
    params.insert("o:enabled", enumValueName(Bool));
    params.insert("o:executable", enumValueName(Bool));
    returns.insert("ruleError", enumRef<RuleEngine::RuleError>());
    returns.insert("o:ruleId", enumValueName(Uuid));
    registerMethod("AddRule", description, params, returns, Types::PermissionScopeConfigureRules);

    params.clear(); returns.clear();
    description = "Edit the parameters of a rule. The configuration of the rule with the given ruleId "
                   "will be replaced with the new given configuration. In ordert to enable or disable a Rule, please use the "
                   "methods \"Rules.EnableRule\" and \"Rules.DisableRule\". If successful, the notification \"Rule.RuleConfigurationChanged\" "
                   "will be emitted.";
    params.insert("ruleId", enumValueName(Uuid));
    params.insert("name", enumValueName(String));
    params.insert("actions", QVariantList() << objectRef("RuleAction"));
    params.insert("o:timeDescriptor", objectRef("TimeDescriptor"));
    params.insert("o:stateEvaluator", objectRef("StateEvaluator"));
    params.insert("o:eventDescriptors", QVariantList() << objectRef("EventDescriptor"));
    params.insert("o:exitActions", QVariantList() << objectRef("RuleAction"));
    params.insert("o:enabled", enumValueName(Bool));
    params.insert("o:executable", enumValueName(Bool));
    returns.insert("ruleError", enumRef<RuleEngine::RuleError>());
    returns.insert("o:rule", objectRef("Rule"));
    registerMethod("EditRule", description, params, returns, Types::PermissionScopeConfigureRules);

    params.clear(); returns.clear();
    description = "Remove a rule";
    params.insert("ruleId", enumValueName(Uuid));
    returns.insert("ruleError", enumRef<RuleEngine::RuleError>());
    registerMethod("RemoveRule", description, params, returns, Types::PermissionScopeConfigureRules);

    params.clear(); returns.clear();
    description = "Find a list of rules containing any of the given parameters.";
    params.insert("thingId", enumValueName(Uuid));
    returns.insert("ruleIds", QVariantList() << enumValueName(Uuid));
    registerMethod("FindRules", description, params, returns, Types::PermissionScopeExecuteRules);

    params.clear(); returns.clear();
    description = "Enabled a rule that has previously been disabled."
                   "If successful, the notification \"Rule.RuleConfigurationChanged\" will be emitted.";
    params.insert("ruleId", enumValueName(Uuid));
    returns.insert("ruleError", enumRef<RuleEngine::RuleError>());
    registerMethod("EnableRule", description, params, returns, Types::PermissionScopeConfigureRules);

    params.clear(); returns.clear();
    description = "Disable a rule. The rule won't be triggered by it's events or state changes while it is disabled. "
                   "If successful, the notification \"Rule.RuleConfigurationChanged\" will be emitted.";
    params.insert("ruleId", enumValueName(Uuid));
    returns.insert("ruleError", enumRef<RuleEngine::RuleError>());
    registerMethod("DisableRule", description, params, returns, Types::PermissionScopeConfigureRules);

    params.clear(); returns.clear();
    description = "Execute the action list of the rule with the given ruleId.";
    params.insert("ruleId", enumValueName(Uuid));
    returns.insert("ruleError", enumRef<RuleEngine::RuleError>());
    registerMethod("ExecuteActions", description, params, returns, Types::PermissionScopeExecuteRules);

    params.clear(); returns.clear();
    description = "Execute the exit action list of the rule with the given ruleId.";
    params.insert("ruleId", enumValueName(Uuid));
    returns.insert("ruleError", enumRef<RuleEngine::RuleError>());
    registerMethod("ExecuteExitActions", description, params, returns, Types::PermissionScopeExecuteRules);

    // Notifications
    params.clear(); returns.clear();
    description = "Emitted whenever a Rule was removed.";
    params.insert("ruleId", enumValueName(Uuid));
    registerNotification("RuleRemoved", description, params);

    params.clear(); returns.clear();
    description = "Emitted whenever a Rule was added.";
    params.insert("rule", objectRef("Rule"));
    registerNotification("RuleAdded", description, params);

    params.clear(); returns.clear();
    description = "Emitted whenever the active state of a Rule changed.";
    params.insert("ruleId", enumValueName(Uuid));
    params.insert("active", enumValueName(Bool));
    registerNotification("RuleActiveChanged", description, params);

    params.clear(); returns.clear();
    description = "Emitted whenever the configuration of a Rule changed.";
    params.insert("rule", objectRef("Rule"));
    registerNotification("RuleConfigurationChanged", description, params);

    connect(m_ruleEngine, &RuleEngine::ruleAdded, this, &RulesHandler::ruleAddedNotification);
    connect(m_ruleEngine, &RuleEngine::ruleRemoved, this, &RulesHandler::ruleRemovedNotification);
    connect(m_ruleEngine, &RuleEngine::ruleActiveChanged, this, &RulesHandler::ruleActiveChangedNotification);
    connect(m_ruleEngine, &RuleEngine::ruleConfigurationChanged, this, &RulesHandler::ruleConfigurationChangedNotification);
}

/*! Returns the name of the \l{RulesHandler}. In this case \b Rules.*/
QString RulesHandler::name() const
{
    return "Rules";
}

JsonReply* RulesHandler::GetRules(const QVariantMap &params)
{
    Q_UNUSED(params)

    QVariantList rulesList;
    foreach (const Rule &rule, m_ruleEngine->rules()) {
        rulesList.append(packRuleDescription(rule));
    }

    QVariantMap returns;
    returns.insert("ruleDescriptions", rulesList);
    return createReply(returns);
}

JsonReply *RulesHandler::GetRuleDetails(const QVariantMap &params)
{
    RuleId ruleId = RuleId(params.value("ruleId").toString());
    Rule rule = m_ruleEngine->findRule(ruleId);
    if (rule.id().isNull()) {
        QVariantMap data;
        data.insert("ruleError", enumValueName<RuleEngine::RuleError>(RuleEngine::RuleErrorRuleNotFound));
        return createReply(data);
    }
    QVariantMap returns;
    returns.insert("ruleError", enumValueName<RuleEngine::RuleError>(RuleEngine::RuleErrorNoError));
    returns.insert("rule", pack(rule));
    return createReply(returns);
}

JsonReply* RulesHandler::AddRule(const QVariantMap &params)
{
    Rule rule = unpack<Rule>(params);
    rule.setId(RuleId::createRuleId());

    RuleEngine::RuleError status = m_ruleEngine->addRule(rule);
    QVariantMap returns;
    if (status ==  RuleEngine::RuleErrorNoError) {
        returns.insert("ruleId", rule.id().toString());
    }
    returns.insert("ruleError", enumValueName<RuleEngine::RuleError>(status));
    return createReply(returns);
}

JsonReply *RulesHandler::EditRule(const QVariantMap &params)
{
    Rule rule = unpack<Rule>(params);

    // FIXME: Edit rule API currently has "ruleId" while the Rule type has "id". Auto unpacking will fail for this property
    rule.setId(params.value("ruleId").toUuid());

    RuleEngine::RuleError status = m_ruleEngine->editRule(rule);
    QVariantMap returns;
    if (status ==  RuleEngine::RuleErrorNoError) {
        returns.insert("rule", pack(m_ruleEngine->findRule(rule.id())));
    }
    returns.insert("ruleError", enumValueName<RuleEngine::RuleError>(status));
    return createReply(returns);
}

JsonReply* RulesHandler::RemoveRule(const QVariantMap &params)
{
    QVariantMap returns;
    RuleId ruleId(params.value("ruleId").toString());
    RuleEngine::RuleError status = m_ruleEngine->removeRule(ruleId);
    returns.insert("ruleError", enumValueName<RuleEngine::RuleError>(status));
    return createReply(returns);
}

JsonReply *RulesHandler::FindRules(const QVariantMap &params)
{
    ThingId thingId = ThingId(params.value("thingId").toString());
    QList<RuleId> rules = m_ruleEngine->findRules(thingId);

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
    RuleEngine::RuleError status = m_ruleEngine->enableRule(RuleId(params.value("ruleId").toString()));
    QVariantMap ret;
    ret.insert("ruleError", enumValueName<RuleEngine::RuleError>(status));
    return createReply(ret);
}

JsonReply *RulesHandler::DisableRule(const QVariantMap &params)
{
    RuleEngine::RuleError status = m_ruleEngine->disableRule(RuleId(params.value("ruleId").toString()));
    QVariantMap ret;
    ret.insert("ruleError", enumValueName<RuleEngine::RuleError>(status));
    return createReply(ret);
}

JsonReply *RulesHandler::ExecuteActions(const QVariantMap &params)
{
    QVariantMap returns;
    RuleId ruleId(params.value("ruleId").toString());
    RuleEngine::RuleError status = m_ruleEngine->executeActions(ruleId);
    returns.insert("ruleError", enumValueName<RuleEngine::RuleError>(status));
    return createReply(returns);
}

JsonReply *RulesHandler::ExecuteExitActions(const QVariantMap &params)
{
    QVariantMap returns;
    RuleId ruleId(params.value("ruleId").toString());
    RuleEngine::RuleError status = m_ruleEngine->executeExitActions(ruleId);
    returns.insert("ruleError", enumValueName<RuleEngine::RuleError>(status));
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
    params.insert("rule", pack(rule));

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
    params.insert("rule", pack(rule));

    emit RuleConfigurationChanged(params);
}

QVariantMap RulesHandler::packRuleDescription(const Rule &rule)
{
    QVariantMap ruleDescriptionMap;
    ruleDescriptionMap.insert("id", rule.id().toString());
    ruleDescriptionMap.insert("name", rule.name());
    ruleDescriptionMap.insert("enabled", rule.enabled());
    ruleDescriptionMap.insert("active", rule.active());
    ruleDescriptionMap.insert("executable", rule.executable());
    return ruleDescriptionMap;
}

}
