#include "rule.h"

Rule::Rule(const QUuid &id, const QUuid &triggerId, const Action &action):
    m_id(id),
    m_triggerId(triggerId),
    m_action(action)
{
}

QUuid Rule::id() const
{
    return m_id;
}

QUuid Rule::triggerId() const
{
    return m_triggerId;
}

Action Rule::action() const
{
    return m_action;
}
