#include "paramtype.h"

ParamType::ParamType(const QString &name, const QVariant::Type type, const QVariant &defaultValue):
    m_name(name),
    m_type(type),
    m_defaultValue(defaultValue)
{
}

QString ParamType::name() const
{
    return m_name;
}

void ParamType::setName(const QString &name)
{
    m_name = name;
}

QVariant::Type ParamType::type() const
{
    return m_type;
}

void ParamType::setType(QVariant::Type type)
{
    m_type = type;
}

QVariant ParamType::defaultValue() const
{
    return m_defaultValue;
}

void ParamType::setDefaultValue(const QVariant &defaultValue)
{
    m_defaultValue = defaultValue;
}

QVariant ParamType::minValue() const
{
    return m_minValue;
}

void ParamType::setMinValue(const QVariant &minValue)
{
    m_minValue = minValue;
}

QVariant ParamType::maxValue() const
{
    return m_maxValue;
}

void ParamType::setMaxValue(const QVariant &maxValue)
{
    m_maxValue = maxValue;
}
