/*!
    \class DeviceClass
    \brief Describes \l{Device}{Devices}.

    \ingroup devices
    \inmodule libhive

    It holds information general information about devices and their vendors and
    describes what actions, triggers and states a device supports. As this is
    just a description of device and does not represent actual \l{Device}{Devices},
    the actions, triggers and states are described in form of \l{TriggerType},
    \l{StateType} and \l{ActionType}

    \sa Device
*/

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

bool DeviceClass::isValid() const
{
    return !m_id.isNull() && !m_pluginId.isNull();
}

QString DeviceClass::name() const
{
    return m_name;
}

void DeviceClass::setName(const QString &name)
{
    m_name = name;
}

QList<StateType> DeviceClass::states() const
{
    return m_states;
}

void DeviceClass::setStates(const QList<StateType> &states)
{
    m_states = states;
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
