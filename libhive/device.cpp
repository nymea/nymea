#include "device.h"

Device::Device(const QUuid &id, const QUuid &deviceClassId, QObject *parent):
    QObject(parent),
    m_id(id),
    m_deviceClassId(deviceClassId)
{

}

Device::Device(const QUuid &deviceClassId, QObject *parent):
    QObject(parent),
    m_id(QUuid::createUuid()),
    m_deviceClassId(deviceClassId)
{

}

QUuid Device::id() const
{
    return m_id;
}

QUuid Device::deviceClassId() const
{
    return m_deviceClassId;
}

QString Device::name() const
{
    return m_name;
}

void Device::setName(const QString &name)
{
    m_name = name;
}

QList<Trigger> Device::triggers() const
{
    return m_triggers;
}

void Device::setTriggers(const QList<Trigger> triggers)
{
    m_triggers = triggers;
}

QVariantMap Device::params() const
{
    return m_params;
}

void Device::setParams(const QVariantMap &params)
{
    m_params = params;
}
