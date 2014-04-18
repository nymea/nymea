#ifndef PARAMTYPE_H
#define PARAMTYPE_H

#include <QVariant>

class ParamType
{
public:
    ParamType(const QString &name, const QVariant::Type type, const QVariant &defaultValue = QVariant());

    QString name() const;
    void setName(const QString &name);

    QVariant::Type type() const;
    void setType(QVariant::Type type);

    QVariant defaultValue() const;
    void setDefaultValue(const QVariant &defaultValue);

    QVariant minValue() const;
    void setMinValue(const QVariant &minValue);

    QVariant maxValue() const;
    void setMaxValue(const QVariant &maxValue);

private:
    QString m_name;
    QVariant::Type m_type;
    QVariant m_defaultValue;
    QVariant m_minValue;
    QVariant m_maxValue;
};

#endif // PARAMTYPE_H
