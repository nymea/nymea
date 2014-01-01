#include "rule.h"

Rule::Rule(const QUuid &id, const QUuid &triggerId, const QUuid &actionId):
    m_id(id),
    m_triggerId(triggerId),
    m_actionId(actionId)
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

QUuid Rule::actionId() const
{
    return m_actionId;
}
