#include "action.h"

Action::Action(const QUuid &deviceId, const QUuid &actionTypeId) :
    m_actionTypeId(actionTypeId),
    m_deviceId(deviceId)
{
}

bool Action::isValid() const
{
    return !m_actionTypeId.isNull() && !m_deviceId.isNull();
}

QUuid Action::actionTypeId() const
{
    return m_actionTypeId;
}

QUuid Action::deviceId() const
{
    return m_deviceId;
}

QVariantMap Action::params() const
{
    return m_params;
}

void Action::setParams(const QVariantMap &params)
{
    m_params = params;
}
