/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015-2018 Simon Stürz <simon.stuerz@guh.io>              *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
 *                                                                         *
 *  This file is part of nymea.                                            *
 *                                                                         *
 *  This library is free software; you can redistribute it and/or          *
 *  modify it under the terms of the GNU Lesser General Public             *
 *  License as published by the Free Software Foundation; either           *
 *  version 2.1 of the License, or (at your option) any later version.     *
 *                                                                         *
 *  This library is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
 *  Lesser General Public License for more details.                        *
 *                                                                         *
 *  You should have received a copy of the GNU Lesser General Public       *
 *  License along with this library; If not, see                           *
 *  <http://www.gnu.org/licenses/>.                                        *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*!
  \class Device
  \brief A Device represents a installed and configured hardware device.

  \ingroup devices
  \inmodule libnymea

  This class holds the values for configured devices. It is associated with a \{DeviceClass} which
  can be used to get more details about the device.

  \sa DeviceClass, DeviceDescriptor
*/

/*! \fn void Device::stateValueChanged(const QUuid &stateTypeId, const QVariant &value)
    This signal is emitted when the \l{State} with the given \a stateTypeId changed.
    The \a value parameter describes the new value of the State.
*/
#include "device.h"
#include "types/event.h"
#include "loggingcategories.h"

#include <QDebug>

/*! Construct an Device with the given \a pluginId, \a id, \a deviceClassId and \a parent. */
Device::Device(const PluginId &pluginId, const DeviceId &id, const DeviceClassId &deviceClassId, QObject *parent):
    QObject(parent),
    m_id(id),
    m_deviceClassId(deviceClassId),
    m_pluginId(pluginId)
{

}

/*! Construct an Device with the given \a pluginId, \a deviceClassId and \a parent. A new DeviceId will be created for this Device. */
Device::Device(const PluginId &pluginId, const DeviceClassId &deviceClassId, QObject *parent):
    QObject(parent),
    m_id(DeviceId::createDeviceId()),
    m_deviceClassId(deviceClassId),
    m_pluginId(pluginId)
{

}

void Device::setupCompleted()
{
    m_setupComplete = true;
}

/*! Returns the id of this Device. */
DeviceId Device::id() const
{
    return m_id;
}

/*! Returns the deviceClassId of the associated \l{DeviceClass}. */
DeviceClassId Device::deviceClassId() const
{
    return m_deviceClassId;
}

/*! Returns the id of the \l{DevicePlugin} this Device is managed by. */
PluginId Device::pluginId() const
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
    if (m_name != name) {
        m_name = name;
        emit nameChanged();
    }
}

/*! Returns the parameter of this Device. It must match the parameter description in the associated \l{DeviceClass}. */
ParamList Device::params() const
{
    return m_params;
}

/*! Sets the \a params of this Device. It must match the parameter description in the associated \l{DeviceClass}. */
void Device::setParams(const ParamList &params)
{
    m_params = params;
}

/*! Returns the value of the \l{Param} of this Device with the given \a paramTypeId. */
QVariant Device::paramValue(const ParamTypeId &paramTypeId) const
{
    foreach (const Param &param, m_params) {
        if (param.paramTypeId() == paramTypeId) {
            return param.value();
        }
    }
    return QVariant();
}

/*! Sets the \a value of the \l{Param} with the given \a paramTypeId. */
void Device::setParamValue(const ParamTypeId &paramTypeId, const QVariant &value)
{
    ParamList params;
    foreach (Param param, m_params) {
        if (param.paramTypeId() == paramTypeId) {
            param.setValue(value);
        }
        params << param;
    }
    m_params = params;
}

/*! Returns the states of this Device. It must match the \l{StateType} description in the associated \l{DeviceClass}. */
QList<State> Device::states() const
{
    return m_states;
}

/*! Returns true, a \l{Param} with the given \a paramTypeId exists for this Device. */
bool Device::hasParam(const ParamTypeId &paramTypeId) const
{
    return m_params.hasParam(paramTypeId);
}

/*! Set the \l{State}{States} of this \l{Device} to the given \a states.*/
void Device::setStates(const QList<State> &states)
{
    m_states = states;
}

/*! Returns true, a \l{State} with the given \a stateTypeId exists for this Device. */
bool Device::hasState(const StateTypeId &stateTypeId) const
{
    foreach (const State &state, m_states) {
        if (state.stateTypeId() == stateTypeId) {
            return true;
        }
    }
    return false;
}

/*! For convenience, this finds the \l{State} matching the given \a stateTypeId and returns the current valie in this Device. */
QVariant Device::stateValue(const StateTypeId &stateTypeId) const
{
    foreach (const State &state, m_states) {
        if (state.stateTypeId() == stateTypeId) {
            return state.value();
        }
    }
    return QVariant();
}

/*! For convenience, this finds the \l{State} matching the given \a stateTypeId in this Device and sets the current value to \a value. */
void Device::setStateValue(const StateTypeId &stateTypeId, const QVariant &value)
{
    for (int i = 0; i < m_states.count(); ++i) {
        if (m_states.at(i).stateTypeId() == stateTypeId) {
            if (m_states.at(i).value() == value)
                return;


            // TODO: check min/max value + possible values
            //       to prevent an invalid state type from the plugin side

            State newState(stateTypeId, m_id);
            newState.setValue(value);
            m_states[i] = newState;
            emit stateValueChanged(stateTypeId, value);
            return;
        }
    }
    qCWarning(dcDeviceManager) << "Failed setting state for" << m_name << value;
}

/*! Returns the \l{State} with the given \a stateTypeId of this Device. */
State Device::state(const StateTypeId &stateTypeId) const
{
    for (int i = 0; i < m_states.count(); ++i) {
        if (m_states.at(i).stateTypeId() == stateTypeId) {
            return m_states.at(i);
        }
    }
    return State(StateTypeId(), DeviceId());
}

/*! Returns the \l{DeviceId} of the parent Device from Device. If the parentId
    is not set, this device is a parent device.
*/
DeviceId Device::parentId() const
{
    return m_parentId;
}

/*! Sets the \a parentId of this Device. If the parentId
    is not set, this device is a parent device.
*/
void Device::setParentId(const DeviceId &parentId)
{
    m_parentId = parentId;
}

/*! Returns true, if setup of this Device is already completed. */
bool Device::setupComplete() const
{
    return m_setupComplete;
}

/*! Returns true if this device has been auto-created (not created by the user) */
bool Device::autoCreated() const
{
    return m_autoCreated;
}

void Device::setSetupComplete(const bool &complete)
{
    m_setupComplete = complete;
}

Devices::Devices(const QList<Device*> &other)
{
    foreach (Device* device, other) {
        this->append(device);
    }
}

Device *Devices::findById(const DeviceId &id)
{
    foreach (Device *device, *this) {
        if (device->id() == id) {
            return device;
        }
    }
    return nullptr;
}

QDebug operator<<(QDebug dbg, Device *device)
{
    dbg.nospace() << "Device(" << device->name();
    dbg.nospace() << ", id: " << device->id().toString();
    dbg.nospace() << ", deviceClassId: " << device->deviceClassId().toString() << ")";
    return dbg.space();
}

/*! Filter a Devices list by a parameter. Only Devices having a parameter of the given
    \a paramTypeId will be returned. If \a value is given and it is not null, only Devices
    with the given \a paramTypeId and the same \a value will be returned.
 */
Devices Devices::filterByParam(const ParamTypeId &paramTypeId, const QVariant &value)
{
    Devices ret;
    foreach (Device* device, *this) {
        if (paramTypeId != paramTypeId) {
            continue;
        }
        if (!value.isNull() && device->paramValue(paramTypeId) != value) {
            continue;
        }
        ret << device;
    }
    return ret;
}
