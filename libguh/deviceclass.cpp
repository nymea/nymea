/*!
    \class DeviceClass
    \brief Describes \l{Device}{Devices}.

    \ingroup devices
    \inmodule libguh

    It holds information general information about devices and their vendors and
    describes what actions, events and states a device supports. As this is
    just a description of device and does not represent actual \l{Device}{Devices},
    the actions, events and states are described in form of \l{EventType},
    \l{StateType} and \l{ActionType}

    \sa Device
*/

#include "deviceclass.h"

/*! Constructs a DeviceClass with the give \a pluginId and \a id.
    When implementing a plugin, create a DeviceClass for each device you support.
    Generate a new uuid (e.g. uuidgen) and hardode it into the plugin. The id
    should never change or it will appear as a new DeviceClass in the system.
 */
DeviceClass::DeviceClass(const QUuid &pluginId, const QUuid &id):
    m_id(id),
    m_pluginId(pluginId)
{

}

/*! Returns the id of this DeviceClass. */
QUuid DeviceClass::id() const
{
    return m_id;
}

/*! Returns the pluginId this DeviceClass is managed by. */
QUuid DeviceClass::pluginId() const
{
    return m_pluginId;
}

/*! Returns true if this DeviceClass's id and pluginId are valid uuids. */
bool DeviceClass::isValid() const
{
    return !m_id.isNull() && !m_pluginId.isNull();
}

/*! Returns the name of this DeviceClass's. This is visible to the user. */
QString DeviceClass::name() const
{
    return m_name;
}

/*! Set the \a name of this DeviceClass's. This is visible to the user. */
void DeviceClass::setName(const QString &name)
{
    m_name = name;
}

/*! Returns the statesTypes of this DeviceClass. \{Device}{Devices} created
    from this DeviceClass must have their states matching to this template. */
QList<StateType> DeviceClass::states() const
{
    return m_states;
}

/*! Set the \a stateTypes of this DeviceClass. \{Device}{Devices} created
    from this DeviceClass must have their states matching to this template. */
void DeviceClass::setStates(const QList<StateType> &stateTypes)
{
    m_states = stateTypes;
}

/*! Returns the eventTypes of this DeviceClass. \{Device}{Devices} created
    from this DeviceClass must have their events matching to this template. */
QList<EventType> DeviceClass::events() const
{
    return m_events;
}

/*! Set the \a eventTypes of this DeviceClass. \{Device}{Devices} created
    from this DeviceClass must have their events matching to this template. */
void DeviceClass::setEvents(const QList<EventType> &eventTypes)
{
    m_events = eventTypes;
}

/*! Returns the actionTypes of this DeviceClass. \{Device}{Devices} created
    from this DeviceClass must have their actions matching to this template. */
QList<ActionType> DeviceClass::actions() const
{
    return m_actions;
}

/*! Set the \a actionTypes of this DeviceClass. \{Device}{Devices} created
    from this DeviceClass must have their actions matching to this template. */
void DeviceClass::setActions(const QList<ActionType> &actionTypes)
{
    m_actions = actionTypes;
}

/*! Returns the params description of this DeviceClass. \{Device}{Devices} created
    from this DeviceClass must have their params matching to this template. */
QVariantList DeviceClass::params() const
{
    return m_params;
}

/*! Set the \a params of this DeviceClass. \{Device}{Devices} created
    from this DeviceClass must have their actions matching to this template. */
void DeviceClass::setParams(const QVariantList &params)
{
    m_params = params;
}

/*! Compare this \a deviceClass to another. This is effectively the same as calling a.id() == b.id(). Returns true if the ids match.*/
bool DeviceClass::operator==(const DeviceClass &deviceClass) const
{
    return m_id == deviceClass.id();
}
