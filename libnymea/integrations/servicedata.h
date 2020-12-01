#ifndef SERVICEDATA_H
#define SERVICEDATA_H

#include "typeutils.h"

#include <QDateTime>
#include <QHash>
#include <QVariant>

class ServiceData
{
public:
    ServiceData(const ThingId &thingId, const QDateTime &timestamp = QDateTime::currentDateTime());

    ThingId thingId() const;
    void setThingId(const ThingId &thingId);

    QDateTime timestamp() const;
    void setTimestamp(const QDateTime &timestamp);

    QHash<QString, QVariant> data() const;
    void insert(const QString &key, const QVariant &data);
    void insert(const QHash<QString, QVariant> data);

private:
    ThingId m_thingId;
    QDateTime m_timestamp;
    QHash<QString, QVariant> m_data;
};

#endif // SERVICEDATA_H
