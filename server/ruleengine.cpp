/*!
    \class RuleEngine
    \brief The Engine that evaluates \l{Rule}{Rules} and finds
    \l{Action}{Actions} to be executed.

    \ingroup rules
    \inmodule server

    You can add, remove and update rules and query the engine for actions to be executed
    for a given \l{Trigger}.

    \sa Trigger, Rule, Action
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
    \value RuleErrorTriggerTypeNotFound
        Couldn't find a \l{TriggerType} with the given id.
    */

#include "ruleengine.h"

#include "hivecore.h"
#include "devicemanager.h"
#include "device.h"

#include <QSettings>
#include <QDebug>
#include <QStringList>
#include <QStandardPaths>
#include <QCoreApplication>

/*! Constructs the RuleEngine with the given \a parent. Although it wouldn't harm to have multiple RuleEngines, there is one
    instance available from \l{HiveCore}. This one should be used instead of creating multiple ones.
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

        settings.beginGroup("trigger");
        Trigger trigger(settings.value("triggerTypeId").toUuid(), settings.value("deviceId").toUuid(), settings.value("params").toMap());
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

        Rule rule = Rule(QUuid(idString), trigger, states, actions);
        m_rules.append(rule);
    }

}

/*! Ask the Engine to evaluate all the rules for the given \a trigger.
    This will search all the \l{Rule}{Rules} triggered by this \l{Trigger}
    and evaluate it's states according to its type. It will return a
    list of all \l{Action}{Actions} that should be executed. */
QList<Action> RuleEngine::evaluateTrigger(const Trigger &trigger)
{
    QList<Action> actions;
    for (int i = 0; i < m_rules.count(); ++i) {
        if (m_rules.at(i).trigger() == trigger) {
            bool statesMatching = true;
            qDebug() << "checking states";
            foreach (const State &state, m_rules.at(i).states()) {
                Device *device = HiveCore::instance()->deviceManager()->findConfiguredDevice(state.deviceId());
                if (!device) {
                    qWarning() << "Device referenced in rule cannot be found";
                    break;
                }
                if (state.value() != device->stateValue(state.stateTypeId())) {
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

/*! Add a new \l{Rule} with the given \a trigger and \a actions to the engine.
    For convenience, this creates a Rule without any \l{State} comparison. */
RuleEngine::RuleError RuleEngine::addRule(const Trigger &trigger, const QList<Action> &actions)
{
    return addRule(trigger, QList<State>(), actions);
}

/*! Add a new \l{Rule} with the given \a trigger, \a states and \a actions to the engine. */
RuleEngine::RuleError RuleEngine::addRule(const Trigger &trigger, const QList<State> &states, const QList<Action> &actions)
{
    qDebug() << "adding rule: Trigger:" << trigger.triggerTypeId() << "with" << actions.count() << "actions";
    DeviceClass triggerDeviceClass = HiveCore::instance()->deviceManager()->findDeviceClassforTrigger(trigger.triggerTypeId());

    Device *device = HiveCore::instance()->deviceManager()->findConfiguredDevice(trigger.deviceId());
    if (!device) {
        qWarning() << "Cannot create rule. No configured device for triggerTypeId" << trigger.triggerTypeId();
        return RuleErrorDeviceNotFound;
    }
    DeviceClass deviceClass = HiveCore::instance()->deviceManager()->findDeviceClass(device->deviceClassId());
    qDebug() << "found deviceClass" << deviceClass.name();

    bool triggerTypeFound = false;
    foreach (const TriggerType &triggerType, deviceClass.triggers()) {
        if (triggerType.id() == trigger.triggerTypeId()) {
            triggerTypeFound = true;
        }
    }
    if (!triggerTypeFound) {
        qWarning() << "Cannot create rule. Device " + device->name() + " has no trigger type:" << trigger.triggerTypeId();
        return RuleErrorTriggerTypeNotFound;
    }

    Rule rule = Rule(QUuid::createUuid(), trigger, states, actions);
    m_rules.append(rule);
    emit ruleAdded(rule.id());

    QSettings settings(m_settingsFile);
    settings.beginGroup(rule.id().toString());
    settings.beginGroup("trigger");
    settings.setValue("triggerTypeId", trigger.triggerTypeId());
    settings.setValue("deviceId", trigger.deviceId());
    settings.setValue("params", trigger.params());
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
