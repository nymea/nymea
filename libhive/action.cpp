/*!
    \class Action
    \brief Holds information required to execute an action described by a \l{ActionType}.

    \ingroup types
    \inmodule libhive

    It is bound to a \l{Device} and an \l{ActionType} and holds the parameters
    for the execution of the action.

    The params must match the template as described in \l{ActionType}.

    \sa Device, ActionType
*/
#include "action.h"

Action::Action(const QUuid &deviceId, const QUuid &actionTypeId) :
    m_actionTypeId(actionTypeId),
    m_deviceId(deviceId)
{
}

/*! An Action is valid if \l{actionTypeId} and \l{deviceId} are valid uuids. */
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
