#include "action.h"

Action::Action(const QUuid &id) :
    m_id(id)
{
}

bool Action::isValid() const
{
    return !m_id.isNull();
}

QUuid Action::id() const
{
    return m_id;
}

QString Action::name() const
{
    return m_name;
}

void Action::setName(const QString &name)
{
    m_name = name;
}

QVariantList Action::params() const
{
    return m_params;
}

void Action::setParams(const QVariantList &params)
{
    m_params = params;
}

