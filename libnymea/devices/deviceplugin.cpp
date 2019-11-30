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
  \class DevicePlugin
  \brief This is the base class interface for device plugins.

  \ingroup devices
  \inmodule libnymea

*/


/*! \fn void DevicePlugin::configValueChanged(const ParamTypeId &paramTypeId, const QVariant &value);
    This signal is emitted when the \l{Param} with a certain \a paramTypeId of a \l{Device} configuration changed the \a value.
*/

/*! \fn void DevicePlugin::autoDevicesAppeared(const DeviceDescriptors &deviceDescriptors)
    A plugin emits this signal when new \l{Device} appeared. For instance a plugin connected to a bridge might detect
    that new devices have been connected to the bridge. Emitting this signal will cause those devices to be added
    to the nymea system and setupDevice will be called for them.
*/

/*! \fn void DevicePlugin::autoDeviceDisappeared(const DeviceId &id)
    A plugin emits this signal when a device with the given \a id and which was created by \l{DevicePlugin::autoDevicesAppeared}
    has been removed from the system. When emitting this signal, nymea will remove the device from the system and with it all the
    associated rules and child devices. Because of this, this signal should only be emitted when it's certain that the given device
    will never return to be available any more.
*/

/*! \fn void DevicePlugin::emitEvent(const Event &event)
    To produce a new event in the system, a plugin creates a new \l{Event} and emits this signal it with that \a event.
    Usually events are emitted in response to incoming data or other other events happening. Find a configured
    \l{Device} from the \l{DeviceManager} and get its \l{EventType}{EventTypes}, then
    create a \l{Event} complying to that \l{EventType} and emit it here.
*/

/*! \fn void DevicePlugin::init()
    This will be called after constructing the DevicePlugin. Override this to do any
    initialisation work you need to do.
*/

#include "deviceplugin.h"
#include "devicemanager.h"
#include "deviceutils.h"
#include "loggingcategories.h"
#include "devicediscoveryinfo.h"
#include "devicesetupinfo.h"
#include "devicepairinginfo.h"
#include "deviceactioninfo.h"
#include "browseresult.h"
#include "browseritemresult.h"
#include "browseractioninfo.h"
#include "browseritemactioninfo.h"

#include "nymeasettings.h"

#include "hardware/radio433/radio433.h"
#include "network/upnp/upnpdiscovery.h"

#include <QDebug>
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QCoreApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QStandardPaths>

/*! DevicePlugin constructor. DevicePlugins will be instantiated by the DeviceManager, its \a parent. */
DevicePlugin::DevicePlugin(QObject *parent):
    QObject(parent)
{
}

/*! Virtual destructor... */
DevicePlugin::~DevicePlugin()
{

}

/*! Returns the name of this DevicePlugin. */
QString DevicePlugin::pluginName() const
{
    return m_metaData.pluginName();
}

/*! Returns the displayName of this DevicePlugin, to be shown to the user, translated. */
QString DevicePlugin::pluginDisplayName() const
{
    return m_metaData.pluginDisplayName();
}

/*! Returns the id of this DevicePlugin.
 *  When implementing a plugin, generate a new uuid and return it here. Always return the
 *  same uuid and don't change it or configurations can't be matched any more. */
PluginId DevicePlugin::pluginId() const
{
    return m_metaData.pluginId();
}

/*! Returns the list of \l{Vendor}{Vendors} supported by this DevicePlugin. */
Vendors DevicePlugin::supportedVendors() const
{
    return m_metaData.vendors();
}

/*! Return a list of \l{DeviceClass}{DeviceClasses} describing all the devices supported by this plugin.
    If a DeviceClass has an invalid parameter it will be ignored.
*/
DeviceClasses DevicePlugin::supportedDevices() const
{
    return m_metaData.deviceClasses();
}

/*! Override this if your plugin supports Device with DeviceClass::CreationMethodAuto.
    This will be called at startup, after the configured devices have been loaded.
    This is the earliest time you should start emitting autoDevicesAppeared(). If you
    are monitoring some hardware/service for devices to appear, start monitoring now.
    If you are building the devices based on a static list, you may emit
    autoDevicesAppeard() in here.
*/
void DevicePlugin::startMonitoringAutoDevices()
{

}

/*! A plugin must reimplement this if it supports a DeviceClass with createMethod \l{DeviceManager}{CreateMethodDiscovery}.
    When the nymea system needs to discover available devices, this will be called on the plugin. The plugin implementation
    is set to discover devices for the \l{DeviceClassId} given in the \a info object.
    When devices are discovered, they should be added to the info object by calling \l{DeviceDiscoveryInfo::addDeviceDescriptor}.
    Once the discovery is complete a plugin must finish it by calling \l{DeviceDiscoveryInfo::finish} using \l{Device::DeviceErrorNoError}
    in case of success, or a matching error code otherwise. An optional display message can be passed optionally which might be shown
    to the user, indicating more details about the error. The displayMessage must be made translatable by wrapping it in a QT_TR_NOOP()
    statement.

    A discovery might be cancelled by nymea. In which case the \l{DeviceDiscoveryInfo::aborted} signal will be emitted.
    The info object must not be accessed after \l{DeviceDiscoveryInfo::destroyed} is emitted.
*/
void DevicePlugin::discoverDevices(DeviceDiscoveryInfo *info)
{
    info->finish(Device::DeviceErrorUnsupportedFeature);
}

/*! This will be called when a new device is created. The plugin can do a setup of the device by reimplementing this method.
    The passed \a info object will contain the information about the new \l{Device}. When the setup is completed, a plugin
    must finish it by calling \l{DeviceSetupInfo::finish} on the \a info object. In case of success, \{Device::DeviceErrorNoError}
    must be used, or an appropriate \l{Device::DeviceError} in case of failure. An optional display message can be passed optionally which might be shown
    to the user, indicating more details about the error. The displayMessage must be made translatable by wrapping it in a QT_TR_NOOP()
    statement.

    A setup might be cancelled by nymea. In which case the \{DeviceSetupInfo::aborted} signal will be emitted.
    The info object must not be accessed after \l{DeviceSetupInfo::destroyed} is emitted.
*/
void DevicePlugin::setupDevice(DeviceSetupInfo *info)
{
    info->finish(Device::DeviceErrorNoError);
}

/*! This will be called when a new \a device was added successfully and the device setup is finished. A plugin can optionally
    trigger additional code to operate on the device by reimplementing this method.*/
void DevicePlugin::postSetupDevice(Device *device)
{
    Q_UNUSED(device)
}

/*! This will be called when a \a device removed. The plugin has the chance to do some teardown.
    The device is still valid during this call, but already removed from the system.
    The device will be deleted as soon as this method returns.
*/
void DevicePlugin::deviceRemoved(Device *device)
{
    Q_UNUSED(device)
}

/*! This method will be called to initiate a pairing. The plugin can do a initialisation for an upcoming pairing process.
    Depending on the setupMethod of a device class, different actions may be required here.
    SetupMethodDisplayPin should trigger the device to display a pin that will be entered in the client.
    SetupMethodOAuth should generate the OAuthUrl which will be opened on the client to allow the user logging in and obtain
    the OAuth code.
    SetupMethodEnterPin, SetupMethodPushButton and SetupMethodUserAndPassword will typically not require to do anything here.
    It is not required to reimplement this method for those setup methods, however, a Plugin reimplementing it must call
    \l{DevicePairingInfo::finish}{finish()} on the \l{DevicePairingInfo} object and can provide an optional displayMessage which
    might be presented to the user. Those strings need to be wrapped in QT_TR_NOOP() in order to be translatable for the client's
    locale.
*/
void DevicePlugin::startPairing(DevicePairingInfo *info)
{
    DeviceClass deviceClass = m_metaData.deviceClasses().findById(info->deviceClassId());
    if (!deviceClass.isValid()) {
        info->finish(Device::DeviceErrorDeviceClassNotFound);
        return;
    }
    switch (deviceClass.setupMethod()) {
    case DeviceClass::SetupMethodJustAdd:
        info->finish(Device::DeviceErrorSetupMethodNotSupported);
        return;
    case DeviceClass::SetupMethodEnterPin:
    case DeviceClass::SetupMethodPushButton:
    case DeviceClass::SetupMethodUserAndPassword:
        info->finish(Device::DeviceErrorNoError);
        return;
    case DeviceClass::SetupMethodDisplayPin:
    case DeviceClass::SetupMethodOAuth:
        // Those need to be handled by the plugin or it'll fail anyways.
        qCWarning(dcDevice()) << "StartPairing called but Plugin does not reimplement it.";
        info->finish(Device::DeviceErrorUnsupportedFeature);
    }
}

/*! Confirms the pairing of the given \a info. \a username and \a secret are filled in depending on the setupmethod of the device class.
    \a username will be used for SetupMethodUserAndPassword. \a secret will be used for SetupMethodUserAndPassword, SetupMethodDisplayPin
    and SetupMethodOAuth.
    Once the pairing is completed, the plugin implementation should call the info's finish() method reporting about the status of
    the pairing operation. The optional displayMessage needs to be wrapped in QT_TR_NOOP in order to be translatable to the client's
    locale.
*/
void DevicePlugin::confirmPairing(DevicePairingInfo *info, const QString &username, const QString &secret)
{
    Q_UNUSED(username)
    Q_UNUSED(secret)

    qCWarning(dcDeviceManager) << "Plugin does not implement pairing.";
    info->finish(Device::DeviceErrorUnsupportedFeature);
}

/*! This will be called to execute actions on the device. The given \a info object contains
    information about the target \l{Device} and the \l{Action} to be executed.

    When the execution is completed, a plugin must finish it by calling \l{DeviceActionInfo::finish} on the \a info
    object. In case of success, \l{Device::DeviceErrorNoError} must be used, or an appropriate \l{Device::DeviceError}
    in case of failure. An optional display message can be passed which might be shown
    to the user, indicating more details about the error. The displayMessage must be made translatable by wrapping it in a QT_TR_NOOP()
    statement.

    An action execution might be cancelled by nymea. In which case the \l{DeviceActionInfo::aborted} signal will be emitted.
    The info object must not be accessed after \l{DeviceActionInfo::destroyed} is emitted.
*/
void DevicePlugin::executeAction(DeviceActionInfo *info)
{
    info->finish(Device::DeviceErrorUnsupportedFeature);
}

/*! A plugin must implement this if devices support browsing ("browsable" being true in the metadata JSON).
    When the system calls this method, the \a result must be filled with entries from the browser using
    \l{BrowseResult::addItems}. The \a info object will contain information about which device and which item/node
    should be browsed. If the itemId is empty it means that the root node of the file system should be returned otherwise
    all the children of the given item/node should be returned.


    Each item in the result set shall be uniquely identifiable using its \l{BrowserItem::id}{id} property.
    The system might call this method again, with an itemId returned in a previous query, provided
    that item's \l{BrowserItem::browsable} property is true. In this case all children of the given
    item shall be returned. All browser \l{BrowserItem::displayName} properties shall be localized
    using the given locale in the  \a info object.

    If a returned item's \l{BrowserItem::executable} property is set to true, the system might call \l{DevicePlugin::executeBrowserAction}
    for this itemId.


    An item might have additional actions which must be defined in the plugin metadata JSON as "browserItemActionTypes". Such actions
    might be context actions to items in a browser. For instance, a file browser might add copy/cut/paste actions to an item. The
    system might call \l{DevicePlugin::executeBrowserItemAction} on such items.

    When done, the browse result must be completed by calling \l{BrowserResult::finish} with \l{Device::DeviceErrorNoError}
    in case of success or an appropriate \l{Device::DeviceError} otherwise. An optional display message can be passed which might be shown
    to the user, indicating more details about the error. The displayMessage must be made translatable by wrapping it in a QT_TR_NOOP()
    statement.
 */
void DevicePlugin::browseDevice(BrowseResult *result)
{
    qCWarning(dcDevice()) << "Device claims" << result->device()->deviceClass().name() << "to be browsable but plugin does not reimplement browseDevice!";
    result->finish(Device::DeviceErrorUnsupportedFeature);
}

/*! A plugin must implement this if devices support browsing ("browsable" being true in the metadata JSON).
    When the system calls this method, the \a result must be filled with a single item identified by the \l{DeviceItemResult::itemId}
    using \l{BrowseResult::addItem}. The \a result object will contain information about which device and which item/node
    should be browsed. If the itemId is empty it means that the root node of the file system should be returned.

    Each item in the result set shall be uniquely identifiable using its \l{BrowserItem::id}{id} property. The system might
    call this method again, with an itemId returned in a previous query, provided that item's \l{BrowserItem::browsable}
    property is true. In this case all children of the given item shall be returned. All browser \l{BrowserItem::displayName} properties shall be localized
    using the given locale in the \a info object.


    When done, the browse result must be completed by calling \l{BrowserItemResult::finish} with \l{Device::DeviceErrorNoError}
    in case of success or an appropriate \l{Device::DeviceError} otherwise. An optional display message can be passed which might be shown
    to the user, indicating more details about the error. The displayMessage must be made translatable by wrapping it in a QT_TR_NOOP()
    statement.
 */
void DevicePlugin::browserItem(BrowserItemResult *result)
{
    qCWarning(dcDevice()) << "Device claims" << result->device()->deviceClass().name() << "to be browsable but plugin does not reimplement browserItem!";
    result->finish(Device::DeviceErrorUnsupportedFeature);
}

/*! This will be called to execute browser items on the device. For instance, a file browser might execute a file here.
    The given \a info object contains information about the target \l{Device} and the \l{BrowserAction} to be executed.

    When the execution is completed, a plugin must finish it by calling \l{BrowserActionInfo::finish} on the \a info
    object. In case of success, \{Device::DeviceErrorNoError} must be used, or an appropriate \l{Device::DeviceError}
    in case of failure. An optional display message can be passed which might be shown
    to the user, indicating more details about the error. The displayMessage must be made translatable by wrapping it in a QT_TR_NOOP()
    statement.

    An action execution might be cancelled by nymea. In which case the \{BrowserActionInfo::aborted} signal will be emitted.
    The info object must not be accessed after \l{BrowsereActionInfo::destroyed} is emitted.
*/
void DevicePlugin::executeBrowserItem(BrowserActionInfo *info)
{
    qCWarning(dcDevice()) << "Device claims" << info->device()->deviceClass().name() << "to be browsable but plugin does not reimplement browserItem!";
    info->finish(Device::DeviceErrorUnsupportedFeature);
}

/*! This will be called to execute browser item actions on the device. For instance a file browser might have
    "browserItemActionTypes" defined in the JSON in order to support context options like copy/cut/paste.
    The given \a info object contains information about the target \l{Device} and the \l{BrowserItemAction} to be executed.

    When the execution is completed, a plugin must finish it by calling \l{BrowserItemActionInfo::finish} on the \a info
    object. In case of success, \l{Device::DeviceErrorNoError} must be used, or an appropriate \l{Device::DeviceError}
    in case of failure. An optional display message can be passed which might be shown
    to the user, indicating more details about the error. The displayMessage must be made translatable by wrapping it in a QT_TR_NOOP()
    statement.

    An action execution might be cancelled by nymea. In which case the \{BrowserItemActionInfo::aborted} signal will be emitted.
    The info object must not be accessed after \l{BrowserItemActionInfo::destroyed} is emitted.
*/
void DevicePlugin::executeBrowserItemAction(BrowserItemActionInfo *info)
{
    qCWarning(dcDevice()) << "Device claims" << info->device()->deviceClass().name() << "to be browsable but plugin does not reimplement browserItemAction!";
    info->finish(Device::DeviceErrorUnsupportedFeature);
}

/*! Returns the configuration description of this DevicePlugin as a list of \l{ParamType}{ParamTypes}. */
ParamTypes DevicePlugin::configurationDescription() const
{
    return m_metaData.pluginSettings();
}

void DevicePlugin::initPlugin(const PluginMetadata &metadata, DeviceManager *deviceManager, HardwareManager *hardwareManager)
{
    m_metaData = metadata;
    m_deviceManager = deviceManager;
    m_hardwareManager = hardwareManager;
    m_storage = new QSettings(NymeaSettings::settingsPath() + "/pluginconfig-" + pluginId().toString().remove(QRegExp("[{}]")) + ".conf", QSettings::IniFormat, this);
}

/*! Returns a map containing the plugin configuration.
    When implementing a new plugin, override this and fill in the empty configuration if your plugin requires any.
*/
ParamList DevicePlugin::configuration() const
{
    return m_config;
}

/*! Use this to retrieve the values for your parameters. Values might not be set
    at the time when your plugin is loaded, but will be set soon after. Listen to
    configurationValueChanged() to know when something changes.
    When implementing a new plugin, specify in configurationDescription() what you want to see here.
    Returns the config value of a \l{Param} with the given \a paramTypeId of this DevicePlugin.
*/
QVariant DevicePlugin::configValue(const ParamTypeId &paramTypeId) const
{
    return m_config.paramValue(paramTypeId);
}

/*! Will be called by the DeviceManager to set a plugin's \a configuration. */
Device::DeviceError DevicePlugin::setConfiguration(const ParamList &configuration)
{
    foreach (const Param &param, configuration) {
        qCDebug(dcDeviceManager()) << "* Set plugin configuration" << param;
        Device::DeviceError result = setConfigValue(param.paramTypeId(), param.value());
        if (result != Device::DeviceErrorNoError)
            return result;

    }
    return Device::DeviceErrorNoError;
}

/*! Can be called in the DevicePlugin to set a plugin's \l{Param} with the given \a paramTypeId and \a value. */
Device::DeviceError DevicePlugin::setConfigValue(const ParamTypeId &paramTypeId, const QVariant &value)
{
    bool found = false;
    foreach (const ParamType &paramType, configurationDescription()) {
        if (paramType.id() == paramTypeId) {
            found = true;
            Device::DeviceError result = DeviceUtils::verifyParam(paramType, Param(paramTypeId, value));
            if (result != Device::DeviceErrorNoError)
                return result;

            break;
        }
    }

    if (!found) {
        qCWarning(dcDeviceManager()) << QString("Could not find plugin parameter with the id %1.").arg(paramTypeId.toString());
        return Device::DeviceErrorInvalidParameter;
    }

    if (m_config.hasParam(paramTypeId)) {
        if (!m_config.setParamValue(paramTypeId, value)) {
            qCWarning(dcDeviceManager()) << "Could not set param value" << value << "for param with id" << paramTypeId.toString();
            return Device::DeviceErrorInvalidParameter;
        }
    } else {
        m_config.append(Param(paramTypeId, value));
    }

    emit configValueChanged(paramTypeId, value);
    return Device::DeviceErrorNoError;
}

bool DevicePlugin::isBuiltIn() const
{
    return m_metaData.isBuiltIn();
}

/*! Returns a list of all configured devices belonging to this plugin. */
Devices DevicePlugin::myDevices() const
{
    QList<Device*> ret;
    foreach (Device *device, m_deviceManager->configuredDevices()) {
        if (device->pluginId() == pluginId()) {
            ret.append(device);
        }
    }
    return ret;
}


/*! Returns the pointer to the main \l{HardwareManager} of this server. */
HardwareManager *DevicePlugin::hardwareManager() const
{
    return m_hardwareManager;
}

/*! Returns a pointer to a QSettings object which is reserved for this plugin.
    The plugin can store arbitrary data in this.
    */
QSettings* DevicePlugin::pluginStorage() const
{
    return m_storage;
}

void DevicePlugin::setMetaData(const PluginMetadata &metaData)
{
    m_metaData = metaData;
}

DevicePlugins::DevicePlugins()
{

}

DevicePlugins::DevicePlugins(const QList<DevicePlugin *> &other): QList<DevicePlugin *>(other)
{

}

DevicePlugin *DevicePlugins::findById(const PluginId &id) const
{
    foreach (DevicePlugin *plugin, *this) {
        if (plugin->pluginId() == id) {
            return plugin;
        }
    }
    return nullptr;
}

QVariant DevicePlugins::get(int index) const
{
    return QVariant::fromValue(at(index));
}

void DevicePlugins::put(const QVariant &variant)
{
    append(variant.value<DevicePlugin*>());
}
