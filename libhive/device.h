#ifndef DEVICE_H
#define DEVICE_H

#include "state.h"

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

    QVariantMap params() const;
    void setParams(const QVariantMap &params);

    QList<State> states() const;
    void setStates(const QList<State> &states);

    QVariant stateValue(const QUuid &stateTypeId) const;
    void setStateValue(const QUuid &stateTypeId, const QVariant &value);

private:
    Device(const QUuid &pluginId, const QUuid &id, const QUuid &deviceClassId, QObject *parent = 0);
    Device(const QUuid &pluginId, const QUuid &deviceClassId, QObject *parent = 0);

private:
    QUuid m_id;
    QUuid m_deviceClassId;
    QUuid m_pluginId;
    QString m_name;
    QVariantMap m_params;
    QList<State> m_states;
};

#endif
