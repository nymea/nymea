#include "rule.h"

#include <QDebug>

Rule::Rule(const QUuid &id, const Trigger &trigger, const QList<State> states, const QList<Action> &actions):
    m_id(id),
    m_trigger(trigger),
    m_states(states),
    m_actions(actions)
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
