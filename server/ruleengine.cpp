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
    qDebug() << "loading rules from" << rulesFileName;
    foreach (const QString &idString, settings.childGroups()) {
        qDebug() << "found rule" << idString;

        settings.beginGroup(idString);

        settings.beginGroup("trigger");
        Trigger trigger(settings.value("triggerTypeId").toUuid(), settings.value("deviceId").toUuid(), settings.value("params").toMap());
        settings.endGroup();

        settings.beginGroup("action");
        Action action = Action(settings.value("deviceId").toUuid(), settings.value("id").toUuid());
        action.setName(settings.value("name").toString());
        action.setParams(settings.value("params").toList());
        settings.endGroup();
        settings.endGroup();

        Rule rule = Rule(QUuid(idString), trigger, action);
        m_rules.append(rule);
    }

}

QList<Action> RuleEngine::evaluateTrigger(const Trigger &trigger)
{
    QList<Action> actions;
    for (int i = 0; i < m_rules.count(); ++i) {
        if (m_rules.at(i).trigger() == trigger) {
            actions << m_rules.at(i).action();
        }
    }
    return actions;
}

RuleEngine::RuleError RuleEngine::addRule(const Trigger &trigger, const Action &action)
{

    Device *device = HiveCore::instance()->deviceManager()->findConfiguredDevice(trigger.deviceId());
    if (!device) {
        qWarning() << "Cannot create rule. No configured device for triggerTypeId" << trigger.triggerTypeId();
        return RuleErrorDeviceNotFound;
    }
    DeviceClass deviceClass = HiveCore::instance()->deviceManager()->findDeviceClass(device->deviceClassId());

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

    Rule rule = Rule(QUuid::createUuid(), trigger, action);
    m_rules.append(rule);

    QSettings settings(rulesFileName);
    settings.beginGroup(rule.id().toString());
    settings.beginGroup("trigger");
    settings.setValue("triggerTypeId", trigger.triggerTypeId());
    settings.setValue("deviceId", trigger.deviceId());
    settings.setValue("params", trigger.params());
    settings.endGroup();
    settings.beginGroup("action");
    settings.setValue("id", rule.action().id());
    settings.setValue("deviceId", rule.action().deviceId());
    settings.setValue("name", rule.action().name());
    settings.setValue("params", rule.action().params());
    settings.endGroup();

    return RuleErrorNoError;
}

QList<Rule> RuleEngine::rules() const
{
    return m_rules;
}
