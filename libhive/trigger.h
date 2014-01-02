#ifndef TRIGGER_H
#define TRIGGER_H

#include <QString>
#include <QUuid>
#include <QVariantList>

class Trigger
{
public:
    Trigger(const QUuid &triggerTypeId, const QVariantMap &params);

    QUuid triggerTypeId() const;

    QVariantMap params() const;
    void setParams(const QVariantMap &params);

private:
    QUuid m_triggerTypeId;
    QVariantMap m_params;
};

#endif // TRIGGER_H
