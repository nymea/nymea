#ifndef DEVICECLASS_H
#define DEVICECLASS_H

#include "triggertype.h"
#include "actiontype.h"
#include "statetype.h"

#include <QList>
#include <QUuid>

class DeviceClass
{
public:
    DeviceClass(const QUuid &pluginId = QUuid(), const QUuid &id = QUuid());

    QUuid id() const;
    QUuid pluginId() const;
    bool isValid() const;

    QString name() const;
    void setName(const QString &name);

    QList<StateType> states() const;
    void setStates(const QList<StateType> &stateTypes);

    QList<TriggerType> triggers() const;
    void setTriggers(const QList<TriggerType> &triggerTypes);

    QList<ActionType> actions() const;
    void setActions(const QList<ActionType> &actionTypes);

    QVariantList params() const;
    void setParams(const QVariantList &params);

    bool operator==(const DeviceClass &device) const;

private:
    QUuid m_id;
    QUuid m_pluginId;
    QString m_name;
    QList<StateType> m_states;
    QList<TriggerType> m_triggers;
    QList<ActionType> m_actions;
    QVariantList m_params;
};

#endif
