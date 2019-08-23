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


// Signals

/*! \fn void DeviceManager::loaded();
    The DeviceManager will emit this signal when all \l{Device}{Devices} are loaded.
*/

/*! \fn void DeviceManager::languageUpdated();
    The DeviceManager will emit this signal when all system language has been updated.
*/

/*! \fn void DeviceManager::pluginConfigChanged(const PluginId &id, const ParamList &config);
    The DeviceManager will emit this signal when the \a config \l{ParamList}{Params} of the \l{DevicePlugin}{plugin} with the given \a id has changed.
*/

/*! \fn void DeviceManager::deviceSetupFinished(Device *device, DeviceError status);
    This signal is emitted when the setup of a \a device is finished. The \a status parameter describes the
    \l{Device::DeviceError}{DeviceError} that occurred.
*/

/*! \fn void DeviceManager::deviceStateChanged(Device *device, const QUuid &stateTypeId, const QVariant &value);
    This signal is emitted when the \l{State} of a \a device changed. The \a stateTypeId parameter describes the
    \l{StateType} and the \a value parameter holds the new value.
*/

/*! \fn void DeviceManager::deviceDisappeared(const DeviceId &deviceId);
    This signal is emitted when the automatically created \l{Device} with the given \a deviceId dissapeard. This signal will
    create the Devices.DeviceRemoved notification.
*/

/*! \fn void DeviceManager::deviceRemoved(const DeviceId &deviceId);
    This signal is emitted when the \l{Device} with the given \a deviceId was removed from the system. This signal will
    create the Devices.DeviceRemoved notification.
*/

/*! \fn void DeviceManager::deviceAdded(Device *device);
    This signal is emitted when a \a \device  was added to the system. This signal will
    create the Devices.DeviceAdded notification.
*/

/*! \fn void DeviceManager::deviceChanged(Device *device);
    This signal is emitted when a \a \device  was changed in the system (by edit or rediscover). This signal will
    create the Devices.DeviceParamsChanged notification.
*/

/*! \fn void DeviceManager::deviceReconfigurationFinished(Device *device, DeviceError status);
    This signal is emitted when the edit process of a \a device is finished.  The \a status parameter describes the
    \l{Device::DeviceError}{DeviceError} that occurred.
*/

/*! \fn void DeviceManager::devicesDiscovered(const DeviceClassId &deviceClassId, const QList<DeviceDescriptor> &devices);
    This signal is emitted when the discovery of a \a deviceClassId is finished. The \a devices parameter describes the
    list of \l{DeviceDescriptor}{DeviceDescriptors} of all discovered \l{Device}{Devices}.
    \sa discoverDevices()
*/

/*! \fn void DeviceManager::actionExecutionFinished(const ActionId &actionId, DeviceError status);
    The DeviceManager will emit a this signal when the \l{Action} with the given \a actionId is finished.
    The \a status of the \l{Action} execution will be described as \l{Device::DeviceError}{DeviceError}.
*/

/*! \fn void DeviceManager::pairingFinished(const PairingTransactionId &pairingTransactionId, DeviceError status, const DeviceId &deviceId = DeviceId());
    The DeviceManager will emit a this signal when the pairing of a \l{Device} with the \a deviceId and \a pairingTransactionId is finished.
    The \a status of the pairing will be described as \l{Device::DeviceError}{DeviceError}.
*/

/*! \fn void DeviceManager::eventTriggered(const Event &event)
    The DeviceManager will emit a \l{Event} described in \a event whenever a Device
    creates one. Normally only \l{nymeaserver::NymeaCore} should connect to this and execute actions
    after checking back with the \{nymeaserver::RulesEngine}. Exceptions might be monitoring interfaces
    or similar, but you should never directly react to this in a \l{DevicePlugin}.
*/

#include "devicemanagerimplementation.h"
#include "translator.h"

#include "loggingcategories.h"
#include "typeutils.h"
#include "nymeasettings.h"

#include "devices/devicepairinginfo.h"
#include "devices/deviceplugin.h"
#include "devices/deviceutils.h"

//#include "unistd.h"

#include "plugintimer.h"

#include <QPluginLoader>
#include <QStaticPlugin>
#include <QtPlugin>
#include <QDebug>
#include <QStringList>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QDir>

/*! Constructs the DeviceManager with the given \a{hardwareManager}, \a locale and \a parent. There should only be one DeviceManager in the system created by \l{nymeaserver::NymeaCore}.
 *  Use \c nymeaserver::NymeaCore::instance()->deviceManager() instead to access the DeviceManager. */
DeviceManagerImplementation::DeviceManagerImplementation(HardwareManager *hardwareManager, const QLocale &locale, QObject *parent) :
    DeviceManager(parent),
    m_hardwareManager(hardwareManager),
    m_locale(locale),
    m_translator(new Translator(this))
{
    qRegisterMetaType<DeviceClassId>();
    qRegisterMetaType<DeviceDescriptor>();

    foreach (const Interface &interface, DeviceUtils::allInterfaces()) {
        m_supportedInterfaces.insert(interface.name(), interface);
    }

    // Give hardware a chance to start up before loading plugins etc.
    QMetaObject::invokeMethod(this, "loadPlugins", Qt::QueuedConnection);
    QMetaObject::invokeMethod(this, "loadConfiguredDevices", Qt::QueuedConnection);
    QMetaObject::invokeMethod(this, "startMonitoringAutoDevices", Qt::QueuedConnection);

    // Make sure this is always emitted after plugins and devices are loaded
    QMetaObject::invokeMethod(this, "onLoaded", Qt::QueuedConnection);
}

/*! Destructor of the DeviceManagerImplementation. Each loaded \l{DevicePlugin} will be deleted. */
DeviceManagerImplementation::~DeviceManagerImplementation()
{
    delete m_translator;

    foreach (Device *device, m_configuredDevices) {
        storeDeviceStates(device);
    }

    foreach (DevicePlugin *plugin, m_devicePlugins) {
        if (plugin->parent() == this) {
            qCDebug(dcDeviceManager()) << "Deleting plugin" << plugin->pluginName();
            delete plugin;
        } else {
            qCDebug(dcDeviceManager()) << "Not deleting plugin" << plugin->pluginName();
        }
    }
}

/*! Returns the list of search direcorys where \l{DevicePlugin} will be searched. */
QStringList DeviceManagerImplementation::pluginSearchDirs()
{
    QStringList searchDirs;
    QByteArray envPath = qgetenv("NYMEA_PLUGINS_PATH");
    if (!envPath.isEmpty()) {
        searchDirs << QString(envPath).split(':');
    }

    foreach (QString libraryPath, QCoreApplication::libraryPaths()) {
        searchDirs << libraryPath.replace("qt5", "nymea");
    }
    searchDirs << QCoreApplication::applicationDirPath() + "/../lib/nymea/plugins";
    searchDirs << QCoreApplication::applicationDirPath() + "/../plugins/";
    searchDirs << QCoreApplication::applicationDirPath() + "/../../../plugins/";
    return searchDirs;
}

/*! Returns the list of json objects containing the metadata of the installed plugins. */
QList<QJsonObject> DeviceManagerImplementation::pluginsMetadata()
{
    QList<QJsonObject> pluginList;
    foreach (const QString &path, pluginSearchDirs()) {
        QDir dir(path);
        foreach (const QString &entry, dir.entryList()) {
            QFileInfo fi;
            if (entry.startsWith("libnymea_deviceplugin") && entry.endsWith(".so")) {
                fi.setFile(path + "/" + entry);
            } else {
                fi.setFile(path + "/" + entry + "/libnymea_deviceplugin" + entry + ".so");
            }
            if (!fi.exists()) {
                continue;
            }
            QPluginLoader loader(fi.absoluteFilePath());
            pluginList.append(loader.metaData().value("MetaData").toObject());
        }
    }
    return pluginList;
}

/*! Register a DevicePlugin class. This can be used to create devices internally from the guh system without having to create a full plugin.
    The \a metaData contains the static plugin configurations. The DeviceManagerImplementation takes ownership of the object \a plugin and will clean it up when exiting. Do not delete the object yourself. */
void DeviceManagerImplementation::registerStaticPlugin(DevicePlugin *plugin, const PluginMetadata &metaData)
{
    if (!metaData.isValid()) {
        qCWarning(dcDeviceManager()) << "Plugin metadata not valid. Not loading static plugin:" << plugin->pluginName();
        return;
    }
    loadPlugin(plugin, metaData);
}

/*! Returns all the \l{DevicePlugin}{DevicePlugins} loaded in the system. */
DevicePlugins DeviceManagerImplementation::plugins() const
{
    return m_devicePlugins.values();
}

/*! Returns a certain \l{DeviceError} and sets the configuration of the plugin with the given \a pluginId
 *  and the given \a pluginConfig. */
Device::DeviceError DeviceManagerImplementation::setPluginConfig(const PluginId &pluginId, const ParamList &pluginConfig)
{
    DevicePlugin *plugin = m_devicePlugins.value(pluginId);
    if (!plugin) {
        qCWarning(dcDeviceManager()) << "Could not set plugin configuration. There is no plugin with id" << pluginId.toString();
        return Device::DeviceErrorPluginNotFound;
    }

    ParamList params = pluginConfig;
    Device::DeviceError verify = DeviceUtils::verifyParams(plugin->configurationDescription(), params);
    if (verify != Device::DeviceErrorNoError)
        return verify;

    Device::DeviceError result = plugin->setConfiguration(params);
    if (result != Device::DeviceErrorNoError)
        return result;

    NymeaSettings settings(NymeaSettings::SettingsRolePlugins);
    settings.beginGroup("PluginConfig");
    settings.beginGroup(plugin->pluginId().toString());

    foreach (const Param &param, pluginConfig) {
        settings.beginGroup(param.paramTypeId().toString());
        settings.setValue("type", static_cast<int>(param.value().type()));
        settings.setValue("value", param.value());
        settings.endGroup();
    }

    settings.endGroup();
    settings.endGroup();
    emit pluginConfigChanged(plugin->pluginId(), pluginConfig);
    return result;
}

/*! Returns all the \l{Vendor}s loaded in the system. */
Vendors DeviceManagerImplementation::supportedVendors() const
{
    return m_supportedVendors.values();
}

/*! Returns the list of all supported \l{Interfaces for DeviceClasses}{interfaces}. */
Interfaces DeviceManagerImplementation::supportedInterfaces() const
{
    return m_supportedInterfaces.values();
}

/*! Returns all the supported \l{DeviceClass}{DeviceClasses} by all \l{DevicePlugin}{DevicePlugins} loaded in the system.
 *  Optionally filtered by \a vendorId. */
DeviceClasses DeviceManagerImplementation::supportedDevices(const VendorId &vendorId) const
{
    if (vendorId.isNull()) {
        return m_supportedDevices.values();
    }
    QList<DeviceClass> ret;
    foreach (const DeviceClass &deviceClass, m_supportedDevices) {
        if (!vendorId.isNull() && deviceClass.vendorId() != vendorId) {
            continue;
        }
        ret.append(deviceClass);
    }
    return ret;
}
/*! Returns a certain \l{DeviceError} and starts the discovering process of the \l{Device} with the given \a deviceClassId
 *  and the given \a params.*/
Device::DeviceError DeviceManagerImplementation::discoverDevices(const DeviceClassId &deviceClassId, const ParamList &params)
{
    qCDebug(dcDeviceManager) << "discover devices" << params;
    // Create a copy of the parameter list because we might modify it (fillig in default values etc)
    ParamList effectiveParams = params;
    DeviceClass deviceClass = findDeviceClass(deviceClassId);
    if (!deviceClass.isValid()) {
        return Device::DeviceErrorDeviceClassNotFound;
    }
    if (!deviceClass.createMethods().testFlag(DeviceClass::CreateMethodDiscovery)) {
        return  Device::DeviceErrorCreationMethodNotSupported;
    }
    Device::DeviceError result = DeviceUtils::verifyParams(deviceClass.discoveryParamTypes(), effectiveParams);
    if (result != Device::DeviceErrorNoError) {
        return result;
    }
    DevicePlugin *plugin = m_devicePlugins.value(deviceClass.pluginId());
    if (!plugin) {
        return Device::DeviceErrorPluginNotFound;
    }
    m_discoveringPlugins.append(plugin);
    Device::DeviceError ret = plugin->discoverDevices(deviceClassId, effectiveParams);
    if (ret != Device::DeviceErrorAsync) {
        m_discoveringPlugins.removeOne(plugin);
    }
    return ret;
}

/*! Add a new configured device for the given \l{DeviceClass}, the given parameters, \a name and \a id.
 *  \a deviceClassId must refer to an existing \{DeviceClass} and \a params must match the parameter description in the \l{DeviceClass}.
 *  Optionally you can supply an id yourself if you must keep track of the added device. If you don't supply it, a new one will
 *  be generated. Only devices with \l{DeviceClass}{CreateMethodUser} can be created using this method.
 *  Returns \l{DeviceError} to inform about the result. */
Device::DeviceError DeviceManagerImplementation::addConfiguredDevice(const DeviceClassId &deviceClassId, const QString &name, const ParamList &params, const DeviceId id)
{
    DeviceClass deviceClass = findDeviceClass(deviceClassId);
    if (!deviceClass.isValid()) {
        return Device::DeviceErrorDeviceClassNotFound;
    }
    if (deviceClass.createMethods().testFlag(DeviceClass::CreateMethodUser)) {
        return addConfiguredDeviceInternal(deviceClassId, name, params, id);
    }
    return Device::DeviceErrorCreationMethodNotSupported;
}

/*! Add a new configured device for the given \l{DeviceClass} the given DeviceDescriptorId and \a deviceId. Only devices with \l{DeviceClass}{CreateMethodDiscovery}
 *  can be created using this method. The \a deviceClassId must refer to an existing \l{DeviceClass} and the \a deviceDescriptorId must refer to an existing DeviceDescriptorId
 *  from the discovery. The \a name parameter should contain the device name. Optionally device params can be passed. By default the descriptor's params as found by the discovery
 *  are used but can be overridden here.
 *
 *  Returns \l{DeviceError} to inform about the result. */
Device::DeviceError DeviceManagerImplementation::addConfiguredDevice(const DeviceClassId &deviceClassId, const QString &name, const DeviceDescriptorId &deviceDescriptorId, const ParamList &params, const DeviceId &deviceId)
{
    DeviceClass deviceClass = findDeviceClass(deviceClassId);
    if (!deviceClass.isValid()) {
        return Device::DeviceErrorDeviceClassNotFound;
    }
    if (!deviceClass.createMethods().testFlag(DeviceClass::CreateMethodDiscovery)) {
        return Device::DeviceErrorCreationMethodNotSupported;
    }

    DeviceDescriptor descriptor = m_discoveredDevices.take(deviceDescriptorId);
    if (!descriptor.isValid()) {
        return Device::DeviceErrorDeviceDescriptorNotFound;
    }

    // Merge params from discovered descriptor and additional overrides provided on API call. User provided params have higher priority than discovery params.
    ParamList finalParams;
    foreach (const ParamType &paramType, deviceClass.paramTypes()) {
        if (params.hasParam(paramType.id())) {
            finalParams.append(Param(paramType.id(), params.paramValue(paramType.id())));
        } else if (descriptor.params().hasParam(paramType.id())) {
            finalParams.append(Param(paramType.id(), descriptor.params().paramValue(paramType.id())));
        }
    }

    return addConfiguredDeviceInternal(deviceClassId, name, finalParams, deviceId, descriptor.parentDeviceId());
}


/*! Edit the \l{ParamList}{Params} of a configured device with the given \a deviceId to the new given \a params.
 *  The given parameter \a fromDiscovery specifies if the new \a params came
 *  from a discovery or if the user set them. If it came from discovery not writable parameters (readOnly) will be changed too.
 *
 *  Returns \l{DeviceError} to inform about the result. */
Device::DeviceError DeviceManagerImplementation::reconfigureDevice(const DeviceId &deviceId, const ParamList &params, bool fromDiscoveryOrAuto)
{
    Device *device = findConfiguredDevice(deviceId);
    if (!device) {
        qCWarning(dcDeviceManager()) << "Cannot reconfigure device. Device with id" << deviceId.toString() << "not found.";
        return Device::DeviceErrorDeviceNotFound;
    }

    ParamList effectiveParams = params;
    DeviceClass deviceClass = findDeviceClass(device->deviceClassId());
    if (deviceClass.id().isNull()) {
        qCWarning(dcDeviceManager()) << "Cannot reconfigure device. DeviceClass for device" << device->name() << deviceId.toString() << "not found.";
        return Device::DeviceErrorDeviceClassNotFound;
    }

    DevicePlugin *plugin = m_devicePlugins.value(deviceClass.pluginId());
    if (!plugin) {
        qCWarning(dcDeviceManager()) << "Cannot reconfigure device. Plugin for DeviceClass" << deviceClass.displayName() << deviceClass.id().toString() << "not found.";
        return Device::DeviceErrorPluginNotFound;
    }

    // if the params are discovered and not set by the user
    if (!fromDiscoveryOrAuto) {
        // check if one of the given params is not editable
        foreach (const ParamType &paramType, deviceClass.paramTypes()) {
            foreach (const Param &param, params) {
                if (paramType.id() == param.paramTypeId()) {
                    if (paramType.readOnly()) {
                        qCWarning(dcDeviceManager()) << "Cannot reconfigure device. Read-only parameters set by user.";
                        return Device::DeviceErrorParameterNotWritable;
                    }
                }
            }
        }
    }

    Device::DeviceError result = DeviceUtils::verifyParams(deviceClass.paramTypes(), effectiveParams, false);
    if (result != Device::DeviceErrorNoError) {
        qCWarning(dcDeviceManager()) << "Cannot reconfigure device. Params failed validation.";
        return result;
    }

    // first remove the device in the plugin
    plugin->deviceRemoved(device);

    // mark setup as incomplete
    device->setSetupComplete(false);

    // set new params
    foreach (const Param &param, effectiveParams) {
        device->setParamValue(param.paramTypeId(), param.value());
    }

    // try to setup the device with the new params
    Device::DeviceSetupStatus status = plugin->setupDevice(device);
    switch (status) {
    case Device::DeviceSetupStatusFailure:
        qCWarning(dcDeviceManager) << "Device reconfiguration failed. Not saving changes of device parameters. Device setup incomplete.";
        return Device::DeviceErrorSetupFailed;
    case Device::DeviceSetupStatusAsync:
        m_asyncDeviceReconfiguration.append(device);
        return Device::DeviceErrorAsync;
    case Device::DeviceSetupStatusSuccess:
        qCDebug(dcDeviceManager) << "Device reconfiguration succeeded.";
        break;
    }

    storeConfiguredDevices();
    postSetupDevice(device);
    device->setupCompleted();
    emit deviceChanged(device);

    return Device::DeviceErrorNoError;
}

/*! Edit the \l{Param}{Params} of a configured device to the \l{Param}{Params} of the DeviceDescriptor with the
 *  given \a deviceId to the given DeviceDescriptorId.
 *  Only devices with \l{DeviceClass}{CreateMethodDiscovery} can be changed using this method.
 *  The \a deviceDescriptorId must refer to an existing DeviceDescriptorId from the discovery.
 *  This method allows to rediscover a device and update it's \l{Param}{Params}.
 *
 *  Returns \l{DeviceError} to inform about the result. */
Device::DeviceError DeviceManagerImplementation::reconfigureDevice(const DeviceId &deviceId, const DeviceDescriptorId &deviceDescriptorId)
{
    Device *device = findConfiguredDevice(deviceId);
    if (!device) {
        return Device::DeviceErrorDeviceNotFound;
    }

    DeviceClass deviceClass = findDeviceClass(device->deviceClassId());
    if (!deviceClass.isValid()) {
        return Device::DeviceErrorDeviceClassNotFound;
    }
    if (!deviceClass.createMethods().testFlag(DeviceClass::CreateMethodDiscovery)) {
        return Device::DeviceErrorCreationMethodNotSupported;
    }

    DeviceDescriptor descriptor = m_discoveredDevices.take(deviceDescriptorId);
    if (!descriptor.isValid()) {
        return Device::DeviceErrorDeviceDescriptorNotFound;
    }

    return reconfigureDevice(deviceId, descriptor.params(), true);
}

/*! Edit the \a name of the \l{Device} with the given \a deviceId.
    Returns \l{Device::DeviceError}{DeviceError} to inform about the result.
*/
Device::DeviceError DeviceManagerImplementation::editDevice(const DeviceId &deviceId, const QString &name)
{
    Device *device = findConfiguredDevice(deviceId);
    if (!device)
        return Device::DeviceErrorDeviceNotFound;

    device->setName(name);
    storeConfiguredDevices();
    emit deviceChanged(device);

    return Device::DeviceErrorNoError;
}

Device::DeviceError DeviceManagerImplementation::setDeviceSettings(const DeviceId &deviceId, const ParamList &settings)
{
    Device *device = findConfiguredDevice(deviceId);
    if (!device) {
        qCWarning(dcDeviceManager()) << "Cannot set device settings. Device" << deviceId.toString() << "not found";
        return Device::DeviceErrorDeviceNotFound;
    }
    ParamList effectiveSettings = settings;
    Device::DeviceError status = DeviceUtils::verifyParams(findDeviceClass(device->deviceClassId()).settingsTypes(), effectiveSettings);
    if (status != Device::DeviceErrorNoError) {
        qCWarning(dcDeviceManager()) << "Error setting device settings for device" << device->name() << device->id().toString();
        return status;
    }
    foreach (const Param &setting, settings) {
        device->setSettingValue(setting.paramTypeId(), setting.value());
    }
    return Device::DeviceErrorNoError;
}

/*! Initiates a pairing with a \l{DeviceClass}{Device} with the given \a pairingTransactionId, \a deviceClassId, \a name and \a params.
 *  Returns \l{Device::DeviceError}{DeviceError} to inform about the result. */
Device::DeviceError DeviceManagerImplementation::pairDevice(const PairingTransactionId &pairingTransactionId, const DeviceClassId &deviceClassId, const QString &name, const ParamList &params)
{
    DeviceClass deviceClass = findDeviceClass(deviceClassId);
    if (deviceClass.id().isNull()) {
        qCWarning(dcDeviceManager) << "Cannot find a device class with id" << deviceClassId;
        return Device::DeviceErrorDeviceClassNotFound;
    }

    Q_UNUSED(pairingTransactionId)
    Q_UNUSED(params)
    Q_UNUSED(name)
    switch (deviceClass.setupMethod()) {
    case DeviceClass::SetupMethodJustAdd:
        qCWarning(dcDeviceManager) << "Cannot setup this device this way. No need to pair this device.";
        return Device::DeviceErrorSetupMethodNotSupported;
    case DeviceClass::SetupMethodDisplayPin:
        qCWarning(dcDeviceManager) << "SetupMethodDisplayPin not implemented yet for this CreateMethod";
        return Device::DeviceErrorSetupFailed;
    case DeviceClass::SetupMethodEnterPin:
        qCWarning(dcDeviceManager) << "SetupMethodEnterPin not implemented yet for this CreateMethod";
        return Device::DeviceErrorSetupFailed;
    case DeviceClass::SetupMethodPushButton:
        qCWarning(dcDeviceManager) << "SetupMethodPushButton not implemented yet for this CreateMethod";
        return Device::DeviceErrorSetupFailed;
    }

    return Device::DeviceErrorNoError;
}

/*! Initiates a pairing with a \l{DeviceClass}{Device} with the given \a pairingTransactionId, \a deviceClassId, \a name and \a deviceDescriptorId.
 *  Returns \l{Device::DeviceError}{DeviceError} to inform about the result. */
Device::DeviceError DeviceManagerImplementation::pairDevice(const PairingTransactionId &pairingTransactionId, const DeviceClassId &deviceClassId, const QString &name, const DeviceDescriptorId &deviceDescriptorId)
{
    DeviceClass deviceClass = findDeviceClass(deviceClassId);
    if (deviceClass.id().isNull()) {
        qCWarning(dcDeviceManager) << "Cannot find a device class with id" << deviceClassId;
        return Device::DeviceErrorDeviceClassNotFound;
    }

    if (deviceClass.setupMethod() == DeviceClass::SetupMethodJustAdd) {
        qCWarning(dcDeviceManager) << "Cannot setup this device this way. No need to pair this device.";
        return Device::DeviceErrorCreationMethodNotSupported;
    }

    if (!m_discoveredDevices.contains(deviceDescriptorId)) {
        qCWarning(dcDeviceManager) << "Cannot find a DeviceDescriptor with ID" << deviceClassId.toString();
        return Device::DeviceErrorDeviceDescriptorNotFound;
    }

    m_pairingsDiscovery.insert(pairingTransactionId, DevicePairingInfo(deviceClassId, name, deviceDescriptorId));

    if (deviceClass.setupMethod() == DeviceClass::SetupMethodDisplayPin) {
        DeviceDescriptor deviceDescriptor = m_discoveredDevices.value(deviceDescriptorId);

        DevicePlugin *plugin = m_devicePlugins.value(m_supportedDevices.value(deviceClassId).pluginId());
        if (!plugin) {
            qCWarning(dcDeviceManager()) << "Can't find a plugin for this device class";
            return Device::DeviceErrorPluginNotFound;
        }

        return plugin->displayPin(pairingTransactionId, deviceDescriptor);
    }

    return Device::DeviceErrorNoError;
}

/*! Confirms the pairing of a \l{Device} with the given \a pairingTransactionId and \a secret.
 *  Returns \l{Device::DeviceError}{DeviceError} to inform about the result. */
Device::DeviceError DeviceManagerImplementation::confirmPairing(const PairingTransactionId &pairingTransactionId, const QString &secret)
{
    if (m_pairingsJustAdd.contains(pairingTransactionId)) {
        qCWarning(dcDeviceManager) << "This SetupMethod is not implemented yet";
        m_pairingsJustAdd.remove(pairingTransactionId);
        return Device::DeviceErrorSetupFailed;
    }

    if (m_pairingsDiscovery.contains(pairingTransactionId)) {
        DevicePairingInfo pairingInfo = m_pairingsDiscovery.value(pairingTransactionId);
        DeviceClassId deviceClassId = pairingInfo.deviceClassId();
        DeviceDescriptor deviceDescriptor = m_discoveredDevices.value(pairingInfo.deviceDescriptorId());

        DevicePlugin *plugin = m_devicePlugins.value(m_supportedDevices.value(deviceClassId).pluginId());

        if (!plugin) {
            qCWarning(dcDeviceManager) << "Can't find a plugin for this device class";
            return Device::DeviceErrorPluginNotFound;
        }

        Device::DeviceSetupStatus status = plugin->confirmPairing(pairingTransactionId, deviceClassId, deviceDescriptor.params(), secret);
        switch (status) {
        case Device::DeviceSetupStatusSuccess:
            m_pairingsDiscovery.remove(pairingTransactionId);
            // TODO: setup the device if the pairing status can be fetched directly
            return Device::DeviceErrorNoError;
        case Device::DeviceSetupStatusFailure:
            m_pairingsDiscovery.remove(pairingTransactionId);
            return Device::DeviceErrorSetupFailed;
        case Device::DeviceSetupStatusAsync:
            return Device::DeviceErrorAsync;
        }
    }

    return Device::DeviceErrorPairingTransactionIdNotFound;
}

/*! This method will only be used from the DeviceManagerImplementation in order to add a \l{Device} with the given \a deviceClassId, \a name, \a params and \ id.
 *  Returns \l{DeviceError} to inform about the result. */
Device::DeviceError DeviceManagerImplementation::addConfiguredDeviceInternal(const DeviceClassId &deviceClassId, const QString &name, const ParamList &params, const DeviceId id, const DeviceId &parentDeviceId)
{
    DeviceClass deviceClass = findDeviceClass(deviceClassId);
    if (deviceClass.id().isNull()) {
        return Device::DeviceErrorDeviceClassNotFound;
    }

    if (deviceClass.setupMethod() != DeviceClass::SetupMethodJustAdd) {
        return Device::DeviceErrorCreationMethodNotSupported;
    }

    foreach(Device *device, m_configuredDevices) {
        if (device->id() == id) {
            return Device::DeviceErrorDuplicateUuid;
        }
    }

    DevicePlugin *plugin = m_devicePlugins.value(deviceClass.pluginId());
    if (!plugin) {
        return Device::DeviceErrorPluginNotFound;
    }

    ParamList effectiveParams = params;
    Device::DeviceError paramsResult = DeviceUtils::verifyParams(deviceClass.paramTypes(), effectiveParams);
    if (paramsResult != Device::DeviceErrorNoError) {
        return paramsResult;
    }

    Device *device = new Device(plugin, deviceClass, id, this);
    device->setParentId(parentDeviceId);
    if (name.isEmpty()) {
        device->setName(deviceClass.name());
    } else {
        device->setName(name);
    }
    device->setParams(effectiveParams);

    ParamList settings;
    DeviceUtils::verifyParams(deviceClass.settingsTypes(), settings);
    qCDebug(dcDeviceManager()) << "Adding device settings" << settings;
    device->setSettings(settings);

    Device::DeviceSetupStatus status = setupDevice(device);
    switch (status) {
    case Device::DeviceSetupStatusFailure:
        qCWarning(dcDeviceManager) << "Device setup failed. Not adding device to system.";
        delete device;
        return Device::DeviceErrorSetupFailed;
    case Device::DeviceSetupStatusAsync:
        return Device::DeviceErrorAsync;
    case Device::DeviceSetupStatusSuccess:
        qCDebug(dcDeviceManager) << "Device setup complete.";
        break;
    }

    m_configuredDevices.insert(device->id(), device);
    storeConfiguredDevices();
    postSetupDevice(device);

    emit deviceAdded(device);

    return Device::DeviceErrorNoError;
}

/*! Removes a \l{Device} with the given \a deviceId from the list of configured \l{Device}{Devices}.
 *  This method also deletes all saved settings of the \l{Device}.
 *  Returns \l{DeviceError} to inform about the result. */
Device::DeviceError DeviceManagerImplementation::removeConfiguredDevice(const DeviceId &deviceId)
{
    Device *device = m_configuredDevices.take(deviceId);
    if (!device) {
        return Device::DeviceErrorDeviceNotFound;
    }
    m_devicePlugins.value(device->pluginId())->deviceRemoved(device);

    device->deleteLater();

    NymeaSettings settings(NymeaSettings::SettingsRoleDevices);
    settings.beginGroup("DeviceConfig");
    settings.beginGroup(deviceId.toString());
    settings.remove("");
    settings.endGroup();

    NymeaSettings stateCache(NymeaSettings::SettingsRoleDeviceStates);
    stateCache.remove(deviceId.toString());

    emit deviceRemoved(deviceId);

    return Device::DeviceErrorNoError;
}

Device::BrowseResult DeviceManagerImplementation::browseDevice(const DeviceId &deviceId, const QString &itemId, const QLocale &locale)
{
    Device::BrowseResult result;

    Device *device = m_configuredDevices.value(deviceId);
    if (!device) {
        qCWarning(dcDeviceManager()) << "Cannot browse device. No such device:" << deviceId.toString();
        result.status = Device::DeviceErrorDeviceNotFound;
        return result;
    }

    if (!device->deviceClass().browsable()) {
        qCWarning(dcDeviceManager()) << "Cannot browse device. DeviceClass" << device->deviceClass().name() << "is not browsable.";
        result.status = Device::DeviceErrorUnsupportedFeature;
        return result;
    }

    result = device->plugin()->browseDevice(device, result, itemId, locale);
    return result;
}

Device::BrowserItemResult DeviceManagerImplementation::browserItemDetails(const DeviceId &deviceId, const QString &itemId, const QLocale &locale)
{
    Device::BrowserItemResult result;

    Device *device = m_configuredDevices.value(deviceId);
    if (!device) {
        qCWarning(dcDeviceManager()) << "Cannot browse device. No such device:" << deviceId.toString();
        result.status = Device::DeviceErrorDeviceNotFound;
        return result;
    }

    if (!device->deviceClass().browsable()) {
        qCWarning(dcDeviceManager()) << "Cannot browse device. DeviceClass" << device->deviceClass().name() << "is not browsable.";
        result.status = Device::DeviceErrorUnsupportedFeature;
        return result;
    }

    result = device->plugin()->browserItem(device, result, itemId, locale);
    if (result.status == Device::DeviceErrorAsync) {
        // Error or Async
        return result;
    }
    if (result.status != Device::DeviceErrorNoError) {
        qCWarning(dcDeviceManager()) << "Browse device failed:" << result.status;
        return result;
    }
    return result;
}

Device::DeviceError DeviceManagerImplementation::executeBrowserItem(const BrowserAction &browserAction)
{
    Device *device = m_configuredDevices.value(browserAction.deviceId());
    if (!device) {
        return Device::DeviceErrorDeviceNotFound;
    }
    if (!device->deviceClass().browsable()) {
        return Device::DeviceErrorUnsupportedFeature;
    }
    return device->plugin()->executeBrowserItem(device, browserAction);
}

Device::DeviceError DeviceManagerImplementation::executeBrowserItemAction(const BrowserItemAction &browserItemAction)
{
    Device *device = m_configuredDevices.value(browserItemAction.deviceId());
    if (!device) {
        return Device::DeviceErrorDeviceNotFound;
    }
    if (!device->deviceClass().browsable()) {
        return Device::DeviceErrorUnsupportedFeature;
    }
    // TODO: check browserItemAction.params with deviceClass

    return device->plugin()->executeBrowserItemAction(device, browserItemAction);
}

QString DeviceManagerImplementation::translate(const PluginId &pluginId, const QString &string, const QLocale &locale)
{
    return m_translator->translate(pluginId, string, locale);
}

/*! Returns the \l{Device} with the given \a id. Null if the id couldn't be found. */
Device *DeviceManagerImplementation::findConfiguredDevice(const DeviceId &id) const
{
    foreach (Device *device, m_configuredDevices) {
        if (device->id() == id) {
            return device;
        }
    }
    return nullptr;
}

/*! Returns all configured \{Device}{Devices} in the system. */
Devices DeviceManagerImplementation::configuredDevices() const
{
    return m_configuredDevices.values();
}

/*! Returns all \l{Device}{Devices} matching the \l{DeviceClass} referred by \a deviceClassId. */
Devices DeviceManagerImplementation::findConfiguredDevices(const DeviceClassId &deviceClassId) const
{
    QList<Device*> ret;
    foreach (Device *device, m_configuredDevices) {
        if (device->deviceClassId() == deviceClassId) {
            ret << device;
        }
    }
    return ret;
}

/*! Returns all \l{Device}{Devices} with the given \a interface. See also \l{Interfaces for DeviceClasses}{interfaces}. */
Devices DeviceManagerImplementation::findConfiguredDevices(const QString &interface) const
{
    QList<Device*> ret;
    foreach (Device *device, m_configuredDevices) {
        DeviceClass dc = m_supportedDevices.value(device->deviceClassId());
        if (dc.interfaces().contains(interface)) {
            ret.append(device);
        }
    }
    return ret;
}

/*! Returns all child \l{Device}{Devices} of the \l{Device} with the given \a id. */
Devices DeviceManagerImplementation::findChildDevices(const DeviceId &id) const
{
    QList<Device *> ret;
    foreach (Device *d, m_configuredDevices) {
        if (d->parentId() == id) {
            ret.append(d);
        }
    }
    return ret;
}

/*! For conveninece, this returns the \l{DeviceClass} with the id given by \a deviceClassId.
 *  Note: The returned \l{DeviceClass} may be invalid. */
DeviceClass DeviceManagerImplementation::findDeviceClass(const DeviceClassId &deviceClassId) const
{
    foreach (const DeviceClass &deviceClass, m_supportedDevices) {
        if (deviceClass.id() == deviceClassId) {
            return deviceClass;
        }
    }
    return DeviceClass();
}

/*! Execute the given \l{Action}.
 *  This will find the \l{Device} \a action refers to the \l{Action}{deviceId()} and
 *  its \l{DevicePlugin}. Then will dispatch the execution to the \l{DevicePlugin}.*/
Device::DeviceError DeviceManagerImplementation::executeAction(const Action &action)
{
    Action finalAction = action;
    foreach (Device *device, m_configuredDevices) {
        if (action.deviceId() == device->id()) {
            // found device

            // Make sure this device has an action type with this id
            DeviceClass deviceClass = findDeviceClass(device->deviceClassId());
            bool found = false;
            foreach (const ActionType &actionType, deviceClass.actionTypes()) {
                if (actionType.id() == action.actionTypeId()) {
                    ParamList finalParams = action.params();
                    Device::DeviceError paramCheck = DeviceUtils::verifyParams(actionType.paramTypes(), finalParams);
                    if (paramCheck != Device::DeviceErrorNoError) {
                        return paramCheck;
                    }
                    finalAction.setParams(finalParams);
                    found = true;
                    continue;
                }
            }
            if (!found) {
                return Device::DeviceErrorActionTypeNotFound;
            }

            return m_devicePlugins.value(device->pluginId())->executeAction(device, finalAction);
        }
    }
    return Device::DeviceErrorDeviceNotFound;
}

/*! Centralized time tick for the NymeaTimer resource. Ticks every second. */
void DeviceManagerImplementation::timeTick()
{

}

void DeviceManagerImplementation::loadPlugins()
{
    foreach (const QString &path, pluginSearchDirs()) {
        QDir dir(path);
        qCDebug(dcDeviceManager) << "Loading plugins from:" << dir.absolutePath();
        foreach (const QString &entry, dir.entryList()) {
            QFileInfo fi;
            if (entry.startsWith("libnymea_deviceplugin") && entry.endsWith(".so")) {
                fi.setFile(path + "/" + entry);
            } else {
                fi.setFile(path + "/" + entry + "/libnymea_deviceplugin" + entry + ".so");
            }

            if (!fi.exists())
                continue;


            QPluginLoader loader;
            loader.setFileName(fi.absoluteFilePath());
            loader.setLoadHints(QLibrary::ResolveAllSymbolsHint);

            qCDebug(dcDeviceManager()) << "Loading plugin from:" << fi.absoluteFilePath();
            if (!loader.load()) {
                qCWarning(dcDeviceManager) << "Could not load plugin data of" << entry << "\n" << loader.errorString();
                continue;
            }

            // Check plugin API version compatibility
            QLibrary lib(fi.absoluteFilePath());
            QFunctionPointer versionFunc = lib.resolve("libnymea_api_version");
            if (!versionFunc) {
                qCWarning(dcDeviceManager()).nospace() << "Unable to resolve version in plugin " << entry << ". Not loading plugin.";
                loader.unload();
                lib.unload();
                continue;

            }
            QString version = reinterpret_cast<QString(*)()>(versionFunc)();
//            QString *version = reinterpret_cast<QString*>(lib.resolve("libnymea_api_version"));
//            if (!version) {
//            }
            lib.unload();
            QStringList parts = version.split('.');
            QStringList coreParts = QString(LIBNYMEA_API_VERSION).split('.');
            if (parts.length() != 3 || parts.at(0).toInt() != coreParts.at(0).toInt() || parts.at(1).toInt() > coreParts.at(1).toInt()) {
                qCWarning(dcDeviceManager()).nospace() << "Libnymea API mismatch for " << entry << ". Core API: " << LIBNYMEA_API_VERSION << ", Plugin API: " << version;
                loader.unload();
                continue;
            }

            PluginMetadata metaData(loader.metaData().value("MetaData").toObject());
            if (!metaData.isValid()) {
                qCWarning(dcDeviceManager()) << "Plugin metadata not valid for" << entry;
                loader.unload();
                continue;
            }

            DevicePlugin *pluginIface = qobject_cast<DevicePlugin *>(loader.instance());
            if (!pluginIface) {
                qCWarning(dcDeviceManager) << "Could not get plugin instance of" << entry;
                loader.unload();
                continue;
            }
            loadPlugin(pluginIface, metaData);
        }
    }
}

void DeviceManagerImplementation::loadPlugin(DevicePlugin *pluginIface, const PluginMetadata &metaData)
{
    pluginIface->setParent(this);
    pluginIface->initPlugin(metaData, this, m_hardwareManager);

    qCDebug(dcDeviceManager) << "**** Loaded plugin" << pluginIface->pluginName();
    foreach (const Vendor &vendor, pluginIface->supportedVendors()) {
        qCDebug(dcDeviceManager) << "* Loaded vendor:" << vendor.name() << vendor.id();
        if (m_supportedVendors.contains(vendor.id()))
            continue;

        m_supportedVendors.insert(vendor.id(), vendor);
    }

    foreach (const DeviceClass &deviceClass, pluginIface->supportedDevices()) {
        if (!m_supportedVendors.contains(deviceClass.vendorId())) {
            qCWarning(dcDeviceManager) << "Vendor not found. Ignoring device. VendorId:" << deviceClass.vendorId() << "DeviceClass:" << deviceClass.name() << deviceClass.id();
            continue;
        }
        m_vendorDeviceMap[deviceClass.vendorId()].append(deviceClass.id());
        m_supportedDevices.insert(deviceClass.id(), deviceClass);
        qCDebug(dcDeviceManager) << "* Loaded device class:" << deviceClass.name();
    }

    NymeaSettings settings(NymeaSettings::SettingsRolePlugins);
    settings.beginGroup("PluginConfig");
    ParamList params;
    if (settings.childGroups().contains(pluginIface->pluginId().toString())) {
        settings.beginGroup(pluginIface->pluginId().toString());

        if (!settings.childGroups().isEmpty()) {
            // Note: since nymea 0.12.2 the param type gets saved too for better data converting
            foreach (const QString &paramTypeIdString, settings.childGroups()) {
                ParamTypeId paramTypeId(paramTypeIdString);
                ParamType paramType = pluginIface->configurationDescription().findById(paramTypeId);
                if (!paramType.isValid()) {
                    qCWarning(dcDeviceManager()) << "Not loading Param for plugin" << pluginIface->pluginName() << "because the ParamType for the saved Param" << ParamTypeId(paramTypeIdString).toString() << "could not be found.";
                    continue;
                }

                QVariant paramValue;
                settings.beginGroup(paramTypeIdString);
                paramValue = settings.value("value", paramType.defaultValue());
                paramValue.convert(settings.value("type").toInt());
                params.append(Param(paramTypeId, paramValue));
                settings.endGroup();
            }
        } else {
            // Note: < nymea 0.12.2
            foreach (const QString &paramTypeIdString, settings.allKeys()) {
                params.append(Param(ParamTypeId(paramTypeIdString), settings.value(paramTypeIdString)));
            }
        }

        settings.endGroup();
    } else if (!pluginIface->configurationDescription().isEmpty()){
        // plugin requires config but none stored. Init with defaults
        foreach (const ParamType &paramType, pluginIface->configurationDescription()) {
            Param param(paramType.id(), paramType.defaultValue());
            params.append(param);
        }
    }
    settings.endGroup();

    if (params.count() > 0) {
        Device::DeviceError status = pluginIface->setConfiguration(params);
        if (status != Device::DeviceErrorNoError) {
            qCWarning(dcDeviceManager) << "Error setting params to plugin. Broken configuration?";
        }
    }

    // Call the init method of the plugin
    pluginIface->init();

    m_devicePlugins.insert(pluginIface->pluginId(), pluginIface);

    connect(pluginIface, &DevicePlugin::emitEvent, this, &DeviceManagerImplementation::onEventTriggered);
    connect(pluginIface, &DevicePlugin::devicesDiscovered, this, &DeviceManagerImplementation::slotDevicesDiscovered, Qt::QueuedConnection);
    connect(pluginIface, &DevicePlugin::deviceSetupFinished, this, &DeviceManagerImplementation::slotDeviceSetupFinished);
    connect(pluginIface, &DevicePlugin::actionExecutionFinished, this, &DeviceManagerImplementation::actionExecutionFinished);
    connect(pluginIface, &DevicePlugin::pairingFinished, this, &DeviceManagerImplementation::slotPairingFinished);
    connect(pluginIface, &DevicePlugin::autoDevicesAppeared, this, &DeviceManagerImplementation::onAutoDevicesAppeared);
    connect(pluginIface, &DevicePlugin::autoDeviceDisappeared, this, &DeviceManagerImplementation::onAutoDeviceDisappeared);
    connect(pluginIface, &DevicePlugin::browseRequestFinished, this, &DeviceManagerImplementation::browseRequestFinished);
    connect(pluginIface, &DevicePlugin::browserItemRequestFinished, this, &DeviceManagerImplementation::browserItemRequestFinished);
    connect(pluginIface, &DevicePlugin::browserItemExecutionFinished, this, &DeviceManagerImplementation::browserItemExecutionFinished);
    connect(pluginIface, &DevicePlugin::browserItemActionExecutionFinished, this, &DeviceManagerImplementation::browserItemActionExecutionFinished);

}


void DeviceManagerImplementation::loadConfiguredDevices()
{
    NymeaSettings settings(NymeaSettings::SettingsRoleDevices);
    settings.beginGroup("DeviceConfig");
    qCDebug(dcDeviceManager) << "Loading devices from" << settings.fileName();
    foreach (const QString &idString, settings.childGroups()) {
        settings.beginGroup(idString);
        QString deviceName = settings.value("devicename").toString();
        DevicePlugin *plugin = m_devicePlugins.value(PluginId(settings.value("pluginid").toString()));
        if (!plugin) {
            qCWarning(dcDeviceManager()) << "Not loading device" << deviceName << idString << "because the plugin for this device could not be found.";
            settings.endGroup(); // DeviceId
            continue;
        }
        DeviceClass deviceClass = findDeviceClass(DeviceClassId(settings.value("deviceClassId").toString()));
        if (!deviceClass.isValid()) {
            qCWarning(dcDeviceManager()) << "Not loading device" << deviceName << idString << "because the device class for this device could not be found.";
            settings.endGroup(); // DeviceId
            continue;
        }

        Device *device = new Device(plugin, deviceClass, DeviceId(idString), this);
        device->m_autoCreated = settings.value("autoCreated").toBool();
        device->setName(deviceName);
        device->setParentId(DeviceId(settings.value("parentid", QUuid()).toString()));


        ParamList params;
        settings.beginGroup("Params");
        if (!settings.childGroups().isEmpty()) {
            foreach (const QString &paramTypeIdString, settings.childGroups()) {
                ParamTypeId paramTypeId(paramTypeIdString);
                ParamType paramType = deviceClass.paramTypes().findById(paramTypeId);
                QVariant defaultValue;
                if (!paramType.isValid()) {
                    // NOTE: We're not skipping unknown parameters to give plugins a chance to still access old values if they change their config and migrate things over.
                    qCWarning(dcDeviceManager()) << "Unknown param" << paramTypeIdString << "for" << device << ". ParamType could not be found in device class.";
                }

                // Note: since nymea 0.12.2
                QVariant paramValue;
                settings.beginGroup(paramTypeIdString);
                paramValue = settings.value("value", paramType.defaultValue());
                paramValue.convert(settings.value("type").toInt());
                params.append(Param(paramTypeId, paramValue));
                settings.endGroup(); // ParamId
            }
        } else {
            foreach (const QString &paramTypeIdString, settings.allKeys()) {
                params.append(Param(ParamTypeId(paramTypeIdString), settings.value(paramTypeIdString)));
            }
        }
        // Make sure all params are around. if they aren't initialize with default values
        foreach (const ParamType &paramType, deviceClass.paramTypes()) {
            if (!params.hasParam(paramType.id())) {
                params.append(Param(paramType.id(), paramType.defaultValue()));
            }
        }
        device->setParams(params);
        settings.endGroup(); // Params

        ParamList deviceSettings;
        settings.beginGroup("Settings");
        if (!settings.childGroups().isEmpty()) {
            foreach (const QString &paramTypeIdString, settings.childGroups()) {
                ParamTypeId paramTypeId(paramTypeIdString);
                ParamType paramType = deviceClass.settingsTypes().findById(paramTypeId);
                if (!paramType.isValid()) {
                    qCWarning(dcDeviceManager()) << "Not loading Setting for device" << device << "because the ParamType for the saved Setting" << ParamTypeId(paramTypeIdString).toString() << "could not be found.";
                    continue;
                }

                // Note: since nymea 0.12.2
                QVariant paramValue;
                settings.beginGroup(paramTypeIdString);
                paramValue = settings.value("value", paramType.defaultValue());
                paramValue.convert(settings.value("type").toInt());
                deviceSettings.append(Param(paramTypeId, paramValue));
                settings.endGroup(); // ParamId
            }
        } else {
            foreach (const QString &paramTypeIdString, settings.allKeys()) {
                params.append(Param(ParamTypeId(paramTypeIdString), settings.value(paramTypeIdString)));
            }
        }
        DeviceUtils::verifyParams(deviceClass.settingsTypes(), deviceSettings);
        device->setSettings(deviceSettings);

        settings.endGroup(); // Settings
        settings.endGroup(); // DeviceId

        // We always add the device to the list in this case. If its in the storedDevices
        // it means that it was working at some point so lets still add it as there might
        // be rules associated with this device.
        m_configuredDevices.insert(device->id(), device);
    }
    settings.endGroup();

    QHash<DeviceId, Device*> setupList = m_configuredDevices;
    while (!setupList.isEmpty()) {
        Device *device = nullptr;
        foreach (Device *d, setupList) {
            if (d->parentId().isNull() || !setupList.contains(d->parentId())) {
                device = d;
                setupList.take(d->id());
                break;
            }
        }
        Q_ASSERT(device != nullptr);

        Device::DeviceSetupStatus status = setupDevice(device);
        if (status == Device::DeviceSetupStatusSuccess)
            postSetupDevice(device);
    }

}

void DeviceManagerImplementation::storeConfiguredDevices()
{
    NymeaSettings settings(NymeaSettings::SettingsRoleDevices);
    settings.beginGroup("DeviceConfig");
    foreach (Device *device, m_configuredDevices) {
        settings.beginGroup(device->id().toString());
        // Note: clean device settings before storing it for clean up
        settings.remove("");
        settings.setValue("autoCreated", device->autoCreated());
        settings.setValue("devicename", device->name());
        settings.setValue("deviceClassId", device->deviceClassId().toString());
        settings.setValue("pluginid", device->pluginId().toString());
        if (!device->parentId().isNull())
            settings.setValue("parentid", device->parentId().toString());

        settings.beginGroup("Params");
        foreach (const Param &param, device->params()) {
            settings.beginGroup(param.paramTypeId().toString());
            settings.setValue("type", static_cast<int>(param.value().type()));
            settings.setValue("value", param.value());
            settings.endGroup(); // ParamTypeId
        }
        settings.endGroup(); // Params

        settings.beginGroup("Settings");
        foreach (const Param &param, device->settings()) {
            settings.beginGroup(param.paramTypeId().toString());
            settings.setValue("type", static_cast<int>(param.value().type()));
            settings.setValue("value", param.value());
            settings.endGroup(); // ParamTypeId
        }
        settings.endGroup(); // Settings


        settings.endGroup(); // DeviceId
    }
    settings.endGroup(); // DeviceConfig
}

void DeviceManagerImplementation::startMonitoringAutoDevices()
{
    foreach (DevicePlugin *plugin, m_devicePlugins) {
        plugin->startMonitoringAutoDevices();
    }
}

void DeviceManagerImplementation::slotDevicesDiscovered(const DeviceClassId &deviceClassId, const QList<DeviceDescriptor> deviceDescriptors)
{
    DevicePlugin *plugin = static_cast<DevicePlugin*>(sender());
    if (!m_discoveringPlugins.contains(plugin)) {
        qWarning(dcDeviceManager()) << "Received a devicesDiscovered signal from" << plugin->pluginName() << "but did not expect it. Ignoring.";
        return;
    }
    m_discoveringPlugins.removeOne(plugin);

    foreach (const DeviceDescriptor &descriptor, deviceDescriptors) {
        m_discoveredDevices.insert(descriptor.id(), descriptor);
    }
    emit devicesDiscovered(deviceClassId, deviceDescriptors);
}

void DeviceManagerImplementation::slotDeviceSetupFinished(Device *device, Device::DeviceSetupStatus status)
{
    Q_ASSERT_X(device, "DeviceManager", "Device must be a valid pointer.");
    if (!device) {
        qCWarning(dcDeviceManager) << "Received deviceSetupFinished for an invalid device... ignoring...";
        return;
    }

    if (device->setupComplete()) {
        qCWarning(dcDeviceManager) << "Received a deviceSetupFinished event, but this Device has been set up before... ignoring...";
        return;
    }

    Q_ASSERT_X(status != Device::DeviceSetupStatusAsync, "DeviceManager", "Bad plugin implementation. You should not emit deviceSetupFinished with status DeviceSetupStatusAsync.");
    if (status == Device::DeviceSetupStatusAsync) {
        qCWarning(dcDeviceManager) << "Bad plugin implementation. Received a deviceSetupFinished event with status Async... ignoring...";
        return;
    }

    if (status == Device::DeviceSetupStatusFailure) {
        if (m_configuredDevices.contains(device->id())) {
            if (m_asyncDeviceReconfiguration.contains(device)) {
                m_asyncDeviceReconfiguration.removeAll(device);
                qCWarning(dcDeviceManager) << QString("Error in device setup after reconfiguration. Device %1 (%2) will not be functional.").arg(device->name()).arg(device->id().toString());

                storeConfiguredDevices();

                // TODO: recover old params.??

                emit deviceChanged(device);
                emit deviceReconfigurationFinished(device, Device::DeviceErrorSetupFailed);
            }
            qCWarning(dcDeviceManager) << QString("Error in device setup. Device %1 (%2) will not be functional.").arg(device->name()).arg(device->id().toString());
            emit deviceSetupFinished(device, Device::DeviceErrorSetupFailed);
            return;
        } else {
            qCWarning(dcDeviceManager) << QString("Error in device setup. Device %1 (%2) will not be added to the configured devices.").arg(device->name()).arg(device->id().toString());
            emit deviceSetupFinished(device, Device::DeviceErrorSetupFailed);
            return;
        }
    }

    // A device might be in here already if loaded from storedDevices. If it's not in the configuredDevices,
    // lets add it now.
    if (!m_configuredDevices.contains(device->id())) {
        m_configuredDevices.insert(device->id(), device);
        emit deviceAdded(device);
        storeConfiguredDevices();
    }

    // if this is a async device edit result
    if (m_asyncDeviceReconfiguration.contains(device)) {
        m_asyncDeviceReconfiguration.removeAll(device);
        storeConfiguredDevices();
        device->setupCompleted();
        emit deviceChanged(device);
        emit deviceReconfigurationFinished(device, Device::DeviceErrorNoError);
        return;
    }

    connect(device, &Device::stateValueChanged, this, &DeviceManagerImplementation::slotDeviceStateValueChanged);
    connect(device, &Device::settingChanged, this, &DeviceManagerImplementation::slotDeviceSettingChanged);

    device->setupCompleted();
    emit deviceSetupFinished(device, Device::DeviceErrorNoError);
}

void DeviceManagerImplementation::slotPairingFinished(const PairingTransactionId &pairingTransactionId, Device::DeviceSetupStatus status)
{
    if (!m_pairingsJustAdd.contains(pairingTransactionId) && !m_pairingsDiscovery.contains(pairingTransactionId)) {
        DevicePlugin *plugin = dynamic_cast<DevicePlugin*>(sender());
        if (plugin) {
            qCWarning(dcDeviceManager) << "Received a pairing finished without waiting for it from plugin:" << plugin->metaObject()->className();
        } else {
            qCWarning(dcDeviceManager) << "Received a pairing finished without waiting for it.";
        }
        return;
    }

    DeviceClassId deviceClassId;
    ParamList params;
    QString deviceName;

    DeviceId deviceId;

    // Do this before checking status to make sure we clean up our hashes properly
    if (m_pairingsJustAdd.contains(pairingTransactionId)) {
        DevicePairingInfo pairingInfo = m_pairingsJustAdd.take(pairingTransactionId);

        deviceClassId = pairingInfo.deviceClassId();
        params = pairingInfo.params();
        deviceName = pairingInfo.deviceName();
    }

    if (m_pairingsDiscovery.contains(pairingTransactionId)) {
        DevicePairingInfo pairingInfo = m_pairingsDiscovery.take(pairingTransactionId);
        DeviceDescriptor descriptor = m_discoveredDevices.take(pairingInfo.deviceDescriptorId());

        deviceClassId = pairingInfo.deviceClassId();
        deviceName = pairingInfo.deviceName();
        params = descriptor.params();
        deviceId = descriptor.deviceId();
    }

    if (status != Device::DeviceSetupStatusSuccess) {
        emit pairingFinished(pairingTransactionId, Device::DeviceErrorSetupFailed);
        return;
    }

    DeviceClass deviceClass = findDeviceClass(deviceClassId);
    DevicePlugin *plugin = m_devicePlugins.value(deviceClass.pluginId());
    if (!plugin) {
        qCWarning(dcDeviceManager) << "Cannot find a plugin for this device class!";
        emit pairingFinished(pairingTransactionId, Device::DeviceErrorPluginNotFound, deviceClass.pluginId().toString());
        return;
    }

    // If we already have a deviceId, we're reconfiguring an existing device
    bool addNewDevice = deviceId.isNull();

    if (!addNewDevice && !m_configuredDevices.contains(deviceId)) {
        qCWarning(dcDeviceManager) << "The device to be reconfigured has disappeared!";
        emit pairingFinished(pairingTransactionId, Device::DeviceErrorDeviceNotFound, deviceId);
        return;
    }

    // Ok... pairing went fine... Let consumers know about it and inform them about the ongoing setup with a deviceId.
    Device *device = nullptr;

    if (addNewDevice) {
        deviceId = DeviceId::createDeviceId();
        qCDebug(dcDeviceManager()) << "Creating new device with ID" << deviceId;
        device = new Device(plugin, deviceClass, deviceId, this);
        if (deviceName.isEmpty()) {
            device->setName(deviceClass.name());
        } else {
            device->setName(deviceName);
        }
    } else {
        device = m_configuredDevices.value(deviceId);
        qCDebug(dcDeviceManager()) << "Reconfiguring device" << device;
    }
    emit pairingFinished(pairingTransactionId, Device::DeviceErrorNoError, deviceId);

    device->setParams(params);
    ParamList settings;
    // Use verifyParams to populate it with defaults
    DeviceUtils::verifyParams(deviceClass.settingsTypes(), settings);
    device->setSettings(settings);

    Device::DeviceSetupStatus setupStatus = setupDevice(device);
    switch (setupStatus) {
    case Device::DeviceSetupStatusFailure:
        qCWarning(dcDeviceManager) << "Device setup failed. Not adding device to system.";
        emit deviceSetupFinished(device, Device::DeviceErrorSetupFailed);
        delete device;
        break;
    case Device::DeviceSetupStatusAsync:
        return;
    case Device::DeviceSetupStatusSuccess:
        qCDebug(dcDeviceManager) << "Device setup complete.";
        break;
    }

    if (addNewDevice) {
        qCDebug(dcDeviceManager()) << "Device added:" << device;
        m_configuredDevices.insert(device->id(), device);
        emit deviceAdded(device);
    } else {
        emit deviceChanged(device);
    }

    storeConfiguredDevices();
    emit deviceSetupFinished(device, Device::DeviceErrorNoError);
    postSetupDevice(device);
}

void DeviceManagerImplementation::onAutoDevicesAppeared(const DeviceClassId &deviceClassId, const QList<DeviceDescriptor> &deviceDescriptors)
{
    DeviceClass deviceClass = findDeviceClass(deviceClassId);
    if (!deviceClass.isValid()) {
        qCWarning(dcDeviceManager()) << "Ignoring auto device appeared for an unknown DeviceClass" << deviceClassId;
        return;
    }

    DevicePlugin *plugin = m_devicePlugins.value(deviceClass.pluginId());
    if (!plugin) {
        return;
    }

    foreach (const DeviceDescriptor &deviceDescriptor, deviceDescriptors) {
        if (!deviceDescriptor.parentDeviceId().isNull() && !m_configuredDevices.contains(deviceDescriptor.parentDeviceId())) {
            qCWarning(dcDeviceManager()) << "Invalid parent device id. Not adding device to the system.";
            continue;
        }

        Device *device = nullptr;

        // If the appreaed auto device holds a valid device id, do a reconfiguration for this device
        if (!deviceDescriptor.deviceId().isNull()) {
            device = findConfiguredDevice(deviceDescriptor.deviceId());
            if (!device) {
                qCWarning(dcDeviceManager()) << "Could not find device for auto device descriptor" << deviceDescriptor.deviceId();
                continue;
            }
            qCDebug(dcDeviceManager()) << "Start reconfiguring auto device" << device;
            reconfigureDevice(deviceDescriptor.deviceId(), deviceDescriptor.params(), true);
            continue;
        }

        device = new Device(plugin, deviceClass, this);
        device->m_autoCreated = true;
        device->setName(deviceDescriptor.title());
        device->setParams(deviceDescriptor.params());
        ParamList settings;
        DeviceUtils::verifyParams(deviceClass.settingsTypes(), settings);
        device->setSettings(settings);
        device->setParentId(deviceDescriptor.parentDeviceId());

        Device::DeviceSetupStatus setupStatus = setupDevice(device);
        switch (setupStatus) {
        case Device::DeviceSetupStatusFailure:
            qCWarning(dcDeviceManager) << "Device setup failed. Not adding device to system.";
            emit deviceSetupFinished(device, Device::DeviceErrorSetupFailed);
            delete device;
            break;
        case Device::DeviceSetupStatusAsync:
            break;
        case Device::DeviceSetupStatusSuccess:
            qCDebug(dcDeviceManager) << "Device setup complete.";
            m_configuredDevices.insert(device->id(), device);
            storeConfiguredDevices();
            emit deviceSetupFinished(device, Device::DeviceErrorNoError);
            emit deviceAdded(device);
            postSetupDevice(device);
            break;
        }
    }
}

void DeviceManagerImplementation::onAutoDeviceDisappeared(const DeviceId &deviceId)
{
    DevicePlugin *plugin = static_cast<DevicePlugin*>(sender());
    Device *device = m_configuredDevices.value(deviceId);

    if (!device) {
        qWarning(dcDeviceManager) << "Received an autoDeviceDisappeared signal but don't know this device:" << deviceId;
        return;
    }

    DeviceClass deviceClass = m_supportedDevices.value(device->deviceClassId());

    if (deviceClass.pluginId() != plugin->pluginId()) {
        qWarning(dcDeviceManager) << "Received a autoDeviceDisappeared signal but emitting plugin does not own the device";
        return;
    }

    if (!device->autoCreated()) {
        qWarning(dcDeviceManager) << "Received an autoDeviceDisappeared signal but device creationMethod is not CreateMothodAuto";
        return;
    }

    emit deviceDisappeared(deviceId);
}

void DeviceManagerImplementation::onLoaded()
{
    qCDebug(dcDeviceManager()) << "Done loading plugins and devices.";
    emit loaded();

    // schedule some housekeeping...
    QTimer::singleShot(0, this, SLOT(cleanupDeviceStateCache()));
}

void DeviceManagerImplementation::cleanupDeviceStateCache()
{
    NymeaSettings settings(NymeaSettings::SettingsRoleDeviceStates);
    foreach (const QString &entry, settings.childGroups()) {
        DeviceId deviceId(entry);
        if (!m_configuredDevices.contains(deviceId)) {
            qCDebug(dcDeviceManager()) << "Device ID" << deviceId << "not found in configured devices. Cleaning up stale device state cache.";
            settings.remove(entry);
        }
    }
}

void DeviceManagerImplementation::onEventTriggered(const Event &event)
{
    // Doing some sanity checks here...
    Device *device = m_configuredDevices.value(event.deviceId());
    if (!device) {
        qCWarning(dcDeviceManager()) << "Invalid device id in emitted event. Not forwarding event.";
        return;
    }
    EventType eventType = device->deviceClass().eventTypes().findById(event.eventTypeId());
    if (!eventType.isValid()) {
        qCWarning(dcDeviceManager()) << "The given device does not have an event type of id " + event.eventTypeId().toString() + ". Not forwarding event.";
        return;
    }
    // All good, forward the event
    emit eventTriggered(event);
}

void DeviceManagerImplementation::slotDeviceStateValueChanged(const StateTypeId &stateTypeId, const QVariant &value)
{
    Device *device = qobject_cast<Device*>(sender());
    if (!device) {
        return;
    }
    emit deviceStateChanged(device, stateTypeId, value);

    Param valueParam(ParamTypeId(stateTypeId.toString()), value);
    Event event(EventTypeId(stateTypeId.toString()), device->id(), ParamList() << valueParam, true);
    emit eventTriggered(event);
}

void DeviceManagerImplementation::slotDeviceSettingChanged(const ParamTypeId &paramTypeId, const QVariant &value)
{
    Device *device = qobject_cast<Device*>(sender());
    if (!device) {
        return;
    }
    storeConfiguredDevices();
    emit deviceSettingChanged(device->id(), paramTypeId, value);
}

Device::DeviceSetupStatus DeviceManagerImplementation::setupDevice(Device *device)
{
    DeviceClass deviceClass = findDeviceClass(device->deviceClassId());
    DevicePlugin *plugin = m_devicePlugins.value(deviceClass.pluginId());

    if (!plugin) {
        qCWarning(dcDeviceManager) << "Can't find a plugin for this device" << device->id();
        return Device::DeviceSetupStatusFailure;
    }

    QList<State> states;
    foreach (const StateType &stateType, deviceClass.stateTypes()) {
        State state(stateType.id(), device->id());
        states.append(state);
    }
    device->setStates(states);
    loadDeviceStates(device);

    Device::DeviceSetupStatus status = plugin->setupDevice(device);
    if (status != Device::DeviceSetupStatusSuccess) {
        return status;
    }

    connect(device, &Device::stateValueChanged, this, &DeviceManagerImplementation::slotDeviceStateValueChanged);
    connect(device, &Device::settingChanged, this, &DeviceManagerImplementation::slotDeviceSettingChanged);

    device->setupCompleted();
    return status;
}

void DeviceManagerImplementation::postSetupDevice(Device *device)
{
    DeviceClass deviceClass = findDeviceClass(device->deviceClassId());
    DevicePlugin *plugin = m_devicePlugins.value(deviceClass.pluginId());

    plugin->postSetupDevice(device);
}

void DeviceManagerImplementation::loadDeviceStates(Device *device)
{
    NymeaSettings settings(NymeaSettings::SettingsRoleDeviceStates);
    settings.beginGroup(device->id().toString());
    DeviceClass deviceClass = m_supportedDevices.value(device->deviceClassId());
    foreach (const StateType &stateType, deviceClass.stateTypes()) {
        if (stateType.cached()) {
            QVariant value;
            // First try to load new style
            if (settings.childGroups().contains(stateType.id().toString())) {
                settings.beginGroup(stateType.id().toString());
                value = settings.value("value", stateType.defaultValue());
                value.convert(settings.value("type").toInt());
                settings.endGroup();
            } else { // Try to fall back to the pre 0.9.0 way of storing states
                value = settings.value(stateType.id().toString(), stateType.defaultValue());
            }
            device->setStateValue(stateType.id(), value);
        } else {
            device->setStateValue(stateType.id(), stateType.defaultValue());
        }
    }
    settings.endGroup();
}

void DeviceManagerImplementation::storeDeviceStates(Device *device)
{
    NymeaSettings settings(NymeaSettings::SettingsRoleDeviceStates);
    settings.beginGroup(device->id().toString());
    DeviceClass deviceClass = m_supportedDevices.value(device->deviceClassId());
    foreach (const StateType &stateType, deviceClass.stateTypes()) {
        if (stateType.cached()) {
            settings.beginGroup(stateType.id().toString());
            settings.setValue("type", static_cast<int>(device->stateValue(stateType.id()).type()));
            settings.setValue("value", device->stateValue(stateType.id()));
            settings.endGroup();
        }
    }
    settings.endGroup();
}

