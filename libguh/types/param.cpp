#include "param.h"

#include <QDebug>

Param::Param(const QString &name, const QVariant &value):
    m_name (name),
    m_value(value),
    m_operand(OperandTypeEquals)
{
}

QString Param::name() const
{
    return m_name;
}

void Param::setName(const QString &name)
{
    m_name = name;
}

QVariant Param::value() const
{
    return m_value;
}

void Param::setValue(const QVariant &value)
{
    m_value = value;
}

Param::OperandType Param::operand() const
{
    return m_operand;
}

void Param::setOperand(Param::OperandType operand)
{
    m_operand = operand;
}

bool Param::isValid() const
{
    return !m_name.isEmpty() && m_value.isValid();
}

QDebug operator<<(QDebug dbg, const Param &param)
{
    dbg.nospace() << "Param(Name: " << param.name() << ", Value:" << param.value() << ")";

    return dbg.space();
}

QDebug operator<<(QDebug dbg, const QList<Param> &params)
{
    dbg.nospace() << "ParamList (count:" << params.count() << ")";
    for (int i = 0; i < params.count(); i++ ) {
        dbg.nospace() << "     " << i << ": " << params.at(i);
    }

    return dbg.space();
}
