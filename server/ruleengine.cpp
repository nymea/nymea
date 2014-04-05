/****************************************************************************
 *                                                                          *
 *  This file is part of guh.                                               *
 *                                                                          *
 *  Guh is free software: you can redistribute it and/or modify             *
 *  it under the terms of the GNU General Public License as published by    *
 *  the Free Software Foundation, version 2 of the License.                 *
 *                                                                          *
 *  Guh is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *  GNU General Public License for more details.                            *
 *                                                                          *
 *  You should have received a copy of the GNU General Public License       *
 *  along with guh.  If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                          *
 ***************************************************************************/

/*!
    \class RuleEngine
    \brief The Engine that evaluates \l{Rule}{Rules} and finds
    \l{Action}{Actions} to be executed.

    \ingroup rules
    \inmodule server

    You can add, remove and update rules and query the engine for actions to be executed
    for a given \l{Event}.

    \sa Event, Rule, Action
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

#include "guhcore.h"

#include "plugin/devicemanager.h"
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
        Event event(settings.value("eventTypeId").toUuid(), settings.value("deviceId").toUuid(), settings.value("params").toMap());
        settings.endGroup();

        settings.beginGroup("states");
        QList<State> states;
        foreach (const QString &stateTypeIdString, settings.childGroups()) {
            settings.beginGroup(stateTypeIdString);
            State state(stateTypeIdString, settings.value("deviceId").toUuid());
            state.setValue(settings.value("value"));
            settings.endGroup();
            states.append(state);
        }
        settings.endGroup();

        settings.beginGroup("actions");
        QList<Action> actions;
        foreach (const QString &actionIdString, settings.childGroups()) {
            settings.beginGroup(actionIdString);
            Action action = Action(settings.value("deviceId").toUuid(), settings.value("actionTypeId").toUuid());
            action.setParams(settings.value("params").toMap());
            settings.endGroup();
            actions.append(action);
        }
        settings.endGroup();

        settings.endGroup();

        Rule rule = Rule(QUuid(idString), event, states, actions);
        m_rules.append(rule);
    }

}

/*! Ask the Engine to evaluate all the rules for the given \a event.
    This will search all the \l{Rule}{Rules} evented by this \l{Event}
    and evaluate it's states according to its type. It will return a
    list of all \l{Action}{Actions} that should be executed. */
QList<Action> RuleEngine::evaluateEvent(const Event &event)
{
    QList<Action> actions;
//    for (int i = 0; i < m_rules.count(); ++i) {
//        if (m_rules.at(i).events().contains(event)) {
//            bool statesMatching = true;
//            qDebug() << "checking states";
//            foreach (const State &state, m_rules.at(i).stateChanges()) {
//                Device *device = GuhCore::instance()->deviceManager()->findConfiguredDevice(state.deviceId());
//                if (!device) {
//                    qWarning() << "Device referenced in rule cannot be found";
//                    break;
//                }
//                if (state.value() != device->stateValue(state.stateTypeId())) {
//                    statesMatching = false;
//                    break;
//                }
//            }

//            qDebug() << "states matching" << statesMatching;
//            if (statesMatching) {
//                actions.append(m_rules.at(i).actions());
//            }
//        }
//    }
    qDebug() << "found" << actions.count() << "actions";
    return actions;
}

/*! Add a new \l{Rule} with the given \a event and \a actions to the engine.
    For convenience, this creates a Rule without any \l{State} comparison. */
RuleEngine::RuleError RuleEngine::addRule(const Event &event, const QList<Action> &actions)
{
    return addRule(event, QList<State>(), actions);
}

/*! Add a new \l{Rule} with the given \a event, \a states and \a actions to the engine. */
RuleEngine::RuleError RuleEngine::addRule(const Event &event, const QList<State> &states, const QList<Action> &actions)
{
    qDebug() << "adding rule: Event:" << event.eventTypeId() << "with" << actions.count() << "actions";
    DeviceClass eventDeviceClass = GuhCore::instance()->deviceManager()->findDeviceClassforEvent(event.eventTypeId());

    Device *device = GuhCore::instance()->deviceManager()->findConfiguredDevice(event.deviceId());
    if (!device) {
        qWarning() << "Cannot create rule. No configured device for eventTypeId" << event.eventTypeId();
        return RuleErrorDeviceNotFound;
    }
    DeviceClass deviceClass = GuhCore::instance()->deviceManager()->findDeviceClass(device->deviceClassId());
    qDebug() << "found deviceClass" << deviceClass.name();

    bool eventTypeFound = false;
    foreach (const EventType &eventType, deviceClass.events()) {
        if (eventType.id() == event.eventTypeId()) {
            eventTypeFound = true;
        }
    }
    if (!eventTypeFound) {
        qWarning() << "Cannot create rule. Device " + device->name() + " has no event type:" << event.eventTypeId();
        return RuleErrorEventTypeNotFound;
    }

    Rule rule = Rule(QUuid::createUuid(), event, states, actions);
    m_rules.append(rule);
    emit ruleAdded(rule.id());

    QSettings settings(m_settingsFile);
    settings.beginGroup(rule.id().toString());
    settings.beginGroup("event");
    settings.setValue("eventTypeId", event.eventTypeId());
    settings.setValue("deviceId", event.deviceId());
    settings.setValue("params", event.params());
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
        settings.setValue("params", action.params());
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
