#ifndef RULEENGINE_H
#define RULEENGINE_H

#include "rule.h"
#include "trigger.h"

#include <QObject>
#include <QList>
#include <QUuid>

class RuleEngine : public QObject
{
    Q_OBJECT
public:
    enum RuleError {
        RuleErrorNoError,
        RuleErrorDeviceNotFound,
        RuleErrorTriggerTypeNotFound
    };

    explicit RuleEngine(QObject *parent = 0);

    QList<Action> evaluateTrigger(const Trigger &trigger);

    RuleError addRule(const Trigger &trigger, const QList<Action> &actions);
    RuleError addRule(const Trigger &trigger, const QList<State> states, const QList<Action> &actions);
    QList<Rule> rules() const;

private:
    QList<Rule> m_rules;

};

#endif // RULEENGINE_H
