#include "state.h"

State::State(const QUuid &stateTypeId, const QUuid &deviceId):
    m_stateTypeId(stateTypeId),
    m_deviceId(deviceId)
{
}

QUuid State::stateTypeId() const
{
    return m_stateTypeId;
}

QUuid State::deviceId() const
{
    return m_deviceId;
}

QVariant State::value() const
{
    return m_value;
}

void State::setValue(const QVariant &value)
{
    m_value = value;
}
