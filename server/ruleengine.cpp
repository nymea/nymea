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
        Rule rule = Rule(QUuid(idString), settings.value("triggerId").toUuid(), settings.value("actionId").toUuid());
        settings.endGroup();
        m_rules.append(rule);
    }

}

QList<Action> RuleEngine::evaluateTrigger(const QUuid &triggerId)
{
    QList<Action> actions;
    for (int i = 0; i < m_rules.count(); ++i) {
        if (m_rules.at(i).triggerId() == triggerId) {
            actions << m_rules.at(i).action();
        }
    }
    return actions;
}

RuleEngine::RuleError RuleEngine::addRule(const QUuid &triggerTypeId, const Action &action)
{

    Rule rule = Rule(QUuid::createUuid(), triggerTypeId, action);
    m_rules.append(rule);

    QSettings settings(rulesFileName);
    settings.beginGroup(rule.id().toString());
    settings.setValue("triggerId", rule.triggerId());

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
