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

QList<QUuid> RuleEngine::evaluateTrigger(const QUuid &triggerId)
{
    QList<QUuid> actions;
    for (int i = 0; i < m_rules.count(); ++i) {
        if (m_rules.at(i).triggerId() == triggerId) {
            actions << m_rules.at(i).actionId();
        }
    }
    return actions;
}

RuleEngine::RuleError RuleEngine::addRule(const QUuid &triggerId, const QUuid &actionId)
{
    if (!HiveCore::instance()->deviceManager()->findTrigger(triggerId).isValid()) {
        qWarning() << "Cannot create rule. No such trigger.";
        return RuleErrorNoSuchTrigger;
    }
    if (!HiveCore::instance()->deviceManager()->findAction(actionId).isValid()) {
        qWarning() << "Cannot create rule. No such action.";
        return RuleErrorNoSuchAction;
    }

    Rule rule = Rule(QUuid::createUuid(), triggerId, actionId);
    m_rules.append(rule);

    QSettings settings(rulesFileName);
    settings.beginGroup(rule.id().toString());
    settings.setValue("triggerId", rule.triggerId());
    settings.setValue("actionId", rule.actionId());

    return RuleErrorNoError;
}

QList<Rule> RuleEngine::rules() const
{
    return m_rules;
}
