/*!
    \class Action
    \brief Holds information required to execute an action described by a \l{ActionType}.

    \ingroup types
    \inmodule libguh

    It is bound to a \l{Device} and an \l{ActionType} and holds the parameters
    for the execution of the action.

    The params must match the template as described in \l{ActionType}.

    \sa Device, ActionType
*/
#include "action.h"

/*! Construct an Action with the given \a deviceId and \a actionTypeId */
Action::Action(const QUuid &deviceId, const QUuid &actionTypeId) :
    m_actionTypeId(actionTypeId),
    m_deviceId(deviceId)
{
}

/*! An Action is valid if actionTypeId and deviceId are valid uuids. Returns true if valid, false if not. */
bool Action::isValid() const
{
    return !m_actionTypeId.isNull() && !m_deviceId.isNull();
}

/*! Returns the actionTypeId for this Action */
QUuid Action::actionTypeId() const
{
    return m_actionTypeId;
}

/*! Returns the deviceId this Action is associated with.*/
QUuid Action::deviceId() const
{
    return m_deviceId;
}

/*! Returns the parameters for this Action.*/
QVariantMap Action::params() const
{
    return m_params;
}

/*! Set the the parameters for this Action. \a params must match the template in the \l{ActionType} referred by \l{Action::actionTypeId()}*/
void Action::setParams(const QVariantMap &params)
{
    m_params = params;
}
