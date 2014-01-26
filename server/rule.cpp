/*!
    \class Rule
    \brief This class represents a rule.

    \ingroup rules
    \inmodule server

    A \l{Rule} is always triggered by a \l{Trigger}, has \l{State}{States}
    to be compared and \l{Action}{Actions} to be executed. Additionally a
    Rule is either of type \l{Rule::RuleTypeAll} or \l{Rule::RuleTypeAny}
    which determines if all or any of the \l{State}{States} must be matching
    in order for the \l{Action}{Actions} to be executed.

    \sa Trigger, State, Action
*/

#include "rule.h"

#include <QDebug>

Rule::Rule(const QUuid &id, const Trigger &trigger, const QList<State> &states, const QList<Action> &actions):
    m_id(id),
    m_trigger(trigger),
    m_states(states),
    m_actions(actions),
    m_ruleType(RuleTypeAll)
{
}

QUuid Rule::id() const
{
    return m_id;
}

Trigger Rule::trigger() const
{
    return m_trigger;
}

QList<State> Rule::states() const
{
    return m_states;
}

QList<Action> Rule::actions() const
{
    return m_actions;
}

Rule::RuleType Rule::ruleType() const
{
    return m_ruleType;
}

void Rule::setRuleType(Rule::RuleType ruleType)
{
    m_ruleType = ruleType;
}
