// SPDX-License-Identifier: LGPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2025, chargebyte austria GmbH
*
* This file is part of nymea.
*
* nymea is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* as published by the Free Software Foundation, either version 3
* of the License, or (at your option) any later version.
*
* nymea is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with nymea. If not, see <https://www.gnu.org/licenses/>.
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

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


#include "thingclass.h"

/*! Constructs a DeviceClass with the give \a pluginId ,\a vendorId and \a id .
    When implementing a plugin, create a DeviceClass for each device you support.
    Generate a new uuid (e.g. uuidgen) and hardode it into the plugin. The id
    should never change or it will appear as a new DeviceClass in the system. */
ThingClass::ThingClass(const PluginId &pluginId, const VendorId &vendorId, const ThingClassId &id):
    m_id(id),
    m_vendorId(vendorId),
    m_pluginId(pluginId)
{

}

/*! Returns the id of this \l{DeviceClass}. */
ThingClassId ThingClass::id() const
{
    return m_id;
}

/*! Returns the VendorId for this \l{DeviceClass} */
VendorId ThingClass::vendorId() const
{
    return m_vendorId;
}

/*! Returns the pluginId this \l{DeviceClass} is managed by. */
PluginId ThingClass::pluginId() const
{
    return m_pluginId;
}

/*! Returns true if this \l{DeviceClass} id, vendorId and pluginId are valid uuids. */
bool ThingClass::isValid() const
{
    return !m_id.isNull() && !m_vendorId.isNull() && !m_pluginId.isNull();
}

/*! Returns the name of this \l{DeviceClass}. This is visible to the user. */
QString ThingClass::name() const
{
    return m_name;
}

/*! Set the \a name of this \l{DeviceClass}. This is visible to the user. */
void ThingClass::setName(const QString &name)
{
    m_name = name;
}

/*! Returns the displayed name of this \l{DeviceClass}. This is visible to the user. */
QString ThingClass::displayName() const
{
    return m_displayName;
}

/*! Set the \a displayName of this \l{DeviceClass}. This is visible to the user. */
void ThingClass::setDisplayName(const QString &displayName)
{
    m_displayName = displayName;
}

/*! Returns the statesTypes of this DeviceClass. \{Device}{Devices} created
    from this \l{DeviceClass} must have their states matching to this template. */
StateTypes ThingClass::stateTypes() const
{
    return m_stateTypes;
}

/*! Returns the \l{StateType} with the given \a stateTypeId of this \l{DeviceClass}.
 * If there is no matching \l{StateType}, an invalid \l{StateType} will be returned.*/
StateType ThingClass::getStateType(const StateTypeId &stateTypeId)
{
    foreach (const StateType &stateType, m_stateTypes) {
        if (stateType.id() == stateTypeId)
            return stateType;
    }
    return StateType(StateTypeId());
}

/*! Set the \a stateTypes of this DeviceClass. \{Device}{Devices} created
    from this \l{DeviceClass} must have their states matching to this template. */
void ThingClass::setStateTypes(const StateTypes &stateTypes)
{
    m_stateTypes = stateTypes;
}

/*! Returns true if this DeviceClass has a \l{StateType} with the given \a stateTypeId. */
bool ThingClass::hasStateType(const StateTypeId &stateTypeId) const
{
    foreach (const StateType &stateType, m_stateTypes) {
        if (stateType.id() == stateTypeId) {
            return true;
        }
    }
    return false;
}

bool ThingClass::hasStateType(const QString &stateTypeName) const
{
    foreach (const StateType &stateType, m_stateTypes) {
        if (stateType.name() == stateTypeName) {
            return true;
        }
    }
    return false;
}

/*! Returns the eventTypes of this DeviceClass. \{Device}{Devices} created
    from this \l{DeviceClass} must have their events matching to this template. */
EventTypes ThingClass::eventTypes() const
{
    return m_eventTypes;
}

/*! Set the \a eventTypes of this DeviceClass. \{Device}{Devices} created
    from this \l{DeviceClass} must have their events matching to this template. */
void ThingClass::setEventTypes(const EventTypes &eventTypes)
{
    m_eventTypes = eventTypes;
}

/*! Returns true if this DeviceClass has a \l{EventType} with the given \a eventTypeId. */
bool ThingClass::hasEventType(const EventTypeId &eventTypeId) const
{
    foreach (const EventType &eventType, m_eventTypes) {
        if (eventType.id() == eventTypeId) {
            return true;
        }
    }
    return false;
}

bool ThingClass::hasEventType(const QString &eventTypeName) const
{
    foreach (const EventType &eventType, m_eventTypes) {
        if (eventType.name() == eventTypeName) {
            return true;
        }
    }
    return false;
}

/*! Returns the actionTypes of this DeviceClass. \{Device}{Devices} created
    from this \l{DeviceClass} must have their actions matching to this template. */
ActionTypes ThingClass::actionTypes() const
{
    return m_actionTypes;
}

/*! Set the \a actionTypes of this DeviceClass. \{Device}{Devices} created
    from this \l{DeviceClass} must have their actions matching to this template. */
void ThingClass::setActionTypes(const ActionTypes &actionTypes)
{
    m_actionTypes = actionTypes;
}

/*! Returns true if this DeviceClass has a \l{ActionType} with the given \a actionTypeId. */
bool ThingClass::hasActionType(const ActionTypeId &actionTypeId) const
{
    foreach (const ActionType &actionType, m_actionTypes) {
        if (actionType.id() == actionTypeId) {
            return true;
        }
    }
    return false;
}

bool ThingClass::hasActionType(const QString &actionTypeName) const
{
    foreach (const ActionType &actionType, m_actionTypes) {
        if (actionType.name() == actionTypeName) {
            return true;
        }
    }
    return false;
}

/*! Returns the browserItemActionTypes of this DeviceClass. \{Device}{Devices} created
    from this \l{DeviceClass} may set those actions to their browser items. */
ActionTypes ThingClass::browserItemActionTypes() const
{
    return m_browserItemActionTypes;
}

/*! Set the \a browserActionTypes of this DeviceClass. \{Device}{Devices} created
    from this \l{DeviceClass} may set those actions to their browser items. */
void ThingClass::setBrowserItemActionTypes(const ActionTypes &browserItemActionTypes)
{
    m_browserItemActionTypes = browserItemActionTypes;
}

/*! Returns true if this DeviceClass has a \l{ActionType} with the given \a actionTypeId. */
bool ThingClass::hasBrowserItemActionType(const ActionTypeId &actionTypeId)
{
    foreach (const ActionType &actionType, m_browserItemActionTypes) {
        if (actionType.id() == actionTypeId) {
            return true;
        }
    }
    return false;
}

/*! Returns the params description of this DeviceClass. \{Device}{Devices} created
    from this \l{DeviceClass} must have their params matching to this template. */
ParamTypes ThingClass::paramTypes() const
{
    return m_paramTypes;
}

/*! Set the \a paramsTypes of this DeviceClass. \{Device}{Devices} created
    from this \l{DeviceClass} must have their params matching to this template. */
void ThingClass::setParamTypes(const ParamTypes &params)
{
    m_paramTypes = params;
}

/*! Returns the settings description of this DeviceClass. \{Device}{Devices} created
    from this \l{DeviceClass} must have their settings matching to this template. */
ParamTypes ThingClass::settingsTypes() const
{
    return m_settingsTypes;
}

/*! Set the \a settingsTypes of this DeviceClass. \{Device}{Devices} created
    from this \l{DeviceClass} must have their settings matching to this template. */
void ThingClass::setSettingsTypes(const ParamTypes &settingsTypes)
{
    m_settingsTypes = settingsTypes;
}

/*! Returns the discovery params description of this DeviceClass. \{Device}{Devices} created
    from this \l{DeviceClass} must have their params matching to this template. */
ParamTypes ThingClass::discoveryParamTypes() const
{
    return m_discoveryParamTypes;
}
/*! Set the \a params of this DeviceClass for the discovery. \{Device}{Devices} created
    from this \l{DeviceClass} must have their actions matching to this template. */
void ThingClass::setDiscoveryParamTypes(const ParamTypes &params)
{
    m_discoveryParamTypes = params;
}

/*! Returns the \l{DeviceClass::CreateMethod}s of this \l{DeviceClass}.*/
ThingClass::CreateMethods ThingClass::createMethods() const
{
    return m_createMethods;
}

/*! Set the \a createMethods of this \l{DeviceClass}.
    \sa CreateMethod, */
void ThingClass::setCreateMethods(ThingClass::CreateMethods createMethods)
{
    m_createMethods = createMethods;
}

/*! Returns the \l{DeviceClass::SetupMethod} of this \l{DeviceClass}.*/
ThingClass::SetupMethod ThingClass::setupMethod() const
{
    return m_setupMethod;
}

/*! Set the \a setupMethod of this \l{DeviceClass}.
    \sa SetupMethod, */
void ThingClass::setSetupMethod(ThingClass::SetupMethod setupMethod)
{
    m_setupMethod = setupMethod;
}

ThingClass::DiscoveryType ThingClass::discoveryType() const
{
    return m_discoveryType;
}

void ThingClass::setDiscoveryType(DiscoveryType discoveryType)
{
    m_discoveryType = discoveryType;
}

/*! Returns the \l{Interfaces for DeviceClasses}{interfaces} of this \l{DeviceClass}.*/
QStringList ThingClass::interfaces() const
{
    return m_interfaces;
}

/*! Set the \a interfaces of this \l{DeviceClass}.

    \note You can find information about interfaces \l{Interfaces for DeviceClasses}{here}.
*/
void ThingClass::setInterfaces(const QStringList &interfaces)
{
    m_interfaces = interfaces;
}

/*! Returns the interfaces that a thing does not directly implement, but it may still cater for
    them by creating childs that do implement those. This is used as a hint for clients to
    filter for desired interfaces during thing setup.
 */
QStringList ThingClass::providedInterfaces() const
{
    return m_providedInterfaces;
}

/*! Set the list of provided interfaces. This list should contain interfaces for things that
    may be created as childs of this thing class.
*/
void ThingClass::setProvidedInterfaces(const QStringList &providedInterfaces)
{
    m_providedInterfaces = providedInterfaces;
}

/*! Returns whether \l{Device}{Devices} created from this \l{DeviceClass} are browsable */
bool ThingClass::browsable() const
{
    return m_browsable;
}

/*! Sets whether \l{Device}{Devices} created from this \l{DeviceClass} are browsable */
void ThingClass::setBrowsable(bool browsable)
{
    m_browsable = browsable;
}

/*! Compare this \a thingClass to another. This is effectively the same as calling a.id() == b.id(). Returns true if the ids match.*/
bool ThingClass::operator==(const ThingClass &thingClass) const
{
    return m_id == thingClass.id();
}

QDebug operator<<(QDebug dbg, const ThingClass &thingClass)
{
    QDebugStateSaver saver(dbg);
    dbg.nospace() << "ThingClass(" << thingClass.id().toString() << "Name: " << thingClass.name() << ")";
    return dbg;
}

ThingClasses::ThingClasses()
{

}

ThingClasses::ThingClasses(const QList<ThingClass> &other): QList<ThingClass> (other)
{
}

ThingClass ThingClasses::findById(const ThingClassId &id) const
{
    foreach (const ThingClass &thingClass, *this) {
        if (thingClass.id() == id) {
            return thingClass;
        }
    }
    return ThingClass();
}

QVariant ThingClasses::get(int index) const
{
    return QVariant::fromValue(at(index));
}

void ThingClasses::put(const QVariant &variant)
{
    append(variant.value<ThingClass>());
}
