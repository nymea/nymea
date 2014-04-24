#include "paramdescriptor.h"

ParamDescriptor::ParamDescriptor(const QString &name, const QVariant &value):
    Param(name, value),
    m_operand(OperandTypeEquals)
{
}

ParamDescriptor::OperandType ParamDescriptor::operand() const
{
    return m_operand;
}

void ParamDescriptor::setOperand(ParamDescriptor::OperandType operand)
{
    m_operand = operand;
}

