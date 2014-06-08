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
    qDebug() << "laoding rules from" << m_settingsFile;
    QSettings settings(m_settingsFile);
    foreach (const QString &idString, settings.childGroups()) {
        qDebug() << "found rule" << idString;

        settings.beginGroup(idString);

        settings.beginGroup("event");
        EventTypeId eventTypeId(settings.value("eventTypeId").toString());
        DeviceId deviceId(settings.value("deviceId").toString());
        QList<ParamDescriptor> params;
        foreach (QString groupName, settings.childGroups()) {
            if (groupName.startsWith("ParamDescriptor-")) {
                settings.beginGroup(groupName);
                ParamDescriptor paramDescriptor(groupName.remove(QRegExp("^ParamDescriptor-")), settings.value("value"));
                paramDescriptor.setOperand((ParamDescriptor::OperandType)settings.value("operand").toInt());
                params.append(paramDescriptor);
                settings.endGroup();
            }
        }
        EventDescriptor eventDescriptor(eventTypeId, deviceId, params);
        settings.endGroup();

        settings.beginGroup("states");
        QList<State> states;
        foreach (const QString &stateTypeIdString, settings.childGroups()) {
            settings.beginGroup(stateTypeIdString);
            State state(StateTypeId(stateTypeIdString), DeviceId(settings.value("deviceId").toString()));
            state.setValue(settings.value("value"));
            settings.endGroup();
            states.append(state);
        }
        settings.endGroup();

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

        Rule rule = Rule(QUuid(idString), eventDescriptor, states, actions);
        m_rules.append(rule);
    }

}

/*! Ask the Engine to evaluate all the rules for the given \a event.
    This will search all the \l{Rule}{Rules} evented by this \l{Event}
    and evaluate it's states according to its type. It will return a
    list of all \l{Action}{Actions} that should be executed. */
QList<Action> RuleEngine::evaluateEvent(const Event &event)
{
    qDebug() << "got event:" << event;
    QList<Action> actions;
    for (int i = 0; i < m_rules.count(); ++i) {
        qDebug() << "evaluating rule" << i << m_rules.at(i).eventDescriptors();
        if (containsEvent(m_rules.at(i), event)) {
            bool statesMatching = true;
            qDebug() << "checking states:" << m_rules.at(i).states();
            foreach (const State &state, m_rules.at(i).states()) {
                Device *device = GuhCore::instance()->deviceManager()->findConfiguredDevice(state.deviceId());
                if (!device) {
                    qWarning() << "Device referenced in rule cannot be found";
                    break;
                }
                if (state.value() != device->stateValue(state.stateTypeId())) {
                    qDebug() << "State value not matching:" << state.value() << device->stateValue(state.stateTypeId());
                    statesMatching = false;
                    break;
                }
            }

            qDebug() << "states matching" << statesMatching;
            if (statesMatching) {
                actions.append(m_rules.at(i).actions());
            }
        }
    }
    qDebug() << "found" << actions.count() << "actions";
    return actions;
}

/*! Add a new \l{Rule} with the given \a event and \a actions to the engine.
    For convenience, this creates a Rule without any \l{State} comparison. */
RuleEngine::RuleError RuleEngine::addRule(const EventDescriptor &eventDescriptor, const QList<Action> &actions)
{
    return addRule(eventDescriptor, QList<State>(), actions);
}

/*! Add a new \l{Rule} with the given \a event, \a states and \a actions to the engine. */
RuleEngine::RuleError RuleEngine::addRule(const EventDescriptor &eventDescriptor, const QList<State> &states, const QList<Action> &actions)
{
    Device *device = GuhCore::instance()->deviceManager()->findConfiguredDevice(eventDescriptor.deviceId());
    if (!device) {
        qWarning() << "Cannot create rule. No configured device for eventTypeId" << eventDescriptor.eventTypeId();
        return RuleErrorDeviceNotFound;
    }
    DeviceClass deviceClass = GuhCore::instance()->deviceManager()->findDeviceClass(device->deviceClassId());
    qDebug() << "found deviceClass" << deviceClass.name();

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

    Rule rule = Rule(QUuid::createUuid(), eventDescriptor, states, actions);
    m_rules.append(rule);
    emit ruleAdded(rule.id());

    QSettings settings(m_settingsFile);
    settings.beginGroup(rule.id().toString());
    settings.beginGroup("event");
    settings.setValue("eventTypeId", eventDescriptor.eventTypeId());
    settings.setValue("deviceId", eventDescriptor.deviceId());
    foreach (const ParamDescriptor &paramDescriptor, eventDescriptor.paramDescriptors()) {
        settings.beginGroup("ParamDescriptor-" + paramDescriptor.name());
        settings.setValue("value", paramDescriptor.value());
        settings.setValue("operand", paramDescriptor.operand());
        settings.endGroup();
    }

    settings.endGroup();

    settings.beginGroup("states");
    foreach (const State &state, states) {
        settings.beginGroup(state.stateTypeId().toString());
        settings.setValue("deviceId", state.deviceId());
        settings.setValue("value", state.value());
        settings.endGroup();
    }

    settings.endGroup();

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
RuleEngine::RuleError RuleEngine::removeRule(const QUuid &ruleId)
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

bool RuleEngine::containsEvent(const Rule &rule, const Event &event)
{
    foreach (const EventDescriptor &eventDescriptor, rule.eventDescriptors()) {
        if (eventDescriptor == event) {
            return true;
        }
    }
    return false;
}
