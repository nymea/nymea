#ifndef DEVICECLASS_H
#define DEVICECLASS_H

#include "triggertype.h"
#include "actiontype.h"

#include <QList>
#include <QUuid>

class DeviceClass
{
public:
    DeviceClass(const QUuid &pluginId, const QUuid &id);
    virtual ~DeviceClass();

    QUuid id() const;
    QUuid pluginId() const;

    QString name() const;
    void setName(const QString &name);

    QList<TriggerType> triggers() const;
    void setTriggers(const QList<TriggerType> &triggers);

    QList<ActionType> actions() const;
    void setActions(const QList<ActionType> &actions);

    QVariantList params() const;
    void setParams(const QVariantList &params);

    bool operator==(const DeviceClass &device) const;

private:
    QUuid m_id;
    QUuid m_pluginId;
    QString m_name;
    QList<TriggerType> m_triggers;
    QList<ActionType> m_actions;
    QVariantList m_params;
};

#endif
