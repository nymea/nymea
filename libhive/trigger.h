#ifndef TRIGGER_H
#define TRIGGER_H

#include <QString>
#include <QUuid>
#include <QVariantList>

class Trigger
{
public:
    Trigger(const QUuid &id);

    QUuid id() const;

    QString name() const;
    void setName(const QString &name);

    QVariantList params() const;
    void setParams(const QVariantList &params);

private:
    QUuid m_id;
    QString m_name;
    QVariantList m_params;
};

#endif // TRIGGER_H
