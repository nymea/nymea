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
#include "ruleengine/ruleengine.h"
#include "loggingcategories.h"

#include <QDebug>

namespace nymeaserver {

/*! Constructs a new \l{RulesHandler} with the given \a parent. */
RulesHandler::RulesHandler(QObject *parent) :
    JsonHandler(parent)
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

    QVariantMap paramDescriptor;
    paramDescriptor.insert("o:paramTypeId", enumValueName(Uuid));
    paramDescriptor.insert("o:paramName", enumValueName(Uuid));
    paramDescriptor.insert("value", enumValueName(Variant));
    paramDescriptor.insert("operator", enumRef<Types::ValueOperator>());
    registerObject("ParamDescriptor", paramDescriptor);

    QVariantMap eventDescriptor;
    eventDescriptor.insert("o:eventTypeId", enumValueName(Uuid));
    eventDescriptor.insert("o:deviceId", enumValueName(Uuid));
    eventDescriptor.insert("o:interface", enumValueName(String));
    eventDescriptor.insert("o:interfaceEvent", enumValueName(String));
    eventDescriptor.insert("o:paramDescriptors", QVariantList() << objectRef("ParamDescriptor"));
    registerObject("EventDescriptor", eventDescriptor);

    QVariantMap stateDescriptor;
    stateDescriptor.insert("o:stateTypeId", enumValueName(Uuid));
    stateDescriptor.insert("o:deviceId", enumValueName(Uuid));
    stateDescriptor.insert("o:interface", enumValueName(String));
    stateDescriptor.insert("o:interfaceState", enumValueName(String));
    stateDescriptor.insert("value", enumValueName(Variant));
    stateDescriptor.insert("operator", enumRef<Types::ValueOperator>());
    registerObject("StateDescriptor", stateDescriptor);

    QVariantMap stateEvaluator;
    stateEvaluator.insert("o:stateDescriptor", objectRef("StateDescriptor"));
    stateEvaluator.insert("o:childEvaluators", QVariantList() << objectRef("StateEvaluator"));
    stateEvaluator.insert("o:operator", enumRef<Types::StateOperator>());
    registerObject("StateEvaluator", stateEvaluator);

    QVariantMap repeatingOption;
    repeatingOption.insert("mode", enumRef<RepeatingOption::RepeatingMode>());
    repeatingOption.insert("o:weekDays", QVariantList() << enumValueName(Int));
    repeatingOption.insert("o:monthDays", QVariantList() << enumValueName(Int));
    registerObject("RepeatingOption", repeatingOption);

    QVariantMap calendarItem;
    calendarItem.insert("o:datetime", enumValueName(Uint));
    calendarItem.insert("o:startTime", enumValueName(Time));
    calendarItem.insert("duration", enumValueName(Uint));
    calendarItem.insert("o:repeating", objectRef("RepeatingOption"));
    registerObject("CalendarItem", calendarItem);

    QVariantMap timeEventItem;
    timeEventItem.insert("o:datetime", enumValueName(Uint));
    timeEventItem.insert("o:time", enumValueName(Time));
    timeEventItem.insert("o:repeating", objectRef("RepeatingOption"));
    registerObject("TimeEventItem", timeEventItem);

    QVariantMap timeDescriptor;
    timeDescriptor.insert("o:calendarItems", QVariantList() << objectRef("CalendarItem"));
    timeDescriptor.insert("o:timeEventItems", QVariantList() << objectRef("TimeEventItem"));
    registerObject("TimeDescriptor", timeDescriptor);

    QVariantMap ruleActionParam;
    ruleActionParam.insert("o:paramTypeId", enumValueName(Uuid));
    ruleActionParam.insert("o:paramName", enumValueName(String));
    ruleActionParam.insert("o:value", enumValueName(Variant));
    ruleActionParam.insert("o:eventTypeId", enumValueName(Uuid));
    ruleActionParam.insert("o:eventParamTypeId", enumValueName(Uuid));
    ruleActionParam.insert("o:stateDeviceId", enumValueName(Uuid));
    ruleActionParam.insert("o:stateTypeId", enumValueName(Uuid));
    registerObject("RuleActionParam", ruleActionParam);

    QVariantMap ruleAction;
    ruleAction.insert("o:deviceId", enumValueName(Uuid));
    ruleAction.insert("o:actionTypeId", enumValueName(Uuid));
    ruleAction.insert("o:interface", enumValueName(String));
    ruleAction.insert("o:interfaceAction", enumValueName(String));
    ruleAction.insert("o:browserItemId", enumValueName(String));
    ruleAction.insert("o:ruleActionParams", QVariantList() << objectRef("RuleActionParam"));
    registerObject("RuleAction", ruleAction);

    QVariantMap rule;
    rule.insert("id", enumValueName(Uuid));
    rule.insert("name", enumValueName(String));
    rule.insert("enabled", enumValueName(Bool));
    rule.insert("executable", enumValueName(Bool));
    rule.insert("active", enumValueName(Bool));
    rule.insert("eventDescriptors", QVariantList() << objectRef("EventDescriptor"));
    rule.insert("actions", QVariantList() << objectRef("RuleAction"));
    rule.insert("exitActions", QVariantList() << objectRef("RuleAction"));
    rule.insert("stateEvaluator", objectRef("StateEvaluator"));
    rule.insert("timeDescriptor", objectRef("TimeDescriptor"));
    registerObject("Rule", rule);

    // Methods
    QString description; QVariantMap params; QVariantMap returns;
    description = "Get the descriptions of all configured rules. If you need more information about a specific rule use the "
                   "method Rules.GetRuleDetails.";
    returns.insert("ruleDescriptions", QVariantList() << objectRef("RuleDescription"));
    registerMethod("GetRules", description, params, returns);

    params.clear(); returns.clear();
    description = "Get details for the rule identified by ruleId";
    params.insert("ruleId", enumValueName(Uuid));
    returns.insert("o:rule", objectRef("Rule"));
    returns.insert("ruleError", enumRef<RuleEngine::RuleError>());
    registerMethod("GetRuleDetails", description, params, returns);

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
    registerMethod("AddRule", description, params, returns);

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
    registerMethod("EditRule", description, params, returns);

    params.clear(); returns.clear();
    description = "Remove a rule";
    params.insert("ruleId", enumValueName(Uuid));
    returns.insert("ruleError", enumRef<RuleEngine::RuleError>());
    registerMethod("RemoveRule", description, params, returns);

    params.clear(); returns.clear();
    description = "Find a list of rules containing any of the given parameters.";
    params.insert("deviceId", enumValueName(Uuid));
    returns.insert("ruleIds", QVariantList() << enumValueName(Uuid));
    registerMethod("FindRules", description, params, returns);

    params.clear(); returns.clear();
    description = "Enabled a rule that has previously been disabled."
                   "If successful, the notification \"Rule.RuleConfigurationChanged\" will be emitted.";
    params.insert("ruleId", enumValueName(Uuid));
    returns.insert("ruleError", enumRef<RuleEngine::RuleError>());
    registerMethod("EnableRule", description, params, returns);

    params.clear(); returns.clear();
    description = "Disable a rule. The rule won't be triggered by it's events or state changes while it is disabled. "
                   "If successful, the notification \"Rule.RuleConfigurationChanged\" will be emitted.";
    params.insert("ruleId", enumValueName(Uuid));
    returns.insert("ruleError", enumRef<RuleEngine::RuleError>());
    registerMethod("DisableRule", description, params, returns);

    params.clear(); returns.clear();
    description = "Execute the action list of the rule with the given ruleId.";
    params.insert("ruleId", enumValueName(Uuid));
    returns.insert("ruleError", enumRef<RuleEngine::RuleError>());
    registerMethod("ExecuteActions", description, params, returns);

    params.clear(); returns.clear();
    description = "Execute the exit action list of the rule with the given ruleId.";
    params.insert("ruleId", enumValueName(Uuid));
    returns.insert("ruleError", enumRef<RuleEngine::RuleError>());
    registerMethod("ExecuteExitActions", description, params, returns);

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

    QVariantList rulesList;
    foreach (const Rule &rule, NymeaCore::instance()->ruleEngine()->rules()) {
        rulesList.append(packRuleDescription(rule));
    }

    QVariantMap returns;
    returns.insert("ruleDescriptions", rulesList);
    return createReply(returns);
}

JsonReply *RulesHandler::GetRuleDetails(const QVariantMap &params)
{
    RuleId ruleId = RuleId(params.value("ruleId").toString());
    Rule rule = NymeaCore::instance()->ruleEngine()->findRule(ruleId);
    if (rule.id().isNull()) {
        QVariantMap data;
        data.insert("ruleError", enumValueName<RuleEngine::RuleError>(RuleEngine::RuleErrorRuleNotFound));
        return createReply(data);
    }
    QVariantMap returns;
    returns.insert("ruleError", enumValueName<RuleEngine::RuleError>(RuleEngine::RuleErrorNoError));
    returns.insert("rule", packRule(rule));
    return createReply(returns);
}

JsonReply* RulesHandler::AddRule(const QVariantMap &params)
{
    Rule rule = unpackRule(params);
    rule.setId(RuleId::createRuleId());

    RuleEngine::RuleError status = NymeaCore::instance()->ruleEngine()->addRule(rule);
    QVariantMap returns;
    if (status ==  RuleEngine::RuleErrorNoError) {
        returns.insert("ruleId", rule.id().toString());
    }
    returns.insert("ruleError", enumValueName<RuleEngine::RuleError>(status));
    return createReply(returns);
}

JsonReply *RulesHandler::EditRule(const QVariantMap &params)
{
    Rule rule = unpackRule(params);
    RuleEngine::RuleError status = NymeaCore::instance()->ruleEngine()->editRule(rule);
    QVariantMap returns;
    if (status ==  RuleEngine::RuleErrorNoError) {
        returns.insert("rule", packRule(NymeaCore::instance()->ruleEngine()->findRule(rule.id())));
    }
    returns.insert("ruleError", enumValueName<RuleEngine::RuleError>(status));
    return createReply(returns);
}

JsonReply* RulesHandler::RemoveRule(const QVariantMap &params)
{
    QVariantMap returns;
    RuleId ruleId(params.value("ruleId").toString());
    RuleEngine::RuleError status = NymeaCore::instance()->removeRule(ruleId);
    returns.insert("ruleError", enumValueName<RuleEngine::RuleError>(status));
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
    RuleEngine::RuleError status = NymeaCore::instance()->ruleEngine()->enableRule(RuleId(params.value("ruleId").toString()));
    QVariantMap ret;
    ret.insert("ruleError", enumValueName<RuleEngine::RuleError>(status));
    return createReply(ret);
}

JsonReply *RulesHandler::DisableRule(const QVariantMap &params)
{
    RuleEngine::RuleError status = NymeaCore::instance()->ruleEngine()->disableRule(RuleId(params.value("ruleId").toString()));
    QVariantMap ret;
    ret.insert("ruleError", enumValueName<RuleEngine::RuleError>(status));
    return createReply(ret);
}

JsonReply *RulesHandler::ExecuteActions(const QVariantMap &params)
{
    QVariantMap returns;
    RuleId ruleId(params.value("ruleId").toString());
    RuleEngine::RuleError status = NymeaCore::instance()->ruleEngine()->executeActions(ruleId);
    returns.insert("ruleError", enumValueName<RuleEngine::RuleError>(status));
    return createReply(returns);
}

JsonReply *RulesHandler::ExecuteExitActions(const QVariantMap &params)
{
    QVariantMap returns;
    RuleId ruleId(params.value("ruleId").toString());
    RuleEngine::RuleError status = NymeaCore::instance()->ruleEngine()->executeExitActions(ruleId);
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
    params.insert("rule", packRule(rule));

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
    params.insert("rule", packRule(rule));

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

QVariantMap RulesHandler::packParamDescriptor(const ParamDescriptor &paramDescriptor)
{
    QVariantMap variantMap;
    if (!paramDescriptor.paramTypeId().isNull()) {
        variantMap.insert("paramTypeId", paramDescriptor.paramTypeId().toString());
    } else {
        variantMap.insert("paramName", paramDescriptor.paramName());
    }
    variantMap.insert("value", paramDescriptor.value());
    variantMap.insert("operator", enumValueName<Types::ValueOperator>(paramDescriptor.operatorType()));
    return variantMap;
}

QVariantMap RulesHandler::packEventDescriptor(const EventDescriptor &eventDescriptor)
{
    QVariantMap variant;
    if (eventDescriptor.type() == EventDescriptor::TypeDevice) {
        variant.insert("eventTypeId", eventDescriptor.eventTypeId().toString());
        variant.insert("deviceId", eventDescriptor.deviceId().toString());
    } else {
        variant.insert("interface", eventDescriptor.interface());
        variant.insert("interfaceEvent", eventDescriptor.interfaceEvent());
    }
    QVariantList params;
    foreach (const ParamDescriptor &paramDescriptor, eventDescriptor.paramDescriptors())
        params.append(packParamDescriptor(paramDescriptor));

    variant.insert("paramDescriptors", params);
    return variant;
}

QVariantMap RulesHandler::packStateEvaluator(const StateEvaluator &stateEvaluator)
{
    QVariantMap variantMap;
    if (stateEvaluator.stateDescriptor().isValid())
        variantMap.insert("stateDescriptor", packStateDescriptor(stateEvaluator.stateDescriptor()));

    QVariantList childEvaluators;
    foreach (const StateEvaluator &childEvaluator, stateEvaluator.childEvaluators())
        childEvaluators.append(packStateEvaluator(childEvaluator));

    if (!childEvaluators.isEmpty() || stateEvaluator.stateDescriptor().isValid())
        variantMap.insert("operator", enumValueName<Types::StateOperator>(stateEvaluator.operatorType()));

    if (childEvaluators.count() > 0)
        variantMap.insert("childEvaluators", childEvaluators);

    return variantMap;
}

QVariantMap RulesHandler::packStateDescriptor(const StateDescriptor &stateDescriptor)
{
    QVariantMap variantMap;
    if (stateDescriptor.type() == StateDescriptor::TypeDevice) {
        variantMap.insert("stateTypeId", stateDescriptor.stateTypeId().toString());
        variantMap.insert("deviceId", stateDescriptor.deviceId().toString());
    } else {
        variantMap.insert("interface", stateDescriptor.interface());
        variantMap.insert("interfaceState", stateDescriptor.interfaceState());
    }
    variantMap.insert("value", stateDescriptor.stateValue());
    variantMap.insert("operator", enumValueName<Types::ValueOperator>(stateDescriptor.operatorType()));
    return variantMap;
}

QVariantMap RulesHandler::packTimeDescriptor(const TimeDescriptor &timeDescriptor)
{
    QVariantMap timeDescriptorVariant;

    if (!timeDescriptor.calendarItems().isEmpty()) {
        QVariantList calendarItems;
        foreach (const CalendarItem &calendarItem, timeDescriptor.calendarItems())
            calendarItems.append(packCalendarItem(calendarItem));

        timeDescriptorVariant.insert("calendarItems", calendarItems);
    }

    if (!timeDescriptor.timeEventItems().isEmpty()) {
        QVariantList timeEventItems;
        foreach (const TimeEventItem &timeEventItem, timeDescriptor.timeEventItems())
            timeEventItems.append(packTimeEventItem(timeEventItem));

        timeDescriptorVariant.insert("timeEventItems", timeEventItems);
    }

    return timeDescriptorVariant;
}

QVariantMap RulesHandler::packCalendarItem(const CalendarItem &calendarItem)
{
    QVariantMap calendarItemVariant;
    calendarItemVariant.insert("duration", calendarItem.duration());

    if (!calendarItem.dateTime().isNull() && calendarItem.dateTime().toTime_t() != 0)
        calendarItemVariant.insert("datetime", calendarItem.dateTime().toTime_t());

    if (!calendarItem.startTime().isNull())
        calendarItemVariant.insert("startTime", calendarItem.startTime().toString("hh:mm"));

    if (!calendarItem.repeatingOption().isEmtpy())
        calendarItemVariant.insert("repeating", packRepeatingOption(calendarItem.repeatingOption()));

    return calendarItemVariant;
}

QVariantMap RulesHandler::packRepeatingOption(const RepeatingOption &option)
{
    QVariantMap optionVariant;
    optionVariant.insert("mode", enumValueName<RepeatingOption::RepeatingMode>(option.mode()));
    if (!option.weekDays().isEmpty()) {
        QVariantList weekDaysVariantList;
        foreach (const int& weekDay, option.weekDays())
            weekDaysVariantList.append(QVariant(weekDay));

        optionVariant.insert("weekDays", weekDaysVariantList);
    }

    if (!option.monthDays().isEmpty()) {
        QVariantList monthDaysVariantList;
        foreach (const int& monthDay, option.monthDays())
            monthDaysVariantList.append(QVariant(monthDay));

        optionVariant.insert("monthDays", monthDaysVariantList);
    }
    return optionVariant;
}

QVariantMap RulesHandler::packTimeEventItem(const TimeEventItem &timeEventItem)
{
    QVariantMap timeEventItemVariant;

    if (!timeEventItem.dateTime().isNull() && timeEventItem.dateTime().toTime_t() != 0)
        timeEventItemVariant.insert("datetime", timeEventItem.dateTime().toTime_t());

    if (!timeEventItem.time().isNull())
        timeEventItemVariant.insert("time", timeEventItem.time().toString("hh:mm"));

    if (!timeEventItem.repeatingOption().isEmtpy())
        timeEventItemVariant.insert("repeating", packRepeatingOption(timeEventItem.repeatingOption()));

    return timeEventItemVariant;
}

QVariantMap RulesHandler::packRuleActionParam(const RuleActionParam &ruleActionParam)
{
    QVariantMap variantMap;
    if (!ruleActionParam.paramTypeId().isNull()) {
        variantMap.insert("paramTypeId", ruleActionParam.paramTypeId().toString());
    } else {
        variantMap.insert("paramName", ruleActionParam.paramName());
    }

    if (ruleActionParam.isEventBased()) {
        variantMap.insert("eventTypeId", ruleActionParam.eventTypeId().toString());
        variantMap.insert("eventParamTypeId", ruleActionParam.eventParamTypeId().toString());
    } else if (ruleActionParam.isStateBased()) {
        variantMap.insert("stateDeviceId", ruleActionParam.stateDeviceId().toString());
        variantMap.insert("stateTypeId", ruleActionParam.stateTypeId().toString());
    } else {
        variantMap.insert("value", ruleActionParam.value());
    }
    return variantMap;
}

QVariantMap RulesHandler::packRuleAction(const RuleAction &ruleAction)
{
    QVariantMap variant;
    if (ruleAction.type() == RuleAction::TypeDevice) {
        variant.insert("deviceId", ruleAction.deviceId().toString());
        variant.insert("actionTypeId", ruleAction.actionTypeId().toString());
    } else if (ruleAction.type() == RuleAction::TypeBrowser) {
        variant.insert("deviceId", ruleAction.deviceId().toString());
        variant.insert("browserItemId", ruleAction.browserItemId());
    } else {
        variant.insert("interface", ruleAction.interface());
        variant.insert("interfaceAction", ruleAction.interfaceAction());
    }
    QVariantList params;
    foreach (const RuleActionParam &ruleActionParam, ruleAction.ruleActionParams())
        params.append(packRuleActionParam(ruleActionParam));

    variant.insert("ruleActionParams", params);
    return variant;
}

QVariantMap RulesHandler::packRule(const Rule &rule)
{
    QVariantMap ruleMap;
    ruleMap.insert("id", rule.id().toString());
    ruleMap.insert("name", rule.name());
    ruleMap.insert("enabled", rule.enabled());
    ruleMap.insert("active", rule.active());
    ruleMap.insert("executable", rule.executable());
    ruleMap.insert("timeDescriptor", packTimeDescriptor(rule.timeDescriptor()));

    QVariantList eventDescriptorList;
    foreach (const EventDescriptor &eventDescriptor, rule.eventDescriptors())
        eventDescriptorList.append(packEventDescriptor(eventDescriptor));

    ruleMap.insert("eventDescriptors", eventDescriptorList);
    ruleMap.insert("stateEvaluator", packStateEvaluator(rule.stateEvaluator()));

    QVariantList actionList;
    foreach (const RuleAction &action, rule.actions())
        actionList.append(packRuleAction(action));

    ruleMap.insert("actions", actionList);

    QVariantList exitActionList;
    foreach (const RuleAction &action, rule.exitActions())
        exitActionList.append(packRuleAction(action));

    ruleMap.insert("exitActions", exitActionList);
    return ruleMap;
}

QList<ParamDescriptor> RulesHandler::unpackParamDescriptors(const QVariantList &paramList)
{
    QList<ParamDescriptor> params;
    foreach (const QVariant &paramVariant, paramList)
        params.append(unpackParamDescriptor(paramVariant.toMap()));

    return params;
}

ParamDescriptor RulesHandler::unpackParamDescriptor(const QVariantMap &paramMap)
{
    QString operatorString = paramMap.value("operator").toString();
    Types::ValueOperator valueOperator = enumNameToValue<Types::ValueOperator>(operatorString);

    if (paramMap.contains("paramTypeId")) {
        ParamDescriptor param = ParamDescriptor(ParamTypeId(paramMap.value("paramTypeId").toString()), paramMap.value("value"));
        param.setOperatorType(valueOperator);
        return param;
    }
    ParamDescriptor param = ParamDescriptor(paramMap.value("paramName").toString(), paramMap.value("value"));
    param.setOperatorType(valueOperator);
    return param;
}

EventDescriptor RulesHandler::unpackEventDescriptor(const QVariantMap &eventDescriptorMap)
{
    EventTypeId eventTypeId(eventDescriptorMap.value("eventTypeId").toString());
    DeviceId eventDeviceId(eventDescriptorMap.value("deviceId").toString());
    QString interface = eventDescriptorMap.value("interface").toString();
    QString interfaceEvent = eventDescriptorMap.value("interfaceEvent").toString();
    QList<ParamDescriptor> eventParams = unpackParamDescriptors(eventDescriptorMap.value("paramDescriptors").toList());
    if (!eventDeviceId.isNull() && !eventTypeId.isNull()) {
        return EventDescriptor(eventTypeId, eventDeviceId, eventParams);
    }
    return EventDescriptor(interface, interfaceEvent, eventParams);
}

RepeatingOption RulesHandler::unpackRepeatingOption(const QVariantMap &repeatingOptionMap)
{
    RepeatingOption::RepeatingMode mode = enumNameToValue<RepeatingOption::RepeatingMode>(repeatingOptionMap.value("mode").toString());

    QList<int> weekDays;
    if (repeatingOptionMap.contains("weekDays")) {
        foreach (const QVariant weekDayVariant, repeatingOptionMap.value("weekDays").toList()) {
            weekDays.append(weekDayVariant.toInt());
        }
    }

    QList<int> monthDays;
    if (repeatingOptionMap.contains("monthDays")) {
        foreach (const QVariant monthDayVariant, repeatingOptionMap.value("monthDays").toList()) {
            monthDays.append(monthDayVariant.toInt());
        }
    }

    return RepeatingOption(mode, weekDays, monthDays);
}

CalendarItem RulesHandler::unpackCalendarItem(const QVariantMap &calendarItemMap)
{
    CalendarItem calendarItem;
    calendarItem.setDuration(calendarItemMap.value("duration").toUInt());

    if (calendarItemMap.contains("datetime"))
        calendarItem.setDateTime(QDateTime::fromTime_t(calendarItemMap.value("datetime").toUInt()));

    if (calendarItemMap.contains("startTime"))
        calendarItem.setStartTime(QTime::fromString(calendarItemMap.value("startTime").toString(), "hh:mm"));

    if (calendarItemMap.contains("repeating"))
        calendarItem.setRepeatingOption(unpackRepeatingOption(calendarItemMap.value("repeating").toMap()));

    return calendarItem;
}

TimeDescriptor RulesHandler::unpackTimeDescriptor(const QVariantMap &timeDescriptorMap)
{
    TimeDescriptor timeDescriptor;

    if (timeDescriptorMap.contains("calendarItems")) {
        QList<CalendarItem> calendarItems;
        foreach (const QVariant &calendarItemValiant, timeDescriptorMap.value("calendarItems").toList()) {
            calendarItems.append(unpackCalendarItem(calendarItemValiant.toMap()));
        }
        timeDescriptor.setCalendarItems(calendarItems);
    }

    if (timeDescriptorMap.contains("timeEventItems")) {
        QList<TimeEventItem> timeEventItems;
        foreach (const QVariant &timeEventItemValiant, timeDescriptorMap.value("timeEventItems").toList()) {
            timeEventItems.append(unpackTimeEventItem(timeEventItemValiant.toMap()));
        }
        timeDescriptor.setTimeEventItems(timeEventItems);
    }

    return timeDescriptor;
}

TimeEventItem RulesHandler::unpackTimeEventItem(const QVariantMap &timeEventItemMap)
{
    TimeEventItem timeEventItem;

    if (timeEventItemMap.contains("datetime"))
        timeEventItem.setDateTime(timeEventItemMap.value("datetime").toUInt());

    if (timeEventItemMap.contains("time"))
        timeEventItem.setTime(timeEventItemMap.value("time").toTime());

    if (timeEventItemMap.contains("repeating"))
        timeEventItem.setRepeatingOption(unpackRepeatingOption(timeEventItemMap.value("repeating").toMap()));

    return timeEventItem;
}

StateDescriptor RulesHandler::unpackStateDescriptor(const QVariantMap &stateDescriptorMap)
{
    StateTypeId stateTypeId(stateDescriptorMap.value("stateTypeId").toString());
    DeviceId deviceId(stateDescriptorMap.value("deviceId").toString());
    QString interface(stateDescriptorMap.value("interface").toString());
    QString interfaceState(stateDescriptorMap.value("interfaceState").toString());
    QVariant value = stateDescriptorMap.value("value");
    Types::ValueOperator operatorType = enumNameToValue<Types::ValueOperator>(stateDescriptorMap.value("operator").toString());
    if (!deviceId.isNull() && !stateTypeId.isNull()) {
        StateDescriptor stateDescriptor(stateTypeId, deviceId, value, operatorType);
        return stateDescriptor;
    }
    StateDescriptor stateDescriptor(interface, interfaceState, value, operatorType);
    return stateDescriptor;
}

StateEvaluator RulesHandler::unpackStateEvaluator(const QVariantMap &stateEvaluatorMap)
{
    StateEvaluator ret(unpackStateDescriptor(stateEvaluatorMap.value("stateDescriptor").toMap()));
    if (stateEvaluatorMap.contains("operator")) {
        ret.setOperatorType(enumNameToValue<Types::StateOperator>(stateEvaluatorMap.value("operator").toString()));
    } else {
        ret.setOperatorType(Types::StateOperatorAnd);
    }

    QList<StateEvaluator> childEvaluators;
    foreach (const QVariant &childEvaluator, stateEvaluatorMap.value("childEvaluators").toList())
        childEvaluators.append(unpackStateEvaluator(childEvaluator.toMap()));

    ret.setChildEvaluators(childEvaluators);
    return ret;
}

RuleActionParam RulesHandler::unpackRuleActionParam(const QVariantMap &ruleActionParamMap)
{
    if (ruleActionParamMap.keys().count() == 0)
        return RuleActionParam();

    ParamTypeId paramTypeId = ParamTypeId(ruleActionParamMap.value("paramTypeId").toString());
    QString paramName = ruleActionParamMap.value("paramName").toString();

    RuleActionParam param;
    if (paramTypeId.isNull()) {
        param = RuleActionParam(paramName);
    } else {
        param = RuleActionParam(paramTypeId);
    }
    param.setValue(ruleActionParamMap.value("value"));
    param.setEventTypeId(EventTypeId(ruleActionParamMap.value("eventTypeId").toString()));
    param.setEventParamTypeId(ParamTypeId(ruleActionParamMap.value("eventParamTypeId").toString()));
    param.setStateDeviceId(DeviceId(ruleActionParamMap.value("stateDeviceId").toString()));
    param.setStateTypeId(StateTypeId(ruleActionParamMap.value("stateTypeId").toString()));
    return param;
}

RuleActionParamList RulesHandler::unpackRuleActionParams(const QVariantList &ruleActionParamList)
{
    RuleActionParamList ruleActionParams;
    foreach (const QVariant &paramVariant, ruleActionParamList)
        ruleActionParams.append(unpackRuleActionParam(paramVariant.toMap()));

    return ruleActionParams;
}

RuleAction RulesHandler::unpackRuleAction(const QVariantMap &ruleActionMap)
{
    ActionTypeId actionTypeId(ruleActionMap.value("actionTypeId").toString());
    DeviceId actionDeviceId(ruleActionMap.value("deviceId").toString());
    QString interface = ruleActionMap.value("interface").toString();
    QString interfaceAction = ruleActionMap.value("interfaceAction").toString();
    QString browserItemId = ruleActionMap.value("browserItemId").toString();
    RuleActionParamList actionParamList = unpackRuleActionParams(ruleActionMap.value("ruleActionParams").toList());

    if (!actionDeviceId.isNull() && !actionTypeId.isNull()) {
        return RuleAction(actionTypeId, actionDeviceId, actionParamList);
    } else if (!actionDeviceId.isNull() && !browserItemId.isNull()) {
        return RuleAction(actionDeviceId, browserItemId);
    }
    return RuleAction(interface, interfaceAction, actionParamList);
}

Rule RulesHandler::unpackRule(const QVariantMap &ruleMap)
{
    // The rule id will only be valid if unpacking for edit
    RuleId ruleId = RuleId(ruleMap.value("ruleId").toString());

    QString name = ruleMap.value("name", QString()).toString();

    // By default enabled
    bool enabled = ruleMap.value("enabled", true).toBool();

    // By default executable
    bool executable = ruleMap.value("executable", true).toBool();

    StateEvaluator stateEvaluator = unpackStateEvaluator(ruleMap.value("stateEvaluator").toMap());
    TimeDescriptor timeDescriptor = unpackTimeDescriptor(ruleMap.value("timeDescriptor").toMap());

    QList<EventDescriptor> eventDescriptors;
    if (ruleMap.contains("eventDescriptors")) {
        QVariantList eventDescriptorVariantList = ruleMap.value("eventDescriptors").toList();
        foreach (const QVariant &eventDescriptorVariant, eventDescriptorVariantList) {
            eventDescriptors.append(unpackEventDescriptor(eventDescriptorVariant.toMap()));
        }
    }

    QList<RuleAction> actions;
    if (ruleMap.contains("actions")) {
        QVariantList actionsVariantList = ruleMap.value("actions").toList();
        foreach (const QVariant &actionVariant, actionsVariantList) {
            actions.append(unpackRuleAction(actionVariant.toMap()));
        }
    }

    QList<RuleAction> exitActions;
    if (ruleMap.contains("exitActions")) {
        QVariantList exitActionsVariantList = ruleMap.value("exitActions").toList();
        foreach (const QVariant &exitActionVariant, exitActionsVariantList) {
            exitActions.append(unpackRuleAction(exitActionVariant.toMap()));
        }
    }

    Rule rule;
    rule.setId(ruleId);
    rule.setName(name);
    rule.setTimeDescriptor(timeDescriptor);
    rule.setStateEvaluator(stateEvaluator);
    rule.setEventDescriptors(eventDescriptors);
    rule.setActions(actions);
    rule.setExitActions(exitActions);
    rule.setEnabled(enabled);
    rule.setExecutable(executable);
    return rule;
}

}
