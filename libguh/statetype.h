#ifndef STATETYPE_H
#define STATETYPE_H

#include <QUuid>
#include <QVariant>

class StateType
{
public:
    StateType(const QUuid &id);

    QUuid id() const;

    QString name() const;
    void setName(const QString &name);

    QVariant::Type type() const;
    void setType(const QVariant::Type &type);

    QVariant defaultValue() const;
    void setDefaultValue(const QVariant &defaultValue);

private:
    QUuid m_id;
    QString m_name;
    QVariant::Type m_type;
    QVariant m_defaultValue;
};

#endif // STATETYPE_H
