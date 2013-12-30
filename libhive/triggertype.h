#ifndef TRIGGERTYPE_H
#define TRIGGERTYPE_H

#include <QUuid>
#include <QVariantMap>

class TriggerType
{
public:
    TriggerType(const QUuid &id);

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

#endif // TRIGGERTYPE_H
