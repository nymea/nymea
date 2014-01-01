#ifndef ACTIONTYPE_H
#define ACTIONTYPE_H

#include <QUuid>
#include <QVariantList>

class ActionType
{
public:
    ActionType(const QUuid &id);

    QUuid id() const;

    QString name() const;
    void setName(const QString &name);

    QVariantList parameters() const;
    void setParameters(const QVariantList &parameters);

private:
    QUuid m_id;
    QString m_name;

    QVariantList m_parameters;
};

#endif // ACTIONTYPE_H
