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
    \class DeviceManager
    \brief The main entry point when interacting with \l{Device}{Devices}

    \ingroup devices
    \inmodule libnymea

    The DeviceManager hold  s all information about supported and configured Devices in the system.

    It is also responsible for loading Plugins and managing common hardware resources between
    \l{DevicePlugin}{device plugins}.
*/


/*! \enum DeviceManager::DeviceError

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

/*! \enum DeviceManager::DeviceSetupStatus

    This enum type specifies the setup status of a \l{Device}.

    \value DeviceSetupStatusSuccess
        No Error. Everything went fine.
    \value DeviceSetupStatusFailure
        Something went wrong during the setup.
    \value DeviceSetupStatusAsync
        The status of the \l{Device} setup will be emitted asynchronous.
*/

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
    \l{DeviceManager::DeviceError}{DeviceError} that occurred.
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
    \l{DeviceManager::DeviceError}{DeviceError} that occurred.
*/

/*! \fn void DeviceManager::devicesDiscovered(const DeviceClassId &deviceClassId, const QList<DeviceDescriptor> &devices);
    This signal is emitted when the discovery of a \a deviceClassId is finished. The \a devices parameter describes the
    list of \l{DeviceDescriptor}{DeviceDescriptors} of all discovered \l{Device}{Devices}.
    \sa discoverDevices()
*/

/*! \fn void DeviceManager::actionExecutionFinished(const ActionId &actionId, DeviceError status);
    The DeviceManager will emit a this signal when the \l{Action} with the given \a actionId is finished.
    The \a status of the \l{Action} execution will be described as \l{DeviceManager::DeviceError}{DeviceError}.
*/

/*! \fn void DeviceManager::pairingFinished(const PairingTransactionId &pairingTransactionId, DeviceError status, const DeviceId &deviceId = DeviceId());
    The DeviceManager will emit a this signal when the pairing of a \l{Device} with the \a deviceId and \a pairingTransactionId is finished.
    The \a status of the pairing will be described as \l{DeviceManager::DeviceError}{DeviceError}.
*/

/*! \fn void DeviceManager::eventTriggered(const Event &event)
    The DeviceManager will emit a \l{Event} described in \a event whenever a Device
    creates one. Normally only \l{nymeaserver::NymeaCore} should connect to this and execute actions
    after checking back with the \{nymeaserver::RulesEngine}. Exceptions might be monitoring interfaces
    or similar, but you should never directly react to this in a \l{DevicePlugin}.
*/

#include "devicemanager.h"
#include "loggingcategories.h"

#include "plugin/devicepairinginfo.h"
#include "plugin/deviceplugin.h"
#include "typeutils.h"
#include "nymeasettings.h"
#include "unistd.h"

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
DeviceManager::DeviceManager(HardwareManager *hardwareManager, const QLocale &locale, QObject *parent) :
    QObject(parent),
    m_hardwareManager(hardwareManager),
    m_locale(locale)
{
    qRegisterMetaType<DeviceClassId>();
    qRegisterMetaType<DeviceDescriptor>();

    foreach (const Interface &interface, DevicePlugin::allInterfaces()) {
        m_supportedInterfaces.insert(interface.name(), interface);
    }

    // Give hardware a chance to start up before loading plugins etc.
    QMetaObject::invokeMethod(this, "loadPlugins", Qt::QueuedConnection);
    QMetaObject::invokeMethod(this, "loadConfiguredDevices", Qt::QueuedConnection);
    QMetaObject::invokeMethod(this, "startMonitoringAutoDevices", Qt::QueuedConnection);

    // Make sure this is always emitted after plugins and devices are loaded
    QMetaObject::invokeMethod(this, "onLoaded", Qt::QueuedConnection);
}

/*! Destructor of the DeviceManager. Each loaded \l{DevicePlugin} will be deleted. */
DeviceManager::~DeviceManager()
{
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
QStringList DeviceManager::pluginSearchDirs()
{
    QStringList searchDirs;
    QByteArray envPath = qgetenv("NYMEA_PLUGINS_PATH");
    if (!envPath.isEmpty()) {
        searchDirs << QString(envPath).split(':');
    }
    searchDirs << QCoreApplication::applicationDirPath() + "/../lib/nymea/plugins";
    searchDirs << QCoreApplication::applicationDirPath() + "/../plugins/";
    searchDirs << QCoreApplication::applicationDirPath() + "/../../../plugins/";
    searchDirs << QString("%1").arg(NYMEA_PLUGINS_PATH);
    QString snapDir = QString::fromUtf8(qgetenv("SNAP"));
    if (!snapDir.isEmpty()) {
        searchDirs << QString("%1%2").arg(snapDir).arg(NYMEA_PLUGINS_PATH);
    }
    return searchDirs;
}

/*! Returns the list of json objects containing the metadata of the installed plugins. */
QList<QJsonObject> DeviceManager::pluginsMetadata()
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
    The \a metaData contains the static plugin configurations. The DeviceManager takes ownership of the object \a plugin and will clean it up when exiting. Do not delete the object yourself. */
void DeviceManager::registerStaticPlugin(DevicePlugin *plugin, const QJsonObject &metaData)
{
    if (!verifyPluginMetadata(metaData)) {
        qCWarning(dcDeviceManager()) << "Failed to verify plugin metadata. Not loading static plugin:" << plugin->pluginName();
        return;
    }
    plugin->setParent(this);
    plugin->setMetaData(metaData);
    loadPlugin(plugin);
}

/*! Set the \a locale of all plugins and reload the translated strings. */
void DeviceManager::setLocale(const QLocale &locale)
{
    qCDebug(dcDeviceManager()) << "Setting locale:" << locale;
    m_locale = locale;
    foreach (DevicePlugin *plugin, m_devicePlugins.values()) {
        QCoreApplication::removeTranslator(plugin->translator());
        plugin->setLocale(m_locale);
        QCoreApplication::installTranslator(plugin->translator());
        plugin->loadMetaData();
    }

    // Reload all plugin meta data

    m_supportedVendors.clear();
    m_supportedDevices.clear();

    foreach (DevicePlugin *plugin, m_devicePlugins.values()) {

        foreach (const Vendor &vendor, plugin->supportedVendors()) {
            if (m_supportedVendors.contains(vendor.id()))
                continue;

            m_supportedVendors.insert(vendor.id(), vendor);
        }

        foreach (const DeviceClass &deviceClass, plugin->supportedDevices()) {
            if (!m_supportedVendors.contains(deviceClass.vendorId())) {
                qCWarning(dcDeviceManager) << "Vendor not found. Ignoring device. VendorId:" << deviceClass.vendorId() << "DeviceClass:" << deviceClass.name() << deviceClass.id();
                continue;
            }
            m_supportedDevices.insert(deviceClass.id(), deviceClass);
        }
    }

    emit languageUpdated();
}

/*! Returns the pointer to the \l{HardwareManager} of the system.

  \sa HardwareManager
*/
HardwareManager *DeviceManager::hardwareManager() const
{
    return m_hardwareManager;
}

/*! Returns all the \l{DevicePlugin}{DevicePlugins} loaded in the system. */
QList<DevicePlugin *> DeviceManager::plugins() const
{
    return m_devicePlugins.values();
}

/*! Returns the \l{DevicePlugin} with the given \a id. Null if the id couldn't be found. */
DevicePlugin *DeviceManager::plugin(const PluginId &id) const
{
    return m_devicePlugins.value(id);
}

/*! Returns a certain \l{DeviceError} and sets the configuration of the plugin with the given \a pluginId
 *  and the given \a pluginConfig. */
DeviceManager::DeviceError DeviceManager::setPluginConfig(const PluginId &pluginId, const ParamList &pluginConfig)
{
    DevicePlugin *plugin = m_devicePlugins.value(pluginId);
    if (!plugin) {
        qCWarning(dcDeviceManager()) << "Could not set plugin configuration. There is no plugin with id" << pluginId.toString();
        return DeviceErrorPluginNotFound;
    }

    ParamList params = pluginConfig;
    DeviceError verify = verifyParams(plugin->configurationDescription(), params);
    if (verify != DeviceErrorNoError)
        return verify;

    DeviceError result = plugin->setConfiguration(params);
    if (result != DeviceErrorNoError)
        return result;

    NymeaSettings settings(NymeaSettings::SettingsRolePlugins);
    settings.beginGroup("PluginConfig");
    settings.beginGroup(plugin->pluginId().toString());
    foreach (const Param &param, params) {
        settings.setValue(param.paramTypeId().toString(), param.value());
    }
    settings.endGroup();
    settings.endGroup();
    emit pluginConfigChanged(plugin->pluginId(), pluginConfig);
    return result;
}

/*! Returns all the \l{Vendor}s loaded in the system. */
QList<Vendor> DeviceManager::supportedVendors() const
{
    return m_supportedVendors.values();
}

/*! Returns the list of all supported \l{Interfaces for DeviceClasses}{interfaces}. */
Interfaces DeviceManager::supportedInterfaces() const
{
    return m_supportedInterfaces.values();
}

/*! Returns the interface with the given \a name. If the interface can't be found it will return an invalid interface. */
Interface DeviceManager::findInterface(const QString &name)
{
    return m_supportedInterfaces.value(name);
}

/*! Returns all the supported \l{DeviceClass}{DeviceClasses} by all \l{DevicePlugin}{DevicePlugins} loaded in the system.
 *  Optionally filtered by \a vendorId. */
QList<DeviceClass> DeviceManager::supportedDevices(const VendorId &vendorId) const
{
    QList<DeviceClass> ret;
    if (vendorId.isNull()) {
        ret = m_supportedDevices.values();
    } else {
        foreach (const DeviceClassId &deviceClassId, m_vendorDeviceMap.value(vendorId)) {
            ret.append(m_supportedDevices.value(deviceClassId));
        }
    }
    return ret;
}
/*! Returns a certain \l{DeviceError} and starts the discovering process of the \l{Device} with the given \a deviceClassId
 *  and the given \a params.*/
DeviceManager::DeviceError DeviceManager::discoverDevices(const DeviceClassId &deviceClassId, const ParamList &params)
{
    qCDebug(dcDeviceManager) << "discover devices" << params;
    // Create a copy of the parameter list because we might modify it (fillig in default values etc)
    ParamList effectiveParams = params;
    DeviceClass deviceClass = findDeviceClass(deviceClassId);
    if (!deviceClass.isValid()) {
        return DeviceErrorDeviceClassNotFound;
    }
    if (!deviceClass.createMethods().testFlag(DeviceClass::CreateMethodDiscovery)) {
        return  DeviceErrorCreationMethodNotSupported;
    }
    DeviceError result = verifyParams(deviceClass.discoveryParamTypes(), effectiveParams);
    if (result != DeviceErrorNoError) {
        return result;
    }
    DevicePlugin *plugin = m_devicePlugins.value(deviceClass.pluginId());
    if (!plugin) {
        return DeviceErrorPluginNotFound;
    }
    m_discoveringPlugins.append(plugin);
    DeviceError ret = plugin->discoverDevices(deviceClassId, effectiveParams);
    if (ret != DeviceErrorAsync) {
        m_discoveringPlugins.removeOne(plugin);
    }
    return ret;
}

/*! Add a new configured device for the given \l{DeviceClass}, the given parameters, \a name and \a id.
 *  \a deviceClassId must refer to an existing \{DeviceClass} and \a params must match the parameter description in the \l{DeviceClass}.
 *  Optionally you can supply an id yourself if you must keep track of the added device. If you don't supply it, a new one will
 *  be generated. Only devices with \l{DeviceClass}{CreateMethodUser} can be created using this method.
 *  Returns \l{DeviceError} to inform about the result. */
DeviceManager::DeviceError DeviceManager::addConfiguredDevice(const DeviceClassId &deviceClassId, const QString &name, const ParamList &params, const DeviceId id)
{
    DeviceClass deviceClass = findDeviceClass(deviceClassId);
    if (!deviceClass.isValid()) {
        return DeviceErrorDeviceClassNotFound;
    }
    if (deviceClass.createMethods().testFlag(DeviceClass::CreateMethodUser)) {
        return addConfiguredDeviceInternal(deviceClassId, name, params, id);
    }
    return DeviceErrorCreationMethodNotSupported;
}

/*! Add a new configured device for the given \l{DeviceClass} the given DeviceDescriptorId and \a deviceId. Only devices with \l{DeviceClass}{CreateMethodDiscovery}
 *  can be created using this method. The \a deviceClassId must refer to an existing \l{DeviceClass} and the \a deviceDescriptorId must refer to an existing DeviceDescriptorId
 *  from the discovery. The \a name parameter should contain the device name. Optionally device params can be passed. By default the descriptor's params as found by the discovery
 *  are used but can be overridden here.
 *
 *  Returns \l{DeviceError} to inform about the result. */
DeviceManager::DeviceError DeviceManager::addConfiguredDevice(const DeviceClassId &deviceClassId, const QString &name, const DeviceDescriptorId &deviceDescriptorId, const ParamList &params, const DeviceId &deviceId)
{
    DeviceClass deviceClass = findDeviceClass(deviceClassId);
    if (!deviceClass.isValid()) {
        return DeviceErrorDeviceClassNotFound;
    }
    if (!deviceClass.createMethods().testFlag(DeviceClass::CreateMethodDiscovery)) {
        return DeviceErrorCreationMethodNotSupported;
    }

    DeviceDescriptor descriptor = m_discoveredDevices.take(deviceDescriptorId);
    if (!descriptor.isValid()) {
        return DeviceErrorDeviceDescriptorNotFound;
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

    return addConfiguredDeviceInternal(deviceClassId, name, finalParams, deviceId);
}


/*! Edit the \l{ParamList}{Params} of a configured device with the given \a deviceId to the new given \a params.
 *  The given parameter \a fromDiscovery specifies if the new \a params came
 *  from a discovery or if the user set them. If it came from discovery not writable parameters (readOnly) will be changed too.
 *
 *  Returns \l{DeviceError} to inform about the result. */
DeviceManager::DeviceError DeviceManager::reconfigureDevice(const DeviceId &deviceId, const ParamList &params, bool fromDiscovery)
{
    Device *device = findConfiguredDevice(deviceId);
    if (!device) {
        return DeviceErrorDeviceNotFound;
    }

    ParamList effectiveParams = params;
    DeviceClass deviceClass = findDeviceClass(device->deviceClassId());
    if (deviceClass.id().isNull()) {
        return DeviceErrorDeviceClassNotFound;
    }

    DevicePlugin *plugin = m_devicePlugins.value(deviceClass.pluginId());
    if (!plugin) {
        return DeviceErrorPluginNotFound;
    }

    // if the params are discovered and not set by the user
    if (!fromDiscovery) {
        // check if one of the given params is not editable
        foreach (const ParamType &paramType, deviceClass.paramTypes()) {
            foreach (const Param &param, params) {
                if (paramType.id() == param.paramTypeId()) {
                    if (paramType.readOnly())
                        return DeviceErrorParameterNotWritable;
                }
            }
        }
    }

    DeviceError result = verifyParams(deviceClass.paramTypes(), effectiveParams, false);
    if (result != DeviceErrorNoError) {
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
    DeviceSetupStatus status = plugin->setupDevice(device);
    switch (status) {
    case DeviceSetupStatusFailure:
        qCWarning(dcDeviceManager) << "Device reconfiguration failed. Not saving changes of device parameters. Device setup incomplete.";
        return DeviceErrorSetupFailed;
    case DeviceSetupStatusAsync:
        m_asyncDeviceReconfiguration.append(device);
        return DeviceErrorAsync;
    case DeviceSetupStatusSuccess:
        qCDebug(dcDeviceManager) << "Device reconfiguration succeeded.";
        break;
    }

    storeConfiguredDevices();
    postSetupDevice(device);
    device->setupCompleted();
    emit deviceChanged(device);

    return DeviceErrorNoError;
}

/*! Edit the \l{Param}{Params} of a configured device to the \l{Param}{Params} of the DeviceDescriptor with the
 *  given \a deviceId to the given DeviceDescriptorId.
 *  Only devices with \l{DeviceClass}{CreateMethodDiscovery} can be changed using this method.
 *  The \a deviceDescriptorId must refer to an existing DeviceDescriptorId from the discovery.
 *  This method allows to rediscover a device and update it's \l{Param}{Params}.
 *
 *  Returns \l{DeviceError} to inform about the result. */
DeviceManager::DeviceError DeviceManager::reconfigureDevice(const DeviceId &deviceId, const DeviceDescriptorId &deviceDescriptorId)
{
    Device *device = findConfiguredDevice(deviceId);
    if (!device) {
        return DeviceErrorDeviceNotFound;
    }

    DeviceClass deviceClass = findDeviceClass(device->deviceClassId());
    if (!deviceClass.isValid()) {
        return DeviceErrorDeviceClassNotFound;
    }
    if (!deviceClass.createMethods().testFlag(DeviceClass::CreateMethodDiscovery)) {
        return DeviceErrorCreationMethodNotSupported;
    }

    DeviceDescriptor descriptor = m_discoveredDevices.take(deviceDescriptorId);
    if (!descriptor.isValid()) {
        return DeviceErrorDeviceDescriptorNotFound;
    }

    return reconfigureDevice(deviceId, descriptor.params(), true);
}

/*! Edit the \a name of the \l{Device} with the given \a deviceId.
    Returns \l{DeviceManager::DeviceError}{DeviceError} to inform about the result.
*/
DeviceManager::DeviceError DeviceManager::editDevice(const DeviceId &deviceId, const QString &name)
{
    Device *device = findConfiguredDevice(deviceId);
    if (!device)
        return DeviceErrorDeviceNotFound;

    device->setName(name);
    storeConfiguredDevices();
    emit deviceChanged(device);

    return DeviceErrorNoError;
}

/*! Initiates a pairing with a \l{DeviceClass}{Device} with the given \a pairingTransactionId, \a deviceClassId, \a name and \a params.
 *  Returns \l{DeviceManager::DeviceError}{DeviceError} to inform about the result. */
DeviceManager::DeviceError DeviceManager::pairDevice(const PairingTransactionId &pairingTransactionId, const DeviceClassId &deviceClassId, const QString &name, const ParamList &params)
{
    DeviceClass deviceClass = findDeviceClass(deviceClassId);
    if (deviceClass.id().isNull()) {
        qCWarning(dcDeviceManager) << "Cannot find a device class with id" << deviceClassId;
        return DeviceErrorDeviceClassNotFound;
    }

    Q_UNUSED(pairingTransactionId)
    Q_UNUSED(params)
    Q_UNUSED(name)
    switch (deviceClass.setupMethod()) {
    case DeviceClass::SetupMethodJustAdd:
        qCWarning(dcDeviceManager) << "Cannot setup this device this way. No need to pair this device.";
        return DeviceErrorSetupMethodNotSupported;
    case DeviceClass::SetupMethodDisplayPin:
        qCWarning(dcDeviceManager) << "SetupMethodDisplayPin not implemented yet for this CreateMethod";
        return DeviceErrorSetupFailed;
    case DeviceClass::SetupMethodEnterPin:
        qCWarning(dcDeviceManager) << "SetupMethodEnterPin not implemented yet for this CreateMethod";
        return DeviceErrorSetupFailed;
    case DeviceClass::SetupMethodPushButton:
        qCWarning(dcDeviceManager) << "SetupMethodPushButton not implemented yet for this CreateMethod";
        return DeviceErrorSetupFailed;
    }

    return DeviceErrorNoError;
}

/*! Initiates a pairing with a \l{DeviceClass}{Device} with the given \a pairingTransactionId, \a deviceClassId, \a name and \a deviceDescriptorId.
 *  Returns \l{DeviceManager::DeviceError}{DeviceError} to inform about the result. */
DeviceManager::DeviceError DeviceManager::pairDevice(const PairingTransactionId &pairingTransactionId, const DeviceClassId &deviceClassId, const QString &name, const DeviceDescriptorId &deviceDescriptorId)
{
    DeviceClass deviceClass = findDeviceClass(deviceClassId);
    if (deviceClass.id().isNull()) {
        qCWarning(dcDeviceManager) << "Cannot find a device class with id" << deviceClassId;
        return DeviceErrorDeviceClassNotFound;
    }

    if (deviceClass.setupMethod() == DeviceClass::SetupMethodJustAdd) {
        qCWarning(dcDeviceManager) << "Cannot setup this device this way. No need to pair this device.";
        return DeviceErrorCreationMethodNotSupported;
    }

    if (!m_discoveredDevices.contains(deviceDescriptorId)) {
        qCWarning(dcDeviceManager) << "Cannot find a DeviceDescriptor with ID" << deviceClassId.toString();
        return DeviceErrorDeviceDescriptorNotFound;
    }

    m_pairingsDiscovery.insert(pairingTransactionId, DevicePairingInfo(deviceClassId, name, deviceDescriptorId));

    if (deviceClass.setupMethod() == DeviceClass::SetupMethodDisplayPin) {
        DeviceDescriptor deviceDescriptor = m_discoveredDevices.value(deviceDescriptorId);

        DevicePlugin *plugin = m_devicePlugins.value(m_supportedDevices.value(deviceClassId).pluginId());
        if (!plugin) {
            qCWarning(dcDeviceManager()) << "Can't find a plugin for this device class";
            return DeviceErrorPluginNotFound;
        }

        return plugin->displayPin(pairingTransactionId, deviceDescriptor);
    }

    return DeviceErrorNoError;
}

/*! Confirms the pairing of a \l{Device} with the given \a pairingTransactionId and \a secret.
 *  Returns \l{DeviceManager::DeviceError}{DeviceError} to inform about the result. */
DeviceManager::DeviceError DeviceManager::confirmPairing(const PairingTransactionId &pairingTransactionId, const QString &secret)
{
    if (m_pairingsJustAdd.contains(pairingTransactionId)) {
        qCWarning(dcDeviceManager) << "This SetupMethod is not implemented yet";
        m_pairingsJustAdd.remove(pairingTransactionId);
        return DeviceErrorSetupFailed;
    }

    if (m_pairingsDiscovery.contains(pairingTransactionId)) {
        DevicePairingInfo pairingInfo = m_pairingsDiscovery.value(pairingTransactionId);
        DeviceClassId deviceClassId = pairingInfo.deviceClassId();
        DeviceDescriptor deviceDescriptor = m_discoveredDevices.value(pairingInfo.deviceDescriptorId());

        DevicePlugin *plugin = m_devicePlugins.value(m_supportedDevices.value(deviceClassId).pluginId());

        if (!plugin) {
            qCWarning(dcDeviceManager) << "Can't find a plugin for this device class";
            return DeviceErrorPluginNotFound;
        }

        DeviceSetupStatus status = plugin->confirmPairing(pairingTransactionId, deviceClassId, deviceDescriptor.params(), secret);
        switch (status) {
        case DeviceSetupStatusSuccess:
            m_pairingsDiscovery.remove(pairingTransactionId);
            // TODO: setup the device if the pairing status can be fetched directly
            return DeviceErrorNoError;
        case DeviceSetupStatusFailure:
            m_pairingsDiscovery.remove(pairingTransactionId);
            return DeviceErrorSetupFailed;
        case DeviceSetupStatusAsync:
            return DeviceErrorAsync;
        }
    }

    return DeviceErrorPairingTransactionIdNotFound;
}

/*! This method will only be used from the DeviceManager in order to add a \l{Device} with the given \a deviceClassId, \a name, \a params and \ id.
 *  Returns \l{DeviceError} to inform about the result. */
DeviceManager::DeviceError DeviceManager::addConfiguredDeviceInternal(const DeviceClassId &deviceClassId, const QString &name, const ParamList &params, const DeviceId id)
{
    ParamList effectiveParams = params;
    DeviceClass deviceClass = findDeviceClass(deviceClassId);
    if (deviceClass.id().isNull()) {
        return DeviceErrorDeviceClassNotFound;
    }

    if (deviceClass.setupMethod() != DeviceClass::SetupMethodJustAdd) {
        return DeviceErrorCreationMethodNotSupported;
    }

    DeviceError result = verifyParams(deviceClass.paramTypes(), effectiveParams);
    if (result != DeviceErrorNoError) {
        return result;
    }

    foreach(Device *device, m_configuredDevices) {
        if (device->id() == id) {
            return DeviceErrorDuplicateUuid;
        }
    }

    DevicePlugin *plugin = m_devicePlugins.value(deviceClass.pluginId());
    if (!plugin) {
        return DeviceErrorPluginNotFound;
    }

    Device *device = new Device(plugin->pluginId(), id, deviceClassId, this);
    if (name.isEmpty()) {
        device->setName(deviceClass.name());
    } else {
        device->setName(name);
    }
    device->setParams(effectiveParams);

    DeviceSetupStatus status = setupDevice(device);
    switch (status) {
    case DeviceSetupStatusFailure:
        qCWarning(dcDeviceManager) << "Device setup failed. Not adding device to system.";
        delete device;
        return DeviceErrorSetupFailed;
    case DeviceSetupStatusAsync:
        return DeviceErrorAsync;
    case DeviceSetupStatusSuccess:
        qCDebug(dcDeviceManager) << "Device setup complete.";
        break;
    }

    m_configuredDevices.insert(device->id(), device);
    storeConfiguredDevices();
    postSetupDevice(device);

    emit deviceAdded(device);

    return DeviceErrorNoError;
}

/*! Removes a \l{Device} with the given \a deviceId from the list of configured \l{Device}{Devices}.
 *  This method also deletes all saved settings of the \l{Device}.
 *  Returns \l{DeviceError} to inform about the result. */
DeviceManager::DeviceError DeviceManager::removeConfiguredDevice(const DeviceId &deviceId)
{
    Device *device = m_configuredDevices.take(deviceId);
    if (!device) {
        return DeviceErrorDeviceNotFound;
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

    return DeviceErrorNoError;
}

/*! Returns the \l{Device} with the given \a id. Null if the id couldn't be found. */
Device *DeviceManager::findConfiguredDevice(const DeviceId &id) const
{
    foreach (Device *device, m_configuredDevices) {
        if (device->id() == id) {
            return device;
        }
    }
    return nullptr;
}

/*! Returns all configured \{Device}{Devices} in the system. */
QList<Device *> DeviceManager::configuredDevices() const
{
    return m_configuredDevices.values();
}

/*! Returns all \l{Device}{Devices} matching the \l{DeviceClass} referred by \a deviceClassId. */
QList<Device *> DeviceManager::findConfiguredDevices(const DeviceClassId &deviceClassId) const
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
QList<Device *> DeviceManager::findConfiguredDevices(const QString &interface) const
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
QList<Device *> DeviceManager::findChildDevices(const DeviceId &id) const
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
DeviceClass DeviceManager::findDeviceClass(const DeviceClassId &deviceClassId) const
{
    foreach (const DeviceClass &deviceClass, m_supportedDevices) {
        if (deviceClass.id() == deviceClassId) {
            return deviceClass;
        }
    }
    return DeviceClass();
}

/*! Verify if the given \a params matches the given \a paramTypes. Ith \a requireAll
 *  is true, all \l{ParamList}{Params} has to be valid. Returns \l{DeviceError} to inform about the result.*/
DeviceManager::DeviceError DeviceManager::verifyParams(const QList<ParamType> paramTypes, ParamList &params, bool requireAll)
{
    foreach (const Param &param, params) {
        DeviceManager::DeviceError result = verifyParam(paramTypes, param);
        if (result != DeviceErrorNoError) {
            return result;
        }
    }
    if (!requireAll) {
        return DeviceErrorNoError;
    }
    foreach (const ParamType &paramType, paramTypes) {
        bool found = false;
        foreach (const Param &param, params) {
            if (paramType.id() == param.paramTypeId()) {
                found = true;
            }
        }

        // This paramType has a default value... lets fill in that one.
        if (!paramType.defaultValue().isNull() && !found) {
            found = true;
            params.append(Param(paramType.id(), paramType.defaultValue()));
        }

        if (!found) {
            qCWarning(dcDeviceManager) << "Missing parameter:" << paramType.name();
            return DeviceErrorMissingParameter;
        }
    }
    return DeviceErrorNoError;
}

/*! Verify if the given \a param matches one of the given \a paramTypes. Returns \l{DeviceError} to inform about the result.*/
DeviceManager::DeviceError DeviceManager::verifyParam(const QList<ParamType> paramTypes, const Param &param)
{
    foreach (const ParamType &paramType, paramTypes) {
        if (paramType.id() == param.paramTypeId()) {
            return verifyParam(paramType, param);
        }
    }

    qCWarning(dcDeviceManager) << "Invalid parameter" << param.paramTypeId().toString() << "in parameter list";
    return DeviceErrorInvalidParameter;
}

/*! Verify if the given \a param matches the given \a paramType. Returns \l{DeviceError} to inform about the result.*/
DeviceManager::DeviceError DeviceManager::verifyParam(const ParamType &paramType, const Param &param)
{
    if (paramType.id() != param.paramTypeId()) {
        qCWarning(dcDeviceManager) << "Parameter id" << param.paramTypeId().toString() << "does not match with ParamType id" << paramType.id().toString();
        return DeviceErrorInvalidParameter;
    }

    if (!param.value().canConvert(paramType.type())) {
        qCWarning(dcDeviceManager) << "Wrong parameter type for param" << param.paramTypeId().toString() << " Got:" << param.value() << " Expected:" << QVariant::typeToName(paramType.type());
        return DeviceErrorInvalidParameter;
    }

    if (!param.value().convert(paramType.type())) {
        qCWarning(dcDeviceManager) << "Could not convert value of param" << param.paramTypeId().toString() << " to:" << QVariant::typeToName(paramType.type()) << " Got:" << param.value();
        return DeviceErrorInvalidParameter;
    }

    if (paramType.type() == QVariant::Int) {
        if (paramType.maxValue().isValid() && param.value().toInt() > paramType.maxValue().toInt()) {
            qCWarning(dcDeviceManager) << "Value out of range for param" << param.paramTypeId().toString() << " Got:" << param.value() << " Max:" << paramType.maxValue();
            return DeviceErrorInvalidParameter;
        }

        if (paramType.minValue().isValid() && param.value().toInt() < paramType.minValue().toInt()) {
            qCWarning(dcDeviceManager) << "Value out of range for param" << param.paramTypeId().toString() << " Got:" << param.value() << " Min:" << paramType.minValue();
            return DeviceErrorInvalidParameter;
        }
    } else if (paramType.type() == QVariant::UInt) {
        if (paramType.maxValue().isValid() && param.value().toUInt() > paramType.maxValue().toUInt()) {
            qCWarning(dcDeviceManager) << "Value out of range for param" << param.paramTypeId().toString() << " Got:" << param.value() << " Max:" << paramType.maxValue();
            return DeviceErrorInvalidParameter;
        }

        if (paramType.minValue().isValid() && param.value().toUInt() < paramType.minValue().toUInt()) {
            qCWarning(dcDeviceManager) << "Value out of range for param" << param.paramTypeId().toString() << " Got:" << param.value() << " Min:" << paramType.minValue();
            return DeviceErrorInvalidParameter;
        }
    } else if (paramType.type() == QVariant::Double) {
        if (paramType.maxValue().isValid() && param.value().toDouble() > paramType.maxValue().toDouble()) {
            qCWarning(dcDeviceManager) << "Value out of range for param" << param.paramTypeId().toString() << " Got:" << param.value() << " Max:" << paramType.maxValue();
            return DeviceErrorInvalidParameter;
        }

        if (paramType.minValue().isValid() && param.value().toDouble() < paramType.minValue().toDouble()) {
            qCWarning(dcDeviceManager) << "Value out of range for param" << param.paramTypeId().toString() << " Got:" << param.value() << " Min:" << paramType.minValue();
            return DeviceErrorInvalidParameter;
        }
    } else {
        if (paramType.maxValue().isValid() && param.value() > paramType.maxValue()) {
            qCWarning(dcDeviceManager) << "Value out of range for param" << param.paramTypeId().toString() << " Got:" << param.value() << " Max:" << paramType.maxValue();
            return DeviceErrorInvalidParameter;
        }

        if (paramType.minValue().isValid() && param.value() < paramType.minValue()) {
            qCWarning(dcDeviceManager) << "Value out of range for param" << param.paramTypeId().toString() << " Got:" << param.value() << " Min:" << paramType.minValue();
            return DeviceErrorInvalidParameter;
        }
    }

    if (!paramType.allowedValues().isEmpty() && !paramType.allowedValues().contains(param.value())) {
        QStringList allowedValues;
        foreach (const QVariant &value, paramType.allowedValues()) {
            allowedValues.append(value.toString());
        }

        qCWarning(dcDeviceManager) << "Value not in allowed values for param" << param.paramTypeId().toString() << " Got:" << param.value() << " Allowed:" << allowedValues.join(",");
        return DeviceErrorInvalidParameter;
    }

    return DeviceErrorNoError;
}

/*! Execute the given \l{Action}.
 *  This will find the \l{Device} \a action refers to the \l{Action}{deviceId()} and
 *  its \l{DevicePlugin}. Then will dispatch the execution to the \l{DevicePlugin}.*/
DeviceManager::DeviceError DeviceManager::executeAction(const Action &action)
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
                    DeviceError paramCheck = verifyParams(actionType.paramTypes(), finalParams);
                    if (paramCheck != DeviceErrorNoError) {
                        return paramCheck;
                    }
                    finalAction.setParams(finalParams);
                    found = true;
                    continue;
                }
            }
            if (!found) {
                return DeviceErrorActionTypeNotFound;
            }

            return m_devicePlugins.value(device->pluginId())->executeAction(device, finalAction);
        }
    }
    return DeviceErrorDeviceNotFound;
}

/*! Centralized time tick for the NymeaTimer resource. Ticks every second. */
void DeviceManager::timeTick()
{

}

void DeviceManager::loadPlugins()
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

            if (!loader.load()) {
                qCWarning(dcDeviceManager) << "Could not load plugin data of" << entry << "\n" << loader.errorString();
                continue;
            }

            DevicePlugin *pluginIface = qobject_cast<DevicePlugin *>(loader.instance());
            if (!pluginIface) {
                qCWarning(dcDeviceManager) << "Could not get plugin instance of" << entry;
                continue;
            }
            pluginIface->setParent(this);

            if (!verifyPluginMetadata(loader.metaData().value("MetaData").toObject()))
                continue;

            pluginIface->setMetaData(loader.metaData().value("MetaData").toObject());

            loadPlugin(pluginIface);
        }
    }
}

void DeviceManager::loadPlugin(DevicePlugin *pluginIface)
{
    pluginIface->setLocale(m_locale);
    qApp->installTranslator(pluginIface->translator());

    pluginIface->initPlugin(this);

    qCDebug(dcDeviceManager) << "**** Loaded plugin" << pluginIface->pluginName();
    foreach (const Vendor &vendor, pluginIface->supportedVendors()) {
        qCDebug(dcDeviceManager) << "* Loaded vendor:" << vendor.name();
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
        foreach (const QString &paramTypeIdString, settings.allKeys()) {
            Param param(ParamTypeId(paramTypeIdString), settings.value(paramTypeIdString));
            params.append(param);
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
        DeviceError status = pluginIface->setConfiguration(params);
        if (status != DeviceErrorNoError) {
            qCWarning(dcDeviceManager) << "Error setting params to plugin. Broken configuration?";
        }
    }

    // Call the init method of the plugin
    pluginIface->init();

    m_devicePlugins.insert(pluginIface->pluginId(), pluginIface);

    connect(pluginIface, &DevicePlugin::emitEvent, this, &DeviceManager::eventTriggered);
    connect(pluginIface, &DevicePlugin::devicesDiscovered, this, &DeviceManager::slotDevicesDiscovered, Qt::QueuedConnection);
    connect(pluginIface, &DevicePlugin::deviceSetupFinished, this, &DeviceManager::slotDeviceSetupFinished);
    connect(pluginIface, &DevicePlugin::actionExecutionFinished, this, &DeviceManager::actionExecutionFinished);
    connect(pluginIface, &DevicePlugin::pairingFinished, this, &DeviceManager::slotPairingFinished);
    connect(pluginIface, &DevicePlugin::autoDevicesAppeared, this, &DeviceManager::onAutoDevicesAppeared);
    connect(pluginIface, &DevicePlugin::autoDeviceDisappeared, this, &DeviceManager::onAutoDeviceDisappeared);
}

void DeviceManager::loadConfiguredDevices()
{
    NymeaSettings settings(NymeaSettings::SettingsRoleDevices);
    settings.beginGroup("DeviceConfig");
    qCDebug(dcDeviceManager) << "Loading devices from" << settings.fileName();
    foreach (const QString &idString, settings.childGroups()) {
        settings.beginGroup(idString);
        Device *device = new Device(PluginId(settings.value("pluginid").toString()), DeviceId(idString), DeviceClassId(settings.value("deviceClassId").toString()), this);
        device->m_autoCreated = settings.value("autoCreated").toBool();
        device->setName(settings.value("devicename").toString());
        device->setParentId(DeviceId(settings.value("parentid", QUuid()).toString()));

        ParamList params;
        settings.beginGroup("Params");
        foreach (const QString &paramTypeIdString, settings.allKeys()) {
            params.append(Param(ParamTypeId(paramTypeIdString), settings.value(paramTypeIdString)));
        }
        device->setParams(params);
        settings.endGroup();
        settings.endGroup();

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

        DeviceSetupStatus status = setupDevice(device);
        if (status == DeviceSetupStatus::DeviceSetupStatusSuccess)
            postSetupDevice(device);
    }

}

void DeviceManager::storeConfiguredDevices()
{
    NymeaSettings settings(NymeaSettings::SettingsRoleDevices);
    settings.beginGroup("DeviceConfig");
    foreach (Device *device, m_configuredDevices) {
        settings.beginGroup(device->id().toString());
        settings.setValue("autoCreated", device->autoCreated());
        settings.setValue("devicename", device->name());
        settings.setValue("deviceClassId", device->deviceClassId().toString());
        settings.setValue("pluginid", device->pluginId().toString());
        if (!device->parentId().isNull())
            settings.setValue("parentid", device->parentId().toString());

        settings.beginGroup("Params");
        foreach (const Param &param, device->params()) {
            settings.setValue(param.paramTypeId().toString(), param.value());
        }
        settings.endGroup();
        settings.endGroup();
    }
    settings.endGroup();
}

void DeviceManager::startMonitoringAutoDevices()
{
    foreach (DevicePlugin *plugin, m_devicePlugins) {
        plugin->startMonitoringAutoDevices();
    }
}

void DeviceManager::slotDevicesDiscovered(const DeviceClassId &deviceClassId, const QList<DeviceDescriptor> deviceDescriptors)
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

void DeviceManager::slotDeviceSetupFinished(Device *device, DeviceManager::DeviceSetupStatus status)
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

    Q_ASSERT_X(status != DeviceSetupStatusAsync, "DeviceManager", "Bad plugin implementation. You should not emit deviceSetupFinished with status DeviceSetupStatusAsync.");
    if (status == DeviceSetupStatusAsync) {
        qCWarning(dcDeviceManager) << "Bad plugin implementation. Received a deviceSetupFinished event with status Async... ignoring...";
        return;
    }

    if (status == DeviceSetupStatusFailure) {
        if (m_configuredDevices.contains(device->id())) {
            if (m_asyncDeviceReconfiguration.contains(device)) {
                m_asyncDeviceReconfiguration.removeAll(device);
                qCWarning(dcDeviceManager) << QString("Error in device setup after reconfiguration. Device %1 (%2) will not be functional.").arg(device->name()).arg(device->id().toString());

                storeConfiguredDevices();

                // TODO: recover old params.??

                emit deviceChanged(device);
                emit deviceReconfigurationFinished(device, DeviceError::DeviceErrorSetupFailed);
            }
            qCWarning(dcDeviceManager) << QString("Error in device setup. Device %1 (%2) will not be functional.").arg(device->name()).arg(device->id().toString());
            emit deviceSetupFinished(device, DeviceError::DeviceErrorSetupFailed);
            return;
        } else {
            qCWarning(dcDeviceManager) << QString("Error in device setup. Device %1 (%2) will not be added to the configured devices.").arg(device->name()).arg(device->id().toString());
            emit deviceSetupFinished(device, DeviceError::DeviceErrorSetupFailed);
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
        emit deviceReconfigurationFinished(device, DeviceManager::DeviceErrorNoError);
        return;
    }

    connect(device, SIGNAL(stateValueChanged(QUuid,QVariant)), this, SLOT(slotDeviceStateValueChanged(QUuid,QVariant)));

    device->setupCompleted();
    emit deviceSetupFinished(device, DeviceManager::DeviceErrorNoError);
}

void DeviceManager::slotPairingFinished(const PairingTransactionId &pairingTransactionId, DeviceManager::DeviceSetupStatus status)
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

    if (status != DeviceSetupStatusSuccess) {
        emit pairingFinished(pairingTransactionId, DeviceErrorSetupFailed);
        return;
    }

    DeviceClass deviceClass = findDeviceClass(deviceClassId);
    DevicePlugin *plugin = m_devicePlugins.value(deviceClass.pluginId());
    if (!plugin) {
        qCWarning(dcDeviceManager) << "Cannot find a plugin for this device class!";
        emit pairingFinished(pairingTransactionId, DeviceErrorPluginNotFound, deviceClass.pluginId().toString());
        return;
    }

    // If we already have a deviceId, we're reconfiguring an existing device
    bool addNewDevice = deviceId.isNull();

    if (!addNewDevice && !m_configuredDevices.contains(deviceId)) {
        qCWarning(dcDeviceManager) << "The device to be reconfigured has disappeared!";
        emit pairingFinished(pairingTransactionId, DeviceErrorDeviceNotFound, deviceId);
        return;
    }

    // Ok... pairing went fine... Let consumers know about it and inform them about the ongoing setup with a deviceId.
    Device *device = nullptr;

    if (addNewDevice) {
        deviceId = DeviceId::createDeviceId();
        qCDebug(dcDeviceManager()) << "Creating new device with ID" << deviceId;
        device = new Device(plugin->pluginId(), deviceId, deviceClassId, this);
        if (deviceName.isEmpty()) {
            device->setName(deviceClass.name());
        } else {
            device->setName(deviceName);
        }
    } else {
        qCDebug(dcDeviceManager()) << "Reconfiguring device" << device;
        device = m_configuredDevices.value(deviceId);
    }
    emit pairingFinished(pairingTransactionId, DeviceErrorNoError, deviceId);

    device->setParams(params);

    DeviceSetupStatus setupStatus = setupDevice(device);
    switch (setupStatus) {
    case DeviceSetupStatusFailure:
        qCWarning(dcDeviceManager) << "Device setup failed. Not adding device to system.";
        emit deviceSetupFinished(device, DeviceError::DeviceErrorSetupFailed);
        delete device;
        break;
    case DeviceSetupStatusAsync:
        return;
    case DeviceSetupStatusSuccess:
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
    emit deviceSetupFinished(device, DeviceError::DeviceErrorNoError);
    postSetupDevice(device);
}

void DeviceManager::onAutoDevicesAppeared(const DeviceClassId &deviceClassId, const QList<DeviceDescriptor> &deviceDescriptors)
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
        Device *device = new Device(plugin->pluginId(), deviceClassId, this);
        device->m_autoCreated = true;
        device->setName(deviceDescriptor.title());
        device->setParams(deviceDescriptor.params());
        device->setParentId(deviceDescriptor.parentDeviceId());

        DeviceSetupStatus setupStatus = setupDevice(device);
        switch (setupStatus) {
        case DeviceSetupStatusFailure:
            qCWarning(dcDeviceManager) << "Device setup failed. Not adding device to system.";
            emit deviceSetupFinished(device, DeviceError::DeviceErrorSetupFailed);
            delete device;
            break;
        case DeviceSetupStatusAsync:
            break;
        case DeviceSetupStatusSuccess:
            qCDebug(dcDeviceManager) << "Device setup complete.";
            m_configuredDevices.insert(device->id(), device);
            storeConfiguredDevices();
            emit deviceSetupFinished(device, DeviceError::DeviceErrorNoError);
            emit deviceAdded(device);
            postSetupDevice(device);
            break;
        }
    }
}

void DeviceManager::onAutoDeviceDisappeared(const DeviceId &deviceId)
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

void DeviceManager::onLoaded()
{
    qCWarning(dcDeviceManager()) << "Done loading plugins and devices.";
    emit loaded();

    // schedule some housekeeping...
    QTimer::singleShot(0, this, SLOT(cleanupDeviceStateCache()));
}

void DeviceManager::cleanupDeviceStateCache()
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

void DeviceManager::slotDeviceStateValueChanged(const QUuid &stateTypeId, const QVariant &value)
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

bool DeviceManager::verifyPluginMetadata(const QJsonObject &data)
{
    QStringList requiredFields;
    requiredFields << "name" << "id" << "vendors";

    foreach (const QString &field, requiredFields) {
        if (!data.contains(field)) {
            qCWarning(dcDeviceManager) << "Error loading plugin. Incomplete metadata. Missing field:" << field;
            qCWarning(dcDeviceManager) << data;
            return false;
        }
    }
    return true;
}

DeviceManager::DeviceSetupStatus DeviceManager::setupDevice(Device *device)
{
    DeviceClass deviceClass = findDeviceClass(device->deviceClassId());
    DevicePlugin *plugin = m_devicePlugins.value(deviceClass.pluginId());

    if (!plugin) {
        qCWarning(dcDeviceManager) << "Can't find a plugin for this device" << device->id();
        return DeviceSetupStatusFailure;
    }

    QList<State> states;
    foreach (const StateType &stateType, deviceClass.stateTypes()) {
        State state(stateType.id(), device->id());
        states.append(state);
    }
    device->setStates(states);
    loadDeviceStates(device);

    DeviceSetupStatus status = plugin->setupDevice(device);
    if (status != DeviceSetupStatusSuccess) {
        return status;
    }

    connect(device, SIGNAL(stateValueChanged(QUuid,QVariant)), this, SLOT(slotDeviceStateValueChanged(QUuid,QVariant)));

    device->setupCompleted();
    return status;
}

void DeviceManager::postSetupDevice(Device *device)
{
    DeviceClass deviceClass = findDeviceClass(device->deviceClassId());
    DevicePlugin *plugin = m_devicePlugins.value(deviceClass.pluginId());

    plugin->postSetupDevice(device);
}

void DeviceManager::loadDeviceStates(Device *device)
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

void DeviceManager::storeDeviceStates(Device *device)
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

