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


/*! \fn void DevicePlugin::devicesDiscovered(const DeviceClassId &deviceClassId, const QList<DeviceDescriptor> &devices);
    Emit this signal when the discovery of a \a deviceClassId of this DevicePlugin is finished. The \a devices parameter describes the
    list of \l{DeviceDescriptor}{DeviceDescriptors} of all discovered \l{Device}{Devices}.
    Note: During a discovery a plugin should always return the full result set. So even if a device is already known to the system and
    a later discovery finds the device again, it should be included in the result set but the DeviceDescriptor's deviceId should be set
    to the device ID.
    \sa discoverDevices()
*/

/*! \fn void DevicePlugin::pairingFinished(const PairingTransactionId &pairingTransactionId, Device::DeviceSetupStatus status);
    This signal is emitted when the pairing of a \a pairingTransactionId is finished.
    The \a status of the  will be described as \l{Device::DeviceError}{DeviceError}.
    \sa confirmPairing()
*/

/*! \fn void DevicePlugin::deviceSetupFinished(Device *device, Device::DeviceSetupStatus status);
    This signal is emitted when the setup of a \a device in this DevicePlugin is finished. The \a status parameter describes the
    \l{Device::DeviceError}{DeviceError} that occurred.
*/

/*! \fn void DevicePlugin::configValueChanged(const ParamTypeId &paramTypeId, const QVariant &value);
    This signal is emitted when the \l{Param} with a certain \a paramTypeId of a \l{Device} configuration changed the \a value.
*/

/*! \fn void DevicePlugin::actionExecutionFinished(const ActionId &id, Device::DeviceError status)
    This signal is to be emitted when you previously have returned \l{DeviceManager}{DeviceErrorAsync}
    in a call of executeAction(). The \a id refers to the executed \l{Action}. The \a status of the \l{Action}
    execution will be described as \l{Device::DeviceError}{DeviceError}.
*/

/*! \fn void DevicePlugin::autoDevicesAppeared(const DeviceClassId &deviceClassId, const QList<DeviceDescriptor> &deviceDescriptors)
    This signal is emitted when a new \l{Device} of certain \a deviceClassId appeared. The description of the \l{Device}{Devices}
    will be in \a deviceDescriptors. This signal can only emitted from devices with the \l{DeviceClass}{CreateMethodAuto}.
*/

/*! \fn void DevicePlugin::autoDeviceDisappeared(const DeviceId &id)
    Emit this signal when a device with the given \a id and which was created by \l{DevicePlugin::autoDevicesAppeared} has been removed from the system.
    Be careful with this, as this will completely remove the device from the system and with it all the associated rules. Only
    emit this if you are sure that a device will never come back. This signal should not be emitted for child auto devices
    when the parent who created them is removed. The system will automatically remove all child devices in such a case.
*/

/*! \fn void DevicePlugin::emitEvent(const Event &event)
    To produce a new event in the system, create a new \l{Event} and emit it with \a event.
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
#include <QSettings>
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

/*! Reimplement this if you support a DeviceClass with createMethod \l{DeviceManager}{CreateMethodDiscovery}.
    This will be called to discover Devices for the given \a deviceClassId with the given \a params. This will always
    be an async operation. Return \l{DeviceManager}{DeviceErrorAsync} or \l{DeviceManager}{DeviceErrorNoError}
    if the discovery has been started successfully. Return an appropriate error otherwise.
    Once devices are discovered, emit devicesDiscovered().
*/
Device::DeviceError DevicePlugin::discoverDevices(const DeviceClassId &deviceClassId, const ParamList &params)
{
    Q_UNUSED(deviceClassId)
    Q_UNUSED(params)
    return Device::DeviceErrorCreationMethodNotSupported;
}

/*! This will be called when a new device is created. The plugin has the chance to do some setup.
    Return \l{DeviceManager}{DeviceSetupStatusFailure} if something bad happened during the setup in which case the \a device
    will be disabled. Return \l{DeviceManager}{DeviceSetupStatusSuccess} if everything went well. If you can't tell yet and
    need more time to set up the \a device (note: you should never block in this method) you can
    return \l{DeviceManager}{DeviceSetupStatusAsync}. In that case the \l{DeviceManager} will wait for you to emit
    \l{DevicePlugin}{deviceSetupFinished} to report the status.
*/
Device::DeviceSetupStatus DevicePlugin::setupDevice(Device *device)
{
    Q_UNUSED(device)
    return Device::DeviceSetupStatusSuccess;
}

/*! This will be called when a new \a device was added successfully and the device setup is finished.*/
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

/*! This will be called to initiate a pairing procedure. The \a {devicePairingInfo} object
    will contain all the required info to proceed the pairing. Depending on the \l {DeviceClass::setupMethod}
    different values of the devicePairingInfo will be used.

    Start the pairing (e.g. Request a device to show a pin to be entered, or create the OAuth URL that needs
    to be shown to the user to log in) in this method. For some setup methods, for instance common cases
    of \l {DeviceClass::SetupMethodUserAndPassword} might not require anything to do in here. This method
    can still be overridden in case some preparation on the device needs to be performed or if an informational
    message should be shown to the user.
*/
DevicePairingInfo DevicePlugin::pairDevice(DevicePairingInfo &devicePairingInfo)
{
    qCWarning(dcDeviceManager) << "Plugin does not implement pairDevice. Defaults will be used.";
    devicePairingInfo.setStatus(Device::DeviceErrorNoError);
    return devicePairingInfo;
}

/*! Confirms the pairing of a \a deviceClassId with the given \a pairingTransactionId and \a params.
    Returns \l{Device::DeviceError}{DeviceError} to inform about the result. The optional paramerter
    \a secret contains for example the pin for \l{Device}{Devices} with the setup method \l{DeviceClass::SetupMethodDisplayPin}.
*/
DevicePairingInfo DevicePlugin::confirmPairing(DevicePairingInfo &devicePairingInfo, const QString &username, const QString &secret)
{
    Q_UNUSED(devicePairingInfo)
    Q_UNUSED(username)
    Q_UNUSED(secret)

    qCWarning(dcDeviceManager) << "Plugin does not implement pairing.";
    devicePairingInfo.setStatus(Device::DeviceErrorNoError);
    return devicePairingInfo;
}

/*! This will be called to actually execute actions on the hardware. The \{Device} and
    the \{Action} are contained in the \a device and \a action parameters.
    Return the appropriate \l{Device::DeviceError}{DeviceError}.

    It is possible to execute actions asynchronously. You never should do anything blocking for
    a long time (e.g. wait on a network reply from the internet) but instead return
    Device::DeviceErrorAsync and continue processing in an async manner. Once
    you have the reply ready, emit actionExecutionFinished() with the appropriate parameters.

    \sa actionExecutionFinished()
*/
Device::DeviceError DevicePlugin::executeAction(Device *device, const Action &action)
{
    Q_UNUSED(device)
    Q_UNUSED(action)
    return Device::DeviceErrorNoError;
}

/*! Implement this if your devices support browsing (set "browsable" to true in the metadata).
 *  When the system calls this method, fill the \a result object's items list with entries from the browser.
 *  If \a itemId is empty it means that the root node of the file system should be returned. Each item in
 *  the result set shall be uniquely identifiable using its \l{BrowserItem::id}{id} property.
 *  The system might call this method again, with an \a itemId returned in a previous query, provided
 *  that item's \l{BrowserItem::browsable} property is true. In this case all children of the given
 *  item shall be returned. All browser \l {BrowserItem::displayName} properties shall be localized
 *  using the given \a locale.
 *  When done, set the \l{BrowserResult::status}{result's status} field approprietly. Set the result's
 *  status to Device::DeviceErrorAsync if this operation requires async behavior and emit
 *  \l{browseRequestFinished} when done.
 */
Device::BrowseResult DevicePlugin::browseDevice(Device *device, Device::BrowseResult result, const QString &itemId, const QLocale &locale)
{
    Q_UNUSED(device)
    Q_UNUSED(itemId)
    Q_UNUSED(locale)

    result.status = Device::DeviceErrorUnsupportedFeature;
    return result;
}

/*! Implement this if your devices support browsing (set "browsable" to true in the metadata).
 *  When the system calls this method, fetch the item details required to create a BrowserItem
 *  for the item with the given \a id and append that one item to the \a result.
 *  When done, set the \l{BrowserResult::status}{result's status} field approprietly. Set the result's
 *  status to Device::DeviceErrorAsync if this operation requires async behavior and emit
 *  \l{browserItemRequestFinished} when done.
 */
Device::BrowserItemResult DevicePlugin::browserItem(Device *device, Device::BrowserItemResult result, const QString &itemId, const QLocale &locale)
{
    Q_UNUSED(device)
    Q_UNUSED(itemId)
    Q_UNUSED(locale)

    result.status = Device::DeviceErrorUnsupportedFeature;
    return result;
}

/*! Implement this if your devices support browsing and execute the itemId defined in \a browserAction.
 *  Return Device::DeviceErrorAsync if this operation requires async behavior and emit
 *  \l{browserItemExecutionFinished} when done.
 */
Device::DeviceError DevicePlugin::executeBrowserItem(Device *device, const BrowserAction &browserAction)
{
    Q_UNUSED(device)
    Q_UNUSED(browserAction)
    return Device::DeviceErrorUnsupportedFeature;
}

/*! Implement this if your devices support browsing and execute the item's action for the itemId defined
 *  in \a browserItemAction.
 *  Return Device::DeviceErrorAsync if this operation requires async behavior and emit
 *  \l{browserItemActionExecutionFinished} when done.
 */
Device::DeviceError DevicePlugin::executeBrowserItemAction(Device *device, const BrowserItemAction &browserItemAction)
{
    Q_UNUSED(device)
    Q_UNUSED(browserItemAction)
    return Device::DeviceErrorUnsupportedFeature;
}

/*! Returns the configuration description of this DevicePlugin as a list of \l{ParamType}{ParamTypes}. */
ParamTypes DevicePlugin::configurationDescription() const
{
    return m_metaData.pluginSettings();
}

/*! This will be called when the DeviceManager initializes the plugin and set up the things behind the scenes.
    When implementing a new plugin, use \l{DevicePlugin::init()} instead in order to do initialisation work.
    The \l{DevicePlugin::init()} method will be called once the plugin configuration has been loaded. */
void DevicePlugin::initPlugin(const PluginMetadata &metadata, DeviceManager *deviceManager, HardwareManager *hardwareManager)
{
    m_metaData = metadata;
    m_deviceManager = deviceManager;
    m_hardwareManager = hardwareManager;
    m_storage = new QSettings(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/pluginconfig-" + pluginId().toString().remove(QRegExp("[{}]")) + ".conf", QSettings::IniFormat, this);
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
