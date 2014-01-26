/*!
  \class State
  \brief Holds the parameters of a State of a \l{Device}.

  \ingroup types
  \inmodule libhive

  States hold the state values for devices. A State is associated to a \l{Device} by
  the \l{Device::deviceId} {deviceId} and represents the value of a state described in a \l{StateType}

  \sa StateType
*/

#include "state.h"

State::State(const QUuid &stateTypeId, const QUuid &deviceId):
    m_stateTypeId(stateTypeId),
    m_deviceId(deviceId)
{
}

/*! Returns the id of the StateType describing this State. */
QUuid State::stateTypeId() const
{
    return m_stateTypeId;
}

/*! Returns the id of the StateType describing this State. */
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
