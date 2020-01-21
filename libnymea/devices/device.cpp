/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2020, nymea GmbH
* Contact: contact@nymea.io
*
* This file is part of nymea.
* This project including source code and documentation is protected by
* copyright law, and remains the property of nymea GmbH. All rights, including
* reproduction, publication, editing and translation, are reserved. The use of
* this project is subject to the terms of a license agreement to be concluded
* with nymea GmbH in accordance with the terms of use of nymea GmbH, available
* under https://nymea.io/license
*
* GNU Lesser General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU Lesser General Public License as published by the Free
* Software Foundation; version 3. This project is distributed in the hope that
* it will be useful, but WITHOUT ANY WARRANTY; without even the implied
* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*!
  \class Device
  \brief A Device represents a installed and configured hardware device.

  \ingroup devices
  \inmodule libnymea

  This class holds the values for configured devices. It is associated with a \{DeviceClass} which
  can be used to get more details about the device.

  \sa DeviceClass, DeviceDescriptor
*/

/*! \enum Device::DeviceError

    This enum type specifies the errors that can happen when working with \l{Device}{Devices}.

    \value DeviceErrorNoError
        No Error. Everything went fine.
    \value DeviceErrorPluginNotFound
        Couldn't find the Plugin for the given id.
    \value DeviceErrorVendorNotFound
        Couldn't find the Vendor for the given id.
    \value DeviceErrorDeviceNotFound
        Couldn't find a \l{Device} for the given id.
    \value DeviceErrorDeviceClassNotFound
        Couldn't find a \l{DeviceClass} for the given id.
    \value DeviceErrorActionTypeNotFound
        Couldn't find the \l{ActionType} for the given id.
    \value DeviceErrorStateTypeNotFound
        Couldn't find the \l{StateType} for the given id.
    \value DeviceErrorEventTypeNotFound
        Couldn't find the \l{EventType} for the given id.
    \value DeviceErrorDeviceDescriptorNotFound
        Couldn't find the \l{DeviceDescriptor} for the given id.
    \value DeviceErrorMissingParameter
        Parameters do not comply to the template.
    \value DeviceErrorInvalidParameter
        One of the given parameter is not valid.
    \value DeviceErrorSetupFailed
        Error setting up the \l{Device}. It will not be functional.
    \value DeviceErrorDuplicateUuid
        Error setting up the \l{Device}. The given DeviceId already exists.
    \value DeviceErrorCreationMethodNotSupported
        Error setting up the \l{Device}. This \l{DeviceClass}{CreateMethod} is not supported for this \l{Device}.
    \value DeviceErrorSetupMethodNotSupported
        Error setting up the \l{Device}. This \l{DeviceClass}{SetupMethod} is not supported for this \l{Device}.
    \value DeviceErrorHardwareNotAvailable
        The Hardware of the \l{Device} is not available.
    \value DeviceErrorHardwareFailure
        The Hardware of the \l{Device} has an error.
    \value DeviceErrorAsync
        The response of the \l{Device} will be asynchronously.
    \value DeviceErrorDeviceInUse
        The \l{Device} is currently bussy.
    \value DeviceErrorPairingTransactionIdNotFound
        Couldn't find the PairingTransactionId for the given id.
    \value DeviceErrorAuthentificationFailure
        The device could not authentificate with something.
    \value DeviceErrorDeviceIsChild
        The device is a child device and can not be deleted directly.
    \value DeviceErrorDeviceInRule
        The device is in a rule and can not be deleted withou \l{nymeaserver::RuleEngine::RemovePolicy}.
    \value DeviceErrorParameterNotWritable
        One of the given device params is not writable.
*/

/*! \enum Device::DeviceSetupStatus

    This enum type specifies the setup status of a \l{Device}.

    \value DeviceSetupStatusSuccess
        No Error. Everything went fine.
    \value DeviceSetupStatusFailure
        Something went wrong during the setup.
    \value DeviceSetupStatusAsync
        The status of the \l{Device} setup will be emitted asynchronous.
*/

/*! \fn void Device::stateValueChanged(const StateTypeId &stateTypeId, const QVariant &value)
    This signal is emitted when the \l{State} with the given \a stateTypeId changed.
    The \a value parameter describes the new value of the State.
*/
#include "device.h"
#include "deviceplugin.h"
#include "types/event.h"
#include "loggingcategories.h"

#include <QDebug>

/*! Construct an Device with the given \a pluginId, \a id, \a deviceClassId and \a parent. */
Device::Device(DevicePlugin *plugin, const DeviceClass &deviceClass, const DeviceId &id, QObject *parent):
    QObject(parent),
    m_deviceClass(deviceClass),
    m_plugin(plugin),
    m_id(id)
{

}

/*! Construct an Device with the given \a pluginId, \a deviceClassId and \a parent. A new DeviceId will be created for this Device. */
Device::Device(DevicePlugin *plugin, const DeviceClass &deviceClass, QObject *parent):
    QObject(parent),
    m_deviceClass(deviceClass),
    m_plugin(plugin),
    m_id(DeviceId::createDeviceId())
{

}

/*! Returns the id of this Device. */
DeviceId Device::id() const
{
    return m_id;
}

/*! Returns the deviceClassId of the associated \l{DeviceClass}. */
DeviceClassId Device::deviceClassId() const
{
    return m_deviceClass.id();
}

/*! Returns the id of the \l{DevicePlugin} this Device is managed by. */
PluginId Device::pluginId() const
{
    return m_plugin->pluginId();
}

/*! Returns the \l{DeviceClass} of this device. */
DeviceClass Device::deviceClass() const
{
    return m_deviceClass;
}

/*! Returns the the \l{DevicePlugin} this Device is managed by. */
DevicePlugin *Device::plugin() const
{
    return m_plugin;
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

ParamList Device::settings() const
{
    return m_settings;
}

bool Device::hasSetting(const ParamTypeId &paramTypeId) const
{
    return m_settings.hasParam(paramTypeId);
}

void Device::setSettings(const ParamList &settings)
{
    m_settings = settings;
    foreach (const Param &param, m_settings) {
        emit settingChanged(param.paramTypeId(), param.value());
    }
}

QVariant Device::setting(const ParamTypeId &paramTypeId) const
{
    foreach (Param setting, m_settings) {
        if (setting.paramTypeId() == paramTypeId) {
            return setting.value();
        }
    }
    return QVariant();
}

void Device::setSettingValue(const ParamTypeId &paramTypeId, const QVariant &value)
{
    ParamList settings;
    bool found = false;
    bool changed = false;
    foreach (Param param, m_settings) {
        if (param.paramTypeId() == paramTypeId) {
            found = true;
            if (param.value() != value) {
                param.setValue(value);
                changed = true;
            }
        }
        settings << param;
    }
    if (!found) {
        qCWarning(dcDeviceManager()) << "Device" << m_name << "(" << m_id.toString() << ") does not have a setting with id" << paramTypeId;
        return;
    }
    if (changed) {
        m_settings = settings;
        emit settingChanged(paramTypeId, value);
    }
}

/*! Returns the states of this Device. It must match the \l{StateType} description in the associated \l{DeviceClass}. */
States Device::states() const
{
    return m_states;
}

/*! Returns true, a \l{Param} with the given \a paramTypeId exists for this Device. */
bool Device::hasParam(const ParamTypeId &paramTypeId) const
{
    return m_params.hasParam(paramTypeId);
}

/*! Set the \l{State}{States} of this \l{Device} to the given \a states.*/
void Device::setStates(const States &states)
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

/*! Returns true, if setup of this Device is already completed. This method is deprecated, use setupStatus() instead. */
bool Device::setupComplete() const
{
    return m_setupStatus == DeviceSetupStatusComplete;
}

/*! Returns the setup error display message, if any. */
QString Device::setupDisplayMessage() const
{
    return m_setupDisplayMessage;
}

/*! Returns true if this device has been auto-created (not created by the user) */
bool Device::autoCreated() const
{
    return m_autoCreated;
}

/* Returns the current device setup status. */
Device::DeviceSetupStatus Device::setupStatus() const
{
    return m_setupStatus;
}

Device::DeviceError Device::setupError() const
{
    return m_setupError;
}

void Device::setSetupStatus(Device::DeviceSetupStatus status, Device::DeviceError setupError, const QString &displayMessage)
{
    m_setupStatus = status;
    m_setupError = setupError;
    m_setupDisplayMessage = displayMessage;
    emit setupStatusChanged();
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

/*! Find a certain device by its \a params. All parameters must
    match or the device will not be found. Be prepared for nullptrs.
*/
Device *Devices::findByParams(const ParamList &params) const
{
    foreach (Device *device, *this) {
        bool matching = true;
        foreach (const Param &param, params) {
            if (device->paramValue(param.paramTypeId()) != param.value()) {
                matching = false;
            }
        }
        if (matching) {
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

Devices Devices::filterByDeviceClassId(const DeviceClassId &deviceClassId)
{
    Devices ret;
    foreach (Device* device, *this) {
        if (device->deviceClassId() == deviceClassId) {
            ret << device;
        }
    }
    return ret;
}

Devices Devices::filterByParentDeviceId(const DeviceId &deviceId)
{
    Devices ret;
    foreach (Device *device, *this) {
        if (device->parentId() == deviceId) {
            ret << device;
        }
    }
    return ret;
}

Devices Devices::filterByInterface(const QString &interface)
{
    Devices ret;
    foreach (Device *device, *this) {
        if (device->deviceClass().interfaces().indexOf(interface) >= 0) {
            ret.append(device);
        }
    }
    return ret;
}

QVariant Devices::get(int index) const
{
    return QVariant::fromValue(at(index));
}

void Devices::put(const QVariant &variant)
{
    append(variant.value<Device*>());
}
