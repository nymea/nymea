#include "trigger.h"

Trigger::Trigger(const QUuid &triggerTypeId, const QVariantMap &params):
    m_triggerTypeId(triggerTypeId),
    m_params(params)
{
}

QUuid Trigger::triggerTypeId() const
{
    return m_triggerTypeId;
}

QVariantMap Trigger::params() const
{
    return m_params;
}

void Trigger::setParams(const QVariantMap &params)
{
    m_params = params;
}
