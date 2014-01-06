#include "statetype.h"

StateType::StateType(const QUuid &id):
    m_id(id)
{
}

QUuid StateType::id() const
{
    return m_id;
}

QString StateType::name() const
{
    return m_name;
}

void StateType::setName(const QString &name)
{
    m_name = name;
}

QVariant::Type StateType::type() const
{
    return m_type;
}

void StateType::setType(const QVariant::Type &type)
{
    m_type = type;
}

QVariant StateType::defaultValue() const
{
    return m_defaultValue;
}

void StateType::setDefaultValue(const QVariant &defaultValue)
{
    m_defaultValue = defaultValue;
}
