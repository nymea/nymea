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

/*!
    \class RuleEngine
    \brief The Engine that evaluates \l{Rule}{Rules} and finds
    \l{Action}{Actions} to be executed.

    \ingroup rules
    \inmodule server

    You can add, remove and update rules and query the engine for actions to be executed
    for a given \l{Event} described by an \l{EventDescriptor}.

    \sa Event, EventDescriptor, Rule, Action
*/

/*! \fn void RuleEngine::ruleAdded(const QUuid &ruleId)
    Will be emitted whenever a new \l{Rule} is added to this Engine.
    \a ruleId holds the id of the new rule.*/

/*! \fn void RuleEngine::ruleRemoved(const QUuid &ruleId)
    Will be emitted whenever a \l{Rule} is removed from this Engine.
    \a ruleId holds the id of the removed rule. You should remove any references
    or copies you hold for this rule.*/

/*! \enum RuleEngine::RuleError
    \value RuleErrorNoError
        No error happened. Everything is fine.
    \value RuleErrorRuleNotFound
        Couldn't find a \l{Rule} with the given id.
    \value RuleErrorDeviceNotFound
        Couldn't find a \l{Device} with the given id.
    \value RuleErrorEventTypeNotFound
        Couldn't find a \l{EventType} with the given id.
    */

#include "ruleengine.h"
#include "types/paramdescriptor.h"
#include "types/eventdescriptor.h"

#include "guhcore.h"

#include "devicemanager.h"
#include "plugin/device.h"

#include <QSettings>
#include <QDebug>
#include <QStringList>
#include <QStandardPaths>
#include <QCoreApplication>

/*! Constructs the RuleEngine with the given \a parent. Although it wouldn't harm to have multiple RuleEngines, there is one
    instance available from \l{GuhCore}. This one should be used instead of creating multiple ones.
    */
RuleEngine::RuleEngine(QObject *parent) :
    QObject(parent)
{
    m_settingsFile = QCoreApplication::instance()->organizationName() + "/rules";
    QSettings settings(m_settingsFile);
    qDebug() << "laoding rules from" << settings.fileName();
    foreach (const QString &idString, settings.childGroups()) {
        qDebug() << "found rule" << idString;

        settings.beginGroup(idString);

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
                        paramDescriptor.setOperatorType((ValueOperator)settings.value("operator").toInt());
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

        settings.beginGroup("actions");
        QList<Action> actions;
        foreach (const QString &actionIdString, settings.childGroups()) {
            settings.beginGroup(actionIdString);
            Action action = Action(ActionTypeId(settings.value("actionTypeId").toString()), DeviceId(settings.value("deviceId").toString()));
            QList<Param> params;
            foreach (QString paramNameString, settings.childGroups()) {
                if (paramNameString.startsWith("Param-")) {
                    settings.beginGroup(paramNameString);
                    Param param(paramNameString.remove(QRegExp("^Param-")), settings.value("value"));
                    params.append(param);
                    settings.endGroup();
                }
            }
            action.setParams(params);

            settings.endGroup();
            actions.append(action);
        }
        settings.endGroup();

        settings.endGroup();

        Rule rule = Rule(RuleId(idString), eventDescriptorList, stateEvaluator, actions);
        m_rules.append(rule);
    }

}

/*! Ask the Engine to evaluate all the rules for the given \a event.
    This will search all the \l{Rule}{Rules} evented by this \l{Event}
    and evaluate it's states according to its type. It will return a
    list of all \l{Action}{Actions} that should be executed. */
QList<Action> RuleEngine::evaluateEvent(const Event &event)
{
    Device *device = GuhCore::instance()->findConfiguredDevice(event.deviceId());

    qDebug() << "got event:" << event << device->name();

    QList<Action> actions;
    for (int i = 0; i < m_rules.count(); ++i) {
        qDebug() << "evaluating rule" << i << m_rules.at(i).eventDescriptors();
        if (containsEvent(m_rules.at(i), event)) {
            if (m_rules.at(i).stateEvaluator().evaluate()) {
                qDebug() << "states matching!";
                actions.append(m_rules.at(i).actions());
            }
        }
    }
    qDebug() << "found" << actions.count() << "actions";
    return actions;
}

/*! Add a new \l{Rule} with the given \a EventDescriptorList and \a actions to the engine.
    For convenience, this creates a Rule without any \l{State} comparison. */
RuleEngine::RuleError RuleEngine::addRule(const RuleId &ruleId, const QList<EventDescriptor> &eventDescriptorList, const QList<Action> &actions)
{
    return addRule(ruleId, eventDescriptorList, StateEvaluator(), actions);
}

/*! Add a new \l{Rule} with the given \a event, \a states and \a actions to the engine. */
RuleEngine::RuleError RuleEngine::addRule(const RuleId &ruleId, const QList<EventDescriptor> &eventDescriptorList, const StateEvaluator &stateEvaluator, const QList<Action> &actions)
{
    if (ruleId.isNull()) {
        return RuleErrorInvalidRuleId;
    }
    if (!findRule(ruleId).id().isNull()) {
        qWarning() << "Already have a rule with this id!";
        return RuleErrorInvalidRuleId;
    }
    foreach (const EventDescriptor &eventDescriptor, eventDescriptorList) {
        Device *device = GuhCore::instance()->findConfiguredDevice(eventDescriptor.deviceId());
        if (!device) {
            qWarning() << "Cannot create rule. No configured device for eventTypeId" << eventDescriptor.eventTypeId();
            return RuleErrorDeviceNotFound;
        }
        DeviceClass deviceClass = GuhCore::instance()->findDeviceClass(device->deviceClassId());

        bool eventTypeFound = false;
        foreach (const EventType &eventType, deviceClass.eventTypes()) {
            if (eventType.id() == eventDescriptor.eventTypeId()) {
                eventTypeFound = true;
            }
        }
        if (!eventTypeFound) {
            qWarning() << "Cannot create rule. Device " + device->name() + " has no event type:" << eventDescriptor.eventTypeId();
            return RuleErrorEventTypeNotFound;
        }
    }

    foreach (const Action &action, actions) {
        Device *device = GuhCore::instance()->findConfiguredDevice(action.deviceId());
        if (!device) {
            qWarning() << "Cannot create rule. No configured device for actionTypeId" << action.actionTypeId();
            return RuleErrorDeviceNotFound;
        }
        DeviceClass deviceClass = GuhCore::instance()->findDeviceClass(device->deviceClassId());

        bool actionTypeFound = false;
        foreach (const ActionType &actionType, deviceClass.actionTypes()) {
            if (actionType.id() == action.actionTypeId()) {
                actionTypeFound = true;
            }
        }
        if (!actionTypeFound) {
            qWarning() << "Cannot create rule. Device " + device->name() + " has no action type:" << action.actionTypeId();
            return RuleErrorActionTypeNotFound;
        }
    }

    Rule rule = Rule(ruleId, eventDescriptorList, stateEvaluator, actions);
    m_rules.append(rule);
    emit ruleAdded(rule.id());

    QSettings settings(m_settingsFile);
    settings.beginGroup(rule.id().toString());
    settings.beginGroup("events");
    for (int i = 0; i < eventDescriptorList.count(); i++) {
        const EventDescriptor &eventDescriptor = eventDescriptorList.at(i);
        settings.beginGroup("EventDescriptor-" + QString::number(i));
        settings.setValue("deviceId", eventDescriptor.deviceId().toString());
        settings.setValue("eventTypeId", eventDescriptor.eventTypeId());

        foreach (const ParamDescriptor &paramDescriptor, eventDescriptor.paramDescriptors()) {
            settings.beginGroup("ParamDescriptor-" + paramDescriptor.name());
            settings.setValue("value", paramDescriptor.value());
            settings.setValue("operator", paramDescriptor.operatorType());
            settings.endGroup();
        }
        settings.endGroup();
    }
    settings.endGroup();

    stateEvaluator.dumpToSettings(settings, "stateEvaluator");

    settings.beginGroup("actions");
    foreach (const Action &action, rule.actions()) {
        settings.beginGroup(action.actionTypeId().toString());
        settings.setValue("deviceId", action.deviceId());
        settings.setValue("actionTypeId", action.actionTypeId());
        foreach (const Param &param, action.params()) {
            settings.beginGroup("Param-" + param.name());
            settings.setValue("value", param.value());
            settings.endGroup();
        }

        settings.endGroup();
    }

    settings.endGroup();

    return RuleErrorNoError;
}

/*! Returns a list of all \l{Rule}{Rules} loaded in this Engine.*/
QList<Rule> RuleEngine::rules() const
{
    return m_rules;
}

/*! Removes the \l{Rule} with the given \a ruleId from the Engine.
    Returns \l{RuleEngine::RuleError} which describes whether the operation
    was successful or not. */
RuleEngine::RuleError RuleEngine::removeRule(const RuleId &ruleId)
{
    for (int i = 0; i < m_rules.count(); ++i) {
        Rule rule = m_rules.at(i);
        if (rule.id() == ruleId) {

            m_rules.takeAt(i);

            QSettings settings(m_settingsFile);
            settings.beginGroup(rule.id().toString());
            settings.remove("");
            settings.endGroup();

            emit ruleRemoved(rule.id());
            return RuleErrorNoError;
        }
    }
    return RuleErrorRuleNotFound;
}

Rule RuleEngine::findRule(const RuleId &ruleId)
{
    foreach (const Rule &rule, m_rules) {
        if (rule.id() == ruleId) {
            return rule;
        }
    }
    return Rule();
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
