#include "deviceclass.h"

DeviceClass::DeviceClass(const QUuid &pluginId, const QUuid &id):
    m_id(id),
    m_pluginId(pluginId)
{

}

DeviceClass::~DeviceClass()
{

}

QUuid DeviceClass::id() const
{
    return m_id;
}

QUuid DeviceClass::pluginId() const
{
    return m_pluginId;
}

QString DeviceClass::name() const
{
    return m_name;
}

void DeviceClass::setName(const QString &name)
{
    m_name = name;
}

QList<TriggerType> DeviceClass::triggers() const
{
    return m_triggers;
}

void DeviceClass::setTriggers(const QList<TriggerType> &triggers)
{
    m_triggers = triggers;
}

QList<ActionType> DeviceClass::actions() const
{
    return m_actions;
}

void DeviceClass::setActions(const QList<ActionType> &actions)
{
    m_actions = actions;
}

QVariantList DeviceClass::params() const
{
    return m_params;
}

void DeviceClass::setParams(const QVariantList &params)
{
    m_params = params;
}

bool DeviceClass::operator==(const DeviceClass &deviceClass) const
{
    return m_id == deviceClass.id();
}
