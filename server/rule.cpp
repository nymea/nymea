/*!
    \class Rule
    \brief This class represents a rule.

    \ingroup rules
    \inmodule server

    A Rule is always triggered by a \l{Trigger}, has \l{State}{States}
    to be compared and \l{Action}{Actions} to be executed. Additionally a
    Rule is either of type \l{Rule::RuleTypeAll} or \l{Rule::RuleTypeAny}
    which determines if all or any of the \l{State}{States} must be matching
    in order for the \l{Action}{Actions} to be executed.

    \sa Trigger, State, Action
*/

/*! \enum Rule::RuleType

    Note: There is no RuleTypeNone. If you don't want to compare any
    states, construct a rule without states in which case it doesn't
    matter what the Rule's type is.

    \value RuleTypeAll
    All States must match in order for the Rule to apply.
    \value RuleTypeAny
    Any State must match in order for the Rule to apply.
*/

#include "rule.h"

#include <QDebug>

/*! Constructs a Rule with the given \a id, \a trigger, \a states and \a actions. The ruleType will default to
 \l{Rule::RuleTypeAll}.*/
Rule::Rule(const QUuid &id, const Trigger &trigger, const QList<State> &states, const QList<Action> &actions):
    m_id(id),
    m_trigger(trigger),
    m_states(states),
    m_actions(actions),
    m_ruleType(RuleTypeAll)
{
}

/*! Returns the id or the Rule. */
QUuid Rule::id() const
{
    return m_id;
}

/*! Returns the \l{Trigger} that triggers this Rule.*/
Trigger Rule::trigger() const
{
    return m_trigger;
}

/*! Returns the \l{State}{States} that need to be matching in order for this to Rule apply. */
QList<State> Rule::states() const
{
    return m_states;
}

/*! Returns the \l{Action}{Actions} to be executed when this Rule is triggerd and states match. */
QList<Action> Rule::actions() const
{
    return m_actions;
}

/*! Returns the type of the rule. This defines how states are compared. The default is \l{Rule::RuleTypeAll}.*/
Rule::RuleType Rule::ruleType() const
{
    return m_ruleType;
}

/*! Set the type of the rule to \a ruleType. This defines how states are compared. The default is \l{Rule::RuleTypeAll}.*/
void Rule::setRuleType(Rule::RuleType ruleType)
{
    m_ruleType = ruleType;
}
