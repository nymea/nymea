/*!
  \class State
  \brief Holds the parameters of a State of a \l{Device}.

  \ingroup types
  \inmodule libhive

  States hold the state values for devices. A State is associated to a \l{Device} by
  the \l{State::deviceId()} and represents the value of a state described in a \l{StateType}

  \sa StateType
*/

#include "state.h"

/*! Constructs a State reflecting the \l{StateType} given by \a stateTypeId
    and associated with the \l{Device} given by \a deviceId */
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

/*! Returns the state's value.*/
QVariant State::value() const
{
    return m_value;
}

/*! Set the state's value to \a value.*/
void State::setValue(const QVariant &value)
{
    m_value = value;
}
