/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stuerz <simon.stuerz@guh.guru>                *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
 *                                                                         *
 *  This file is part of guh.                                              *
 *                                                                         *
 *  Guh is free software: you can redistribute it and/or modify            *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  Guh is distributed in the hope that it will be useful,                 *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with guh. If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*! \class DeviceClass
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

/*! \enum DeviceClass::CreateMethod

    This enum type specifies the CreateMethod of this \l{DeviceClass}

    \value CreateMethodUser
        The user will specify the \l{Param}s.
    \value CreateMethodAuto
        The device will be created automaticaly.
    \value CreateMethodDiscovery
        The device will be discovered and added by the user by specifying the DeviceDescriptorId.
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

/*! \enum DeviceClass::BasicTag

    This enum type specifies the basic tags which describe the categories of the DeviceClass.
    A DeviceClass can have multiple tags.

    \value BasicTagService
        The \l{DeviceClass} describes a service.
    \value BasicTagDevice
        The \l{DeviceClass} describes a real device.
    \value BasicTagSensor
        The \l{DeviceClass} describes a sensor. Any device which can measure or detect something is a sensor.
    \value BasicTagActuator
        The \l{DeviceClass} describes an actuator. Any device which can do something is an actuator.
    \value BasicTagLighting
        The \l{DeviceClass} describes a lighting device.
    \value BasicTagEnergy
        The \l{DeviceClass} describes an energy device.
    \value BasicTagMultimedia
        The \l{DeviceClass} describes a multimedia device/service.
    \value BasicTagWeather
        The \l{DeviceClass} describes a weather device/service.
    \value BasicTagGateway
        The \l{DeviceClass} describes a gateway device.
    \value BasicTagHeating
        The \l{DeviceClass} describes a heating device.
    \value BasicTagCooling
        The \l{DeviceClass} describes a cooling device.
    \value BasicTagNotification
        The \l{DeviceClass} describes a notification service.
    \value BasicTagSecurity
        The \l{DeviceClass} describes a security device/service.
    \value BasicTagTime
        The \l{DeviceClass} describes a time device/service.
    \value BasicTagShading
        The \l{DeviceClass} describes a shading device.
    \value BasicTagAppliance
        The \l{DeviceClass} describes an appliance.
    \value BasicTagCamera
        The \l{DeviceClass} describes a camera device.
    \value BasicTagLock
        The \l{DeviceClass} describes a lock device.

*/

/*! \enum DeviceClass::DeviceIcon

    This enum type specifies the default device icon of the DeviceClass.
    Each client should have an icon set containing this list of icon types.
    If a client want's to offer special icons for a special DeviceClass or device type,
    he can bring it's own icon for a special DeviceClassId. If there is no special icon
    defined, he can fallback to the default icon.

    \value DeviceIconBed
    \value DeviceIconBlinds
    \value DeviceIconCeilingLamp
    \value DeviceIconCouch
    \value DeviceIconDeskLamp
    \value DeviceIconDesk
    \value DeviceIconHifi
    \value DeviceIconPower
    \value DeviceIconEnergy
    \value DeviceIconRadio
    \value DeviceIconSmartPhone
    \value DeviceIconSocket
    \value DeviceIconStandardLamp
    \value DeviceIconSun
    \value DeviceIconTablet
    \value DeviceIconThermometer
    \value DeviceIconTune
    \value DeviceIconTv
    \value DeviceIconBattery
    \value DeviceIconDishwasher
    \value DeviceIconWashingMachine
    \value DeviceIconLaundryDryer
    \value DeviceIconIrHeater
    \value DeviceIconRadiator
    \value DeviceIconSwitch
    \value DeviceIconMotionDetectors
    \value DeviceIconWeather
    \value DeviceIconTime
    \value DeviceIconLightBulb
    \value DeviceIconGateway
    \value DeviceIconMail
    \value DeviceIconNetwork
    \value DeviceIconCloud
*/


#include "deviceclass.h"

/*! Constructs a DeviceClass with the give \a pluginId ,\a vendorId and \a id .
    When implementing a plugin, create a DeviceClass for each device you support.
    Generate a new uuid (e.g. uuidgen) and hardode it into the plugin. The id
    should never change or it will appear as a new DeviceClass in the system.*/
DeviceClass::DeviceClass(const PluginId &pluginId, const VendorId &vendorId, const DeviceClassId &id):
    m_id(id),
    m_vendorId(vendorId),
    m_pluginId(pluginId),
    m_deviceIcon(DeviceIconPower),
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

/*! Returns true if this \l{DeviceClass}'s id, vendorId and pluginId are valid uuids. */
bool DeviceClass::isValid() const
{
    return !m_id.isNull() && !m_vendorId.isNull() && !m_pluginId.isNull();
}

/*! Returns the name of this \l{DeviceClass}'s. This is visible to the user. */
QString DeviceClass::name() const
{
    return m_name;
}

/*! Set the \a name of this DeviceClass's. This is visible to the user. */
void DeviceClass::setName(const QString &name)
{
    m_name = name;
}

/*! Returns the default \l{DeviceIcon} of this \l{DeviceClass}'s. */
DeviceClass::DeviceIcon DeviceClass::deviceIcon() const
{
    return m_deviceIcon;
}

/*! Set the \a deviceIcon of this DeviceClass.

    \sa DeviceClass::DeviceIcon
*/
void DeviceClass::setDeviceIcon(const DeviceClass::DeviceIcon &deviceIcon)
{
    m_deviceIcon = deviceIcon;
}

/*! Returns the list of basicTags of this DeviceClass.

    \sa DeviceClass::BasicTag
*/
QList<DeviceClass::BasicTag> DeviceClass::basicTags() const
{
    return m_basicTags;
}

/*! Set the list of \a basicTags of this DeviceClass.

    \sa DeviceClass::BasicTag
*/
void DeviceClass::setBasicTags(const QList<DeviceClass::BasicTag> &basicTags)
{
    m_basicTags = basicTags;
}

/*! Returns the statesTypes of this DeviceClass. \{Device}{Devices} created
    from this \l{DeviceClass} must have their states matching to this template. */
QList<StateType> DeviceClass::stateTypes() const
{
    return m_stateTypes;
}

/*! Set the \a stateTypes of this DeviceClass. \{Device}{Devices} created
    from this \l{DeviceClass} must have their states matching to this template. */
void DeviceClass::setStateTypes(const QList<StateType> &stateTypes)
{
    m_stateTypes = stateTypes;

    m_allEventTypes = m_eventTypes;
    foreach (const StateType &stateType, m_stateTypes) {
        EventType eventType(EventTypeId(stateType.id().toString()));
        eventType.setName(QString("%1 changed").arg(stateType.name()));
        ParamType paramType("value", stateType.type());
        eventType.setParamTypes(QList<ParamType>() << paramType);
        m_allEventTypes.append(eventType);
    }
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
QList<EventType> DeviceClass::eventTypes() const
{
    return m_allEventTypes;
}

/*! Set the \a eventTypes of this DeviceClass. \{Device}{Devices} created
    from this \l{DeviceClass} must have their events matching to this template. */
void DeviceClass::setEventTypes(const QList<EventType> &eventTypes)
{
    m_eventTypes = eventTypes;

    m_allEventTypes = m_eventTypes;
    foreach (const StateType &stateType, m_stateTypes) {
        EventType eventType(EventTypeId(stateType.id().toString()));
        eventType.setName(QString("%1 changed").arg(stateType.name()));
        ParamType paramType("value", stateType.type());
        eventType.setParamTypes(QList<ParamType>() << paramType);
        m_allEventTypes.append(eventType);
    }
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
QList<ActionType> DeviceClass::actionTypes() const
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
QList<ParamType> DeviceClass::paramTypes() const
{
    return m_paramTypes;
}

/*! Set the \a params of this DeviceClass. \{Device}{Devices} created
    from this \l{DeviceClass} must have their actions matching to this template. */
void DeviceClass::setParamTypes(const QList<ParamType> &params)
{
    m_paramTypes = params;
}

/*! Returns the discovery params description of this DeviceClass. \{Device}{Devices} created
    from this \l{DeviceClass} must have their params matching to this template. */
QList<ParamType> DeviceClass::discoveryParamTypes() const
{
    return m_discoveryParamTypes;
}
/*! Set the \a params of this DeviceClass for the discovery. \{Device}{Devices} created
    from this \l{DeviceClass} must have their actions matching to this template. */
void DeviceClass::setDiscoveryParamTypes(const QList<ParamType> &params)
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

/*! Compare this \a deviceClass to another. This is effectively the same as calling a.id() == b.id(). Returns true if the ids match.*/
bool DeviceClass::operator==(const DeviceClass &deviceClass) const
{
    return m_id == deviceClass.id();
}
