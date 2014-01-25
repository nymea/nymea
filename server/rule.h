#ifndef RULE_H
#define RULE_H

#include "state.h"
#include "action.h"
#include "trigger.h"

#include <QUuid>

class Rule
{
public:
    enum RuleType {
        RuleTypeAll,
        RuleTypeAny
    };

    Rule(const QUuid &id, const Trigger &trigger, const QList<State> states, const QList<Action> &actions);

    QUuid id() const;
    Trigger trigger() const;
    QList<State> states() const;
    QList<Action> actions() const;

    RuleType ruleType() const;
    void setRuleType(RuleType ruleType);

private:
    QUuid m_id;
    Trigger m_trigger;
    QList<State> m_states;
    QList<Action> m_actions;
    RuleType m_ruleType;
};

#endif // RULE_H
