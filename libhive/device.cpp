/*!
  \class Device
  \brief A Device represents a installed and configured hardware device.

  \ingroup devices
  \inmodule libhive

  This class holds the values for configured devices. It is associated with a \{DeviceClass} which
  can be used to get more details about the device.

  \sa DeviceClass
*/

#include "device.h"

#include <QDebug>

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

/*! Returns the id of this Device. */
QUuid Device::id() const
{
    return m_id;
}

/*! Returns the deviceClassId of the associated \l{DeviceClass}. */
QUuid Device::deviceClassId() const
{
    return m_deviceClassId;
}

/*! Returns the id of the \l{DevicePlugin} this Device is managed by. */
QUuid Device::pluginId() const
{
    return m_pluginId;
}

/*! Returns the name of this Device. This is visible to the user. */
QString Device::name() const
{
    return m_name;
}

/*! Set the \a name for this Device. This is visible to the user.*/
void Device::setName(const QString &name)
{
    m_name = name;
}

/*! Returns the parameter of this Device. It must match the parameter description in the associated \l{DeviceClass}. */
QVariantMap Device::params() const
{
    return m_params;
}

/*! Sets the \a params of this Device. It must match the parameter description in the associated \l{DeviceClass}. */
void Device::setParams(const QVariantMap &params)
{
    m_params = params;
}

/*! Returns the states of this Device. It must match the \l{StateType} description in the associated \l{DeviceClass}. */
QList<State> Device::states() const
{
    return m_states;
}

/*! Sets the \a states of this Device. It must match the \l{StateType} description in the associated \l{DeviceClass}. */
void Device::setStates(const QList<State> &states)
{
    m_states = states;
}

/*! For convenience, this finds the \l{State} matching the given \a stateTypeId and returns the current valie in this Device. */
QVariant Device::stateValue(const QUuid &stateTypeId) const
{
    foreach (const State &state, m_states) {
        if (state.stateTypeId() == stateTypeId) {
            return state.value();
        }
    }
    return QVariant();
}

/*! For convenience, this finds the \l{State} matching the given \a stateTypeId in this Device and sets the current value to \a value. */
void Device::setStateValue(const QUuid &stateTypeId, const QVariant &value)
{
    for (int i = 0; i < m_states.count(); ++i) {
        if (m_states.at(i).stateTypeId() == stateTypeId) {
            State newState(stateTypeId, m_id);
            newState.setValue(value);
            m_states[i] = newState;
            return;
        }
    }
    qWarning() << "failed setting state for" << m_name;
}
