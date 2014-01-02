#include "trigger.h"

Trigger::Trigger(const QUuid &triggerTypeId, const QUuid &deviceId, const QVariantMap &params):
    m_triggerTypeId(triggerTypeId),
    m_deviceId(deviceId),
    m_params(params)
{
}

QUuid Trigger::triggerTypeId() const
{
    return m_triggerTypeId;
}

QUuid Trigger::deviceId() const
{
    return m_deviceId;
}

QVariantMap Trigger::params() const
{
    return m_params;
}

void Trigger::setParams(const QVariantMap &params)
{
    m_params = params;
}

bool Trigger::operator ==(const Trigger &other) const
{
    return m_triggerTypeId == other.triggerTypeId()
            && m_deviceId == other.deviceId()
            && m_params == other.params();
}
