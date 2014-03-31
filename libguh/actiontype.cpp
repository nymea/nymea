/*!
    \class ActionType
    \brief Describes an \l{Action} for a \l{Device}.

    \ingroup types
    \inmodule libguh

    ActionTypes are contained in \l{DeviceClass} templates returned
    by \l{DevicePlugin}{DevicePlugins} in order to describe the hardware supported
    by the plugin.

    All Actions must have valid a ActionType in order to be usful.
    \sa Action
*/

#include "actiontype.h"

/*! Constructs an ActionType with the given \a id.*/
ActionType::ActionType(const QUuid &id):
    m_id(id)
{
}

/*! Returns the id of this ActionType.*/
QUuid ActionType::id() const
{
    return m_id;
}

/*! Returns the name of this ActionType */
QString ActionType::name() const
{
    return m_name;
}

/*! Set the \a name for this Action. This will be visible to to the user.*/
void ActionType::setName(const QString &name)
{
    m_name = name;
}

/*! Returns the parameter description of this ActionType. \l{Action}{Actions} created
    from this ActionType must have their parameters matching to this template. */
QVariantList ActionType::parameters() const
{
    return m_parameters;
}

/*! Set the parameter description of this ActionType. \l{Action}{Actions} created
    from this ActionType must have their \a parameters matching to this template. */
void ActionType::setParameters(const QVariantList &parameters)
{
    m_parameters = parameters;
}
