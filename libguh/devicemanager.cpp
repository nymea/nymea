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

/*!
    \class DeviceManager
    \brief The main entry point when interacting with \l{Device}{Devices}

    \ingroup devices
    \inmodule libguh

    The DeviceManager hold  s all information about supported and configured Devices in the system.

    It is also responsible for loading Plugins and managing common hardware resources between
    \l{DevicePlugin}{device plugins}.
*/

/*! \enum DeviceManager::HardwareResource

    This enum type specifies hardware resources which can be requested by \l{DevicePlugin}{DevicePlugins}.

    \value HardwareResourceNone
        No Resource required.
    \value HardwareResourceRadio433
        Refers to the 433 MHz radio.
    \value HardwareResourceRadio868
        Refers to the 868 MHz radio.
    \value HardwareResourceTimer
        Refers to the global timer managed by the \l{DeviceManager}. Plugins should not create their own timers,
        but rather request the global timer using the hardware resources.
    \value HardwareResourceNetworkManager
        Allows to send network requests and receive replies.
    \value HardwareResourceUpnpDisovery
        Allowes to search a UPnP devices in the network.
    \value HardwareResourceBluetoothLE
        Allows to interact with bluetooth low energy devices.
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

/*! \fn void DeviceManager::loaded();
    The DeviceManager will emit this signal when all \l{Device}{Devices} are loaded.
*/

/*! \fn void DeviceManager::deviceSetupFinished(Device *device, DeviceError status);
    This signal is emitted when the setup of a \a device is finished. The \a status parameter describes the
    \l{DeviceManager::DeviceError}{DeviceError} that occurred.
*/

/*! \fn void DeviceManager::deviceStateChanged(Device *device, const QUuid &stateTypeId, const QVariant &value);
    This signal is emitted when the \l{State} of a \a device changed. The \a stateTypeId parameter describes the
    \l{StateType} and the \a value parameter holds the new value.
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
    creates one. Normally only \l{guhserver::GuhCore} should connect to this and execute actions
    after checking back with the \{guhserver::RulesEngine}. Exceptions might be monitoring interfaces
    or similar, but you should never directly react to this in a \l{DevicePlugin}.
*/

#include "devicemanager.h"
#include "loggingcategories.h"

#include "hardware/radio433/radio433.h"

#include "plugin/devicepairinginfo.h"
#include "plugin/deviceplugin.h"
#include "guhsettings.h"

#include <QPluginLoader>
#include <QStaticPlugin>
#include <QtPlugin>
#include <QDebug>
#include <QStringList>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QDir>

/*! Constructs the DeviceManager with the given \a parent. There should only be one DeviceManager in the system created by \l{guhserver::GuhCore}.
 *  Use \c guhserver::GuhCore::instance()->deviceManager() instead to access the DeviceManager. */
DeviceManager::DeviceManager(QObject *parent) :
    QObject(parent),
    m_radio433(0)
{
    qRegisterMetaType<DeviceClassId>();
    qRegisterMetaType<DeviceDescriptor>();

    m_pluginTimer.setInterval(10000);
    connect(&m_pluginTimer, &QTimer::timeout, this, &DeviceManager::timerEvent);

    m_radio433 = new Radio433(this);
    connect(m_radio433, &Radio433::dataReceived, this, &DeviceManager::radio433SignalReceived);
    m_radio433->enable();
    // TODO: error handling if no Radio433 detected (GPIO or network), disable radio433 plugins or something...

    // Network manager
    m_networkManager = new NetworkManager(this);
    connect(m_networkManager, &NetworkManager::replyReady, this, &DeviceManager::replyReady);

    // UPnP discovery
    m_upnpDiscovery = new UpnpDiscovery(this);
    connect(m_upnpDiscovery, &UpnpDiscovery::discoveryFinished, this, &DeviceManager::upnpDiscoveryFinished);
    connect(m_upnpDiscovery, &UpnpDiscovery::upnpNotify, this, &DeviceManager::upnpNotifyReceived);

    // Bluetooth LE
    #ifdef BLUETOOTH_LE
    m_bluetoothScanner = new BluetoothScanner(this);
    if (!m_bluetoothScanner->isAvailable()) {
        delete m_bluetoothScanner;
        m_bluetoothScanner = 0;
    } else {
        connect(m_bluetoothScanner, &BluetoothScanner::bluetoothDiscoveryFinished, this, &DeviceManager::bluetoothDiscoveryFinished);
    }
    #endif

    // Give hardware a chance to start up before loading plugins etc.
    QMetaObject::invokeMethod(this, "loadPlugins", Qt::QueuedConnection);
    QMetaObject::invokeMethod(this, "loadConfiguredDevices", Qt::QueuedConnection);
    QMetaObject::invokeMethod(this, "startMonitoringAutoDevices", Qt::QueuedConnection);
    // Make sure this is always emitted after plugins and devices are loaded
    QMetaObject::invokeMethod(this, "loaded", Qt::QueuedConnection);
}

/*! Destructor of the DeviceManager. Each loaded \l{DevicePlugin} will be deleted. */
DeviceManager::~DeviceManager()
{
    qCDebug(dcApplication) << "Shutting down \"Device Manager\"";
    foreach (DevicePlugin *plugin, m_devicePlugins) {
        delete plugin;
    }
}

/*! Returns the list of search direcorys where \l{DevicePlugin} will be searched. */
QStringList DeviceManager::pluginSearchDirs()
{
    QStringList searchDirs;
    searchDirs << QCoreApplication::applicationDirPath() + "/../lib/guh/plugins";
    searchDirs << QCoreApplication::applicationDirPath() + "/../plugins/";
    searchDirs << QCoreApplication::applicationDirPath() + "/../plugins/deviceplugins";
    searchDirs << QCoreApplication::applicationDirPath() + "/../../../plugins/deviceplugins";
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
            if (entry.startsWith("libguh_deviceplugin") && entry.endsWith(".so")) {
                fi.setFile(path + "/" + entry);
            } else {
                fi.setFile(path + "/" + entry + "/libguh_deviceplugin" + entry + ".so");
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
    if (!plugin)
        return DeviceErrorPluginNotFound;

    ParamList params = pluginConfig;

    DeviceError verify = verifyParams(plugin->configurationDescription(), params);
    if (verify != DeviceErrorNoError)
        return verify;

    DeviceError result = plugin->setConfiguration(pluginConfig);
    if (result != DeviceErrorNoError)
        return result;

    GuhSettings settings(GuhSettings::SettingsRolePlugins);
    settings.beginGroup("PluginConfig");
    settings.beginGroup(plugin->pluginId().toString());
    foreach (const Param &param, pluginConfig) {
        settings.setValue(param.name(), param.value());
    }
    settings.endGroup();
    settings.endGroup();
    return result;
}

/*! Returns all the \l{Vendor}s loaded in the system. */
QList<Vendor> DeviceManager::supportedVendors() const
{
    return m_supportedVendors.values();
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

/*! Add a new configured device for the given \l{DeviceClass}, the given parameters and \a id.
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
 *  from the discovery.
 *  Returns \l{DeviceError} to inform about the result. */
DeviceManager::DeviceError DeviceManager::addConfiguredDevice(const DeviceClassId &deviceClassId, const QString &name, const DeviceDescriptorId &deviceDescriptorId, const DeviceId &deviceId)
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

    return addConfiguredDeviceInternal(deviceClassId, name, descriptor.params(), deviceId);
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
                if (paramType.name() == param.name()) {
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
        device->setParamValue(param.name(), param.value());
    }

    // try to setup the device with the new params
    DeviceSetupStatus status = plugin->setupDevice(device);
    switch (status) {
    case DeviceSetupStatusFailure:
        qCWarning(dcDeviceManager) << "Device reconfiguration failed. Not saving changes of device paramters. Device setup incomplete.";
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

/*! Initiates a pairing with a \l{DeviceClass}{Device} with the given \a pairingTransactionId, \a deviceClassId and \a params.
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

/*! Initiates a pairing with a \l{DeviceClass}{Device} with the given \a pairingTransactionId, \a deviceClassId and \a deviceDescriptorId.
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
            qWarning() << "Can't find a plugin for this device class";
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
        qCWarning(dcDeviceManager) << "this SetupMethod is not implemented yet";
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

    m_configuredDevices.append(device);
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
    Device *device = findConfiguredDevice(deviceId);
    if (!device) {
        return DeviceErrorDeviceNotFound;
    }

    m_configuredDevices.removeAll(device);
    m_devicePlugins.value(device->pluginId())->deviceRemoved(device);

    // check if this plugin still needs the guhTimer call
    bool pluginNeedsTimer = false;
    foreach (Device* d, m_configuredDevices) {
        if (d->pluginId() == device->pluginId()) {
            pluginNeedsTimer = true;
            break;
        }
    }

    // if this plugin doesn't need any longer the guhTimer call
    if (!pluginNeedsTimer) {
        m_pluginTimerUsers.removeAll(plugin(device->pluginId()));
        if (m_pluginTimerUsers.isEmpty()) {
            m_pluginTimer.stop();
        }
    }
    device->deleteLater();

    GuhSettings settings(GuhSettings::SettingsRoleDevices);
    settings.beginGroup("DeviceConfig");
    settings.beginGroup(deviceId.toString());
    settings.remove("");
    settings.endGroup();

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
    return 0;
}

/*! Returns all configured \{Device}{Devices} in the system. */
QList<Device *> DeviceManager::configuredDevices() const
{
    return m_configuredDevices;
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

/*! Returns all child \l{Device}{Devices} of the given \a device. */
QList<Device *> DeviceManager::findChildDevices(Device *device) const
{
    QList<Device *> ret;
    foreach (Device *d, m_configuredDevices) {
        if (d->parentId() == device->id()) {
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
            if (paramType.name() == param.name()) {
                found = true;
            }
        }

        // This paramType has a default value... lets fill in that one.
        if (!paramType.defaultValue().isNull() && !found) {
            found = true;
            params.append(Param(paramType.name(), paramType.defaultValue()));
        }

        if (!found) {
            qCWarning(dcDeviceManager) << "Missing parameter:" << paramType.name();
            return DeviceErrorMissingParameter;
        }
    }
    return DeviceErrorNoError;
}

DeviceManager::DeviceError DeviceManager::verifyParam(const QList<ParamType> paramTypes, const Param &param)
{
    foreach (const ParamType &paramType, paramTypes) {
        if (paramType.name() == param.name()) {
            return verifyParam(paramType, param);
        }
    }
    qCWarning(dcDeviceManager) << "Invalid parameter" << param.name() << "in parameter list";
    return DeviceErrorInvalidParameter;
}

DeviceManager::DeviceError DeviceManager::verifyParam(const ParamType &paramType, const Param &param)
{
    if (paramType.name() == param.name()) {

        if (!param.value().canConvert(paramType.type())) {
            qCWarning(dcDeviceManager) << "Wrong parameter type for param" << param.name() << " Got:" << param.value() << " Expected:" << QVariant::typeToName(paramType.type());
            return DeviceErrorInvalidParameter;
        }

        if (!param.value().convert(paramType.type())) {
            qCWarning(dcDeviceManager) << "Could not convert value of param" << param.name() << " to:" << QVariant::typeToName(paramType.type()) << " Got:" << param.value();
            return DeviceErrorInvalidParameter;
        }

        if (paramType.maxValue().isValid() && param.value() > paramType.maxValue()) {
            qCWarning(dcDeviceManager) << "Value out of range for param" << param.name() << " Got:" << param.value() << " Max:" << paramType.maxValue();
            return DeviceErrorInvalidParameter;
        }

        if (paramType.minValue().isValid() && param.value() < paramType.minValue()) {
            qCWarning(dcDeviceManager) << "Value out of range for param" << param.name() << " Got:" << param.value() << " Min:" << paramType.minValue();
            return DeviceErrorInvalidParameter;
        }

        if (!paramType.allowedValues().isEmpty() && !paramType.allowedValues().contains(param.value())) {
            QStringList allowedValues;
            foreach (const QVariant &value, paramType.allowedValues()) {
                allowedValues.append(value.toString());
            }

            qCWarning(dcDeviceManager) << "Value not in allowed values for param" << param.name() << " Got:" << param.value() << " Allowed:" << allowedValues.join(",");
            return DeviceErrorInvalidParameter;
        }
        return DeviceErrorNoError;
    }

    qCWarning(dcDeviceManager) << "Parameter name" << param.name() << "does not match with ParamType name" << paramType.name();
    return DeviceErrorInvalidParameter;
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

void DeviceManager::loadPlugins()
{
    foreach (const QString &path, pluginSearchDirs()) {
        QDir dir(path);
        qCDebug(dcDeviceManager) << "Loading plugins from:" << dir.absolutePath();
        foreach (const QString &entry, dir.entryList()) {
            QFileInfo fi;
            if (entry.startsWith("libguh_deviceplugin") && entry.endsWith(".so")) {
                fi.setFile(path + "/" + entry);
            } else {
                fi.setFile(path + "/" + entry + "/libguh_deviceplugin" + entry + ".so");
            }
            if (!fi.exists()) {
                continue;
            }

            QPluginLoader loader;
            loader.setFileName(fi.absoluteFilePath());
            loader.setLoadHints(QLibrary::ResolveAllSymbolsHint);

            if (!loader.load()) {
                qCWarning(dcDeviceManager) << "Could not load plugin data of" << entry;
                continue;
            }

            DevicePlugin *pluginIface = qobject_cast<DevicePlugin *>(loader.instance());
            if (!pluginIface) {
                qCWarning(dcDeviceManager) << "Could not get plugin instance of" << entry;
                continue;
            }

            if (verifyPluginMetadata(loader.metaData().value("MetaData").toObject())) {
                pluginIface->initPlugin(loader.metaData().value("MetaData").toObject(), this);
                qCDebug(dcDeviceManager) << "**** Loaded plugin" << pluginIface->pluginName();
                foreach (const Vendor &vendor, pluginIface->supportedVendors()) {
                    qCDebug(dcDeviceManager) << "* Loaded vendor:" << vendor.name();
                    if (m_supportedVendors.contains(vendor.id())) {
                        continue;
                    }
                    m_supportedVendors.insert(vendor.id(), vendor);
                }

                foreach (const DeviceClass &deviceClass, pluginIface->supportedDevices()) {
                    if (!m_supportedVendors.contains(deviceClass.vendorId())) {
                        qCWarning(dcDeviceManager) << "! Vendor not found. Ignoring device. VendorId:" << deviceClass.vendorId() << "DeviceClass:" << deviceClass.name() << deviceClass.id();
                        continue;
                    }
                    m_vendorDeviceMap[deviceClass.vendorId()].append(deviceClass.id());
                    m_supportedDevices.insert(deviceClass.id(), deviceClass);
                    qCDebug(dcDeviceManager) << "* Loaded device class:" << deviceClass.name();
                }

                GuhSettings settings(GuhSettings::SettingsRolePlugins);
                settings.beginGroup("PluginConfig");
                ParamList params;
                if (settings.childGroups().contains(pluginIface->pluginId().toString())) {
                    settings.beginGroup(pluginIface->pluginId().toString());
                    foreach (const QString &paramName, settings.allKeys()) {
                        Param param(paramName, settings.value(paramName));
                        params.append(param);
                    }
                    settings.endGroup();
                } else if (pluginIface->configurationDescription().count() > 0){
                    // plugin requires config but none stored. Init with defaults
                    foreach (const ParamType &paramType, pluginIface->configurationDescription()) {
                        Param param(paramType.name(), paramType.defaultValue());
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

                m_devicePlugins.insert(pluginIface->pluginId(), pluginIface);

                connect(pluginIface, &DevicePlugin::emitEvent, this, &DeviceManager::eventTriggered);
                connect(pluginIface, &DevicePlugin::devicesDiscovered, this, &DeviceManager::slotDevicesDiscovered);
                connect(pluginIface, &DevicePlugin::deviceSetupFinished, this, &DeviceManager::slotDeviceSetupFinished);
                connect(pluginIface, &DevicePlugin::actionExecutionFinished, this, &DeviceManager::actionExecutionFinished);
                connect(pluginIface, &DevicePlugin::pairingFinished, this, &DeviceManager::slotPairingFinished);
                connect(pluginIface, &DevicePlugin::autoDevicesAppeared, this, &DeviceManager::autoDevicesAppeared);
            }
        }
    }
}

void DeviceManager::loadConfiguredDevices()
{
    GuhSettings settings(GuhSettings::SettingsRoleDevices);
    settings.beginGroup("DeviceConfig");
    qCDebug(dcDeviceManager) << "loading devices from" << settings.fileName();
    foreach (const QString &idString, settings.childGroups()) {
        settings.beginGroup(idString);
        Device *device = new Device(PluginId(settings.value("pluginid").toString()), DeviceId(idString), DeviceClassId(settings.value("deviceClassId").toString()), this);
        device->setName(settings.value("devicename").toString());
        device->setParentId(DeviceId(settings.value("parentid", QUuid()).toString()));

        ParamList params;
        settings.beginGroup("Params");
        foreach (QString paramNameString, settings.allKeys()) {
            Param param(paramNameString);
            param.setValue(settings.value(paramNameString));
            params.append(param);
        }
        device->setParams(params);
        settings.endGroup();
        settings.endGroup();

        // We always add the device to the list in this case. If its in the storedDevices
        // it means that it was working at some point so lets still add it as there might
        // be rules associated with this device. Device::setupCompleted() will be false.
        DeviceSetupStatus status = setupDevice(device);
        m_configuredDevices.append(device);

        if (status == DeviceSetupStatus::DeviceSetupStatusSuccess)
            postSetupDevice(device);
    }
    settings.endGroup();
}

void DeviceManager::storeConfiguredDevices()
{
    GuhSettings settings(GuhSettings::SettingsRoleDevices);
    settings.beginGroup("DeviceConfig");
    foreach (Device *device, m_configuredDevices) {
        settings.beginGroup(device->id().toString());
        settings.setValue("devicename", device->name());
        settings.setValue("deviceClassId", device->deviceClassId().toString());
        settings.setValue("pluginid", device->pluginId().toString());
        if (!device->parentId().isNull())
            settings.setValue("parentid", device->parentId().toString());

        settings.beginGroup("Params");
        foreach (const Param &param, device->params()) {
            settings.setValue(param.name(), param.value());
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
        if (m_configuredDevices.contains(device)) {
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
    if (!m_configuredDevices.contains(device)) {
        m_configuredDevices.append(device);
        emit deviceAdded(device);
        storeConfiguredDevices();
    }

    DevicePlugin *plugin = m_devicePlugins.value(device->pluginId());
    if (plugin->requiredHardware().testFlag(HardwareResourceTimer)) {
        if (!m_pluginTimer.isActive()) {
            m_pluginTimer.start();
            // Additionally fire off one event to initialize stuff
            QTimer::singleShot(0, this, SLOT(timerEvent()));
        }
        if (!m_pluginTimerUsers.contains(plugin)) {
            m_pluginTimerUsers.append(plugin);
        }
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

    // Ok... pairing went fine... Let consumers know about it and inform them about the ongoing setup with a deviceId.
    DeviceId id = DeviceId::createDeviceId();
    emit pairingFinished(pairingTransactionId, DeviceErrorNoError, id);

    QList<DeviceId> newDevices;
    Device *device = new Device(plugin->pluginId(), id, deviceClassId, this);
    if (deviceName.isEmpty()) {
        device->setName(deviceClass.name());
    } else {
        device->setName(deviceName);
    }
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
        newDevices.append(id);
        break;
    }

    m_configuredDevices.append(device);
    emit deviceAdded(device);
    storeConfiguredDevices();
    emit deviceSetupFinished(device, DeviceError::DeviceErrorNoError);
    postSetupDevice(device);
}

void DeviceManager::autoDevicesAppeared(const DeviceClassId &deviceClassId, const QList<DeviceDescriptor> &deviceDescriptors)
{
    DeviceClass deviceClass = findDeviceClass(deviceClassId);
    if (!deviceClass.isValid()) {
        return;
    }

    DevicePlugin *plugin = m_devicePlugins.value(deviceClass.pluginId());
    if (!plugin) {
        return;
    }

    foreach (const DeviceDescriptor &deviceDescriptor, deviceDescriptors) {
        Device *device = new Device(plugin->pluginId(), deviceClassId, this);
        device->setName(deviceClass.name());
        device->setParams(deviceDescriptor.params());

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
            m_configuredDevices.append(device);
            storeConfiguredDevices();
            emit deviceSetupFinished(device, DeviceError::DeviceErrorNoError);
            emit deviceAdded(device);
            postSetupDevice(device);
            break;
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

    Param valueParam("value", value);
    Event event(EventTypeId(stateTypeId.toString()), device->id(), ParamList() << valueParam, true);
    emit eventTriggered(event);
}

void DeviceManager::radio433SignalReceived(QList<int> rawData)
{
    QList<DevicePlugin*> targetPlugins;

    foreach (Device *device, m_configuredDevices) {
        DeviceClass deviceClass = m_supportedDevices.value(device->deviceClassId());
        DevicePlugin *plugin = m_devicePlugins.value(deviceClass.pluginId());
        if (plugin->requiredHardware().testFlag(HardwareResourceRadio433) && !targetPlugins.contains(plugin)) {
            targetPlugins.append(plugin);
        }
    }
    foreach (DevicePlugin *plugin, m_discoveringPlugins) {
        if (plugin->requiredHardware().testFlag(HardwareResourceRadio433) && !targetPlugins.contains(plugin)) {
            targetPlugins.append(plugin);
        }
    }

    foreach (DevicePlugin *plugin, targetPlugins) {
        plugin->radioData(rawData);
    }
}

void DeviceManager::replyReady(const PluginId &pluginId, QNetworkReply *reply)
{
    foreach (DevicePlugin *devicePlugin, m_devicePlugins) {
        if (devicePlugin->requiredHardware().testFlag(HardwareResourceNetworkManager) && devicePlugin->pluginId() == pluginId) {
            devicePlugin->networkManagerReplyReady(reply);
        }
    }
}
void DeviceManager::upnpDiscoveryFinished(const QList<UpnpDeviceDescriptor> &deviceDescriptorList, const PluginId &pluginId)
{
    foreach (DevicePlugin *devicePlugin, m_devicePlugins) {
        if (devicePlugin->requiredHardware().testFlag(HardwareResourceUpnpDisovery) && devicePlugin->pluginId() == pluginId) {
            devicePlugin->upnpDiscoveryFinished(deviceDescriptorList);
        }
    }
}

void DeviceManager::upnpNotifyReceived(const QByteArray &notifyData)
{
    foreach (DevicePlugin *devicePlugin, m_devicePlugins) {
        if (devicePlugin->requiredHardware().testFlag(HardwareResourceUpnpDisovery)) {
            devicePlugin->upnpNotifyReceived(notifyData);
        }
    }
}

#ifdef BLUETOOTH_LE
void DeviceManager::bluetoothDiscoveryFinished(const PluginId &pluginId, const QList<QBluetoothDeviceInfo> &deviceInfos)
{
    foreach (DevicePlugin *devicePlugin, m_devicePlugins) {
        if (devicePlugin->requiredHardware().testFlag(HardwareResourceBluetoothLE) && devicePlugin->pluginId() == pluginId) {
            devicePlugin->bluetoothDiscoveryFinished(deviceInfos);
        }
    }
}
#endif

void DeviceManager::timerEvent()
{
    foreach (DevicePlugin *plugin, m_pluginTimerUsers) {
        if (plugin->requiredHardware().testFlag(HardwareResourceTimer)) {
            plugin->guhTimer();
        }
    }
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
        state.setValue(stateType.defaultValue());
        states.append(state);
    }
    device->setStates(states);

    DeviceSetupStatus status = plugin->setupDevice(device);
    if (status != DeviceSetupStatusSuccess) {
        return status;
    }

    if (plugin->requiredHardware().testFlag(HardwareResourceTimer)) {
        if (!m_pluginTimer.isActive()) {
            m_pluginTimer.start();
            // Additionally fire off one event to initialize stuff
            QTimer::singleShot(0, this, SLOT(timerEvent()));
        }
        if (!m_pluginTimerUsers.contains(plugin)) {
            m_pluginTimerUsers.append(plugin);
        }
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

