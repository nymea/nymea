/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
 *  Copyright (C) 2014-2018 Michael Zanetti <michael.zanetti@guh.io>       *
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

/*! \class DeviceClass
    \brief Describes \l{Device}{Devices}.

    \ingroup devices
    \inmodule libnymea

    It holds information general information about devices and their vendors and
    describes what actions, events and states a device supports. As this is
    just a description of device and does not represent actual \l{Device}{Devices},
    the actions, events and states are described in form of \l{EventType},
    \l{StateType} and \l{ActionType}

    \sa Device
*/

/*! \enum DeviceClass::CreateMethod

    This enum type specifies the CreateMethod of this \l{DeviceClass}

    \value CreateMethodUser
        The user will specify the \l{Param}s.
    \value CreateMethodAuto
        The device will be created automatically.
    \value CreateMethodDiscovery
        The device will be discovered and added by the user by selecting the DeviceDescriptorId from the list of discovered possible ones.
*/

/*! \enum DeviceClass::SetupMethod

    This enum type specifies the SetupMethod of this \l{DeviceClass}

    \value SetupMethodJustAdd
        The \l{Device} will be just added. This is the default value.
    \value SetupMethodDisplayPin
        During the setup, a pin will be displayed on the \l{Device}. The pin will be needed in order to pair the device.
    \value SetupMethodEnterPin
        During the setup, a pin will be needed in order to pair the \l{Device}.
    \value SetupMethodPushButton
        During the setup, a button has to be pushed in order to pair the \l{Device}.
*/


#include "deviceclass.h"

/*! Constructs a DeviceClass with the give \a pluginId ,\a vendorId and \a id .
    When implementing a plugin, create a DeviceClass for each device you support.
    Generate a new uuid (e.g. uuidgen) and hardode it into the plugin. The id
    should never change or it will appear as a new DeviceClass in the system. */
DeviceClass::DeviceClass(const PluginId &pluginId, const VendorId &vendorId, const DeviceClassId &id):
    m_id(id),
    m_vendorId(vendorId),
    m_pluginId(pluginId),
    m_createMethods(CreateMethodUser),
    m_setupMethod(SetupMethodJustAdd)
{

}

/*! Returns the id of this \l{DeviceClass}. */
DeviceClassId DeviceClass::id() const
{
    return m_id;
}

/*! Returns the VendorId for this \l{DeviceClass} */
VendorId DeviceClass::vendorId() const
{
    return m_vendorId;
}

/*! Returns the pluginId this \l{DeviceClass} is managed by. */
PluginId DeviceClass::pluginId() const
{
    return m_pluginId;
}

/*! Returns true if this \l{DeviceClass} id, vendorId and pluginId are valid uuids. */
bool DeviceClass::isValid() const
{
    return !m_id.isNull() && !m_vendorId.isNull() && !m_pluginId.isNull();
}

/*! Returns the name of this \l{DeviceClass}. This is visible to the user. */
QString DeviceClass::name() const
{
    return m_name;
}

/*! Set the \a name of this \l{DeviceClass}. This is visible to the user. */
void DeviceClass::setName(const QString &name)
{
    m_name = name;
}

/*! Returns the displayed name of this \l{DeviceClass}. This is visible to the user. */
QString DeviceClass::displayName() const
{
    return m_displayName;
}

/*! Set the \a displayName of this \l{DeviceClass}. This is visible to the user. */
void DeviceClass::setDisplayName(const QString &displayName)
{
    m_displayName = displayName;
}

/*! Returns the statesTypes of this DeviceClass. \{Device}{Devices} created
    from this \l{DeviceClass} must have their states matching to this template. */
StateTypes DeviceClass::stateTypes() const
{
    return m_stateTypes;
}

/*! Returns the \l{StateType} with the given \a stateTypeId of this \l{DeviceClass}.
 * If there is no matching \l{StateType}, an invalid \l{StateType} will be returned.*/
StateType DeviceClass::getStateType(const StateTypeId &stateTypeId)
{
    foreach (const StateType &stateType, m_stateTypes) {
        if (stateType.id() == stateTypeId)
            return stateType;
    }
    return StateType(StateTypeId());
}

/*! Set the \a stateTypes of this DeviceClass. \{Device}{Devices} created
    from this \l{DeviceClass} must have their states matching to this template. */
void DeviceClass::setStateTypes(const QList<StateType> &stateTypes)
{
    m_stateTypes = stateTypes;
}

/*! Returns true if this DeviceClass has a \l{StateType} with the given \a stateTypeId. */
bool DeviceClass::hasStateType(const StateTypeId &stateTypeId)
{
    foreach (const StateType &stateType, m_stateTypes) {
        if (stateType.id() == stateTypeId) {
            return true;
        }
    }
    return false;
}

/*! Returns the eventTypes of this DeviceClass. \{Device}{Devices} created
    from this \l{DeviceClass} must have their events matching to this template. */
EventTypes DeviceClass::eventTypes() const
{
    return m_eventTypes;
}

/*! Set the \a eventTypes of this DeviceClass. \{Device}{Devices} created
    from this \l{DeviceClass} must have their events matching to this template. */
void DeviceClass::setEventTypes(const QList<EventType> &eventTypes)
{
    m_eventTypes = eventTypes;
}

/*! Returns true if this DeviceClass has a \l{EventType} with the given \a eventTypeId. */
bool DeviceClass::hasEventType(const EventTypeId &eventTypeId)
{
    foreach (const EventType &eventType, m_eventTypes) {
        if (eventType.id() == eventTypeId) {
            return true;
        }
    }
    return false;
}

/*! Returns the actionTypes of this DeviceClass. \{Device}{Devices} created
    from this \l{DeviceClass} must have their actions matching to this template. */
ActionTypes DeviceClass::actionTypes() const
{
    return m_actionTypes;
}

/*! Set the \a actionTypes of this DeviceClass. \{Device}{Devices} created
    from this \l{DeviceClass} must have their actions matching to this template. */
void DeviceClass::setActionTypes(const QList<ActionType> &actionTypes)
{
    m_actionTypes = actionTypes;
}

/*! Returns true if this DeviceClass has a \l{ActionType} with the given \a actionTypeId. */
bool DeviceClass::hasActionType(const ActionTypeId &actionTypeId)
{
    foreach (const ActionType &actionType, m_actionTypes) {
        if (actionType.id() == actionTypeId) {
            return true;
        }
    }
    return false;
}

/*! Returns the params description of this DeviceClass. \{Device}{Devices} created
    from this \l{DeviceClass} must have their params matching to this template. */
ParamTypes DeviceClass::paramTypes() const
{
    return m_paramTypes;
}

/*! Set the \a params of this DeviceClass. \{Device}{Devices} created
    from this \l{DeviceClass} must have their actions matching to this template. */
void DeviceClass::setParamTypes(const ParamTypes &params)
{
    m_paramTypes = params;
}

/*! Returns the discovery params description of this DeviceClass. \{Device}{Devices} created
    from this \l{DeviceClass} must have their params matching to this template. */
ParamTypes DeviceClass::discoveryParamTypes() const
{
    return m_discoveryParamTypes;
}
/*! Set the \a params of this DeviceClass for the discovery. \{Device}{Devices} created
    from this \l{DeviceClass} must have their actions matching to this template. */
void DeviceClass::setDiscoveryParamTypes(const ParamTypes &params)
{
    m_discoveryParamTypes = params;
}

/*! Returns the \l{DeviceClass::CreateMethod}s of this \l{DeviceClass}.*/
DeviceClass::CreateMethods DeviceClass::createMethods() const
{
    return m_createMethods;
}

/*! Set the \a createMethods of this \l{DeviceClass}.
    \sa CreateMethod, */
void DeviceClass::setCreateMethods(DeviceClass::CreateMethods createMethods)
{
    m_createMethods = createMethods;
}

/*! Returns the \l{DeviceClass::SetupMethod} of this \l{DeviceClass}.*/
DeviceClass::SetupMethod DeviceClass::setupMethod() const
{
    return m_setupMethod;
}

/*! Set the \a setupMethod of this \l{DeviceClass}.
    \sa SetupMethod, */
void DeviceClass::setSetupMethod(DeviceClass::SetupMethod setupMethod)
{
    m_setupMethod = setupMethod;
}

/*! Returns the pairing information of this \l{DeviceClass}.*/
QString DeviceClass::pairingInfo() const
{
    return m_pairingInfo;
}

/*! Set the \a pairingInfo of this \l{DeviceClass}. */
void DeviceClass::setPairingInfo(const QString &pairingInfo)
{
    m_pairingInfo = pairingInfo;
}


/*! Returns the \l{Interfaces for DeviceClasses}{interfaces} of this \l{DeviceClass}.*/
QStringList DeviceClass::interfaces() const
{
    return m_interfaces;
}

/*! Set the \a interfaces of this \l{DeviceClass}.

    \note You can find information about interfaces \l{Interfaces for DeviceClasses}{here}.
*/
void DeviceClass::setInterfaces(const QStringList &interfaces)
{
    m_interfaces = interfaces;
}

/*! Compare this \a deviceClass to another. This is effectively the same as calling a.id() == b.id(). Returns true if the ids match.*/
bool DeviceClass::operator==(const DeviceClass &deviceClass) const
{
    return m_id == deviceClass.id();
}

/*! Returns a list of all valid JSON properties a DeviceClass JSON definition can have. */
QStringList DeviceClass::typeProperties()
{
    return QStringList() << "id" << "name" << "displayName" << "createMethods" << "setupMethod"
                         << "interfaces" << "pairingInfo" << "discoveryParamTypes" << "discoveryParamTypes"
                         << "paramTypes" << "stateTypes" << "actionTypes" << "eventTypes";
}

/*! Returns a list of mandatory JSON properties a DeviceClass JSON definition must have. */
QStringList DeviceClass::mandatoryTypeProperties()
{
    return QStringList() << "id" << "name" << "displayName";
}

QDebug operator<<(QDebug &dbg, const DeviceClass &deviceClass)
{
    dbg << "DeviceClass ID:" << deviceClass.id() << "Name:" << deviceClass.name();
    return dbg;
}
