#include "rule.h"

Rule::Rule(const QUuid &id, const Trigger &trigger, const Action &action):
    m_id(id),
    m_trigger(trigger),
    m_action(action)
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

Action Rule::action() const
{
    return m_action;
}
