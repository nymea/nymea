#include "trigger.h"

Trigger::Trigger(const QUuid &deviceClassid, const QVariantMap &params):
    m_deviceClassId(deviceClassid),
    m_params(params)
{
}

QUuid Trigger::deviceClassId() const
{
    return m_deviceClassId;
}

QVariantMap Trigger::params() const
{
    return m_params;
}

void Trigger::setParams(const QVariantMap &params)
{
    m_params = params;
}
