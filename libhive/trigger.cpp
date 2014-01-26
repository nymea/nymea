/*!
    \class Trigger
    \brief Holds information required to emit a trigger described by a \l{TriggerType}.

    \ingroup types
    \inmodule libhive

    It is bound to a \l{Device} and a \l{TriggerType} and holds the parameters
    for the event that happened.

    The params must match the template as described in \l{TriggerType}.

    \sa Device, TriggerType
*/

#include "trigger.h"

/*! Constructs a Trigger reflecting the \l{Trigger} given by \a triggerTypeId, associated with
    the \l{Device} given by \a deviceId and the parameters given by \a params. The parameters must
    match the description in the reflecting \l{Trigger}.*/
Trigger::Trigger(const QUuid &triggerTypeId, const QUuid &deviceId, const QVariantMap &params):
    m_triggerTypeId(triggerTypeId),
    m_deviceId(deviceId),
    m_params(params)
{
}

/*! Returns the id of the \l{TriggerType} which describes this Trigger.*/
QUuid Trigger::triggerTypeId() const
{
    return m_triggerTypeId;
}

/*! Returns the id of the \l{Device} associated with this Trigger.*/
QUuid Trigger::deviceId() const
{
    return m_deviceId;
}

/*! Returns the parameters of this Trigger.*/
QVariantMap Trigger::params() const
{
    return m_params;
}

/*! Set the parameters of this Trigger to \a params.*/
void Trigger::setParams(const QVariantMap &params)
{
    m_params = params;
}

/*! Compare this Trigger to the Trigger given by \a other.
    Triggers are equal (returns true) if triggerTypeId, deviceId and params match. */
bool Trigger::operator ==(const Trigger &other) const
{
    return m_triggerTypeId == other.triggerTypeId()
            && m_deviceId == other.deviceId()
            && m_params == other.params();
}
