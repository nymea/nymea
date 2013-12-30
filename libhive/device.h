#ifndef DEVICE_H
#define DEVICE_H

#include "trigger.h"

#include <QObject>
#include <QUuid>
#include <QVariant>

class Device: public QObject
{
    Q_OBJECT
    Q_PROPERTY(QUuid id READ id CONSTANT)

public:
    Device(const QUuid &deviceClassId, QObject *parent = 0);

    QUuid id() const;
    QUuid deviceClassId() const;

    QString name() const;
    void setName(const QString &name);

    QList<Trigger> triggers() const;
    void setTriggers(const QList<Trigger> triggers);

    QVariantMap params() const;
    void setParams(const QVariantMap &params);

private:
    QUuid m_id;
    QUuid m_deviceClassId;
    QString m_name;
    QList<Trigger> m_triggers;
    QVariantMap m_params;
};

#endif
