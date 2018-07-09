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
*/

/*! \enum nymeaserver::RuleEngine::RemovePolicy

    \value RemovePolicyCascade
        Remove the whole \l{Rule}.
    \value RemovePolicyUpdate
        Remove a \l{Device} from a rule.
*/


#include "ruleengine.h"
#include "nymeacore.h"
#include "loggingcategories.h"
#include "time/calendaritem.h"
#include "time/repeatingoption.h"
#include "time/timeeventitem.h"

#include "types/eventdescriptor.h"
#include "types/paramdescriptor.h"
#include "nymeasettings.h"
#include "devicemanager.h"
#include "plugin/device.h"

#include <QDebug>
#include <QStringList>
#include <QStandardPaths>
#include <QCoreApplication>

namespace nymeaserver {

/*! Constructs the RuleEngine with the given \a parent. Although it wouldn't harm to have multiple RuleEngines, there is one
    instance available from \l{NymeaCore}. This one should be used instead of creating multiple ones.
 */
RuleEngine::RuleEngine(QObject *parent) :
    QObject(parent)
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
            RepeatingOption::RepeatingMode mode = (RepeatingOption::RepeatingMode)settings.value("mode", 0).toInt();

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
            timeEventItem.setDateTime(settings.value("dateTime", 0).toUInt());
            timeEventItem.setTime(QTime::fromString(settings.value("time").toString()));

            QList<int> weekDays;
            QList<int> monthDays;
            RepeatingOption::RepeatingMode mode = (RepeatingOption::RepeatingMode)settings.value("mode", 0).toInt();

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
                DeviceId deviceId(settings.value("deviceId").toString());
                QString interface = settings.value("interface").toString();
                QString interfaceEvent = settings.value("interfaceEvent").toString();

                QList<ParamDescriptor> params;
                foreach (QString groupName, settings.childGroups()) {
                    if (groupName.startsWith("ParamDescriptor-")) {
                        settings.beginGroup(groupName);
                        QString strippedGroupName = groupName.remove(QRegExp("^ParamDescriptor-"));
                        if (!ParamTypeId(strippedGroupName).isNull()) {
                            ParamDescriptor paramDescriptor(ParamTypeId(strippedGroupName), settings.value("value"));
                            paramDescriptor.setOperatorType((Types::ValueOperator)settings.value("operator").toInt());
                            params.append(paramDescriptor);
                        } else {
                            ParamDescriptor paramDescriptor(strippedGroupName, settings.value("value"));
                            paramDescriptor.setOperatorType((Types::ValueOperator)settings.value("operator").toInt());
                            params.append(paramDescriptor);
                        }
                        settings.endGroup();
                    }
                }

                if (!eventTypeId.isNull()) {
                    EventDescriptor eventDescriptor(eventTypeId, deviceId, params);
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
        foreach (const QString &actionNumber, settings.childGroups()) {
            settings.beginGroup(actionNumber);

            RuleActionParamList params;
            foreach (QString paramTypeIdString, settings.childGroups()) {
                if (paramTypeIdString.startsWith("RuleActionParam-")) {
                    settings.beginGroup(paramTypeIdString);
                    QString strippedParamTypeIdString = paramTypeIdString.remove(QRegExp("^RuleActionParam-"));
                    EventTypeId eventTypeId = EventTypeId(settings.value("eventTypeId", EventTypeId()).toString());
                    ParamTypeId eventParamTypeId = ParamTypeId(settings.value("eventParamTypeId", ParamTypeId()).toString());
                    QVariant value = settings.value("value");
                    if (!ParamTypeId(strippedParamTypeIdString).isNull()) {
                        RuleActionParam param(ParamTypeId(strippedParamTypeIdString),
                                              value,
                                              eventTypeId,
                                              eventParamTypeId);
                        params.append(param);
                    } else {
                        RuleActionParam param(strippedParamTypeIdString,
                                              value,
                                              eventTypeId,
                                              eventParamTypeId);
                        params.append(param);
                    }
                    settings.endGroup();
                }
            }

            if (settings.contains("actionTypeId") && settings.contains("deviceId")) {
                RuleAction action = RuleAction(ActionTypeId(settings.value("actionTypeId").toString()), DeviceId(settings.value("deviceId").toString()));
                action.setRuleActionParams(params);
                actions.append(action);
            } else if (settings.contains("interface") && settings.contains("interfaceAction")){
                RuleAction action = RuleAction(settings.value("interface").toString(), settings.value("interfaceAction").toString());
                action.setRuleActionParams(params);
                actions.append(action);
            }

            settings.endGroup();
        }
        settings.endGroup();

        // Load exit actions
        QList<RuleAction> exitActions;
        settings.beginGroup("ruleExitActions");
        foreach (const QString &actionNumber, settings.childGroups()) {
            settings.beginGroup(actionNumber);

            RuleActionParamList params;
            foreach (QString paramTypeIdString, settings.childGroups()) {
                if (paramTypeIdString.startsWith("RuleActionParam-")) {
                    settings.beginGroup(paramTypeIdString);
                    QString strippedParamTypeIdString = paramTypeIdString.remove(QRegExp("^RuleActionParam-"));
                    QVariant value = settings.value("value");
                    if (!ParamTypeId(strippedParamTypeIdString).isNull()) {
                        RuleActionParam param(ParamTypeId(strippedParamTypeIdString), value);
                        params.append(param);
                    } else {
                        RuleActionParam param(strippedParamTypeIdString, value);
                        params.append(param);
                    }
                    settings.endGroup();
                }
            }

            if (settings.contains("actionTypeId") && settings.contains("deviceId")) {
                RuleAction action = RuleAction(ActionTypeId(settings.value("actionTypeId").toString()), DeviceId(settings.value("deviceId").toString()));
                action.setRuleActionParams(params);
                exitActions.append(action);
            } else if (settings.contains("interface") && settings.contains("interfaceAction")) {
                RuleAction action = RuleAction(settings.value("interface").toString(),settings.value("interfaceAction").toString());
                action.setRuleActionParams(params);
                exitActions.append(action);
            }

            settings.endGroup();
        }
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
    Device *device = NymeaCore::instance()->deviceManager()->findConfiguredDevice(event.deviceId());
    DeviceClass deviceClass = NymeaCore::instance()->deviceManager()->findDeviceClass(device->deviceClassId());
    EventType eventType = deviceClass.eventTypes().findById(event.eventTypeId());


    if (event.params().count() == 0) {
        qCDebug(dcRuleEngineDebug).nospace().noquote() << "Evaluate event: " << device->name() << " - " << eventType.name() << " (DeviceId:" << device->id().toString() << ", EventTypeId:" << eventType.id().toString() << ")";
    } else {
        qCDebug(dcRuleEngineDebug).nospace().noquote() << "Evaluate event: " << device->name() << " - " << eventType.name() << " (DeviceId:" << device->id().toString() << ", EventTypeId:" << eventType.id().toString() << ")" << endl << "     " << event.params();
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
        if (rule.eventDescriptors().isEmpty() && rule.timeDescriptor().timeEventItems().isEmpty()) {
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
            if (containsEvent(rule, event, device->deviceClassId())) {
                qCDebug(dcRuleEngineDebug()).nospace().noquote() << "Rule " << rule.name() << " (" << rule.id().toString() << ") contains event " << event.eventId();
                if (rule.statesActive() && rule.timeActive()) {
                    qCDebug(dcRuleEngine).nospace().noquote() << "Rule " << rule.name() << " (" + rule.id().toString() << ") contains event" << event.eventId() << "and all states match.";
                    rules.append(rule);
                } else {
                    qCDebug(dcRuleEngine).nospace().noquote() << "Rule " << rule.name() << " (" + rule.id().toString() << ") contains event" << event.eventId() << "but state are not matching.";
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

    foreach (const Rule &r, m_rules.values()) {
        Rule rule = m_rules.value(r.id());
        if (!rule.enabled()) {
            qCDebug(dcRuleEngineDebug()) << "Skipping rule" + rule.name() + "because it is disabled";
            continue;
        }

        // If no timeDescriptor, do nothing
        if (rule.timeDescriptor().isEmpty())
            continue;

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
    qCDebug(dcRuleEngine()) << "EvaluateTimeEvent evaluated" << rules.count() << "to be executed";
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
        qCWarning(dcRuleEngine) << "Rule inconsistent.";
        return RuleErrorInvalidRuleFormat;
    }

    // Check IDs in each EventDescriptor
    foreach (const EventDescriptor &eventDescriptor, rule.eventDescriptors()) {
        if (!eventDescriptor.isValid()) {
            qWarning(dcRuleEngine()) << "EventDescriptor is incomplete. It must have either eventTypeId and deviceId, or interface and interfaceEvent";
            return RuleErrorEventTypeNotFound;
        }
        if (eventDescriptor.type() == EventDescriptor::TypeDevice) {
            // check deviceId
            Device *device = NymeaCore::instance()->deviceManager()->findConfiguredDevice(eventDescriptor.deviceId());
            if (!device) {
                qCWarning(dcRuleEngine) << "Cannot create rule. No configured device for eventTypeId" << eventDescriptor.eventTypeId();
                return RuleErrorDeviceNotFound;
            }

            // Check eventTypeId for this deivce
            DeviceClass deviceClass = NymeaCore::instance()->deviceManager()->findDeviceClass(device->deviceClassId());
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
        } else {
            // Interface based event
            Interface iface = NymeaCore::instance()->deviceManager()->supportedInterfaces().findByName(eventDescriptor.interface());
            if (!iface.isValid()) {
                qWarning(dcRuleEngine()) << "No such interface:" << eventDescriptor.interface();
                return RuleErrorInterfaceNotFound;
            }
            if (iface.eventTypes().findByName(eventDescriptor.interfaceEvent()).name().isEmpty()) {
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
    foreach (const RuleAction &action, rule.actions()) {
        if (!action.isValid()) {
            qWarning(dcRuleEngine()) << "Action is incomplete. It must have either actionTypeId and deviceId, or interface and interfaceAction";
            return RuleErrorActionTypeNotFound;
        }
        if (action.type() == RuleAction::TypeDevice) {
            Device *device = NymeaCore::instance()->deviceManager()->findConfiguredDevice(action.deviceId());
            if (!device) {
                qCWarning(dcRuleEngine) << "Cannot create rule. No configured device for action with actionTypeId" << action.actionTypeId();
                return RuleErrorDeviceNotFound;
            }

            DeviceClass deviceClass = NymeaCore::instance()->deviceManager()->findDeviceClass(device->deviceClassId());
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
                        QVariant::Type eventParamType = getEventParamType(ruleActionParam.eventTypeId(), ruleActionParam.eventParamTypeId());
                        QVariant v(eventParamType);
                        QVariant::Type actionParamType = getActionParamType(action.actionTypeId(), ruleActionParam.paramTypeId());
                        if (eventParamType != actionParamType && !v.canConvert(actionParamType)) {
                            qCWarning(dcRuleEngine) << "Cannot create rule. RuleActionParam" << ruleActionParam.paramTypeId().toString() << " and given event param " << ruleActionParam.eventParamTypeId().toString() << "have not the same type:";
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
                        DeviceManager::DeviceError paramCheck = NymeaCore::instance()->deviceManager()->verifyParams(actionType.paramTypes(), finalParams);
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

        } else { // Is TypeInterface
            Interface iface = NymeaCore::instance()->deviceManager()->supportedInterfaces().findByName(action.interface());
            if (!iface.isValid()) {
                qCWarning(dcRuleEngine()) << "Cannot create rule. No such interface:" << action.interface();
                return RuleError::RuleErrorInterfaceNotFound;
            }
            ActionType ifaceActionType = iface.actionTypes().findByName(action.interfaceAction());
            if (ifaceActionType.name().isEmpty()) {
                qCWarning(dcRuleEngine()) << "Cannot create rule. Interface" << iface.name() << "does not implement action" << action.interfaceAction();
                return RuleError::RuleErrorActionTypeNotFound;
            }
            foreach (const ParamType &ifaceActionParamType, ifaceActionType.paramTypes()) {
                if (!action.ruleActionParams().hasParam(ifaceActionParamType.name())) {
                    qCWarning(dcRuleEngine()) << "Cannot create rule. Interface action" << iface.name() << ":" << action.interfaceAction() << "requires a" << ifaceActionParamType.name() << "param of type" << QVariant::typeToName(ifaceActionParamType.type());
                    return RuleError::RuleErrorMissingParameter;
                }
                if (!action.ruleActionParam(ifaceActionParamType.name()).value().canConvert(ifaceActionParamType.type())) {
                    qCWarning(dcRuleEngine()) << "Cannot create rule. Interface action parameter" << iface.name() << ":" << action.interfaceAction() << ":" << ifaceActionParamType.name() << "has wrong type. Expected" << QVariant::typeToName(ifaceActionParamType.type());
                    return RuleError::RuleErrorInvalidParameter;
                }
            }
            // TODO: Check params
        }
    }

    // Check exit actions
    foreach (const RuleAction &ruleAction, rule.exitActions()) {
        if (!ruleAction.isValid()) {
            qWarning(dcRuleEngine()) << "Exit Action is incomplete. It must have either actionTypeId and deviceId, or interface and interfaceAction";
            return RuleErrorActionTypeNotFound;
        }

        if (ruleAction.type() == RuleAction::TypeDevice) {
            Device *device = NymeaCore::instance()->deviceManager()->findConfiguredDevice(ruleAction.deviceId());
            if (!device) {
                qCWarning(dcRuleEngine) << "Cannot create rule. No configured device for exit action with actionTypeId" << ruleAction.actionTypeId();
                return RuleErrorDeviceNotFound;
            }

            DeviceClass deviceClass = NymeaCore::instance()->deviceManager()->findDeviceClass(device->deviceClassId());
            if (!deviceClass.hasActionType(ruleAction.actionTypeId())) {
                qCWarning(dcRuleEngine) << "Cannot create rule. Device " + device->name() + " has no action type:" << ruleAction.actionTypeId();
                return RuleErrorActionTypeNotFound;
            }

            // verify action params
            foreach (const ActionType &actionType, deviceClass.actionTypes()) {
                if (actionType.id() == ruleAction.actionTypeId()) {
                    ParamList finalParams = ruleAction.toAction().params();
                    DeviceManager::DeviceError paramCheck = NymeaCore::instance()->deviceManager()->verifyParams(actionType.paramTypes(), finalParams);
                    if (paramCheck != DeviceManager::DeviceErrorNoError) {
                        qCWarning(dcRuleEngine) << "Cannot create rule. Got an invalid exit actionParam.";
                        return RuleErrorInvalidRuleActionParameter;
                    }
                }
            }

            // Exit action can never be event based.
            if (ruleAction.isEventBased()) {
                qCWarning(dcRuleEngine) << "Cannot create rule. Got exitAction with an actionParam containing an eventTypeId. ";
                return RuleErrorInvalidRuleActionParameter;
            }

            foreach (const RuleActionParam &ruleActionParam, ruleAction.ruleActionParams()) {
                if (!ruleActionParam.isValid()) {
                    qCWarning(dcRuleEngine) << "Cannot create rule. Got an actionParam with \"value\" AND \"eventTypeId\".";
                    return RuleEngine::RuleErrorInvalidRuleActionParameter;
                }
            }

        } else { // Is TypeInterface
            Interface iface = NymeaCore::instance()->deviceManager()->supportedInterfaces().findByName(ruleAction.interface());
            if (!iface.isValid()) {
                qCWarning(dcRuleEngine()) << "Cannot create rule. No such interface:" << ruleAction.interface();
                return RuleError::RuleErrorInterfaceNotFound;
            }
            ActionType ifaceActionType = iface.actionTypes().findByName(ruleAction.interfaceAction());
            if (ifaceActionType.name().isEmpty()) {
                qCWarning(dcRuleEngine()) << "Cannot create rule. Interface" << iface.name() << "does not implement action" << ruleAction.interfaceAction();
                return RuleError::RuleErrorActionTypeNotFound;
            }
            foreach (const ParamType &ifaceActionParamType, ifaceActionType.paramTypes()) {
                if (!ruleAction.ruleActionParams().hasParam(ifaceActionParamType.name())) {
                    qCWarning(dcRuleEngine()) << "Cannot create rule. Interface action" << iface.name() << ":" << ruleAction.interfaceAction() << "requires a" << ifaceActionParamType.name() << "param of type" << QVariant::typeToName(ifaceActionParamType.type());
                    return RuleError::RuleErrorMissingParameter;
                }
                if (!ruleAction.ruleActionParam(ifaceActionParamType.name()).value().canConvert(ifaceActionParamType.type())) {
                    qCWarning(dcRuleEngine()) << "Cannot create rule. Interface action parameter" << iface.name() << ":" << ruleAction.interfaceAction() << ":" << ifaceActionParamType.name() << "has wrong type. Expected" << QVariant::typeToName(ifaceActionParamType.type());
                    return RuleError::RuleErrorInvalidParameter;
                }
            }
            // TODO: Check params
        }
    }

    appendRule(rule);
    saveRule(rule);

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

    NymeaSettings settings(NymeaSettings::SettingsRoleRules);
    settings.beginGroup(ruleId.toString());
    settings.remove("");
    settings.endGroup();

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

    NymeaCore::instance()->logEngine()->logRuleEnabledChanged(rule, true);
    qCDebug(dcRuleEngine()) << "Rule" << rule.name() << rule.id() << "enabled.";

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

    NymeaCore::instance()->logEngine()->logRuleEnabledChanged(rule, false);
    qCDebug(dcRuleEngine()) << "Rule" << rule.name() << rule.id() << "disabled.";
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
    NymeaCore::instance()->logEngine()->logRuleActionsExecuted(rule);
    NymeaCore::instance()->executeRuleActions(rule.actions());
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
    NymeaCore::instance()->logEngine()->logRuleExitActionsExecuted(rule);
    NymeaCore::instance()->executeRuleActions(rule.exitActions());
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
QList<RuleId> RuleEngine::findRules(const DeviceId &deviceId) const
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

/*! Returns all devices that are somehow contained in a rule */
QList<DeviceId> RuleEngine::devicesInRules() const
{
    QList<DeviceId> tmp;
    foreach (const Rule &rule, m_rules) {
        foreach (const EventDescriptor &descriptor, rule.eventDescriptors()) {
            if (!tmp.contains(descriptor.deviceId()) && !descriptor.deviceId().isNull()) {
                tmp.append(descriptor.deviceId());
            }
        }
        foreach (const DeviceId &deviceId, rule.stateEvaluator().containedDevices()) {
            if (!tmp.contains(deviceId) && !deviceId.isNull()) {
                tmp.append(deviceId);
            }
        }
        foreach (const RuleAction &action, rule.actions()) {
            if (!tmp.contains(action.deviceId()) && !action.deviceId().isNull()) {
                tmp.append(action.deviceId());
            }
        }
        foreach (const RuleAction &exitAction, rule.exitActions()) {
            if (!tmp.contains(exitAction.deviceId()) && !exitAction.deviceId().isNull()) {
                tmp.append(exitAction.deviceId());
            }
        }
    }
    return tmp;
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
    NymeaSettings settings(NymeaSettings::SettingsRoleRules);
    settings.beginGroup(id.toString());
    settings.remove("");
    settings.endGroup();

    Rule newRule;
    newRule.setId(id);
    newRule.setName(rule.name());
    newRule.setEventDescriptors(eventDescriptors);
    newRule.setStateEvaluator(stateEvalatuator);
    newRule.setActions(actions);
    newRule.setExitActions(exitActions);
    m_rules[id] = newRule;

    // save it
    saveRule(newRule);
    emit ruleConfigurationChanged(newRule);
}

bool RuleEngine::containsEvent(const Rule &rule, const Event &event, const DeviceClassId &deviceClassId)
{
    foreach (const EventDescriptor &eventDescriptor, rule.eventDescriptors()) {
        // If this is a device based rule, eventTypeId and deviceId must match
        if (eventDescriptor.type() == EventDescriptor::TypeDevice) {
            if (eventDescriptor.eventTypeId() != event.eventTypeId() ||  eventDescriptor.deviceId() != event.deviceId()) {
                continue;
            }
        }

        // If this is a interface based rule, the device must implement the interface
        if (eventDescriptor.type() == EventDescriptor::TypeInterface) {
            DeviceClass dc = NymeaCore::instance()->deviceManager()->findDeviceClass(deviceClassId);
            if (!dc.interfaces().contains(eventDescriptor.interface())) {
                // DeviceClass for this event doesn't implement the interface for this eventDescriptor
                continue;
            }

            EventType et = dc.eventTypes().findById(event.eventTypeId());
            if (et.name() != eventDescriptor.interfaceEvent()) {
                // The fired event name does not match with the eventDescriptor's interfaceEvent
                continue;
            }
        }

        // Ok, either device/eventTypeId or interface/interfaceEvent are matching. Compare the paramdescriptor
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
                DeviceClass dc = NymeaCore::instance()->deviceManager()->findDeviceClass(deviceClassId);
                EventType et = dc.eventTypes().findById(event.eventTypeId());
                ParamType pt = et.paramTypes().findByName(paramDescriptor.paramName());
                paramValue = event.param(pt.id()).value();
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
        if (stateEvaluator.stateDescriptor().type() == StateDescriptor::TypeDevice) {
            if (stateEvaluator.stateDescriptor().stateTypeId().toString() == stateChangeEvent.eventTypeId().toString()) {
                return true;
            }
        } else {
            Device *device = NymeaCore::instance()->deviceManager()->findConfiguredDevice(stateChangeEvent.deviceId());
            DeviceClass deviceClass = NymeaCore::instance()->deviceManager()->findDeviceClass(device->deviceClassId());
            if (deviceClass.interfaces().contains(stateEvaluator.stateDescriptor().interface())) {
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

bool RuleEngine::checkEventDescriptors(const QList<EventDescriptor> eventDescriptors, const EventTypeId &eventTypeId)
{
    foreach (const EventDescriptor eventDescriptor, eventDescriptors) {
        if (eventDescriptor.eventTypeId() == eventTypeId) {
            return true;
        }
    }

    return false;
}

QVariant::Type RuleEngine::getActionParamType(const ActionTypeId &actionTypeId, const ParamTypeId &paramTypeId)
{
    foreach (const DeviceClass &deviceClass, NymeaCore::instance()->deviceManager()->supportedDevices()) {
        foreach (const ActionType &actionType, deviceClass.actionTypes()) {
            if (actionType.id() == actionTypeId) {
                foreach (const ParamType &paramType, actionType.paramTypes()) {
                    if (paramType.id() == paramTypeId) {
                        return paramType.type();
                    }
                }
            }
        }
    }

    return QVariant::Invalid;
}

QVariant::Type RuleEngine::getEventParamType(const EventTypeId &eventTypeId, const ParamTypeId &paramTypeId)
{
    foreach (const DeviceClass &deviceClass, NymeaCore::instance()->deviceManager()->supportedDevices()) {
        foreach (const EventType &eventType, deviceClass.eventTypes()) {
            if (eventType.id() == eventTypeId) {
                foreach (const ParamType &paramType, eventType.paramTypes()) {
                    if (paramType.id() == paramTypeId) {
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
    Rule newRule = rule;
    newRule.setStatesActive(newRule.stateEvaluator().evaluate());
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

            const CalendarItem &calendarItem = rule.timeDescriptor().calendarItems().at(i);
            if (calendarItem.dateTime().isValid())
                settings.setValue("dateTime", calendarItem.dateTime().toTime_t());

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
            const TimeEventItem &timeEventItem = rule.timeDescriptor().timeEventItems().at(i);

            if (timeEventItem.dateTime().isValid())
                settings.setValue("dateTime", timeEventItem.dateTime().toTime_t());

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
        const EventDescriptor &eventDescriptor = rule.eventDescriptors().at(i);
        settings.beginGroup("EventDescriptor-" + QString::number(i));
        settings.setValue("deviceId", eventDescriptor.deviceId().toString());
        settings.setValue("eventTypeId", eventDescriptor.eventTypeId().toString());
        settings.setValue("interface", eventDescriptor.interface());
        settings.setValue("interfaceEvent", eventDescriptor.interfaceEvent());

        foreach (const ParamDescriptor &paramDescriptor, eventDescriptor.paramDescriptors()) {
            if (!paramDescriptor.paramTypeId().isNull()) {
                settings.beginGroup("ParamDescriptor-" + paramDescriptor.paramTypeId().toString());
            } else {
                settings.beginGroup("ParamDescriptor-" + paramDescriptor.paramName());
            }
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
        if (!action.deviceId().isNull() && !action.actionTypeId().isNull()) {
            settings.setValue("deviceId", action.deviceId().toString());
            settings.setValue("actionTypeId", action.actionTypeId().toString());
        } else {
            settings.setValue("interface", action.interface());
            settings.setValue("interfaceAction", action.interfaceAction());
        }
        foreach (const RuleActionParam &param, action.ruleActionParams()) {
            if (!param.paramTypeId().isNull()) {
                settings.beginGroup("RuleActionParam-" + param.paramTypeId().toString());
            } else {
                settings.beginGroup("RuleActionParam-" + param.paramName());
            }
            settings.setValue("value", param.value());
            if (param.eventTypeId() != EventTypeId()) {
                settings.setValue("eventTypeId", param.eventTypeId().toString());
                settings.setValue("eventParamTypeId", param.eventParamTypeId());
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
        if (!action.deviceId().isNull() && !action.actionTypeId().isNull()) {
            settings.setValue("deviceId", action.deviceId().toString());
            settings.setValue("actionTypeId", action.actionTypeId().toString());
        } else {
            settings.setValue("interface", action.interface());
            settings.setValue("interfaceAction", action.interfaceAction());
        }
        foreach (const RuleActionParam &param, action.ruleActionParams()) {
            if (!param.paramTypeId().isNull()) {
                settings.beginGroup("RuleActionParam-" + param.paramTypeId().toString());
            } else {
                settings.beginGroup("RuleActionParam-" + param.paramName());
            }
            settings.setValue("value", param.value());
            settings.endGroup();
        }
        i++;
        settings.endGroup();
    }
    settings.endGroup();
    qCDebug(dcRuleEngineDebug()) << "Saved rule to config:" << rule;
}

}
