#ifndef PARAMDESCRIPTOR_H
#define PARAMDESCRIPTOR_H

#include "param.h"

class ParamDescriptor : public Param
{
public:
    enum OperandType {
        OperandTypeEquals,
        OperandTypeNotEquals,
        OperandTypeLess,
        OperandTypeGreater,
        OperandTypeLessOrEqual,
        OperandTypeGreaterOrEqual
    };
    ParamDescriptor(const QString &name, const QVariant &value = QVariant());

    OperandType operand() const;
    void setOperand(OperandType operand);

private:
    OperandType m_operand;
};

#endif // PARAMDESCRIPTOR_H
