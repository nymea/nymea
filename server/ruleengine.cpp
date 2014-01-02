#include "ruleengine.h"

#include "hivecore.h"
#include "devicemanager.h"

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
        settings.beginGroup("action");
        Action action = Action(settings.value("deviceId").toUuid(), settings.value("id").toUuid());
        action.setName(settings.value("name").toString());
        action.setParams(settings.value("params").toList());
        settings.endGroup();
        settings.endGroup();

        Rule rule = Rule(QUuid(idString), settings.value("triggerTypeId").toUuid(), action);
        m_rules.append(rule);
    }

}

QList<Action> RuleEngine::evaluateTrigger(const Trigger &trigger)
{
    QList<Action> actions;
    for (int i = 0; i < m_rules.count(); ++i) {
<<<<<<< HEAD
//        if (m_rules.at(i).triggerTypeId() == trigger.) {
//            actions << m_rules.at(i).action();
//        }
=======
        if (m_rules.at(i).triggerTypeId() == trigger.deviceClassId()) {
            actions << m_rules.at(i).action();
        }
>>>>>>> meister anker plugin added
    }
    return actions;
}

RuleEngine::RuleError RuleEngine::addRule(const QUuid &triggerTypeId, const Action &action)
{

    Rule rule = Rule(QUuid::createUuid(), triggerTypeId, action);
    m_rules.append(rule);

    QSettings settings(rulesFileName);
    settings.beginGroup(rule.id().toString());
<<<<<<< HEAD
    settings.setValue("triggerTypeId", rule.triggerTypeId());
=======
    settings.setValue("triggerId", rule.triggerTypeId());
>>>>>>> meister anker plugin added

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
