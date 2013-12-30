#include "trigger.h"

Trigger::Trigger(const QUuid &id):
    m_id(id)
{
}

QUuid Trigger::id() const
{
    return m_id;
}

QString Trigger::name() const
{
    return m_name;
}

void Trigger::setName(const QString &name)
{
    m_name = name;
}

QVariantList Trigger::params() const
{
    return m_params;
}

void Trigger::setParams(const QVariantList &params)
{
    m_params = params;
}
