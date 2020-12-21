#include "servicedata.h"

ServiceData::ServiceData(const ThingId &thingId, const QDateTime &timestamp):
    m_thingId(thingId),
    m_timestamp(timestamp)
{

}

ThingId ServiceData::thingId() const
{
    return m_thingId;
}

void ServiceData::setThingId(const ThingId &thingId)
{
    m_thingId = thingId;
}

QDateTime ServiceData::timestamp() const
{
    return m_timestamp;
}

void ServiceData::setTimestamp(const QDateTime &timestamp)
{
    m_timestamp = timestamp;
}

QHash<QString, QVariant> ServiceData::data() const
{
    return m_data;
}

void ServiceData::insert(const QString &key, const QVariant &data)
{
    m_data.insert(key, data);
}

void ServiceData::insert(const QHash<QString, QVariant> data)
{
    foreach (const QString &key, data.keys()) {
        m_data.insert(key, data.value(key));
    }
}
