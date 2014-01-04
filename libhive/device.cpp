#include "device.h"

Device::Device(const QUuid &pluginId, const QUuid &id, const QUuid &deviceClassId, QObject *parent):
    QObject(parent),
    m_id(id),
    m_deviceClassId(deviceClassId),
    m_pluginId(pluginId)
{

}

Device::Device(const QUuid &pluginId, const QUuid &deviceClassId, QObject *parent):
    QObject(parent),
    m_id(QUuid::createUuid()),
    m_deviceClassId(deviceClassId),
    m_pluginId(pluginId)
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

QUuid Device::pluginId() const
{
    return m_pluginId;
}

QString Device::name() const
{
    return m_name;
}

void Device::setName(const QString &name)
{
    m_name = name;
}

QVariantMap Device::params() const
{
    return m_params;
}

void Device::setParams(const QVariantMap &params)
{
    m_params = params;
}

QList<State> Device::states() const
{
    return m_states;
}

void Device::setStates(const QList<State> &states)
{
    m_states = states;
}
