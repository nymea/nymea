#ifndef DEVICECLASS_H
#define DEVICECLASS_H

#include "triggertype.h"

#include <QList>
#include <QUuid>

class DeviceClass
{
public:
    DeviceClass(const QUuid &id);
    virtual ~DeviceClass();

    QUuid id() const;

    QString name() const;
    void setName(const QString &name);

    QList<TriggerType> triggers() const;
    void setTriggers(const QList<TriggerType> &triggers);

private:
    QUuid m_id;
    QString m_name;
    QList<TriggerType> m_triggers;
};

#endif
