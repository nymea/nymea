#ifndef PARAM_H
#define PARAM_H

#include <QString>
#include <QVariant>

class Param
{
public:
    enum OperandType {
        OperandTypeEquals,
        OperandTypeNotEquals,
        OperandTypeLess,
        OperandTypeGreater,
        OperandTypeLessThan,
        OperandTypeGreaterThan
    };

    Param(const QString &name, const QVariant &value = QVariant());

    QString name() const;
    void setName(const QString &name);

    QVariant value() const;
    void setValue(const QVariant &value);

    OperandType operand() const;
    void setOperand(OperandType operand);

    bool isValid() const;

private:
    QString m_name;
    QVariant m_value;
    OperandType m_operand;
};

QDebug operator<<(QDebug dbg, const Param &param);
QDebug operator<<(QDebug dbg, const QList<Param> &params);

#endif // PARAM_H
