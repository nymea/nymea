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
    \class guhserver::RuleEngine
    \brief The Engine that evaluates \l{Rule}{Rules} and finds \l{Action}{Actions} to be executed.

    \ingroup rules
    \inmodule core

    You can add, remove and update rules and query the engine for actions to be executed
    for a given \l{Event} described by an \l{EventDescriptor}.

    \sa Event, EventDescriptor, Rule, RuleAction
*/

/*! \fn void guhserver::RuleEngine::ruleAdded(const Rule &rule)
    Will be emitted whenever a new \l{Rule} is added to this Engine.
    The \a rule parameter holds the entire new rule.*/

/*! \fn void guhserver::RuleEngine::ruleRemoved(const RuleId &ruleId)
    Will be emitted whenever a \l{Rule} is removed from this Engine.
    \a ruleId holds the id of the removed rule. You should remove any references
    or copies you hold for this rule.*/

/*! \fn void guhserver::RuleEngine::ruleConfigurationChanged(const Rule &rule)
    Will be emitted whenever a \l{Rule} changed his enable/disable status.
    The parameter \a rule holds the changed rule.*/

/*! \enum guhserver::RuleEngine::RuleError
    \value RuleErrorNoError
        No error happened. Everything is fine.
    \value RuleErrorInvalidRuleId
        The given RuleId is not valid.
    \value RuleErrorRuleNotFound
        Couldn't find a \l{Rule} with the given id.
    \value RuleErrorDeviceNotFound
        Couldn't find a \l{Device} with the given id.
    \value RuleErrorEventTypeNotFound
        Couldn't find a \l{EventType} with the given id.
    \value RuleErrorStateTypeNotFound
        Couldn't find a \l{StateType} with the given id.
    \value RuleErrorActionTypeNotFound
        Couldn't find a \l{ActionType} with the given id.
    \value RuleErrorInvalidParameter
        The given \l{Param} is not valid.
    \value RuleErrorInvalidRuleFormat
        The format of the rule is not valid. (i.e. add \l{Rule} with exitActions and eventDescriptors)
    \value RuleErrorMissingParameter
        One of the given \l{Param}{Params} is missing.
    \value RuleErrorInvalidRuleActionParameter
        One of the given \l{RuleActionParam}{RuleActionParams} is not valid.
    \value RuleErrorInvalidStateEvaluatorValue
        One of the given \l{StateEvaluator}{StateEvaluators} has an invalid \l{State} value.
    \value RuleErrorTypesNotMatching
        The types of the \l{RuleActionParam} and the corresponding \l{Event} \l{Param} do not match.
    \value RuleErrorNotExecutable
        This rule is not executable.
    \value RuleErrorContainsEventBasesAction
        This rule contains an \l{Action} which depends on an \l{Event} value. This \l{Rule} cannot execute
        the \l{Action}{Actions} without the \l{Event} value.
    \value RuleErrorNoExitActions
        This rule does not have any ExitActions which means they cannot be executed.
*/

/*! \enum guhserver::RuleEngine::RemovePolicy

    \value RemovePolicyCascade
        Remove the whole \l{Rule}.
    \value RemovePolicyUpdate
        Remove a \l{Device} from a rule.
*/


#include "ruleengine.h"
#include "loggingcategories.h"
#include "types/paramdescriptor.h"
#include "types/eventdescriptor.h"
#include "guhsettings.h"

#include "guhcore.h"

#include "devicemanager.h"
#include "plugin/device.h"

#include <QDebug>
#include <QStringList>
#include <QStandardPaths>
#include <QCoreApplication>

namespace guhserver {

/*! Constructs the RuleEngine with the given \a parent. Although it wouldn't harm to have multiple RuleEngines, there is one
    instance available from \l{GuhCore}. This one should be used instead of creating multiple ones.
 */
RuleEngine::RuleEngine(QObject *parent) :
    QObject(parent)
{
    GuhSettings settings(GuhSettings::SettingsRoleRules);
    qCDebug(dcRuleEngine) << "loading rules from" << settings.fileName();
    foreach (const QString &idString, settings.childGroups()) {
        settings.beginGroup(idString);

        QString name = settings.value("name", idString).toString();
        bool enabled = settings.value("enabled", true).toBool();
        bool executable = settings.value("executable", true).toBool();

        qCDebug(dcRuleEngine) << "load rule" << name << idString;

        QList<EventDescriptor> eventDescriptorList;
        settings.beginGroup("events");

        foreach (QString eventGroupName, settings.childGroups()) {
            if (eventGroupName.startsWith("EventDescriptor-")) {
                settings.beginGroup(eventGroupName);
                EventTypeId eventTypeId(settings.value("eventTypeId").toString());
                DeviceId deviceId(settings.value("deviceId").toString());

                QList<ParamDescriptor> params;
                foreach (QString groupName, settings.childGroups()) {
                    if (groupName.startsWith("ParamDescriptor-")) {
                        settings.beginGroup(groupName);
                        ParamDescriptor paramDescriptor(groupName.remove(QRegExp("^ParamDescriptor-")), settings.value("value"));
                        paramDescriptor.setOperatorType((Types::ValueOperator)settings.value("operator").toInt());
                        params.append(paramDescriptor);
                        settings.endGroup();
                    }
                }

                EventDescriptor eventDescriptor(eventTypeId, deviceId, params);
                eventDescriptorList.append(eventDescriptor);
                settings.endGroup();
            }
        }
        settings.endGroup();

        StateEvaluator stateEvaluator = StateEvaluator::loadFromSettings(settings, "stateEvaluator");
        
        QList<RuleAction> actions;
        settings.beginGroup("ruleActions");
        foreach (const QString &actionNumber, settings.childGroups()) {
            settings.beginGroup(actionNumber);

            RuleAction action = RuleAction(ActionTypeId(settings.value("actionTypeId").toString()),
                                           DeviceId(settings.value("deviceId").toString()));

            RuleActionParamList params;
            foreach (QString paramNameString, settings.childGroups()) {
                if (paramNameString.startsWith("RuleActionParam-")) {
                    settings.beginGroup(paramNameString);
                    RuleActionParam param(paramNameString.remove(QRegExp("^RuleActionParam-")),
                                          settings.value("value",QVariant()),
                                          EventTypeId(settings.value("eventTypeId", EventTypeId()).toString()),
                                          settings.value("eventParamName", QString()).toString());
                    params.append(param);
                    settings.endGroup();
                }
            }

            action.setRuleActionParams(params);
            actions.append(action);

            settings.endGroup();
        }
        settings.endGroup();

        QList<RuleAction> exitActions;
        settings.beginGroup("ruleExitActions");
        foreach (const QString &actionNumber, settings.childGroups()) {
            settings.beginGroup(actionNumber);

            RuleAction action = RuleAction(ActionTypeId(settings.value("actionTypeId").toString()),
                                           DeviceId(settings.value("deviceId").toString()));

            RuleActionParamList params;
            foreach (QString paramNameString, settings.childGroups()) {
                if (paramNameString.startsWith("RuleActionParam-")) {
                    settings.beginGroup(paramNameString);
                    RuleActionParam param(paramNameString.remove(QRegExp("^RuleActionParam-")), settings.value("value"));
                    params.append(param);
                    settings.endGroup();
                }
            }
            action.setRuleActionParams(params);
            exitActions.append(action);
            settings.endGroup();
        }
        settings.endGroup();

        Rule rule;
        rule.setId(RuleId(idString));
        rule.setName(name);
        rule.setEventDescriptors(eventDescriptorList);
        rule.setStateEvaluator(stateEvaluator);
        rule.setActions(actions);
        rule.setExitActions(exitActions);
        rule.setEnabled(enabled);
        rule.setExecutable(executable);
        appendRule(rule);
        settings.endGroup();
    }
}

/*! Destructor of the \l{RuleEngine}. */
RuleEngine::~RuleEngine()
{
    qCDebug(dcApplication) << "Shutting down \"Rule Engine\"";
}

/*! Ask the Engine to evaluate all the rules for the given \a event.
    This will search all the \l{Rule}{Rules} triggered by the given \a event
    and evaluate their states in the system. It will return a
    list of all \l{Rule}{Rules} that are triggered or change its active state
    because of this \a event.
*/
QList<Rule> RuleEngine::evaluateEvent(const Event &event)
{
    Device *device = GuhCore::instance()->deviceManager()->findConfiguredDevice(event.deviceId());

    qCDebug(dcRuleEngine) << "got event:" << event << device->name() << event.eventTypeId();

    QList<Rule> rules;
    foreach (const RuleId &id, m_ruleIds) {
        Rule rule = m_rules.value(id);
        if (!rule.enabled())
            continue;

        if (rule.eventDescriptors().isEmpty()) {
            // This rule seems to have only states, check on state changed
            if (containsState(rule.stateEvaluator(), event)) {
                if (rule.stateEvaluator().evaluate()) {
                    if (m_activeRules.contains(rule.id())) {
                        qCDebug(dcRuleEngine) << "Rule" << rule.id() << "still in active state.";
                    } else {
                        qCDebug(dcRuleEngine) << "Rule" << rule.id() << "entered active state.";
                        rule.setActive(true);
                        m_rules[rule.id()] = rule;
                        m_activeRules.append(rule.id());
                        rules.append(rule);
                    }
                } else {
                    if (m_activeRules.contains(rule.id())) {
                        qCDebug(dcRuleEngine) << "Rule" << rule.id() << "left active state.";
                        rule.setActive(false);
                        m_rules[rule.id()] = rule;
                        m_activeRules.removeAll(rule.id());
                        rules.append(rule);
                    }
                }
            }
        } else {
            if (containsEvent(rule, event)) {
                if (rule.stateEvaluator().evaluate()) {
                    qCDebug(dcRuleEngine) << "Rule" << rule.id() << "contains event" << event.eventId() << "and all states match.";
                    rules.append(rule);
                }
            }
        }
    }
    return rules;
}

QList<Rule> RuleEngine::evaluateTime(const QDateTime &dateTime)
{
    Q_UNUSED(dateTime)
    QList<Rule> rules;

    return rules;
}

RuleEngine::RuleError RuleEngine::addRule(const Rule &rule, bool fromEdit)
{
    if (rule.id().isNull())
        return RuleErrorInvalidRuleId;

    if (!findRule(rule.id()).id().isNull()) {
        qCWarning(dcRuleEngine) << "Already have a rule with this id.";
        return RuleErrorInvalidRuleId;
    }

    if (!rule.isConsistent()) {
        qCWarning(dcRuleEngine) << "Rule inconsistent.";
        return RuleErrorInvalidRuleFormat;
    }

    // Check IDs in each EventDescriptor
    foreach (const EventDescriptor &eventDescriptor, rule.eventDescriptors()) {
        // check deviceId
        Device *device = GuhCore::instance()->deviceManager()->findConfiguredDevice(eventDescriptor.deviceId());
        if (!device) {
            qCWarning(dcRuleEngine) << "Cannot create rule. No configured device for eventTypeId" << eventDescriptor.eventTypeId();
            return RuleErrorDeviceNotFound;
        }

        // Check eventTypeId for this deivce
        DeviceClass deviceClass = GuhCore::instance()->deviceManager()->findDeviceClass(device->deviceClassId());
        bool eventTypeFound = false;
        foreach (const EventType &eventType, deviceClass.eventTypes()) {
            if (eventType.id() == eventDescriptor.eventTypeId()) {
                eventTypeFound = true;
            }
        }
        if (!eventTypeFound) {
            qCWarning(dcRuleEngine) << "Cannot create rule. Device " + device->name() + " has no event type:" << eventDescriptor.eventTypeId();
            return RuleErrorEventTypeNotFound;
        }
    }

    // Check state evaluator
    if (!rule.stateEvaluator().isValid()) {
        qCWarning(dcRuleEngine) << "Cannot create rule. Got an invalid StateEvaluator.";
        return RuleErrorInvalidStateEvaluatorValue;
    }

    // Check time descriptor
    if (!rule.timeDescriptor().isEmpty()) {

        if (rule.timeDescriptor().isValid()) {
            qCDebug(dcRuleEngine()) << "Cannot create rule. Got invalid timeDescriptor.";
            return RuleErrorInvalidTimeDescriptor;
        }

        // validate CalendarItems
        if (!rule.timeDescriptor().calendarItems().isEmpty()) {
            foreach (const CalendarItem &calendarItem, rule.timeDescriptor().calendarItems()) {
                if (!calendarItem.isValid()) {
                    qCDebug(dcRuleEngine()) << "Cannot create rule. Got invalid calendarItem.";
                    return RuleErrorInvalidCalendarItem;
                }

                // validate RepeatingOptions
                if (!calendarItem.repeatingOption().isEmtpy() && !calendarItem.repeatingOption().isValid()) {
                    qCDebug(dcRuleEngine()) << "Cannot create rule. Got invalid repeatingOption in calendarItem.";
                    return RuleErrorInvalidRepeatingOption;
                }
            }
        }

        // validate TimeEventItems
        if (!rule.timeDescriptor().timeEventItems().isEmpty()) {
            foreach (const TimeEventItem &timeEventItem, rule.timeDescriptor().timeEventItems()) {
                if (!timeEventItem.isValid()) {
                    qCDebug(dcRuleEngine()) << "Cannot create rule. Got invalid timeEventItem.";
                    return RuleErrorInvalidTimeEventItem;
                }

                // validate RepeatingOptions
                if (!timeEventItem.repeatingOption().isEmtpy() && !timeEventItem.repeatingOption().isValid()) {
                    qCDebug(dcRuleEngine()) << "Cannot create rule. Got invalid repeatingOption in timeEventItem.";
                    return RuleErrorInvalidRepeatingOption;
                }
            }
        }
    }


    // Check actions
    foreach (const RuleAction &action, rule.actions()) {
        Device *device = GuhCore::instance()->deviceManager()->findConfiguredDevice(action.deviceId());
        if (!device) {
            qCWarning(dcRuleEngine) << "Cannot create rule. No configured device for action with actionTypeId" << action.actionTypeId();
            return RuleErrorDeviceNotFound;
        }

        DeviceClass deviceClass = GuhCore::instance()->deviceManager()->findDeviceClass(device->deviceClassId());
        if (!deviceClass.hasActionType(action.actionTypeId())) {
            qCWarning(dcRuleEngine) << "Cannot create rule. Device " + device->name() + " has no action type:" << action.actionTypeId();
            return RuleErrorActionTypeNotFound;
        }

        // check possible eventTypeIds in params
        if (action.isEventBased()) {
            foreach (const RuleActionParam &ruleActionParam, action.ruleActionParams()) {
                if (ruleActionParam.eventTypeId() != EventTypeId()) {
                    // We have an eventTypeId
                    if (rule.eventDescriptors().isEmpty()) {
                        qCWarning(dcRuleEngine) << "Cannot create rule. RuleAction" << action.actionTypeId() << "contains an eventTypeId, but there are no eventDescriptors.";
                        return RuleErrorInvalidRuleActionParameter;
                    }

                    // now check if this eventType is in the eventDescriptorList of this rule
                    if (!checkEventDescriptors(rule.eventDescriptors(), ruleActionParam.eventTypeId())) {
                        qCWarning(dcRuleEngine) << "Cannot create rule. EventTypeId from RuleAction" << action.actionTypeId() << "not in eventDescriptors.";
                        return RuleErrorInvalidRuleActionParameter;
                    }

                    // check if the param type of the event and the action match
                    QVariant::Type eventParamType = getEventParamType(ruleActionParam.eventTypeId(), ruleActionParam.eventParamName());
                    QVariant::Type actionParamType = getActionParamType(action.actionTypeId(), ruleActionParam.name());
                    if (eventParamType != actionParamType) {
                        qCWarning(dcRuleEngine) << "Cannot create rule. RuleActionParam" << ruleActionParam.name() << " and given event param " << ruleActionParam.eventParamName() << "have not the same type:";
                        qCWarning(dcRuleEngine) << "        -> actionParamType:" << actionParamType;
                        qCWarning(dcRuleEngine) << "        ->  eventParamType:" << eventParamType;
                        return RuleErrorTypesNotMatching;
                    }
                }
            }
        } else {
            // verify action params
            foreach (const ActionType &actionType, deviceClass.actionTypes()) {
                if (actionType.id() == action.actionTypeId()) {
                    ParamList finalParams = action.toAction().params();
                    DeviceManager::DeviceError paramCheck = GuhCore::instance()->deviceManager()->verifyParams(actionType.paramTypes(), finalParams);
                    if (paramCheck != DeviceManager::DeviceErrorNoError) {
                        qCWarning(dcRuleEngine) << "Cannot create rule. Got an invalid actionParam.";
                        return RuleErrorInvalidRuleActionParameter;
                    }
                }
            }
        }

        foreach (const RuleActionParam &ruleActionParam, action.ruleActionParams()) {
            if (!ruleActionParam.isValid()) {
                qCWarning(dcRuleEngine) << "Cannot create rule. Got an actionParam with \"value\" AND \"eventTypeId\".";
                return RuleEngine::RuleErrorInvalidRuleActionParameter;
            }
        }
    }

    // Check exit actions
    foreach (const RuleAction &action, rule.exitActions()) {
        Device *device = GuhCore::instance()->deviceManager()->findConfiguredDevice(action.deviceId());
        if (!device) {
            qCWarning(dcRuleEngine) << "Cannot create rule. No configured device for exit action with actionTypeId" << action.actionTypeId();
            return RuleErrorDeviceNotFound;
        }

        DeviceClass deviceClass = GuhCore::instance()->deviceManager()->findDeviceClass(device->deviceClassId());
        if (!deviceClass.hasActionType(action.actionTypeId())) {
            qCWarning(dcRuleEngine) << "Cannot create rule. Device " + device->name() + " has no action type:" << action.actionTypeId();
            return RuleErrorActionTypeNotFound;
        }

        // verify action params
        foreach (const ActionType &actionType, deviceClass.actionTypes()) {
            if (actionType.id() == action.actionTypeId()) {
                ParamList finalParams = action.toAction().params();
                DeviceManager::DeviceError paramCheck = GuhCore::instance()->deviceManager()->verifyParams(actionType.paramTypes(), finalParams);
                if (paramCheck != DeviceManager::DeviceErrorNoError) {
                    qCWarning(dcRuleEngine) << "Cannot create rule. Got an invalid exit actionParam.";
                    return RuleErrorInvalidRuleActionParameter;
                }
            }
        }

        // Exit action can never be event based.
        if (action.isEventBased()) {
            qCWarning(dcRuleEngine) << "Cannot create rule. Got exitAction with an actionParam containing an eventTypeId. ";
            return RuleErrorInvalidRuleActionParameter;
        }

        foreach (const RuleActionParam &ruleActionParam, action.ruleActionParams()) {
            if (!ruleActionParam.isValid()) {
                qCWarning(dcRuleEngine) << "Cannot create rule. Got an actionParam with \"value\" AND \"eventTypeId\".";
                return RuleEngine::RuleErrorInvalidRuleActionParameter;
            }
        }
    }

    appendRule(rule);
    saveRule(rule);

    if (!fromEdit)
        emit ruleAdded(rule);

    return RuleErrorNoError;
}

RuleEngine::RuleError RuleEngine::editRule(const Rule &rule)
{
    if (rule.id().isNull())
        return RuleErrorInvalidRuleId;

    // Store rule in case the add new rule fails
    Rule oldRule = findRule(rule.id());
    if (oldRule.id().isNull()) {
        qCWarning(dcRuleEngine) << "Cannot edit rule. There is no rule with id:" << rule.id().toString();
        return RuleErrorRuleNotFound;
    }

    // First remove old rule with this id
    RuleError removeResult = removeRule(oldRule.id(), true);
    if (removeResult != RuleErrorNoError) {
        qCWarning(dcRuleEngine) << "Cannot edit rule. Could not remove the old rule.";
        // no need to restore, rule is still in system
        return removeResult;
    }

    // The rule is removed, now add the new one
    RuleError addResult = addRule(rule);
    if (addResult != RuleErrorNoError) {
        qCWarning(dcRuleEngine) << "Cannot edit rule. Could not add the new rule. Restoring the old rule.";
        // restore old rule
        appendRule(oldRule);
        return addResult;
    }

    // Successfully changed the rule
    emit ruleConfigurationChanged(rule);
    return RuleErrorNoError;
}

/*! Returns a list of all \l{Rule}{Rules} loaded in this Engine.
    Be aware that this does not necessarily reflect the order of the rules in the engine.
    Use ruleIds() if you need the correct order.
*/
QList<Rule> RuleEngine::rules() const
{
    return m_rules.values();
}

/*! Returns a list of all ruleIds loaded in this Engine. */
QList<RuleId> RuleEngine::ruleIds() const
{
    return m_ruleIds;
}

/*! Removes the \l{Rule} with the given \a ruleId from the Engine.
    Returns \l{RuleError} which describes whether the operation
    was successful or not. If \a fromEdit is true, the notification Rules.RuleRemoved
    will not be emitted.
*/
RuleEngine::RuleError RuleEngine::removeRule(const RuleId &ruleId, bool fromEdit)
{
    int index = m_ruleIds.indexOf(ruleId);

    if (index < 0) {
        return RuleErrorRuleNotFound;
    }

    m_ruleIds.takeAt(index);
    m_rules.remove(ruleId);
    m_activeRules.removeAll(ruleId);

    GuhSettings settings(GuhSettings::SettingsRoleRules);
    settings.beginGroup(ruleId.toString());
    settings.remove("");
    settings.endGroup();

    if (!fromEdit)
        emit ruleRemoved(ruleId);

    return RuleErrorNoError;
}

/*! Enables the rule with the given \a ruleId that has been previously disabled.

    \sa disableRule()
*/
RuleEngine::RuleError RuleEngine::enableRule(const RuleId &ruleId)
{
    if (!m_rules.contains(ruleId)) {
        qCWarning(dcRuleEngine) << "Rule not found. Can't enable it";
        return RuleErrorRuleNotFound;
    }
    Rule rule = m_rules.value(ruleId);
    rule.setEnabled(true);
    m_rules[ruleId] = rule;

    saveRule(rule);
    emit ruleConfigurationChanged(rule);

    return RuleErrorNoError;
}

/*! Disables the rule with the given \a ruleId. Disabled rules won't be triggered.

    \sa enableRule()
*/
RuleEngine::RuleError RuleEngine::disableRule(const RuleId &ruleId)
{
    if (!m_rules.contains(ruleId)) {
        qCWarning(dcRuleEngine) << "Rule not found. Can't disable it";
        return RuleErrorRuleNotFound;
    }

    Rule rule = m_rules.value(ruleId);
    rule.setEnabled(false);
    m_rules[ruleId] = rule;
    saveRule(rule);
    emit ruleConfigurationChanged(rule);
    return RuleErrorNoError;
}

/*! Executes the list of \l{Action}{Actions} of the rule with the given \a ruleId.
    Returns the corresponding RuleEngine::RuleError to inform about the result.

    \sa executeExitActions()
*/
RuleEngine::RuleError RuleEngine::executeActions(const RuleId &ruleId)
{
    // check if rule exits
    if (!m_rules.contains(ruleId)) {
        qCWarning(dcRuleEngine) << "Not executing rule actions: rule not found.";
        return RuleErrorRuleNotFound;
    }

    Rule rule = m_rules.value(ruleId);

    // check if rule is executable
    if (!rule.executable()) {
        qCWarning(dcRuleEngine) << "Not executing rule actions: rule is not executable.";
        return RuleErrorNotExecutable;
    }

    // check if an Action is eventBased
    foreach (const RuleAction &ruleAction, rule.actions()) {
        if (ruleAction.isEventBased()) {
            qCWarning(dcRuleEngine) << "Not executing rule actions: rule action depends on an event:" << ruleAction.actionTypeId() << ruleAction.ruleActionParams();
            return RuleErrorContainsEventBasesAction;
        }
    }

    qCDebug(dcRuleEngine) << "Executing rule actions of rule" << rule.name() << rule.id();
    GuhCore::instance()->executeRuleActions(rule.actions());
    return RuleErrorNoError;
}

/*! Executes the list of \l{Action}{ExitActions} of the rule with the given \a ruleId.
    Returns the corresponding RuleEngine::RuleError to inform about the result.

    \sa executeActions()
*/
RuleEngine::RuleError RuleEngine::executeExitActions(const RuleId &ruleId)
{
    // check if rule exits
    if (!m_rules.contains(ruleId)) {
        qCWarning(dcRuleEngine) << "Not executing rule exit actions: rule not found.";
        return RuleErrorRuleNotFound;
    }

    Rule rule = m_rules.value(ruleId);

    // check if rule is executable
    if (!rule.executable()) {
        qCWarning(dcRuleEngine) << "Not executing rule exit actions: rule is not executable.";
        return RuleErrorNotExecutable;
    }

    if (rule.exitActions().isEmpty()) {
        qCWarning(dcRuleEngine) << "Not executing rule exit actions: rule has no exit actions.";
        return RuleErrorNoExitActions;
    }

    qCDebug(dcRuleEngine) << "Executing rule exit actions of rule" << rule.name() << rule.id();
    GuhCore::instance()->executeRuleActions(rule.exitActions());
    return RuleErrorNoError;
}

/*! Returns the \l{Rule} with the given \a ruleId. If the \l{Rule} does not exist, it will return \l{Rule::Rule()} */
Rule RuleEngine::findRule(const RuleId &ruleId)
{
    if (!m_rules.contains(ruleId))
        return Rule();

    return m_rules.value(ruleId);
}

/*! Returns a list of all \l{Rule}{Rules} loaded in this Engine, which contains a \l{Device} with the given \a deviceId. */
QList<RuleId> RuleEngine::findRules(const DeviceId &deviceId)
{
    // Find all offending rules
    QList<RuleId> offendingRules;
    foreach (const Rule &rule, m_rules) {
        bool offending = false;
        foreach (const EventDescriptor &eventDescriptor, rule.eventDescriptors()) {
            if (eventDescriptor.deviceId() == deviceId) {
                offending = true;
                break;
            }
        }

        if (!offending && rule.stateEvaluator().containsDevice(deviceId))
            offending = true;

        if (!offending) {
            foreach (const RuleAction &action, rule.actions()) {
                if (action.deviceId() == deviceId) {
                    offending = true;
                    break;
                }
            }
        }

        if (!offending) {
            foreach (const RuleAction &action, rule.exitActions()) {
                if (action.deviceId() == deviceId) {
                    offending = true;
                    break;
                }
            }
        }

        if (offending)
            offendingRules.append(rule.id());

    }
    return offendingRules;
}

/*! Removes a \l{Device} from a \l{Rule} with the given \a id and \a deviceId. */
void RuleEngine::removeDeviceFromRule(const RuleId &id, const DeviceId &deviceId)
{
    if (!m_rules.contains(id))
        return;

    Rule rule = m_rules.value(id);

    // remove device from eventDescriptors
    QList<EventDescriptor> eventDescriptors = rule.eventDescriptors();
    QList<int> removeIndexes;
    for (int i = 0; i < eventDescriptors.count(); i++) {
        if (eventDescriptors.at(i).deviceId() == deviceId) {
            removeIndexes.append(i);
        }
    }
    while (removeIndexes.count() > 0) {
        eventDescriptors.takeAt(removeIndexes.takeLast());
    }

    // remove device from state evaluators
    StateEvaluator stateEvalatuator = rule.stateEvaluator();
    stateEvalatuator.removeDevice(deviceId);

    // remove device from actions
    QList<RuleAction> actions = rule.actions();
    for (int i = 0; i < actions.count(); i++) {
        if (actions.at(i).deviceId() == deviceId) {
            removeIndexes.append(i);
        }
    }
    while (removeIndexes.count() > 0) {
        actions.takeAt(removeIndexes.takeLast());
    }

    // remove device from exit actions
    QList<RuleAction> exitActions = rule.exitActions();
    for (int i = 0; i < exitActions.count(); i++) {
        if (exitActions.at(i).deviceId() == deviceId) {
            removeIndexes.append(i);
        }
    }
    while (removeIndexes.count() > 0) {
        exitActions.takeAt(removeIndexes.takeLast());
    }

    // remove the rule from settings
    GuhSettings settings(GuhSettings::SettingsRoleRules);
    settings.beginGroup(id.toString());
    settings.remove("");
    settings.endGroup();

    Rule newRule;
    newRule.setId(id);
    newRule.setName(rule.name());
    newRule.setEventDescriptors(eventDescriptors);
    newRule.setStateEvaluator(stateEvalatuator);
    newRule.setActions(actions);
    m_rules[id] = newRule;

    // save it
    saveRule(newRule);
    emit ruleConfigurationChanged(newRule);
}

bool RuleEngine::containsEvent(const Rule &rule, const Event &event)
{
    foreach (const EventDescriptor &eventDescriptor, rule.eventDescriptors()) {
        if (eventDescriptor == event) {
            return true;
        }
    }
    return false;
}

bool RuleEngine::containsState(const StateEvaluator &stateEvaluator, const Event &stateChangeEvent)
{
    if (stateEvaluator.stateDescriptor().isValid() && stateEvaluator.stateDescriptor().stateTypeId().toString() == stateChangeEvent.eventTypeId().toString()) {
        return true;
    }
    foreach (const StateEvaluator &childEvaluator, stateEvaluator.childEvaluators()) {
        if (containsState(childEvaluator, stateChangeEvent)) {
            return true;
        }
    }

    return false;
}

bool RuleEngine::checkEventDescriptors(const QList<EventDescriptor> eventDescriptors, const EventTypeId &eventTypeId)
{
    foreach (const EventDescriptor eventDescriptor, eventDescriptors) {
        if (eventDescriptor.eventTypeId() == eventTypeId) {
            return true;
        }
    }
    return false;
}

QVariant::Type RuleEngine::getActionParamType(const ActionTypeId &actionTypeId, const QString &paramName)
{
    foreach (const DeviceClass &deviceClass, GuhCore::instance()->deviceManager()->supportedDevices()) {
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

QVariant::Type RuleEngine::getEventParamType(const EventTypeId &eventTypeId, const QString &paramName)
{
    foreach (const DeviceClass &deviceClass, GuhCore::instance()->deviceManager()->supportedDevices()) {
        foreach (const EventType &eventType, deviceClass.eventTypes()) {
            if (eventType.id() == eventTypeId) {
                foreach (const ParamType &paramType, eventType.paramTypes()) {
                    if (paramType.name() == paramName) {
                        return paramType.type();
                    }
                }
            }
        }
    }
    return QVariant::Invalid;
}

void RuleEngine::appendRule(const Rule &rule)
{
    m_rules.insert(rule.id(), rule);
    m_ruleIds.append(rule.id());
}

void RuleEngine::saveRule(const Rule &rule)
{
    // Save Events / EventDescriptors
    GuhSettings settings(GuhSettings::SettingsRoleRules);
    settings.beginGroup(rule.id().toString());
    settings.setValue("name", rule.name());
    settings.setValue("enabled", rule.enabled());
    settings.setValue("executable", rule.executable());
    settings.beginGroup("events");
    for (int i = 0; i < rule.eventDescriptors().count(); i++) {
        const EventDescriptor &eventDescriptor = rule.eventDescriptors().at(i);
        settings.beginGroup("EventDescriptor-" + QString::number(i));
        settings.setValue("deviceId", eventDescriptor.deviceId().toString());
        settings.setValue("eventTypeId", eventDescriptor.eventTypeId().toString());

        foreach (const ParamDescriptor &paramDescriptor, eventDescriptor.paramDescriptors()) {
            settings.beginGroup("ParamDescriptor-" + paramDescriptor.name());
            settings.setValue("value", paramDescriptor.value());
            settings.setValue("operator", paramDescriptor.operatorType());
            settings.endGroup();
        }
        settings.endGroup();
    }
    settings.endGroup();

    // Save StateEvaluator
    rule.stateEvaluator().dumpToSettings(settings, "stateEvaluator");

    // Save ruleActions
    int i = 0;
    settings.beginGroup("ruleActions");
    foreach (const RuleAction &action, rule.actions()) {
        settings.beginGroup(QString::number(i));
        settings.setValue("deviceId", action.deviceId().toString());
        settings.setValue("actionTypeId", action.actionTypeId().toString());
        foreach (const RuleActionParam &param, action.ruleActionParams()) {
            settings.beginGroup("RuleActionParam-" + param.name());
            settings.setValue("value", param.value());
            if (param.eventTypeId() != EventTypeId()) {
                settings.setValue("eventTypeId", param.eventTypeId().toString());
                settings.setValue("eventParamName", param.eventParamName());
            }
            settings.endGroup();
        }
        i++;
        settings.endGroup();
    }
    settings.endGroup();

    // Save ruleExitActions
    settings.beginGroup("ruleExitActions");
    i = 0;
    foreach (const RuleAction &action, rule.exitActions()) {
        settings.beginGroup(QString::number(i));
        settings.setValue("deviceId", action.deviceId().toString());
        settings.setValue("actionTypeId", action.actionTypeId().toString());
        foreach (const RuleActionParam &param, action.ruleActionParams()) {
            settings.beginGroup("RuleActionParam-" + param.name());
            settings.setValue("value", param.value());
            settings.endGroup();
        }
        i++;
        settings.endGroup();
    }
    settings.endGroup();
}

}
