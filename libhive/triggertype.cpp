#include "triggertype.h"

TriggerType::TriggerType(const QUuid &id):
    m_id(id)
{
}

QUuid TriggerType::id() const
{
    return m_id;
}

QString TriggerType::name() const
{
    return m_name;
}

void TriggerType::setName(const QString &name)
{
    m_name = name;
}

QVariantList TriggerType::parameters() const
{
    return m_parameters;
}

void TriggerType::setParameters(const QVariantList &parameters)
{
    m_parameters = parameters;
}
