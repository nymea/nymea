#include "rule.h"

Rule::Rule(const QUuid &id, const QUuid &triggerTypeId, const Action &action):
    m_id(id),
    m_triggerTypeId(triggerTypeId),
    m_action(action)
{
}

QUuid Rule::id() const
{
    return m_id;
}

QUuid Rule::triggerTypeId() const
{
    return m_triggerTypeId;
}

Action Rule::action() const
{
    return m_action;
}
