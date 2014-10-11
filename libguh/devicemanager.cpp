/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
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

    The DeviceManager holds all information about supported and configured Devices in the system.

    It is also responsible for loading Plugins and managing common hardware resources between
    \l{DevicePlugin}{device plugins}.

*/

/*!
    \enum DeviceManager::HardwareResource

    This enum type specifies hardware resources which can be requested by \l{DevicePlugin}{DevicePlugins}.

    \value HardwareResourceNone
        No Resource required.
    \value HardwareResourceRadio433
        Refers to the 433 MHz radio.
    \value HardwareResourceRadio868
        Refers to the 868 MHz radio.
    \value HardwareResourceTimer
        Refers to the global timer managed by the \l{DeviceManager}. Plugins should not create their own timers, but rather request the global timer using the hardware resources.
*/

/*!
    \enum DeviceManager::DeviceError

    This enum type specifies the errors that can happen when working with \l{Device}{Devices}.

    \value DeviceErrorNoError
        No Error. Everything went fine.
    \value DeviceErrorDeviceNotFound
        Couldn't find a \l{Device} for the given id.
    \value DeviceErrorDeviceClassNotFound
        Couldn't find a \l{DeviceClass} for the given id.
    \value DeviceErrorActionTypeNotFound
        Couldn't find the \l{ActionType} for the given id.
    \value DeviceErrorMissingParameter
        Parameters do not comply to the template.
    \value DeviceErrorPluginNotFound
        Couldn't find the Plugin for the given id.
    \value DeviceErrorSetupFailed
        Error setting up the \{Device}. It will not be functional.
*/

/*! \fn void DeviceManager::emitEvent(const Event &event)
    The DeviceManager will emit a \l{Event} described in \a event whenever a Device
    creates one. Normally only \l{GuhCore} should connect to this and execute actions
    after checking back with the \{RulesEngine}. Exceptions might be monitoring interfaces
    or similar, but you should never directly react to this in a \l{DevicePlugin}.
*/

#include "devicemanager.h"

#include "hardware/radio433/radio433.h"

#include "plugin/device.h"
#include "plugin/deviceclass.h"
#include "plugin/deviceplugin.h"

#include <QPluginLoader>
#include <QStaticPlugin>
#include <QtPlugin>
#include <QDebug>
#include <QSettings>
#include <QStringList>
#include <QCoreApplication>

/*! Constructs the DeviceManager with the given \a parent. There should only be one DeviceManager in the system created by \l{GuhCore}.
    Use \c GuhCore::instance()->deviceManager() instead to access the DeviceManager.
*/
DeviceManager::DeviceManager(QObject *parent) :
    QObject(parent),
    m_radio433(0)
{
    qRegisterMetaType<DeviceClassId>();
    qRegisterMetaType<DeviceDescriptor>();

    m_pluginTimer.setInterval(15000);
    connect(&m_pluginTimer, &QTimer::timeout, this, &DeviceManager::timerEvent);

    m_settingsFile = QCoreApplication::instance()->organizationName() + "/devices";

    // Give hardware a chance to start up before loading plugins etc.
    QMetaObject::invokeMethod(this, "loadPlugins", Qt::QueuedConnection);
    QMetaObject::invokeMethod(this, "loadConfiguredDevices", Qt::QueuedConnection);
    QMetaObject::invokeMethod(this, "startMonitoringAutoDevices", Qt::QueuedConnection);
    // Make sure this is always emitted after plugins and devices are loaded
    QMetaObject::invokeMethod(this, "loaded", Qt::QueuedConnection);

    m_radio433 = new Radio433(this);
    connect(m_radio433, &Radio433::dataReceived, this, &DeviceManager::radio433SignalReceived);
    m_radio433->enable();
    // TODO: error handling if no Radio433 detected (GPIO or network), disable radio433 plugins or something...
}

DeviceManager::~DeviceManager()
{
    qDebug() << "Shutting down DeviceManager";
    foreach (DevicePlugin *plugin, m_devicePlugins) {
        delete plugin;
    }
}

/*! Returns all the \l{DevicePlugin}{DevicePlugins} loaded in the system. */
QList<DevicePlugin *> DeviceManager::plugins() const
{
    return m_devicePlugins.values();
}

/*! Returns the \{DevicePlugin} with the given \a id. Null if the id couldn't be found. */
DevicePlugin *DeviceManager::plugin(const PluginId &id) const
{
    return m_devicePlugins.value(id);
}

DeviceManager::DeviceError DeviceManager::setPluginConfig(const PluginId &pluginId, const ParamList &pluginConfig)
{
    DevicePlugin *plugin = m_devicePlugins.value(pluginId);
    if (!plugin) {
        return DeviceErrorPluginNotFound;
    }
    DeviceError result = plugin->setConfiguration(pluginConfig);
    if (result != DeviceErrorNoError) {
        return result;
    }
    QSettings settings;
    settings.beginGroup("PluginConfig");
    settings.beginGroup(plugin->pluginId().toString());
    foreach (const Param &param, pluginConfig) {
        settings.setValue(param.name(), param.value());
    }
    settings.endGroup();
    settings.endGroup();
    return result;
}

QList<Vendor> DeviceManager::supportedVendors() const
{
    return m_supportedVendors.values();
}

/*! Returns all the supported \l{DeviceClass}{DeviceClasses} by all \l{DevicePlugin}{DevicePlugins} loaded in the system.
    Optionally filtered by vendorId. */
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

DeviceManager::DeviceError DeviceManager::discoverDevices(const DeviceClassId &deviceClassId, const ParamList &params)
{
    qDebug() << "DeviceManager discoverdevices" << params;
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
    QPair<DeviceError, QString> ret = plugin->discoverDevices(deviceClassId, effectiveParams);
    if (ret.first != DeviceErrorAsync) {
        m_discoveringPlugins.removeOne(plugin);
    }
    return ret;
}

/*! Add a new configured device for the given \l{DeviceClass} and the given parameters.
 \a deviceClassId must refer to an existing \{DeviceClass} and \a params must match the parameter description in the \l{DeviceClass}.
    Optionally you can supply an id yourself if you must keep track of the added device. If you don't supply it, a new one will
    be generated. Only devices with \l{DeviceClass::CreateMethodUser} can be created using this method.
    Returns \l{DeviceManager::DeviceError} to inform about the result.
    \list
    \li DeviceManager::DeviceErrorNoError: The device has been created, set up and added to the list of configured devices correctly.
    \li DeviceManager::DeviceErrorAsync: The device has been created, but the setup requires async operations. For instance a network query.
    In this case, you should wait for the \l{deviceSetupFinished()} signal to get the final results.
    \li DeviceManager::DeviceErrorCreateMethodNotSupported: The deviceId you've chosen refers to a DeviceClass which can't be created manually.
    \li DeviceManager::DeviceErrorDeviceClassNotFound: The given deviceClassId can't be found in the list of supported devices.
    \li DeviceManager::DeviceErrorMissingParameter: The given params do not suffice for the given deviceClassId.
    \li DeviceManager::DeviceErrorDeviceParameterError: The given params can't be matched to the ParamTypes for the given deviceClassId.
    \li DeviceManager::DeviceErrorDuplicateUuid: The given uuid already exists.
    \li DeviceManager::DeviceErrorPluginNotFound: Couldn't find a plugin that handles this deviceClassId.
    \li DeviceManager::DeviceErrorSetupFailed: The device couldn't be set up. Even though you supplied all the parameters correctly, something
    went wrong during setup. Reasons may be a hardware/network failure, wrong username/password or similar, depending on what the device plugin
    needs to do in order to set up the device.
*/
DeviceManager::DeviceError DeviceManager::addConfiguredDevice(const DeviceClassId &deviceClassId, const ParamList &params, const DeviceId id)
{
    DeviceClass deviceClass = findDeviceClass(deviceClassId);
    if (!deviceClass.isValid()) {
        return DeviceErrorDeviceClassNotFound;
    }
    if (deviceClass.createMethods().testFlag(DeviceClass::CreateMethodUser)) {
        return addConfiguredDeviceInternal(deviceClassId, params, id);
    }
    return DeviceErrorCreationMethodNotSupported;
}

DeviceManager::DeviceError DeviceManager::addConfiguredDevice(const DeviceClassId &deviceClassId, const DeviceDescriptorId &deviceDescriptorId, const DeviceId &deviceId)
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

    return addConfiguredDeviceInternal(deviceClassId, descriptor.params(), deviceId);
}

DeviceManager::DeviceError DeviceManager::pairDevice(const PairingTransactionId &pairingTransactionId, const DeviceClassId &deviceClassId, const ParamList &params)
{
    DeviceClass deviceClass = findDeviceClass(deviceClassId);
    if (deviceClass.id().isNull()) {
        qWarning() << "cannot find a device class with id" << deviceClassId;
        return DeviceErrorDeviceClassNotFound;
    }

    switch (deviceClass.setupMethod()) {
    case DeviceClass::SetupMethodJustAdd:
        qWarning() << "Cannot setup this device this way. No need to pair this device.";
        return DeviceErrorSetupMethodNotSupported;
    case DeviceClass::SetupMethodDisplayPin:
        qWarning() << "SetupMethodDisplayPin not implemented yet for this CreateMethod";
        return DeviceErrorSetupFailed;
    case DeviceClass::SetupMethodEnterPin:
        qWarning() << "SetupMethodEnterPin not implemented yet for this CreateMethod";
        return DeviceErrorSetupFailed;
    case DeviceClass::SetupMethodPushButton:
        qWarning() << "SetupMethodPushButton not implemented yet for this CreateMethod";
        return DeviceErrorSetupFailed;
    }

    return DeviceErrorNoError;
}

DeviceManager::DeviceError DeviceManager::pairDevice(const PairingTransactionId &pairingTransactionId, const DeviceClassId &deviceClassId, const DeviceDescriptorId &deviceDescriptorId)
{
    DeviceClass deviceClass = findDeviceClass(deviceClassId);
    if (deviceClass.id().isNull()) {
        qWarning() << "cannot find a device class with id" << deviceClassId;
        return DeviceErrorDeviceClassNotFound;
    }

    if (deviceClass.setupMethod() == DeviceClass::SetupMethodJustAdd) {
        qWarning() << "Cannot setup this device this way. No need to pair this device.";
        return DeviceErrorCreationMethodNotSupported;
    }

    if (!m_discoveredDevices.contains(deviceDescriptorId)) {
        qWarning() << "Cannot find a DeviceDescriptor with ID" << deviceClassId.toString();
        return DeviceErrorDeviceDescriptorNotFound;
    }

    m_pairingsDiscovery.insert(pairingTransactionId, qMakePair<DeviceClassId, DeviceDescriptorId>(deviceClassId, deviceDescriptorId));

    if (deviceClass.setupMethod() == DeviceClass::SetupMethodDisplayPin) {
        // TODO: fetch PIN from device plugin
        qWarning() << "SetupMethodDisplayPin not implemented yet";
        return DeviceErrorSetupFailed;
    }

    return DeviceErrorNoError;
}

DeviceManager::DeviceError DeviceManager::confirmPairing(const PairingTransactionId &pairingTransactionId, const QString &secret)
{
    if (m_pairingsJustAdd.contains(pairingTransactionId)) {
        qWarning() << "this SetupMethod is not implemented yet";
        m_pairingsJustAdd.remove(pairingTransactionId);
        return DeviceErrorSetupFailed;
    }

    if (m_pairingsDiscovery.contains(pairingTransactionId)) {
        DeviceDescriptorId deviceDescriptorId = m_pairingsDiscovery.value(pairingTransactionId).second;
        DeviceClassId deviceClassId = m_pairingsDiscovery.value(pairingTransactionId).first;

        DeviceDescriptor deviceDescriptor = m_discoveredDevices.value(deviceDescriptorId);

        DevicePlugin *plugin = m_devicePlugins.value(m_supportedDevices.value(deviceClassId).pluginId());

        if (!plugin) {
            qWarning() << "Can't find a plugin for this device class";
            return DeviceErrorPluginNotFound;
        }

        DeviceSetupStatus status = plugin->confirmPairing(pairingTransactionId, deviceClassId, deviceDescriptor.params());
        switch (status) {
        case DeviceSetupStatusSuccess:
            m_pairingsDiscovery.remove(pairingTransactionId);
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

DeviceManager::DeviceError DeviceManager::addConfiguredDeviceInternal(const DeviceClassId &deviceClassId, const ParamList &params, const DeviceId id)
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
    device->setName(deviceClass.name());
    device->setParams(effectiveParams);

    DeviceSetupStatus status = setupDevice(device);
    switch (status) {
    case DeviceSetupStatusFailure:
        qWarning() << "Device setup failed. Not adding device to system.";
        delete device;
        return DeviceErrorSetupFailed;
    case DeviceSetupStatusAsync:
        return DeviceErrorAsync;
    case DeviceSetupStatusSuccess:
        qDebug() << "Device setup complete.";
        break;
    }

    m_configuredDevices.append(device);
    storeConfiguredDevices();

    return DeviceErrorNoError;
}

DeviceManager::DeviceError DeviceManager::removeConfiguredDevice(const DeviceId &deviceId)
{
    Device *device = findConfiguredDevice(deviceId);
    if (!device) {
        return DeviceErrorDeviceNotFound;
    }

    m_configuredDevices.removeAll(device);
    m_devicePlugins.value(device->pluginId())->deviceRemoved(device);

    m_pluginTimerUsers.removeAll(device);
    if (m_pluginTimerUsers.isEmpty()) {
        m_pluginTimer.stop();
    }

    device->deleteLater();

    QSettings settings(m_settingsFile);
    settings.beginGroup("DeviceConfig");
    settings.beginGroup(deviceId.toString());
    settings.remove("");
    settings.endGroup();

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

/*! For conveninece, this returns the \{DeviceClass} with the id given by \a deviceClassId.
    Note: The returned DeviceClass may be invalid.*/
DeviceClass DeviceManager::findDeviceClass(const DeviceClassId &deviceClassId) const
{
    foreach (const DeviceClass &deviceClass, m_supportedDevices) {
        if (deviceClass.id() == deviceClassId) {
            return deviceClass;
        }
    }
    return DeviceClass();
}

/*! Execute the given \{Action}.
    This will find the \l{Device} \a action refers to in \l{Action::deviceId()} and
    its \l{DevicePlugin}. Then will dispatch the execution to the \l{DevicePlugin}.*/
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
    foreach (const QStaticPlugin &staticPlugin, QPluginLoader::staticPlugins()) {
        DevicePlugin *pluginIface = qobject_cast<DevicePlugin*>(staticPlugin.instance());
        if (verifyPluginMetadata(staticPlugin.metaData().value("MetaData").toObject()) && pluginIface) {
            pluginIface->initPlugin(staticPlugin.metaData().value("MetaData").toObject(), this);
            qDebug() << "*** Loaded plugin" << pluginIface->pluginName();
            foreach (const Vendor &vendor, pluginIface->supportedVendors()) {
                qDebug() << "* Loaded vendor:" << vendor.name();
                if (m_supportedVendors.contains(vendor.id())) {
                    qWarning() << "! Duplicate vendor. Ignoring vendor" << vendor.name();
                    continue;
                }
                m_supportedVendors.insert(vendor.id(), vendor);
            }

            foreach (const DeviceClass &deviceClass, pluginIface->supportedDevices()) {
                if (!m_supportedVendors.contains(deviceClass.vendorId())) {
                    qWarning() << "! Vendor not found. Ignoring device. VendorId:" << deviceClass.vendorId() << "DeviceClass:" << deviceClass.name() << deviceClass.id();
                    continue;
                }
                m_vendorDeviceMap[deviceClass.vendorId()].append(deviceClass.id());
                m_supportedDevices.insert(deviceClass.id(), deviceClass);
                qDebug() << "* Loaded device class:" << deviceClass.name();
            }
            QSettings settings(m_settingsFile);
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
            DeviceError status = pluginIface->setConfiguration(params);
            if (status != DeviceErrorNoError) {
                qWarning() << "Error setting params to plugin. Broken configuration?";
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

void DeviceManager::loadConfiguredDevices()
{
    QSettings settings(m_settingsFile);
    settings.beginGroup("DeviceConfig");
    qDebug() << "loading devices from" << settings.fileName();
    foreach (const QString &idString, settings.childGroups()) {
        settings.beginGroup(idString);
        Device *device = new Device(PluginId(settings.value("pluginid").toString()), DeviceId(idString), DeviceClassId(settings.value("deviceClassId").toString()), this);
        device->setName(settings.value("devicename").toString());

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
        setupDevice(device);
        m_configuredDevices.append(device);
    }
    settings.endGroup();
}

void DeviceManager::storeConfiguredDevices()
{
    QSettings settings(m_settingsFile);
    settings.beginGroup("DeviceConfig");
    foreach (Device *device, m_configuredDevices) {
        settings.beginGroup(device->id().toString());
        settings.setValue("devicename", device->name());
        settings.setValue("deviceClassId", device->deviceClassId().toString());
        settings.setValue("pluginid", device->pluginId().toString());
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
        qWarning() << "Received deviceSetupFinished for an invalid device... ignoring...";
        return;
    }

    if (device->setupComplete()) {
        qWarning() << "Received a deviceSetupFinished event, but this Device has been set up before... ignoring...";
        return;
    }

    Q_ASSERT_X(status != DeviceSetupStatusAsync, "DeviceManager", "Bad plugin implementation. You should not emit deviceSetupFinished with status DeviceSetupStatusAsync.");
    if (status == DeviceSetupStatusAsync) {
        qWarning() << "Bad plugin implementation. Received a deviceSetupFinished event with status Async... ignoring...";
        return;
    }

    if (status == DeviceSetupStatusFailure) {
        if (m_configuredDevices.contains(device)) {
            qWarning() << QString("Error in device setup. Device %1 (%2) will not be functional.").arg(device->name()).arg(device->id().toString());
            emit deviceSetupFinished(device, DeviceError::DeviceErrorSetupFailed);
            return;
        } else {
            qWarning() << QString("Error in device setup. Device %1 (%2) will not be added to the configured devices.").arg(device->name()).arg(device->id().toString());
            emit deviceSetupFinished(device, DeviceError::DeviceErrorSetupFailed);
            return;
        }
    }

    // A device might be in here already if loaded from storedDevices. If it's not in the configuredDevices,
    // lets add it now.
    if (!m_configuredDevices.contains(device)) {
        m_configuredDevices.append(device);
        storeConfiguredDevices();
    }

    DevicePlugin *plugin = m_devicePlugins.value(device->pluginId());
    if (plugin->requiredHardware().testFlag(HardwareResourceTimer)) {
        if (!m_pluginTimer.isActive()) {
            m_pluginTimer.start();
            // Additionally fire off one event to initialize stuff
            QTimer::singleShot(0, this, SLOT(timerEvent()));
        }
        m_pluginTimerUsers.append(device);
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
            qWarning() << "Received a pairing finished without waiting for it from plugin:" << plugin->metaObject()->className();
        } else {
            qWarning() << "Received a pairing finished without waiting for it.";
        }
        return;
    }

    DeviceClassId deviceClassId;
    ParamList params;

    // Do this before checking status to make sure we clean up our hashes properly
    if (m_pairingsJustAdd.contains(pairingTransactionId)) {
        QPair<DeviceClassId, ParamList> pair = m_pairingsJustAdd.take(pairingTransactionId);
        deviceClassId = pair.first;
        params = pair.second;
    }

    if (m_pairingsDiscovery.contains(pairingTransactionId)) {
        QPair<DeviceClassId, DeviceDescriptorId> pair = m_pairingsDiscovery.take(pairingTransactionId);

        DeviceDescriptorId deviceDescriptorId = pair.second;
        DeviceDescriptor descriptor = m_discoveredDevices.take(deviceDescriptorId);

        deviceClassId = pair.first;
        params = descriptor.params();
    }

    if (status != DeviceSetupStatusSuccess) {
        emit pairingFinished(pairingTransactionId, DeviceErrorSetupFailed);
        return;
    }

    DeviceClass deviceClass = findDeviceClass(deviceClassId);
    DevicePlugin *plugin = m_devicePlugins.value(deviceClass.pluginId());
    if (!plugin) {
        qWarning() << "Cannot find a plugin for this device class!";
        emit pairingFinished(pairingTransactionId, DeviceErrorPluginNotFound, deviceClass.pluginId().toString());
        return;
    }

    // Ok... pairing went fine... Let consumers know about it and inform them about the ongoing setup with a deviceId.
    DeviceId id = DeviceId::createDeviceId();
    emit pairingFinished(pairingTransactionId, DeviceErrorNoError, id);

    QList<DeviceId> newDevices;
    Device *device = new Device(plugin->pluginId(), id, deviceClassId, this);
    device->setName(deviceClass.name());
    device->setParams(params);

    DeviceSetupStatus setupStatus = setupDevice(device);
    switch (setupStatus) {
    case DeviceSetupStatusFailure:
        qWarning() << "Device setup failed. Not adding device to system.";
        emit deviceSetupFinished(device, DeviceError::DeviceErrorSetupFailed);
        delete device;
        break;
    case DeviceSetupStatusAsync:
        return;
    case DeviceSetupStatusSuccess:
        qDebug() << "Device setup complete.";
        newDevices.append(id);
        break;
    }

    m_configuredDevices.append(device);
    storeConfiguredDevices();

    emit deviceSetupFinished(device, DeviceError::DeviceErrorNoError);
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
            qWarning() << "Device setup failed. Not adding device to system.";
            emit deviceSetupFinished(device, DeviceError::DeviceErrorSetupFailed);
            delete device;
            break;
        case DeviceSetupStatusAsync:
            break;
        case DeviceSetupStatusSuccess:
            qDebug() << "Device setup complete.";
            emit deviceSetupFinished(device, DeviceError::DeviceErrorNoError);
            m_configuredDevices.append(device);
            storeConfiguredDevices();
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
    Event event(EventTypeId(stateTypeId.toString()), device->id(), ParamList() << valueParam);
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

void DeviceManager::timerEvent()
{
    foreach (Device *device, m_configuredDevices) {
        DeviceClass deviceClass = m_supportedDevices.value(device->deviceClassId());
        DevicePlugin *plugin = m_devicePlugins.value(deviceClass.pluginId());
        if (plugin && plugin->requiredHardware().testFlag(HardwareResourceTimer)) {
            plugin->guhTimer();
        }
    }
}

bool DeviceManager::verifyPluginMetadata(const QJsonObject &data)
{
    QStringList requiredFields;
    requiredFields << "name" << "id" << "vendors";

    foreach (const QString &field, requiredFields) {
        if (!data.contains("name")) {
            qWarning() << "Error loading plugin. Incomplete metadata. Missing field:" << field;
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
        qWarning() << "Can't find a plugin for this device" << device->id();
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
        m_pluginTimerUsers.append(device);
    }

    connect(device, SIGNAL(stateValueChanged(QUuid,QVariant)), this, SLOT(slotDeviceStateValueChanged(QUuid,QVariant)));

    device->setupCompleted();
    return status;
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
        if (!paramType.defaultValue().isNull()) {
            found = true;
            params.append(Param(paramType.name(), paramType.defaultValue()));
        }

        if (!found) {
            qWarning() << "Missing parameter:" << paramType.name();
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
    qWarning() << "Invalid parameter" << param.name() << "in parameter list";
    return DeviceErrorInvalidParameter;
}

DeviceManager::DeviceError DeviceManager::verifyParam(const ParamType &paramType, const Param &param)
{
    if (paramType.name() == param.name()) {
        if (!param.value().canConvert(paramType.type())) {
            qWarning() << "Wrong parameter type for param" << param.name() << " Got:" << param.value() << " Expected:" << QVariant::typeToName(paramType.type());
            return DeviceErrorInvalidParameter;
        }

        if (paramType.maxValue().isValid() && param.value() > paramType.maxValue()) {
            qWarning() << "Value out of range for param" << param.name() << " Got:" << param.value() << " Max:" << paramType.maxValue();
            return DeviceErrorInvalidParameter;
        }
        if (paramType.minValue().isValid() && param.value() < paramType.minValue()) {
            qWarning() << "Value out of range for param" << param.name() << " Got:" << param.value() << " Min:" << paramType.minValue();
            return DeviceErrorInvalidParameter;
        }
        if (!paramType.allowedValues().isEmpty() && !paramType.allowedValues().contains(param.value())) {
            QStringList allowedValues;
            foreach (const QVariant &value, paramType.allowedValues()) {
                allowedValues.append(value.toString());
            }

            qWarning() << "Value not in allowed values for param" << param.name() << " Got:" << param.value() << " Allowed:" << allowedValues.join(",");
            return DeviceErrorInvalidParameter;
        }
        return DeviceErrorNoError;
    }
    qWarning() << "Parameter name" << param.name() << "does not match with ParamType name" << paramType.name();
    return DeviceErrorInvalidParameter;
}
