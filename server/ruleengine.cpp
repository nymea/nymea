#include "ruleengine.h"

#include "hivecore.h"
#include "devicemanager.h"
#include "device.h"

#include <QSettings>
#include <QDebug>
#include <QStringList>
#include <QStandardPaths>

QString rulesFileName = "hiveyourhome/rules";

RuleEngine::RuleEngine(QObject *parent) :
    QObject(parent)
{
    QSettings settings(rulesFileName);
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
            Action action = Action(settings.value("deviceId").toUuid(), settings.value("id").toUuid());
            action.setName(settings.value("name").toString());
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

RuleEngine::RuleError RuleEngine::addRule(const Trigger &trigger, const QList<Action> &actions)
{
    return addRule(trigger, QList<State>(), actions);
}

RuleEngine::RuleError RuleEngine::addRule(const Trigger &trigger, const QList<State> states, const QList<Action> &actions)
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

    QSettings settings(rulesFileName);
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
        settings.beginGroup(action.id().toString());
        settings.setValue("deviceId", action.deviceId());
        settings.setValue("name", action.name());
        settings.setValue("params", action.params());
        settings.endGroup();
    }

    settings.endGroup();

    return RuleErrorNoError;
}

QList<Rule> RuleEngine::rules() const
{
    return m_rules;
}
