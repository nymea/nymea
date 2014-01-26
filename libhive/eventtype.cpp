/*!
    \class EventType
    \brief Describes a \l{Event} for a \l{Device}.

    \ingroup types
    \inmodule libhive

    \sa Event
*/

#include "eventtype.h"

/*! Constructs a EventType object with the given \a id. */
EventType::EventType(const QUuid &id):
    m_id(id)
{
}

/*! Returns the id. */
QUuid EventType::id() const
{
    return m_id;
}

/*! Returns the name of this EventType, e.g. "Temperature changed" */
QString EventType::name() const
{
    return m_name;
}

/*! Set the name for this EventType to \a name, e.g. "Temperature changed" */
void EventType::setName(const QString &name)
{
    m_name = name;
}

/*!
  Holds a map describing possible parameters for a \l{Event} of this EventType.
  e.g. QVariantList(QVariantMap(("name", "temperature"), ("type": QVariant::Real)))
  */
QVariantList EventType::parameters() const
{
    return m_parameters;
}

/*!
  Set the parameter description for this EventType to \a parameters,
  e.g. QVariantList(QVariantMap(("name", "temperature"), ("type": QVariant::Real)))
  */
void EventType::setParameters(const QVariantList &parameters)
{
    m_parameters = parameters;
}
