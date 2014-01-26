/*!
    \class StateType
    \brief Describes a \l{State} for a \l{Device}.

    \ingroup types
    \inmodule libhive

    \sa State
*/

#include "statetype.h"

/*! Constructs a State with the given \a id.
    When creating a \l{DevicePlugin} generate a new uuid for each StateType you define and
    hardcode it into the plugin. */
StateType::StateType(const QUuid &id):
    m_id(id)
{
}

/*! Returns the id of the StateType.*/
QUuid StateType::id() const
{
    return m_id;
}

/*! Returns the name of the StateType. This is visible to the user (e.g. "Temperature").*/
QString StateType::name() const
{
    return m_name;
}

/*! Set the name of the StateType to \a name. This is visible to the user (e.g. "Temperature").*/
void StateType::setName(const QString &name)
{
    m_name = name;
}

/*! Returns the Type of the StateType (e.g. QVariant::Real). */
QVariant::Type StateType::type() const
{
    return m_type;
}

/*! Set the type fo the StateType to \a type (e.g. QVariant::Real).*/
void StateType::setType(const QVariant::Type &type)
{
    m_type = type;
}

/*! Returns the default value of this StateType (e.g. 21.5) */
QVariant StateType::defaultValue() const
{
    return m_defaultValue;
}

/*! Set the default value of this StateType to \a defaultValue (e.g. 21.5). */
void StateType::setDefaultValue(const QVariant &defaultValue)
{
    m_defaultValue = defaultValue;
}
