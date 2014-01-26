/*!
    \class TriggerType
    \brief Describes a \l{Trigger} for a \l{Device}.

    \ingroup types
    \inmodule libhive

    \sa Trigger
*/

#include "triggertype.h"

/*! Constructs a TriggerType object with the given \a id. */
TriggerType::TriggerType(const QUuid &id):
    m_id(id)
{
}

/*! Returns the id. */
QUuid TriggerType::id() const
{
    return m_id;
}

/*! Returns the name of this TriggerType, e.g. "Temperature changed" */
QString TriggerType::name() const
{
    return m_name;
}

/*! Set the name for this TriggerType to \a name, e.g. "Temperature changed" */
void TriggerType::setName(const QString &name)
{
    m_name = name;
}

/*!
  Holds a map describing possible parameters for a \l{Trigger} of this TriggerType.
  e.g. QVariantList(QVariantMap(("name", "temperature"), ("type": QVariant::Real)))
  */
QVariantList TriggerType::parameters() const
{
    return m_parameters;
}

/*!
  Set the parameter description for this TriggerType to \a parameters,
  e.g. QVariantList(QVariantMap(("name", "temperature"), ("type": QVariant::Real)))
  */
void TriggerType::setParameters(const QVariantList &parameters)
{
    m_parameters = parameters;
}
