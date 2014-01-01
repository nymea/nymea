#ifndef DEVICE_H
#define DEVICE_H

#include "trigger.h"
#include "action.h"

#include <QObject>
#include <QUuid>
#include <QVariant>

class Device: public QObject
{
    Q_OBJECT

    friend class DeviceManager;

public:
    QUuid id() const;
    QUuid deviceClassId() const;
    QUuid pluginId() const;

    QString name() const;
    void setName(const QString &name);

    QList<Trigger> triggers() const;
    void setTriggers(const QList<Trigger> triggers);

    QList<Action> actions() const;
    void setActions(const QList<Action> &actions);

    QVariantMap params() const;
    void setParams(const QVariantMap &params);

private:
    Device(const QUuid &pluginId, const QUuid &id, const QUuid &deviceClassId, QObject *parent = 0);
    Device(const QUuid &pluginId, const QUuid &deviceClassId, QObject *parent = 0);

private:
    QUuid m_id;
    QUuid m_deviceClassId;
    QUuid m_pluginId;
    QString m_name;
    QList<Trigger> m_triggers;
    QList<Action> m_actions;
    QVariantMap m_params;
};

#endif
