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
    \class nymeaserver::RuleEngine
    \brief The Engine that evaluates \l{Rule}{Rules} and finds \l{Action}{Actions} to be executed.

    \ingroup rules
    \inmodule core

    You can add, remove and update rules and query the engine for actions to be executed
    for a given \l{Event} described by an \l{EventDescriptor}.

    \sa Event, EventDescriptor, Rule, RuleAction
*/

/*! \fn void nymeaserver::RuleEngine::ruleAdded(const Rule &rule)
    Will be emitted whenever a new \l{Rule} is added to this Engine.
    The \a rule parameter holds the entire new rule.*/

/*! \fn void nymeaserver::RuleEngine::ruleRemoved(const RuleId &ruleId)
    Will be emitted whenever a \l{Rule} is removed from this Engine.
    \a ruleId holds the id of the removed rule. You should remove any references
    or copies you hold for this rule.*/

/*! \fn void nymeaserver::RuleEngine::ruleConfigurationChanged(const Rule &rule)
    Will be emitted whenever a \l{Rule} changed his enable/disable status.
    The parameter \a rule holds the changed rule.*/

/*! \enum nymeaserver::RuleEngine::RuleError
    \value RuleErrorNoError
        No error happened. Everything is fine.
    \value RuleErrorInvalidRuleId
        The given RuleId is not valid.
    \value RuleErrorRuleNotFound
        Couldn't find a \l{Rule} with the given id.
    \value RuleErrorThingNotFound
        Couldn't find a \l{Thing} with the given id.
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
    \value RuleErrorInvalidRepeatingOption
        One of the given \l{RepeatingOption}{RepeatingOption} is not valid.
    \value RuleErrorInvalidCalendarItem
        One of the given \l{CalendarItem}{CalendarItems} is not valid.
    \value RuleErrorInvalidTimeDescriptor
        One of the given \l{TimeDescriptor}{TimeDescriptors} is not valid.
    \value RuleErrorInvalidTimeEventItem
        One of the given \l{TimeEventItem}{TimeEventItems} is not valid.
    \value RuleErrorContainsEventBasesAction
        This rule contains an \l{Action} which depends on an \l{Event} value. This \l{Rule} cannot execute
        the \l{Action}{Actions} without the \l{Event} value.
    \value RuleErrorNoExitActions
        This rule does not have any ExitActions which means they cannot be executed.
    \value RuleErrorInterfaceNotFound
        There is no interface for the given string.
*/

/*! \enum nymeaserver::RuleEngine::RemovePolicy

    \value RemovePolicyCascade
        Remove the whole \l{Rule}.
    \value RemovePolicyUpdate
        Remove a \l{Thing} from a rule.
*/


#include "ruleengine.h"
#include "loggingcategories.h"
#include "time/calendaritem.h"
#include "time/repeatingoption.h"
#include "time/timeeventitem.h"
#include "time/timemanager.h"

#include "types/eventdescriptor.h"
#include "types/paramdescriptor.h"
#include "nymeasettings.h"
#include "integrations/thingmanager.h"
#include "integrations/thing.h"
#include "logging/logengine.h"

#include <QDebug>
#include <QStringList>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QMetaEnum>

NYMEA_LOGGING_CATEGORY(dcRuleEngine, "RuleEngine")
NYMEA_LOGGING_CATEGORY(dcRuleEngineDebug, "RuleEngineDebug")

namespace nymeaserver {

/*! Constructs the RuleEngine with the given \a parent. Although it wouldn't harm to have multiple RuleEngines, there is one
    instance available from \l{NymeaCore}. This one should be used instead of creating multiple ones.
 */
RuleEngine::RuleEngine(ThingManager *thingManager, TimeManager *timeManager, LogEngine *logEngine, QObject *parent) :
    QObject(parent),
    m_thingManager(thingManager),
    m_timeManager(timeManager)
{
    m_logger = logEngine->registerLogSource("rules", {"id", "event"});

    connect(m_thingManager, &ThingManager::eventTriggered, this, &RuleEngine::onEventTriggered);

    connect(m_thingManager, &ThingManager::thingStateChanged, this, [this](Thing *thing, const StateTypeId &stateTypeId, const QVariant &value, const QVariant &/*minValue*/, const QVariant &/*maxValue*/){
        // There can be event based rules that would trigger when a state changes
        // without "binding" to the state (as a stateEvaluator would do). So generate a fake event
        // for every state change.
        Param valueParam(ParamTypeId(stateTypeId.toString()), value);
        Event event(EventTypeId(stateTypeId.toString()), thing->id(), ParamList() << valueParam);
        onEventTriggered(event);
    });

    connect(m_thingManager, &ThingManager::thingRemoved, this, &RuleEngine::onThingRemoved);

    connect(m_timeManager, &TimeManager::dateTimeChanged, this, &RuleEngine::onDateTimeChanged);

    connect(m_thingManager, &ThingManager::loaded, this, [=](){
        init();
        onDateTimeChanged(m_timeManager->currentDateTime());
    });
}

/*! Destructor of the \l{RuleEngine}. */
RuleEngine::~RuleEngine()
{
}

/*! Ask the Engine to evaluate all the rules for the given \a event.
    This will search all the \l{Rule}{Rules} triggered by the given \a event
    and evaluate their states in the system. It will return a
    list of all \l{Rule}{Rules} that are triggered or change its active state
    because of this \a event.
*/
QList<Rule> RuleEngine::evaluateEvent(const Event &event)
{
    Thing *thing = m_thingManager->findConfiguredThing(event.thingId());
    if (!thing) {
        qCWarning(dcRuleEngine()) << "Invalid event. ThingID does not reference a valid thing";
        return QList<Rule>();
    }
    ThingClass thingClass = thing->thingClass();
    EventType eventType = thingClass.eventTypes().findById(event.eventTypeId());


    if (event.params().count() == 0) {
        qCDebug(dcRuleEngineDebug).nospace().noquote() << "Evaluate event: " << thing->name() << " - " << eventType.name() << " (ThingId:" << thing->id().toString() << ", EventTypeId:" << eventType.id().toString() << ")";
    } else {
        qCDebug(dcRuleEngineDebug).nospace().noquote() << "Evaluate event: " << thing->name() << " - " << eventType.name() << " (ThingId:" << thing->id().toString() << ", EventTypeId:" << eventType.id().toString() << ")" << Qt::endl << "     " << event.params();
    }

    QList<Rule> rules;
    foreach (const RuleId &id, ruleIds()) {
        Rule rule = m_rules.value(id);
        if (!rule.enabled()) {
            qCDebug(dcRuleEngineDebug()).nospace().noquote() << "Skipping rule " << rule.name() << " (" << rule.id().toString() << ") "  << " because it is disabled.";
            continue;
        }

        // If we have a state based on this event
        if (containsState(rule.stateEvaluator(), event)) {
            rule.setStatesActive(rule.stateEvaluator().evaluate());
            m_rules[rule.id()] = rule;
        }

        // If this rule does not base on an event, evaluate the rule
        if (rule.eventDescriptors().isEmpty() && rule.timeDescriptor().timeEventItems().isEmpty() && !rule.stateEvaluator().isEmpty()) {
            if (rule.timeActive() && rule.statesActive()) {
                if (!m_activeRules.contains(rule.id())) {
                    qCDebug(dcRuleEngine).nospace().noquote() << "Rule " << rule.name() << " (" << rule.id().toString() << ") active.";
                    rule.setActive(true);
                    m_rules[rule.id()] = rule;
                    m_activeRules.append(rule.id());
                    rules.append(rule);
                }
            } else {
                if (m_activeRules.contains(rule.id())) {
                    qCDebug(dcRuleEngine).nospace().noquote() << "Rule " << rule.name() << " (" << rule.id().toString() << ") inactive.";
                    rule.setActive(false);
                    m_rules[rule.id()] = rule;
                    m_activeRules.removeAll(rule.id());
                    rules.append(rule);
                }
            }
        } else {
            // Event based rule
            if (containsEvent(rule, event, thing->thingClassId())) {
                qCDebug(dcRuleEngineDebug()).nospace().noquote() << "Rule " << rule.name() << " (" << rule.id().toString() << ") contains event. States active:" << rule.statesActive() << "Time active:" << rule.timeActive();
                if (rule.statesActive() && rule.timeActive()) {
                    qCDebug(dcRuleEngine).nospace().noquote() << "Rule " << rule.name() << " (" + rule.id().toString() << ") contains event and all states match.";
                    rules.append(rule);
                } else {
                    qCDebug(dcRuleEngine).nospace().noquote() << "Rule " << rule.name() << " (" + rule.id().toString() << ") contains event but state are not matching.";
                    rules.append(rule);
                }
            }
        }
    }

    return rules;
}

/*! Ask the Engine to evaluate all the rules for the given \a dateTime.
    This will search all the \l{Rule}{Rules} triggered by the given \a dateTime
    and evaluate their \l{CalendarItem}{CalendarItems} and \l{TimeEventItem}{TimeEventItems}.
    It will return a list of all \l{Rule}{Rules} that are triggered or change its active state.
*/
QList<Rule> RuleEngine::evaluateTime(const QDateTime &dateTime)
{
    // Initialize the last datetime if not already set (current time -1 second)
    if (!m_lastEvaluationTime.isValid()) {
        m_lastEvaluationTime = dateTime;
        m_lastEvaluationTime = m_lastEvaluationTime.addSecs(-1);
    }

    QList<Rule> rules;

    qCDebug(dcRuleEngineDebug()) << "Evaluating time event" << dateTime.toString();

    foreach (const Rule &r, m_rules.values()) {
        Rule rule = m_rules.value(r.id());
        if (!rule.enabled()) {
            qCDebug(dcRuleEngineDebug()) << "Skipping rule" << rule.name() << "because it is disabled";
            continue;
        }

        // If no timeDescriptor, do nothing
        if (rule.timeDescriptor().isEmpty()) {
            qCDebug(dcRuleEngineDebug()) << "Skipping rule" << rule.name() << "because it has not time descriptors";
            continue;
        }

        // Check if this rule is based on calendarItems
        if (!rule.timeDescriptor().calendarItems().isEmpty()) {
            rule.setTimeActive(rule.timeDescriptor().evaluate(m_lastEvaluationTime, dateTime));
            m_rules[rule.id()] = rule;

            if (rule.timeDescriptor().timeEventItems().isEmpty() && rule.eventDescriptors().isEmpty()) {

                if (rule.timeActive() && rule.statesActive()) {
                    if (!m_activeRules.contains(rule.id())) {
                        qCDebug(dcRuleEngine) << "Rule" << rule.id().toString() << "active.";
                        rule.setActive(true);
                        m_rules[rule.id()] = rule;
                        m_activeRules.append(rule.id());
                        rules.append(rule);
                    }
                } else {
                    if (m_activeRules.contains(rule.id())) {
                        qCDebug(dcRuleEngine) << "Rule" << rule.id().toString() << "inactive.";
                        rule.setActive(false);
                        m_rules[rule.id()] = rule;
                        m_activeRules.removeAll(rule.id());
                        rules.append(rule);
                    }
                }
            }
        }


        // If we have timeEvent items
        if (!rule.timeDescriptor().timeEventItems().isEmpty()) {
            bool valid = rule.timeDescriptor().evaluate(m_lastEvaluationTime, dateTime);
            if (valid && rule.timeActive()) {
                qCDebug(dcRuleEngine) << "Rule" << rule.id() << "time event triggert.";
                rules.append(rule);
            }
        }
    }

    m_lastEvaluationTime = dateTime;

    if (rules.count() > 0) { // Don't spam the log
        qCDebug(dcRuleEngine()) << "EvaluateTimeEvent evaluated" << rules.count() << "to be executed";
    }

    return rules;
}

/*! Add the given \a rule to the system. If the rule will be added
    from an edit request, the parameter \a fromEdit will be true.
*/
RuleEngine::RuleError RuleEngine::addRule(const Rule &rule, bool fromEdit)
{
    if (rule.id().isNull())
        return RuleErrorInvalidRuleId;

    if (!findRule(rule.id()).id().isNull()) {
        qCWarning(dcRuleEngine) << "Already have a rule with this id.";
        return RuleErrorInvalidRuleId;
    }

    if (!rule.isConsistent()) {
        qCWarning(dcRuleEngine) << "Invalid rule format. (Either missing actions, or exitActions without condition given.)";
        return RuleErrorInvalidRuleFormat;
    }

    // Check IDs in each EventDescriptor
    foreach (const EventDescriptor &eventDescriptor, rule.eventDescriptors()) {
        if (!eventDescriptor.isValid()) {
            qWarning(dcRuleEngine()) << "EventDescriptor is incomplete. It must have either eventTypeId and thingId, or interface and interfaceEvent";
            return RuleErrorEventTypeNotFound;
        }
        if (eventDescriptor.type() == EventDescriptor::TypeThing) {
            // check thingId
            Thing *thing = m_thingManager->findConfiguredThing(eventDescriptor.thingId());
            if (!thing) {
                qCWarning(dcRuleEngine) << "Cannot create rule. No configured thing for eventTypeId" << eventDescriptor.eventTypeId();
                return RuleErrorThingNotFound;
            }

            // Check eventTypeId for this deivce
            ThingClass thingClass = m_thingManager->findThingClass(thing->thingClassId());
            bool eventTypeFound = false;
            foreach (const EventType &eventType, thingClass.eventTypes()) {
                if (eventType.id() == eventDescriptor.eventTypeId()) {
                    eventTypeFound = true;
                }
            }
            // Allow defining event descriptors with a stateTypeId as eventTypeId
            foreach (const StateType &stateType, thingClass.stateTypes()) {
                if (stateType.id() == eventDescriptor.eventTypeId()) {
                    eventTypeFound = true;
                }
            }
            if (!eventTypeFound) {
                qCWarning(dcRuleEngine) << "Cannot create rule. Thing " + thing->name() + " has no event type:" << eventDescriptor.eventTypeId();
                return RuleErrorEventTypeNotFound;
            }
        } else {
            // Interface based event
            Interface iface = m_thingManager->supportedInterfaces().findByName(eventDescriptor.interface());
            if (!iface.isValid()) {
                qWarning(dcRuleEngine()) << "No such interface:" << eventDescriptor.interface();
                return RuleErrorInterfaceNotFound;
            }
            if (iface.eventTypes().findByName(eventDescriptor.interfaceEvent()).name().isEmpty() && iface.stateTypes().findByName(eventDescriptor.interfaceEvent()).name().isEmpty()) {
                qWarning(dcRuleEngine()) << "Interface" << iface.name() << "has no such event:" << eventDescriptor.interfaceEvent();
                return RuleErrorEventTypeNotFound;
            }
        }
    }

    // Check state evaluator
    if (!rule.stateEvaluator().isValid()) {
        qCWarning(dcRuleEngine) << "Cannot create rule. Got an invalid StateEvaluator.";
        return RuleErrorInvalidStateEvaluatorValue;
    }

    // Check time descriptor
    if (!rule.timeDescriptor().isEmpty()) {

        if (!rule.timeDescriptor().isValid()) {
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
    foreach (const RuleAction &ruleAction, rule.actions()) {
        RuleError ruleActionError = checkRuleAction(ruleAction, rule);
        if (ruleActionError != RuleErrorNoError) {
            return ruleActionError;
        }
    }

    // Check exit actions
    foreach (const RuleAction &ruleExitAction, rule.exitActions()) {
        RuleError ruleActionError = checkRuleAction(ruleExitAction, rule);
        if (ruleActionError != RuleErrorNoError) {
            return ruleActionError;
        }
    }

    appendRule(rule);
    saveRule(rule);

    m_logger->log({rule.id().toString(), "created"}, {{"name", rule.name()}});
    if (!fromEdit)
        emit ruleAdded(rule);

    qCDebug(dcRuleEngine()) << "Rule" << rule.name() << rule.id().toString() << "added successfully.";
    return RuleErrorNoError;
}

/*! Edit the given \a rule in the system. The rule with the \l{RuleId} from the given \a rule
    will be removed from the system and readded with the new parameters in the given \a rule.
*/
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
    RuleError addResult = addRule(rule, true);
    if (addResult != RuleErrorNoError) {
        qCWarning(dcRuleEngine) << "Cannot edit rule. Could not add the new rule. Restoring the old rule.";
        // restore old rule
        appendRule(oldRule);
        return addResult;
    }

    // Successfully changed the rule
    emit ruleConfigurationChanged(rule);

    qCDebug(dcRuleEngine()) << "Rule" << rule.id().toString() << "updated.";
    m_logger->log({rule.id().toString(), "changed"}, {{"name", rule.name()}});

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
    Rule rule = m_rules.take(ruleId);
    m_activeRules.removeAll(ruleId);

    NymeaSettings settings(NymeaSettings::SettingsRoleRules);
    settings.beginGroup(ruleId.toString());
    settings.remove("");
    settings.endGroup();

    m_logger->log({ruleId.toString(), "removed"}, {{"name", rule.name()}});

    if (!fromEdit)
        emit ruleRemoved(ruleId);


    qCDebug(dcRuleEngine()) << "Rule" << ruleId.toString() << "removed.";

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
    if (rule.enabled())
        return RuleErrorNoError;

    rule.setEnabled(true);
    m_rules[ruleId] = rule;
    saveRule(rule);
    emit ruleConfigurationChanged(rule);

    m_logger->log({rule.id().toString(), "enabled"}, {{"name", rule.name()}});
    qCDebug(dcRuleEngine()) << "Rule" << rule.name() << rule.id().toString() << "enabled.";

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
    if (!rule.enabled())
        return RuleErrorNoError;

    rule.setEnabled(false);
    m_rules[ruleId] = rule;
    saveRule(rule);
    emit ruleConfigurationChanged(rule);

    m_logger->log({rule.id().toString(), "disabled"}, {{"name", rule.name()}});
    qCDebug(dcRuleEngine()) << "Rule" << rule.name() << rule.id().toString() << "disabled.";
    return RuleErrorNoError;
}

/*! Executes the list of \l{Action}{Actions} of the rule with the given \a ruleId.
    Returns the corresponding RuleEngine::RuleError to inform about the result.

    \sa executeExitActions()
*/
RuleEngine::RuleError RuleEngine::executeActions(const RuleId &ruleId)
{
    // check if rule exists
    if (!m_rules.contains(ruleId)) {
        qCWarning(dcRuleEngine) << "Not executing rule actions: Rule not found.";
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

    qCDebug(dcRuleEngine) << "Executing rule actions of rule" << rule.name() << rule.id().toString();
    m_logger->log({rule.id().toString(), "executed"}, {{"name", rule.name()}});
    executeRuleActions(rule.id(), rule.actions());
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

    qCDebug(dcRuleEngine) << "Executing rule exit actions of rule" << rule.name() << rule.id().toString();
    m_logger->log({rule.id().toString(), "executed"}, {{"name", rule.name()}});
//    m_logEngine->logRuleExitActionsExecuted(rule);
    executeRuleActions(rule.id(), rule.exitActions());
    return RuleErrorNoError;
}

Rule RuleEngine::findRule(const RuleId &ruleId)
{
    if (!m_rules.contains(ruleId))
        return Rule();

    return m_rules.value(ruleId);
}

QList<RuleId> RuleEngine::findRules(const ThingId &thingId) const
{
    // Find all offending rules
    QList<RuleId> offendingRules;
    foreach (const Rule &rule, m_rules) {
        bool offending = false;
        foreach (const EventDescriptor &eventDescriptor, rule.eventDescriptors()) {
            if (eventDescriptor.thingId() == thingId) {
                offending = true;
                break;
            }
        }

        if (!offending && rule.stateEvaluator().containsThing(thingId))
            offending = true;

        if (!offending) {
            foreach (const RuleAction &action, rule.actions()) {
                if (action.thingId() == thingId) {
                    offending = true;
                    break;
                }
                foreach (const RuleActionParam &ruleActionParam, action.ruleActionParams()) {
                    if (ruleActionParam.stateThingId() == thingId) {
                        offending = true;
                        break;
                    }
                }
                if (offending) {
                    break;
                }
            }
        }

        if (!offending) {
            foreach (const RuleAction &action, rule.exitActions()) {
                if (action.thingId() == thingId) {
                    offending = true;
                    break;
                }
                foreach (const RuleActionParam &ruleActionParam, action.ruleActionParams()) {
                    if (ruleActionParam.stateThingId() == thingId) {
                        offending = true;
                        break;
                    }
                }
                if (offending) {
                    break;
                }
            }
        }

        if (offending)
            offendingRules.append(rule.id());

    }
    return offendingRules;
}

/*! Returns all \l Things that are contained in a rule */
QList<ThingId> RuleEngine::thingsInRules() const
{
    QList<ThingId> tmp;
    foreach (const Rule &rule, m_rules) {
        foreach (const EventDescriptor &descriptor, rule.eventDescriptors()) {
            if (!tmp.contains(descriptor.thingId()) && !descriptor.thingId().isNull()) {
                tmp.append(descriptor.thingId());
            }
        }
        foreach (const ThingId &thingId, rule.stateEvaluator().containedThings()) {
            if (!tmp.contains(thingId) && !thingId.isNull()) {
                tmp.append(thingId);
            }
        }
        foreach (const RuleAction &action, rule.actions()) {
            if (!tmp.contains(action.thingId()) && !action.thingId().isNull()) {
                tmp.append(action.thingId());
            }
        }
        foreach (const RuleAction &exitAction, rule.exitActions()) {
            if (!tmp.contains(exitAction.thingId()) && !exitAction.thingId().isNull()) {
                tmp.append(exitAction.thingId());
            }
        }
    }
    return tmp;
}

void RuleEngine::removeThingFromRule(const RuleId &id, const ThingId &thingId)
{
    if (!m_rules.contains(id))
        return;

    Rule rule = m_rules.value(id);

    // remove thing from eventDescriptors
    QList<EventDescriptor> eventDescriptors = rule.eventDescriptors();
    QList<int> removeIndexes;
    for (int i = 0; i < eventDescriptors.count(); i++) {
        if (eventDescriptors.at(i).thingId() == thingId) {
            removeIndexes.append(i);
        }
    }
    while (removeIndexes.count() > 0) {
        eventDescriptors.takeAt(removeIndexes.takeLast());
    }

    // remove thing from state evaluators
    StateEvaluator stateEvalatuator = rule.stateEvaluator();
    stateEvalatuator.removeThing(thingId);

    // remove thing from actions
    QList<RuleAction> actions = rule.actions();
    for (int i = 0; i < actions.count(); i++) {
        if (actions.at(i).thingId() == thingId) {
            removeIndexes.append(i);
            continue;
        }
        foreach (const RuleActionParam &param, actions.at(i).ruleActionParams()) {
            if (param.stateThingId() == thingId) {
                removeIndexes.append(i);
                break;
            }
        }
    }
    while (removeIndexes.count() > 0) {
        actions.takeAt(removeIndexes.takeLast());
    }

    // remove thing from exit actions
    QList<RuleAction> exitActions = rule.exitActions();
    for (int i = 0; i < exitActions.count(); i++) {
        if (exitActions.at(i).thingId() == thingId) {
            removeIndexes.append(i);
            continue;
        }
        foreach (const RuleActionParam &param, exitActions.at(i).ruleActionParams()) {
            if (param.stateThingId() == thingId) {
                removeIndexes.append(i);
                break;
            }
        }
    }
    while (removeIndexes.count() > 0) {
        exitActions.takeAt(removeIndexes.takeLast());
    }

    // remove the rule from settings
    NymeaSettings settings(NymeaSettings::SettingsRoleRules);
    settings.beginGroup(id.toString());
    settings.remove("");
    settings.endGroup();

    if (actions.isEmpty() && exitActions.isEmpty()) {
        // The rule doesn't have any actions any more and is useless at this point... let's remove it altogether
        qCDebug(dcRuleEngine()) << "Rule" << rule.name() << "(" + rule.id().toString() + ")" << "does not have any actions any more. Removing it.";
        m_rules.take(id);
        emit ruleRemoved(id);
        return;
    }

    Rule newRule;
    newRule.setId(id);
    newRule.setName(rule.name());
    newRule.setEventDescriptors(eventDescriptors);
    newRule.setStateEvaluator(stateEvalatuator);
    newRule.setTimeDescriptor(rule.timeDescriptor());
    newRule.setActions(actions);
    newRule.setExitActions(exitActions);
    m_rules[id] = newRule;

    // save it
    saveRule(newRule);
    emit ruleConfigurationChanged(newRule);
}

bool RuleEngine::containsEvent(const Rule &rule, const Event &event, const ThingClassId &thingClassId)
{
    foreach (const EventDescriptor &eventDescriptor, rule.eventDescriptors()) {
        // If this is a thing based rule, eventTypeId and thingId must match
        if (eventDescriptor.type() == EventDescriptor::TypeThing) {
            if (eventDescriptor.eventTypeId() != event.eventTypeId() ||  eventDescriptor.thingId() != event.thingId()) {
                continue;
            }
        }

        // If this is a interface based rule, the thing must implement the interface
        if (eventDescriptor.type() == EventDescriptor::TypeInterface) {
            ThingClass dc = m_thingManager->findThingClass(thingClassId);
            if (!dc.interfaces().contains(eventDescriptor.interface())) {
                // ThingClass for this event doesn't implement the interface for this eventDescriptor
                continue;
            }

            EventType et = dc.eventTypes().findById(event.eventTypeId());
            StateType st = dc.stateTypes().findById(event.eventTypeId());
            if (et.isValid()) {
                if (et.name() != eventDescriptor.interfaceEvent()) {
                    // The fired event name does not match with the eventDescriptor's interfaceEvent
                    continue;
                }
            } else if (st.isValid()) {
                if (st.name() != eventDescriptor.interfaceEvent()) {
                    // The fired event name does not match with the eventDescriptor's interfaceEvent
                    continue;
                }
            }
        }

        // Ok, either thing/eventTypeId or interface/interfaceEvent are matching. Compare the paramdescriptor
        bool allOK = true;
        foreach (const ParamDescriptor &paramDescriptor, eventDescriptor.paramDescriptors()) {
            QVariant paramValue;
            if (!paramDescriptor.paramTypeId().isNull()) {
                paramValue = event.param(paramDescriptor.paramTypeId()).value();
            } else {
                if (paramDescriptor.paramName().isEmpty()) {
                    qWarning(dcRuleEngine()) << "ParamDescriptor invalid. Either paramTypeId or paramName are required";
                    allOK = false;
                    continue;
                }
                ThingClass dc = m_thingManager->findThingClass(thingClassId);
                EventType et = dc.eventTypes().findById(event.eventTypeId());
                StateType st = dc.stateTypes().findById(event.eventTypeId());
                if (et.isValid()) {
                    ParamType pt = et.paramTypes().findByName(paramDescriptor.paramName());
                    paramValue = event.param(pt.id()).value();
                } else if (st.isValid()) {
                    paramValue = event.paramValue(st.id());
                }
            }

            switch (paramDescriptor.operatorType()) {
            case Types::ValueOperatorEquals:
                if (paramValue != paramDescriptor.value()) {
                    allOK = false;
                }
                break;
            case Types::ValueOperatorNotEquals:
                if (paramValue == paramDescriptor.value()) {
                    allOK = false;
                }
                break;
            case Types::ValueOperatorGreater:
                if (event.param(paramDescriptor.paramTypeId()).value() <= paramDescriptor.value()) {
                    allOK = false;
                }
                break;
            case Types::ValueOperatorGreaterOrEqual:
                if (event.param(paramDescriptor.paramTypeId()).value() < paramDescriptor.value()) {
                    allOK = false;
                }
                break;
            case Types::ValueOperatorLess:
                if (event.param(paramDescriptor.paramTypeId()).value() >= paramDescriptor.value()) {
                    allOK = false;
                }
                break;
            case Types::ValueOperatorLessOrEqual:
                if (event.param(paramDescriptor.paramTypeId()).value() < paramDescriptor.value()) {
                    allOK = false;
                }
                break;
            }
        }
        // All matching!
        if (allOK) {
            return true;
        }
    }

    qCDebug(dcRuleEngineDebug()) << "Rule" << rule.name() << "does not match event descriptors";
    return false;
}

bool RuleEngine::containsState(const StateEvaluator &stateEvaluator, const Event &stateChangeEvent)
{
    if (stateEvaluator.stateDescriptor().isValid()) {
        if (stateEvaluator.stateDescriptor().type() == StateDescriptor::TypeThing) {
            if (stateEvaluator.stateDescriptor().thingId() == stateChangeEvent.thingId() && stateEvaluator.stateDescriptor().stateTypeId() == stateChangeEvent.eventTypeId()) {
                return true;
            }
            if (stateEvaluator.stateDescriptor().valueThingId() == stateChangeEvent.thingId() && stateEvaluator.stateDescriptor().valueStateTypeId() == stateChangeEvent.eventTypeId()) {
                return true;
            }
        } else {
            Thing *thing = m_thingManager->findConfiguredThing(stateChangeEvent.thingId());
            ThingClass thingClass = m_thingManager->findThingClass(thing->thingClassId());
            if (thingClass.interfaces().contains(stateEvaluator.stateDescriptor().interface())) {
                return true;
            }
        }
    }

    foreach (const StateEvaluator &childEvaluator, stateEvaluator.childEvaluators()) {
        if (containsState(childEvaluator, stateChangeEvent)) {
            return true;
        }
    }

    return false;
}

RuleEngine::RuleError RuleEngine::checkRuleAction(const RuleAction &ruleAction, const Rule &rule)
{
    if (!ruleAction.isValid()) {
        qWarning(dcRuleEngine()) << "Action is incomplete. It must have either thingId and actionTypeId/browserItemId, or interface and interfaceAction:" << ruleAction;
        return RuleErrorActionTypeNotFound;
    }

    ActionType actionType;
    if (ruleAction.type() == RuleAction::TypeThing) {
        Thing *thing = m_thingManager->findConfiguredThing(ruleAction.thingId());
        if (!thing) {
            qCWarning(dcRuleEngine) << "Cannot create rule. No configured thing with ID" << ruleAction.thingId();
            return RuleErrorThingNotFound;
        }

        ThingClass thingClass = m_thingManager->findThingClass(thing->thingClassId());
        if (!thingClass.hasActionType(ruleAction.actionTypeId())) {
            qCWarning(dcRuleEngine) << "Cannot create rule. Thing " + thing->name() + " has no action type:" << ruleAction.actionTypeId();
            return RuleErrorActionTypeNotFound;
        }

        actionType = thingClass.actionTypes().findById(ruleAction.actionTypeId());
    } else if (ruleAction.type() == RuleAction::TypeInterface) {
        Interface iface = m_thingManager->supportedInterfaces().findByName(ruleAction.interface());
        if (!iface.isValid()) {
            qCWarning(dcRuleEngine()) << "Cannot create rule. No such interface:" << ruleAction.interface();
            return RuleError::RuleErrorInterfaceNotFound;
        }
        actionType = iface.actionTypes().findByName(ruleAction.interfaceAction());
        if (actionType.name().isEmpty()) {
            qCWarning(dcRuleEngine()) << "Cannot create rule. Interface" << iface.name() << "does not implement action" << ruleAction.interfaceAction();
            return RuleError::RuleErrorActionTypeNotFound;
        }
    } else if (ruleAction.type() == RuleAction::TypeBrowser) {
        Thing *thing = m_thingManager->findConfiguredThing(ruleAction.thingId());
        if (!thing) {
            qCWarning(dcRuleEngine) << "Cannot create rule. No configured thing with ID" << ruleAction.thingId();
            return RuleErrorThingNotFound;
        }
        if (ruleAction.browserItemId().isEmpty()) {
            qCWarning(dcRuleEngine()) << "Cannot create rule with empty browserItemId";
            return RuleErrorInvalidRuleActionParameter;
        }

    } else {
        return RuleErrorActionTypeNotFound;
    }

    // Not all rule actions might have an actiontype (e.g. browser item executions)
    if (!actionType.id().isNull()) {
        // Verify given params
        foreach (const RuleActionParam &ruleActionParam, ruleAction.ruleActionParams()) {
            RuleError ruleActionParamError = checkRuleActionParam(ruleActionParam, actionType, rule);
            if (ruleActionParamError != RuleErrorNoError) {
                return ruleActionParamError;
            }
        }

        // Verify all required params are given
        foreach (const ParamType &paramType, actionType.paramTypes()) {
            bool found = !paramType.defaultValue().isNull();
            foreach (const RuleActionParam &ruleActionParam, ruleAction.ruleActionParams()) {
                if (ruleActionParam.paramTypeId() == paramType.id()
                        || ruleActionParam.paramName() == paramType.name()) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                return RuleErrorMissingParameter;
            }
        }
    }

    return RuleErrorNoError;
}

RuleEngine::RuleError RuleEngine::checkRuleActionParam(const RuleActionParam &ruleActionParam, const ActionType &actionType, const Rule &rule)
{
    // Check param identifier (either paramTypeId or paramName)
    ParamType paramType;
    if (!ruleActionParam.paramTypeId().isNull()) {
        paramType = actionType.paramTypes().findById(ruleActionParam.paramTypeId());
    } else if (!ruleActionParam.paramName().isEmpty()) {
        paramType = actionType.paramTypes().findByName(ruleActionParam.paramName());
    } else {
        return RuleErrorInvalidRuleActionParameter;
    }

    if (ruleActionParam.isEventBased()) {
        // We have an eventTypeId, see if the rule actually has such a event
        bool found = false;
        foreach (const EventDescriptor &ed, rule.eventDescriptors()) {
            if (ed.eventTypeId() == ruleActionParam.eventTypeId()) {
                found = true;
            }
        }
        if (!found) {
            qCWarning(dcRuleEngine) << "Cannot create rule. EventTypeId" << ruleActionParam.eventTypeId() << "not found in rule's eventDescriptors.";
            return RuleErrorInvalidRuleActionParameter;
        }

        // check if the param type of the event and the action match
        QMetaType::Type eventParamType = getEventParamType(ruleActionParam.eventTypeId(), ruleActionParam.eventParamTypeId());
        QVariant v(eventParamType);
        if (eventParamType != paramType.type() && !v.canConvert(static_cast<int>(paramType.type()))) {
            qCWarning(dcRuleEngine) << "Cannot create rule. RuleActionParam" << ruleActionParam.paramTypeId().toString() << " and given event param " << ruleActionParam.eventParamTypeId().toString() << "have not the same type:";
            qCWarning(dcRuleEngine) << "        -> actionParamType:" << paramType.type();
            qCWarning(dcRuleEngine) << "        ->  eventParamType:" << eventParamType;
            return RuleErrorTypesNotMatching;
        }
    } else if (ruleActionParam.isStateBased()) {
        Thing *d = m_thingManager->findConfiguredThing(ruleActionParam.stateThingId());
        if (!d) {
            qCWarning(dcRuleEngine()) << "Cannot create Rule. ThingId from RuleActionParam" << ruleActionParam.paramTypeId() << "not found in system.";
            return RuleErrorThingNotFound;
        }
        ThingClass stateThingClass = m_thingManager->findThingClass(d->thingClassId());
        StateType stateType = stateThingClass.stateTypes().findById(ruleActionParam.stateTypeId());
        QMetaType::Type actionParamType = getActionParamType(actionType.id(), ruleActionParam.paramTypeId());
        QVariant v(stateType.type());
        if (actionParamType != stateType.type() && !v.canConvert(static_cast<int>(actionParamType))) {
            qCWarning(dcRuleEngine) << "Cannot create rule. RuleActionParam" << ruleActionParam.paramTypeId().toString() << " and given state based param " << ruleActionParam.stateTypeId().toString() << "have not the same type:";
            qCWarning(dcRuleEngine) << "        -> actionParamType:" << actionParamType;
            qCWarning(dcRuleEngine) << "        ->       stateType:" << stateType.type();
            return RuleErrorTypesNotMatching;
        }
    } else { // Is value based
        if (ruleActionParam.value().isNull()) {
            qCDebug(dcRuleEngine()) << "Cannot create rule. No param value given for action:" << ruleActionParam.paramTypeId().toString();
            return RuleErrorInvalidRuleActionParameter;
        }
        if (paramType.type() != ruleActionParam.value().type() && !ruleActionParam.value().canConvert(static_cast<int>(paramType.type()))) {
            qCWarning(dcRuleEngine) << "Cannot create rule. RuleActionParam" << ruleActionParam.paramTypeId().toString() << " and given state based param " << ruleActionParam.stateTypeId().toString() << "have not the same type:";
            qCWarning(dcRuleEngine) << "        -> actionParamType:" << paramType.type();
            qCWarning(dcRuleEngine) << "        ->       stateType:" << ruleActionParam.value().type();
            return RuleErrorTypesNotMatching;
        }
    }

    return RuleErrorNoError;
}

QMetaType::Type RuleEngine::getActionParamType(const ActionTypeId &actionTypeId, const ParamTypeId &paramTypeId)
{
    foreach (const ThingClass &thingClass, m_thingManager->supportedThings()) {
        foreach (const ActionType &actionType, thingClass.actionTypes()) {
            if (actionType.id() == actionTypeId) {
                foreach (const ParamType &paramType, actionType.paramTypes()) {
                    if (paramType.id() == paramTypeId) {
                        return paramType.type();
                    }
                }
            }
        }
    }

    return QMetaType::UnknownType;
}

QMetaType::Type RuleEngine::getEventParamType(const EventTypeId &eventTypeId, const ParamTypeId &paramTypeId)
{
    foreach (const ThingClass &thingClass, m_thingManager->supportedThings()) {
        foreach (const EventType &eventType, thingClass.eventTypes()) {
            if (eventType.id() == eventTypeId) {
                foreach (const ParamType &paramType, eventType.paramTypes()) {
                    if (paramType.id() == paramTypeId) {
                        return paramType.type();
                    }
                }
            }
        }
        foreach (const StateType &stateType, thingClass.stateTypes()) {
            if (stateType.id() == eventTypeId) {
                return stateType.type();
            }
        }
    }

    return QMetaType::UnknownType;
}

void RuleEngine::appendRule(const Rule &rule)
{
    Rule newRule = rule;
    newRule.setStatesActive(newRule.stateEvaluator().evaluate());
    newRule.setTimeActive(newRule.timeDescriptor().evaluate(QDateTime(), QDateTime::currentDateTime()));
    qCDebug(dcRuleEngine()) << "Adding Rule:" << newRule;
    m_rules.insert(rule.id(), newRule);
    m_ruleIds.append(rule.id());
}

void RuleEngine::saveRule(const Rule &rule)
{
    NymeaSettings settings(NymeaSettings::SettingsRoleRules);
    settings.beginGroup(rule.id().toString());
    settings.setValue("name", rule.name());
    settings.setValue("enabled", rule.enabled());
    settings.setValue("executable", rule.executable());

    // Save timeDescriptor
    settings.beginGroup("timeDescriptor");
    if (!rule.timeDescriptor().isEmpty()) {
        settings.beginGroup("calendarItems");
        for (int i = 0; i < rule.timeDescriptor().calendarItems().count(); i++) {
            settings.beginGroup("CalendarItem-" + QString::number(i));

            CalendarItem calendarItem = rule.timeDescriptor().calendarItems().at(i);
            if (calendarItem.dateTime().isValid())
                settings.setValue("dateTime", calendarItem.dateTime().toSecsSinceEpoch());

            if (calendarItem.startTime().isValid())
                settings.setValue("startTime", calendarItem.startTime().toString("hh:mm"));

            settings.setValue("duration", calendarItem.duration());
            settings.setValue("mode", calendarItem.repeatingOption().mode());

            // Save weekDays
            settings.beginWriteArray("weekDays");
            for (int i = 0; i < calendarItem.repeatingOption().weekDays().count(); ++i) {
                settings.setArrayIndex(i);
                settings.setValue("weekDay", calendarItem.repeatingOption().weekDays().at(i));
            }
            settings.endArray();

            // Save monthDays
            settings.beginWriteArray("monthDays");
            for (int i = 0; i < calendarItem.repeatingOption().monthDays().count(); ++i) {
                settings.setArrayIndex(i);
                settings.setValue("monthDay", calendarItem.repeatingOption().monthDays().at(i));
            }
            settings.endArray();

            settings.endGroup();
        }
        settings.endGroup();

        settings.beginGroup("timeEventItems");
        for (int i = 0; i < rule.timeDescriptor().timeEventItems().count(); i++) {
            settings.beginGroup("TimeEventItem-" + QString::number(i));
            TimeEventItem timeEventItem = rule.timeDescriptor().timeEventItems().at(i);

            if (timeEventItem.dateTime().isValid())
                settings.setValue("dateTime", timeEventItem.dateTime().toSecsSinceEpoch());

            if (timeEventItem.time().isValid())
                settings.setValue("time", timeEventItem.time().toString("hh:mm"));

            settings.setValue("mode", timeEventItem.repeatingOption().mode());

            // Save weekDays
            settings.beginWriteArray("weekDays");
            for (int i = 0; i < timeEventItem.repeatingOption().weekDays().count(); ++i) {
                settings.setArrayIndex(i);
                settings.setValue("weekDay", timeEventItem.repeatingOption().weekDays().at(i));
            }
            settings.endArray();

            // Save monthDays
            settings.beginWriteArray("monthDays");
            for (int i = 0; i < timeEventItem.repeatingOption().monthDays().count(); ++i) {
                settings.setArrayIndex(i);
                settings.setValue("monthDay", timeEventItem.repeatingOption().monthDays().at(i));
            }
            settings.endArray();

            settings.endGroup();
        }
        settings.endGroup();
    }
    settings.endGroup();

    // Save Events / EventDescriptors
    settings.beginGroup("events");
    for (int i = 0; i < rule.eventDescriptors().count(); i++) {
        EventDescriptor eventDescriptor = rule.eventDescriptors().at(i);
        settings.beginGroup("EventDescriptor-" + QString::number(i));
        settings.setValue("thingId", eventDescriptor.thingId().toString());
        settings.setValue("eventTypeId", eventDescriptor.eventTypeId().toString());
        settings.setValue("interface", eventDescriptor.interface());
        settings.setValue("interfaceEvent", eventDescriptor.interfaceEvent());

        foreach (const ParamDescriptor &paramDescriptor, eventDescriptor.paramDescriptors()) {
            if (!paramDescriptor.paramTypeId().isNull()) {
                settings.beginGroup("ParamDescriptor-" + paramDescriptor.paramTypeId().toString());
            } else {
                settings.beginGroup("ParamDescriptor-" + paramDescriptor.paramName());
            }
            settings.setValue("valueType", static_cast<int>(paramDescriptor.value().type()));
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
    settings.beginGroup("ruleActions");
    saveRuleActions(&settings, rule.actions());
    settings.endGroup();

    // Save ruleExitActions
    settings.beginGroup("ruleExitActions");
    saveRuleActions(&settings, rule.exitActions());
    settings.endGroup();
    qCDebug(dcRuleEngineDebug()) << "Saved rule to config:" << rule;
}

void RuleEngine::saveRuleActions(NymeaSettings *settings, const QList<RuleAction> &ruleActions)
{
    int i = 0;
    foreach (const RuleAction &action, ruleActions) {
        settings->beginGroup(QString::number(i));
        if (action.type() == RuleAction::TypeThing) {
            settings->setValue("thingId", action.thingId().toString());
            settings->setValue("actionTypeId", action.actionTypeId().toString());
        } else if (action.type() == RuleAction::TypeBrowser) {
            settings->setValue("thingId", action.thingId().toString());
            settings->setValue("browserItemId", action.browserItemId());
        } else if (action.type() == RuleAction::TypeInterface){
            settings->setValue("interface", action.interface());
            settings->setValue("interfaceAction", action.interfaceAction());
        } else {
            Q_ASSERT_X(false, "RuleEngine::saveRule", "Unhandled rule action type.");
        }
        foreach (const RuleActionParam &param, action.ruleActionParams()) {
            if (!param.paramTypeId().isNull()) {
                settings->beginGroup("RuleActionParam-" + param.paramTypeId().toString());
            } else {
                settings->beginGroup("RuleActionParam-" + param.paramName());
            }
            settings->setValue("valueType", static_cast<int>(param.value().type()));
            settings->setValue("value", param.value());
            if (param.isEventBased()) {
                settings->setValue("eventTypeId", param.eventTypeId().toString());
                settings->setValue("eventParamTypeId", param.eventParamTypeId());
            } else if (param.isStateBased()) {
                settings->setValue("stateThingId", param.stateThingId().toString());
                settings->setValue("stateTypeId", param.stateTypeId());
            }
            settings->endGroup();
        }
        i++;
        settings->endGroup();
    }
}

QList<RuleAction> RuleEngine::loadRuleActions(NymeaSettings *settings)
{
    QList<RuleAction> actions;
    foreach (const QString &actionNumber, settings->childGroups()) {
        settings->beginGroup(actionNumber);

        RuleActionParams params;
        foreach (QString paramTypeIdString, settings->childGroups()) {
            if (paramTypeIdString.startsWith("RuleActionParam-")) {
                settings->beginGroup(paramTypeIdString);
                QString strippedParamTypeIdString = paramTypeIdString.remove(QRegularExpression("^RuleActionParam-"));
                EventTypeId eventTypeId = EventTypeId(settings->value("eventTypeId", EventTypeId()).toString());
                ParamTypeId eventParamTypeId = ParamTypeId(settings->value("eventParamTypeId", ParamTypeId()).toString());
                ThingId stateThingId = ThingId(settings->value("stateThingId", ThingId()).toString());
                if (stateThingId.isNull()) { // nymea < 0.19
                    stateThingId = ThingId(settings->value("stateDeviceId", ThingId()).toString());
                }
                StateTypeId stateTypeId = StateTypeId(settings->value("stateTypeId", StateTypeId()).toString());
                QVariant value = settings->value("value");
                if (settings->contains("valueType")) {
                    QMetaType::Type valueType = static_cast<QMetaType::Type>(settings->value("valueType").toInt());
                    // Note: only warn, and continue with the QVariant guessed type
                    if (valueType == QMetaType::UnknownType) {
                        qCWarning(dcRuleEngine()) << "Could not load the value type of the rule action param " << strippedParamTypeIdString << ". The value type will be guessed by QVariant.";
                    } else if (!value.canConvert(static_cast<int>(valueType))) {
                        qCWarning(dcRuleEngine()) << "Error loading rule action. Could not convert the rule action param value" << value << "to the stored type" << valueType;
                    } else {
                        value.convert(static_cast<int>(valueType));
                    }
                }

                RuleActionParam param;
                if (!ParamTypeId(strippedParamTypeIdString).isNull()) {
                    // By ParamTypeId
                    param = RuleActionParam(ParamTypeId(strippedParamTypeIdString), value);
                } else {
                    // By param name
                    param = RuleActionParam(strippedParamTypeIdString, value);
                }
                param.setEventTypeId(eventTypeId);
                param.setEventParamTypeId(eventParamTypeId);
                param.setStateThingId(stateThingId);
                param.setStateTypeId(stateTypeId);
                params.append(param);
                settings->endGroup();
            }
        }

        if (settings->contains("actionTypeId") && settings->contains("thingId")) {
            RuleAction action = RuleAction(ActionTypeId(settings->value("actionTypeId").toString()), ThingId(settings->value("thingId").toString()));
            action.setRuleActionParams(params);
            actions.append(action);
        } else if (settings->contains("actionTypeId") && settings->contains("deviceId")) {
            // nymea  < 0.19
            RuleAction action = RuleAction(ActionTypeId(settings->value("actionTypeId").toString()), ThingId(settings->value("deviceId").toString()));
            action.setRuleActionParams(params);
            actions.append(action);
        } else if (settings->contains("thingId") && settings->contains("browserItemId")) {
            RuleAction action = RuleAction(ThingId(settings->value("thingId").toString()), settings->value("browserItemId").toString());
            actions.append(action);
        } else if (settings->contains("deviceId") && settings->contains("browserItemId")) {
            // nymea  < 0.19
            RuleAction action = RuleAction(ThingId(settings->value("deviceId").toString()), settings->value("browserItemId").toString());
            actions.append(action);
        } else if (settings->contains("interface") && settings->contains("interfaceAction")){
            RuleAction action = RuleAction(settings->value("interface").toString(), settings->value("interfaceAction").toString());
            action.setRuleActionParams(params);
            actions.append(action);
        }

        settings->endGroup();
    }
    return actions;
}

void RuleEngine::executeRuleActions(const RuleId &ruleId, const QList<RuleAction> &ruleActions)
{
    QList<Action> actions;
    QList<BrowserAction> browserActions;
    foreach (const RuleAction &ruleAction, ruleActions) {
        if (ruleAction.type() == RuleAction::TypeThing) {
            Thing *thing = m_thingManager->findConfiguredThing(ruleAction.thingId());
            if (!thing) {
                qCWarning(dcRuleEngine()) << "Unable to find thing" << ruleAction.thingId() << "for rule action" << ruleAction;
                continue;
            }
            ActionTypeId actionTypeId = ruleAction.actionTypeId();
            ParamList params;
            bool ok = true;
            foreach (const RuleActionParam &ruleActionParam, ruleAction.ruleActionParams()) {
                if (ruleActionParam.isValueBased()) {
                    params.append(Param(ruleActionParam.paramTypeId(), ruleActionParam.value()));
                } else if (ruleActionParam.isStateBased()) {
                    Thing *stateThing = m_thingManager->findConfiguredThing(ruleActionParam.stateThingId());
                    if (!stateThing) {
                        qCWarning(dcRuleEngine()) << "Cannot find thing" << ruleActionParam.stateThingId() << "required by rule action";
                        ok = false;
                        break;
                    }
                    ThingClass stateThingClass = m_thingManager->findThingClass(stateThing->thingClassId());
                    if (!stateThingClass.hasStateType(ruleActionParam.stateTypeId())) {
                        qCWarning(dcRuleEngine()) << "Device" << thing->name() << thing->id() << "does not have a state type" << ruleActionParam.stateTypeId();
                        ok = false;
                        break;
                    }
                    params.append(Param(ruleActionParam.paramTypeId(), stateThing->stateValue(ruleActionParam.stateTypeId())));
                }
            }
            if (!ok) {
                qCWarning(dcRuleEngine()) << "Not executing rule action";
                continue;
            }
            Action action(actionTypeId, thing->id(), Action::TriggeredByRule);
            action.setParams(params);
            actions.append(action);
        } else if (ruleAction.type() == RuleAction::TypeBrowser) {
            Thing *thing = m_thingManager->findConfiguredThing(ruleAction.thingId());
            if (!thing) {
                qCWarning(dcRuleEngine()) << "Unable to find thing" << ruleAction.thingId() << "for rule action" << ruleAction;
                continue;
            }
            BrowserAction browserAction(ruleAction.thingId(), ruleAction.browserItemId());
            browserActions.append(browserAction);
        } else {
            Things things = m_thingManager->findConfiguredThings(ruleAction.interface());
            foreach (Thing* thing, things) {
                ThingClass thingClass = m_thingManager->findThingClass(thing->thingClassId());
                ActionType actionType = thingClass.actionTypes().findByName(ruleAction.interfaceAction());
                if (actionType.id().isNull()) {
                    qCWarning(dcRuleEngine()) << "Error creating Action. The given ThingClass does not implement action:" << ruleAction.interfaceAction();
                    continue;
                }

                ParamList params;
                bool ok = true;
                foreach (const RuleActionParam &ruleActionParam, ruleAction.ruleActionParams()) {
                    ParamType paramType = actionType.paramTypes().findByName(ruleActionParam.paramName());
                    if (paramType.id().isNull()) {
                        qCWarning(dcRuleEngine()) << "Error creating Action. The given ActionType does not have a parameter:" << ruleActionParam.paramName();
                        ok = false;
                        continue;
                    }
                    if (ruleActionParam.isValueBased()) {
                        params.append(Param(paramType.id(), ruleActionParam.value()));
                    } else if (ruleActionParam.isStateBased()) {
                        Thing *stateThing = m_thingManager->findConfiguredThing(ruleActionParam.stateThingId());
                        if (!stateThing) {
                            qCWarning(dcRuleEngine()) << "Cannot find thing" << ruleActionParam.stateThingId() << "required by rule action";
                            ok = false;
                            break;
                        }
                        ThingClass stateThingClass = m_thingManager->findThingClass(stateThing->thingClassId());
                        if (!stateThingClass.hasStateType(ruleActionParam.stateTypeId())) {
                            qCWarning(dcRuleEngine()) << "Thing" << thing->name() << thing->id() << "does not have a state type" << ruleActionParam.stateTypeId();
                            ok = false;
                            break;
                        }
                        params.append(Param(paramType.id(), stateThing->stateValue(ruleActionParam.stateTypeId())));
                    }
                }
                if (!ok) {
                    qCWarning(dcRuleEngine()) << "Not executing rule action";
                    continue;
                }

                Action action = Action(actionType.id(), thing->id(), Action::TriggeredByRule);
                action.setParams(params);
                actions.append(action);
            }
        }
    }

    foreach (const Action &action, actions) {
        qCDebug(dcRuleEngine) << "Executing action" << action.actionTypeId() << action.params();
        ThingActionInfo *info = m_thingManager->executeAction(action);
        connect(info, &ThingActionInfo::finished, this, [=](){
            if (info->status() != Thing::ThingErrorNoError) {
                qCWarning(dcRuleEngine) << "Error executing action:" << info->status() << info->displayMessage();
            }
            ActionType actionType = m_thingManager->findConfiguredThing(action.thingId())->thingClass().actionTypes().findById(action.actionTypeId());
            m_logger->log({ruleId.toString(), "executed"}, {
                              {"name", m_rules.value(ruleId).name()},
                              {"status", QMetaEnum::fromType<Thing::ThingError>().valueToKey(info->status())},
                              {"thingId", info->action().thingId()},
                              {"action", actionType.name()}
                          });
        });
    }

    foreach (const BrowserAction &browserAction, browserActions) {
        BrowserActionInfo *info = m_thingManager->executeBrowserItem(browserAction);
        connect(info, &BrowserActionInfo::finished, this, [this, ruleId, info](){
            if (info->status() != Thing::ThingErrorNoError) {
                qCWarning(dcRuleEngine) << "Error executing browser action:" << info->status();
                m_logger->log({ruleId.toString(), "executed"}, {
                                  {"name", m_rules.value(ruleId).name()},
                                  {"status", QMetaEnum::fromType<Thing::ThingError>().valueToKey(info->status())},
                                  {"thingId", info->browserAction().thingId()},
                                  {"browserItem", info->browserAction().itemId()}
                              });
            }
        });
    }
}

void RuleEngine::onEventTriggered(const Event &event)
{
    foreach (const Rule &rule, evaluateEvent(event)) {
        if (m_executingRules.contains(rule.id())) {
            qCWarning(dcRuleEngine()) << "WARNING: Loop detected in rule execution for rule" << rule.id().toString() << rule.name();
            break;
        }
        m_executingRules.append(rule.id());

        // Event based
        if (!rule.eventDescriptors().isEmpty()) {
            m_logger->log({rule.id().toString()}, {
                              {"name", rule.name()},
                              {"state", "triggered"}
                          });

            QList<RuleAction> tmp;
            if (rule.statesActive() && rule.timeActive()) {
                qCDebug(dcRuleEngineDebug()) << "Executing actions";
                tmp = rule.actions();
            } else {
                qCDebug(dcRuleEngineDebug()) << "Executing exitActions";
                tmp = rule.exitActions();
            }
            // check if we have an event based action or a normal action
            QList<RuleAction> actions;
            foreach (RuleAction action, tmp) {
                if (action.isEventBased()) {
                    RuleActionParams newParams;
                    foreach (RuleActionParam ruleActionParam, action.ruleActionParams()) {
                        // if this event param should be taken over in this action
                        if (event.eventTypeId() == ruleActionParam.eventTypeId()) {
                            QVariant eventValue = event.params().paramValue(ruleActionParam.eventParamTypeId());

                            // TODO: limits / scale calculation -> actionValue = eventValue * x
                            //       something like a EventParamDescriptor

                            ruleActionParam.setValue(eventValue);
                            qCDebug(dcRuleEngine) << "Using param value from event:" << ruleActionParam.value();
                        }
                        newParams.append(ruleActionParam);
                    }
                    action.setRuleActionParams(newParams);
                    actions.append(action);
                } else {
                    actions.append(action);
                }
            }
            executeRuleActions(rule.id(), actions);

        } else {
            // State based rule
            m_logger->log({rule.id().toString()}, {
                              {"name", rule.name()},
                              {"state", rule.active() ? "active" : "inactive"}
                          });
            emit ruleActiveChanged(rule);
            if (rule.active()) {
                executeRuleActions(rule.id(), rule.actions());
            } else {
                executeRuleActions(rule.id(), rule.exitActions());
            }
        }
    }

    m_executingRules.clear();
}

void RuleEngine::onDateTimeChanged(const QDateTime &dateTime)
{
    foreach (const Rule &rule, evaluateTime(dateTime)) {
        // TimeEvent based
        QList<RuleAction> actions;
        if (!rule.timeDescriptor().timeEventItems().isEmpty()) {
            m_logger->log({rule.id().toString(), "triggered"}, {{"name", rule.name()}});
            if (rule.statesActive() && rule.timeActive()) {
                actions.append(rule.actions());
            } else {
                actions.append(rule.exitActions());
            }
        } else {
            // Calendar based rule
            m_logger->log({rule.id().toString(), "triggered"}, {{"name", rule.name()}});
            emit ruleActiveChanged(rule);
            if (rule.active()) {
                actions.append(rule.actions());
            } else {
                actions.append(rule.exitActions());
            }
        }
        executeRuleActions(rule.id(), actions);
    }
}

void RuleEngine::onThingRemoved(const ThingId &thingId)
{
    QList<RuleId> affectedRules;

    foreach (const RuleId &ruleId, findRules(thingId)) {
        if (!affectedRules.contains(ruleId)) {
            affectedRules.append(ruleId);
        }
    }

    while (!affectedRules.isEmpty()) {
        removeRule(affectedRules.takeFirst());
    }
}

void RuleEngine::init()
{
    NymeaSettings settings(NymeaSettings::SettingsRoleRules);
    qCDebug(dcRuleEngine) << "Loading rules from" << settings.fileName();
    foreach (const QString &idString, settings.childGroups()) {
        settings.beginGroup(idString);

        QString name = settings.value("name", idString).toString();
        bool enabled = settings.value("enabled", true).toBool();
        bool executable = settings.value("executable", true).toBool();

        qCDebug(dcRuleEngine) << "Loading rule" << name << idString;

        // Load timeDescriptor
        TimeDescriptor timeDescriptor;
        QList<CalendarItem> calendarItems;
        QList<TimeEventItem> timeEventItems;

        settings.beginGroup("timeDescriptor");

        settings.beginGroup("calendarItems");
        foreach (const QString &childGroup, settings.childGroups()) {
            settings.beginGroup(childGroup);

            CalendarItem calendarItem;
            calendarItem.setDateTime(QDateTime::fromTime_t(settings.value("dateTime", 0).toUInt()));
            calendarItem.setStartTime(QTime::fromString(settings.value("startTime").toString()));
            calendarItem.setDuration(settings.value("duration", 0).toUInt());

            QList<int> weekDays;
            QList<int> monthDays;
            RepeatingOption::RepeatingMode mode = static_cast<RepeatingOption::RepeatingMode>(settings.value("mode", 0).toInt());

            // Load weekDays
            int weekDaysCount = settings.beginReadArray("weekDays");
            for (int i = 0; i < weekDaysCount; ++i) {
                settings.setArrayIndex(i);
                weekDays.append(settings.value("weekDay", 0).toInt());
            }
            settings.endArray();

            // Load weekDays
            int monthDaysCount = settings.beginReadArray("monthDays");
            for (int i = 0; i < monthDaysCount; ++i) {
                settings.setArrayIndex(i);
                monthDays.append(settings.value("monthDay", 0).toInt());
            }
            settings.endArray();

            settings.endGroup();

            calendarItem.setRepeatingOption(RepeatingOption(mode, weekDays, monthDays));
            calendarItems.append(calendarItem);
        }
        settings.endGroup();

        timeDescriptor.setCalendarItems(calendarItems);

        settings.beginGroup("timeEventItems");
        foreach (const QString &childGroup, settings.childGroups()) {
            settings.beginGroup(childGroup);

            TimeEventItem timeEventItem;
            timeEventItem.setDateTime(QDateTime::fromTime_t(settings.value("dateTime", 0).toUInt()));
            timeEventItem.setTime(QTime::fromString(settings.value("time").toString()));

            QList<int> weekDays;
            QList<int> monthDays;
            RepeatingOption::RepeatingMode mode = static_cast<RepeatingOption::RepeatingMode>(settings.value("mode", 0).toInt());

            // Load weekDays
            int weekDaysCount = settings.beginReadArray("weekDays");
            for (int i = 0; i < weekDaysCount; ++i) {
                settings.setArrayIndex(i);
                weekDays.append(settings.value("weekDay", 0).toInt());
            }
            settings.endArray();

            // Load weekDays
            int monthDaysCount = settings.beginReadArray("monthDays");
            for (int i = 0; i < monthDaysCount; ++i) {
                settings.setArrayIndex(i);
                monthDays.append(settings.value("monthDay", 0).toInt());
            }
            settings.endArray();

            settings.endGroup();

            timeEventItem.setRepeatingOption(RepeatingOption(mode, weekDays, monthDays));
            timeEventItems.append(timeEventItem);
        }
        settings.endGroup();

        settings.endGroup();

        timeDescriptor.setTimeEventItems(timeEventItems);

        // Load events
        QList<EventDescriptor> eventDescriptorList;
        settings.beginGroup("events");
        foreach (QString eventGroupName, settings.childGroups()) {
            if (eventGroupName.startsWith("EventDescriptor-")) {
                settings.beginGroup(eventGroupName);
                EventTypeId eventTypeId(settings.value("eventTypeId").toString());
                ThingId thingId(settings.value("thingId").toString());
                if (thingId.isNull()) { // nymea < 0.19
                    thingId = ThingId(settings.value("deviceId").toString());
                }
                QString interface = settings.value("interface").toString();
                QString interfaceEvent = settings.value("interfaceEvent").toString();

                QList<ParamDescriptor> params;
                foreach (QString groupName, settings.childGroups()) {
                    if (groupName.startsWith("ParamDescriptor-")) {
                        settings.beginGroup(groupName);
                        QString strippedGroupName = groupName.remove(QRegularExpression("^ParamDescriptor-"));

                        QVariant value = settings.value("value");
                        if (settings.contains("valueType")) {
                            QMetaType::Type valueType = static_cast<QMetaType::Type>(settings.value("valueType").toInt());
                            // Note: only warn, and continue with the QVariant guessed type
                            if (valueType == QMetaType::UnknownType) {
                                qCWarning(dcRuleEngine()) << name << idString << "Could not load the value type of the param descriptor" << strippedGroupName << ". The value type will be guessed by QVariant.";
                            } else if (!value.canConvert(static_cast<int>(valueType))) {
                                qCWarning(dcRuleEngine()) << "Error loading rule" << name << idString << ". Could not convert the param descriptor value" << value << "to the stored type" << valueType;
                            } else {
                                value.convert(static_cast<int>(valueType));
                            }
                        }

                        if (!ParamTypeId(strippedGroupName).isNull()) {
                            ParamDescriptor paramDescriptor(ParamTypeId(strippedGroupName), value);
                            paramDescriptor.setOperatorType(static_cast<Types::ValueOperator>(settings.value("operator").toInt()));
                            params.append(paramDescriptor);
                        } else {
                            ParamDescriptor paramDescriptor(strippedGroupName, value);
                            paramDescriptor.setOperatorType(static_cast<Types::ValueOperator>(settings.value("operator").toInt()));
                            params.append(paramDescriptor);
                        }
                        settings.endGroup();
                    }
                }

                if (!eventTypeId.isNull()) {
                    ThingId thingId(settings.value("thingId").toString());
                    if (thingId.isNull()) { // nymea < 0.19
                        thingId = ThingId(settings.value("deviceId").toString());
                    }
                    EventDescriptor eventDescriptor(eventTypeId, thingId, params);
                    eventDescriptorList.append(eventDescriptor);
                } else {
                    EventDescriptor eventDescriptor(interface, interfaceEvent, params);
                    eventDescriptorList.append(eventDescriptor);
                }
                settings.endGroup();
            }
        }
        settings.endGroup();


        // Load stateEvaluator
        StateEvaluator stateEvaluator = StateEvaluator::loadFromSettings(settings, "stateEvaluator");

        // Load actions
        QList<RuleAction> actions;
        settings.beginGroup("ruleActions");
        actions = loadRuleActions(&settings);
        settings.endGroup();

        // Load exit actions
        QList<RuleAction> exitActions;
        settings.beginGroup("ruleExitActions");
        exitActions = loadRuleActions(&settings);
        settings.endGroup();

        Rule rule;
        rule.setId(RuleId(idString));
        rule.setName(name);
        rule.setTimeDescriptor(timeDescriptor);
        rule.setEventDescriptors(eventDescriptorList);
        rule.setStateEvaluator(stateEvaluator);
        rule.setActions(actions);
        rule.setExitActions(exitActions);
        rule.setEnabled(enabled);
        rule.setExecutable(executable);
        rule.setStatesActive(rule.stateEvaluator().evaluate());
        appendRule(rule);
        settings.endGroup();
    }

}

}
