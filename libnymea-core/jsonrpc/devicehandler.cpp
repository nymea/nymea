/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
 *                                                                         *
 *  This file is part of nymea.                                            *
 *                                                                         *
 *  nymea is free software: you can redistribute it and/or modify          *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  nymea is distributed in the hope that it will be useful,               *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with nymea. If not, see <http://www.gnu.org/licenses/>.          *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*!
    \class nymeaserver::DeviceHandler
    \brief This subclass of \l{JsonHandler} processes the JSON requests for the \tt Devices namespace of the JSON-RPC API.

    \ingroup json
    \inmodule core

    This \l{JsonHandler} will be created in the \l{JsonRPCServer} and used to handle JSON-RPC requests
    for the \tt {Devices} namespace of the API.

    \sa Device, JsonHandler, JsonRPCServer
*/


/*! \fn void nymeaserver::DeviceHandler::StateChanged(const QVariantMap &params);
    This signal is emitted to the API notifications when a \l{State} has changed.
    The \a params contain the map for the notification.
*/

/*! \fn void nymeaserver::DeviceHandler::DeviceRemoved(const QVariantMap &params);
    This signal is emitted to the API notifications when a \l{Device} has been removed.
    The \a params contain the map for the notification.
*/

/*! \fn void nymeaserver::DeviceHandler::DeviceAdded(const QVariantMap &params);
    This signal is emitted to the API notifications when a \l{Device} has been added.
    The \a params contain the map for the notification.
*/

/*! \fn void nymeaserver::DeviceHandler::DeviceChanged(const QVariantMap &params);
    This signal is emitted to the API notifications when a \l{Device} has been changed or reconfigured.
    The \a params contain the map for the notification.
*/

#include "devicehandler.h"
#include "nymeacore.h"
#include "devices/devicemanager.h"
#include "devices/device.h"
#include "devices/deviceplugin.h"
#include "loggingcategories.h"
#include "types/deviceclass.h"
#include "devices/translator.h"

#include <QDebug>

namespace nymeaserver {

/*! Constructs a new \l DeviceHandler with the given \a parent. */
DeviceHandler::DeviceHandler(QObject *parent) :
    JsonHandler(parent)
{
    QVariantMap returns;
    QVariantMap params;

    params.clear(); returns.clear();
    setDescription("GetSupportedVendors", "Returns a list of supported Vendors.");
    setParams("GetSupportedVendors", params);
    QVariantList vendors;
    vendors.append(JsonTypes::vendorRef());
    returns.insert("vendors", vendors);
    setReturns("GetSupportedVendors", returns);

    params.clear(); returns.clear();
    setDescription("GetSupportedDevices", "Returns a list of supported Device classes, optionally filtered by vendorId.");
    params.insert("o:vendorId", JsonTypes::basicTypeToString(JsonTypes::Uuid));
    setParams("GetSupportedDevices", params);
    QVariantList deviceClasses;
    deviceClasses.append(JsonTypes::deviceClassRef());
    returns.insert("deviceClasses", deviceClasses);
    setReturns("GetSupportedDevices", returns);

    params.clear(); returns.clear();
    setDescription("GetPlugins", "Returns a list of loaded plugins.");
    setParams("GetPlugins", params);
    QVariantList plugins;
    plugins.append(JsonTypes::pluginRef());
    returns.insert("plugins", plugins);
    setReturns("GetPlugins", returns);

    params.clear(); returns.clear();
    setDescription("GetPluginConfiguration", "Get a plugin's params.");
    params.insert("pluginId", JsonTypes::basicTypeToString(JsonTypes::Uuid));
    setParams("GetPluginConfiguration", params);
    QVariantList pluginParams;
    pluginParams.append(JsonTypes::paramRef());
    returns.insert("deviceError", JsonTypes::deviceErrorRef());
    returns.insert("o:configuration", pluginParams);
    setReturns("GetPluginConfiguration", returns);

    params.clear(); returns.clear();
    setDescription("SetPluginConfiguration", "Set a plugin's params.");
    params.insert("pluginId", JsonTypes::basicTypeToString(JsonTypes::Uuid));
    params.insert("configuration", pluginParams);
    setParams("SetPluginConfiguration", params);
    returns.insert("deviceError", JsonTypes::deviceErrorRef());
    setReturns("SetPluginConfiguration", returns);

    params.clear(); returns.clear();
    setDescription("AddConfiguredDevice", "Add a configured device with a setupMethod of SetupMethodJustAdd. "
                                          "For devices with a setupMethod different than SetupMethodJustAdd, use PairDevice. "
                                          "Devices with CreateMethodJustAdd require all parameters to be supplied here. "
                                          "Devices with CreateMethodDiscovery require the use of a deviceDescriptorId. For discovered "
                                          "devices params are not required and will be taken from the DeviceDescriptor, however, they "
                                          "may be overridden by supplying parameters here."
                   );
    params.insert("deviceClassId", JsonTypes::basicTypeToString(JsonTypes::Uuid));
    params.insert("name", JsonTypes::basicTypeToString(JsonTypes::String));
    params.insert("o:deviceDescriptorId", JsonTypes::basicTypeToString(JsonTypes::Uuid));
    QVariantList deviceParams;
    deviceParams.append(JsonTypes::paramRef());
    params.insert("o:deviceParams", deviceParams);
    setParams("AddConfiguredDevice", params);
    returns.insert("deviceError", JsonTypes::deviceErrorRef());
    returns.insert("o:deviceId", JsonTypes::basicTypeToString(JsonTypes::Uuid));
    setReturns("AddConfiguredDevice", returns);

    returns.clear(); // Reused params from above!
    setDescription("PairDevice", "Pair a device. "
                                 "Use this for DeviceClasses with a setupMethod different than SetupMethodJustAdd. "
                                 "Use deviceDescriptorId or deviceParams, depending on the createMethod of the device class. "
                                 "CreateMethodJustAdd takes the parameters you want to have with that device. "
                                 "CreateMethodDiscovery requires the use of a deviceDescriptorId, optionally, parameters can be overridden here. "
                                 "If success is true, the return values will contain a pairingTransactionId, a displayMessage and "
                                 "the setupMethod. Depending on the setupMethod you should either proceed with AddConfiguredDevice "
                                 "or PairDevice."
                   );
    setParams("PairDevice", params);
    returns.insert("deviceError", JsonTypes::deviceErrorRef());
    returns.insert("o:pairingTransactionId", JsonTypes::basicTypeToString(JsonTypes::Uuid));
    returns.insert("o:displayMessage", JsonTypes::basicTypeToString(JsonTypes::String));
    returns.insert("o:setupMethod", JsonTypes::setupMethodRef());
    returns.insert("o:oAuthUrl", JsonTypes::basicTypeToString(JsonTypes::String));
    setReturns("PairDevice", returns);

    params.clear(); returns.clear();
    setDescription("ConfirmPairing", "Confirm an ongoing pairing. For SetupMethodUserAndPassword, provide the username in the \"username\" field "
                                     "and the password in the \"secret\" field. For SetupMethodEnterPin and provide the PIN in the \"secret\" "
                                     "field. For SetupMethodOAuth, return the entire unmodified callback URL containing the code parameter back in the secret field.");
    params.insert("pairingTransactionId", JsonTypes::basicTypeToString(JsonTypes::Uuid));
    params.insert("o:username", JsonTypes::basicTypeToString(JsonTypes::String));
    params.insert("o:secret", JsonTypes::basicTypeToString(JsonTypes::String));
    setParams("ConfirmPairing", params);
    returns.insert("deviceError", JsonTypes::deviceErrorRef());
    returns.insert("o:displayMessage", JsonTypes::basicTypeToString(JsonTypes::String));
    returns.insert("o:deviceId", JsonTypes::basicTypeToString(JsonTypes::Uuid));
    setReturns("ConfirmPairing", returns);

    params.clear(); returns.clear();
    setDescription("GetConfiguredDevices", "Returns a list of configured devices, optionally filtered by deviceId.");
    params.insert("o:deviceId", JsonTypes::basicTypeToString(JsonTypes::Uuid));
    setParams("GetConfiguredDevices", params);
    QVariantList devices;
    devices.append(JsonTypes::deviceRef());
    returns.insert("devices", devices);
    setReturns("GetConfiguredDevices", returns);

    params.clear(); returns.clear();
    setDescription("GetDiscoveredDevices", "Performs a device discovery and returns the results. This function may take a while to return. "
                                           "Note that this method will include all the found devices, that is, including devices that may "
                                           "already have been added. Those devices will have deviceId set to the device id of the already "
                                           "added device. Such results may be used to reconfigure existing devices and might be filtered "
                                           "in cases where only unknown devices are of interest.");
    params.insert("deviceClassId", JsonTypes::basicTypeToString(JsonTypes::Uuid));
    QVariantList discoveryParams;
    discoveryParams.append(JsonTypes::paramRef());
    params.insert("o:discoveryParams", discoveryParams);
    setParams("GetDiscoveredDevices", params);
    returns.insert("deviceError", JsonTypes::deviceErrorRef());
    QVariantList deviceDescriptors;
    deviceDescriptors.append(JsonTypes::deviceDescriptorRef());
    returns.insert("o:deviceDescriptors", deviceDescriptors);
    setReturns("GetDiscoveredDevices", returns);

    params.clear(); returns.clear();
    setDescription("ReconfigureDevice", "Edit the parameter configuration of the device. The device params will be set to the "
                                        "passed parameters and the setup device will be called. If the device is discoverable, "
                                        "you can perform a GetDiscoveredDevices before calling this method and pass "
                                        "the new DeviceDescriptor (rediscover). Only writable parameters can be changed. By default, "
                                        "every Param is writable.");
    params.insert("deviceId", JsonTypes::basicTypeToString(JsonTypes::Uuid));
    params.insert("o:deviceDescriptorId", JsonTypes::basicTypeToString(JsonTypes::Uuid));
    QVariantList newDeviceParams;
    newDeviceParams.append(JsonTypes::paramRef());
    params.insert("o:deviceParams", newDeviceParams);
    setParams("ReconfigureDevice", params);
    returns.insert("deviceError", JsonTypes::deviceErrorRef());
    setReturns("ReconfigureDevice", returns);

    params.clear(); returns.clear();
    setDescription("EditDevice", "Edit the name of a device. This method does not change the "
                                 "configuration of the device.");
    params.insert("deviceId", JsonTypes::basicTypeToString(JsonTypes::Uuid));
    params.insert("name", JsonTypes::basicTypeToString(JsonTypes::String));
    setParams("EditDevice", params);
    returns.insert("deviceError", JsonTypes::deviceErrorRef());
    setReturns("EditDevice", returns);

    params.clear(); returns.clear();
    setDescription("SetDeviceSettings", "Change the settings of a device.");
    params.insert("deviceId", JsonTypes::basicTypeToString(JsonTypes::Uuid));
    params.insert("settings", QVariantList() << JsonTypes::paramRef());
    setParams("SetDeviceSettings", params);
    returns.insert("deviceError", JsonTypes::deviceErrorRef());
    setReturns("SetDeviceSettings", returns);

    params.clear(); returns.clear();
    setDescription("RemoveConfiguredDevice", "Remove a device from the system.");
    params.insert("deviceId", JsonTypes::basicTypeToString(JsonTypes::Uuid));
    QVariantList removePolicyList;
    QVariantMap policy;
    policy.insert("ruleId", JsonTypes::basicTypeToString(JsonTypes::Uuid));
    policy.insert("policy", JsonTypes::removePolicyRef());
    removePolicyList.append(policy);
    params.insert("o:removePolicy", JsonTypes::removePolicyRef());
    params.insert("o:removePolicyList", removePolicyList);
    setParams("RemoveConfiguredDevice", params);
    returns.insert("deviceError", JsonTypes::deviceErrorRef());
    returns.insert("o:ruleIds", QVariantList() << JsonTypes::basicTypeToString(JsonTypes::Uuid));
    setReturns("RemoveConfiguredDevice", returns);

    params.clear(); returns.clear();
    setDescription("GetEventTypes", "Get event types for a specified deviceClassId.");
    params.insert("deviceClassId", JsonTypes::basicTypeToString(JsonTypes::Uuid));
    setParams("GetEventTypes", params);
    QVariantList events;
    events.append(JsonTypes::eventTypeRef());
    returns.insert("eventTypes", events);
    setReturns("GetEventTypes", returns);

    params.clear(); returns.clear();
    setDescription("GetActionTypes", "Get action types for a specified deviceClassId.");
    params.insert("deviceClassId", JsonTypes::basicTypeToString(JsonTypes::Uuid));
    setParams("GetActionTypes", params);
    QVariantList actions;
    actions.append(JsonTypes::actionTypeRef());
    returns.insert("actionTypes", actions);
    setReturns("GetActionTypes", returns);

    params.clear(); returns.clear();
    setDescription("GetStateTypes", "Get state types for a specified deviceClassId.");
    params.insert("deviceClassId", JsonTypes::basicTypeToString(JsonTypes::Uuid));
    setParams("GetStateTypes", params);
    QVariantList states;
    states.append(JsonTypes::stateTypeRef());
    returns.insert("stateTypes", states);
    setReturns("GetStateTypes", returns);

    params.clear(); returns.clear();
    setDescription("GetStateValue", "Get the value of the given device and the given stateType");
    params.insert("deviceId", JsonTypes::basicTypeToString(JsonTypes::Uuid));
    params.insert("stateTypeId", JsonTypes::basicTypeToString(JsonTypes::Uuid));
    setParams("GetStateValue", params);
    returns.insert("deviceError", JsonTypes::deviceErrorRef());
    returns.insert("o:value", JsonTypes::basicTypeToString(JsonTypes::Variant));
    setReturns("GetStateValue", returns);

    params.clear(); returns.clear();
    setDescription("GetStateValues", "Get all the state values of the given device.");
    params.insert("deviceId", JsonTypes::basicTypeToString(JsonTypes::Uuid));
    setParams("GetStateValues", params);
    returns.insert("deviceError", JsonTypes::deviceErrorRef());
    states.clear();
    QVariantMap state;
    state.insert("stateTypeId", JsonTypes::basicTypeToString(JsonTypes::Uuid));
    state.insert("value", JsonTypes::basicTypeToString(JsonTypes::Variant));
    states.append(state);
    returns.insert("o:values", states);
    setReturns("GetStateValues", returns);

    params.clear(); returns.clear();
    setDescription("BrowseDevice", "Browse a device. If a DeviceClass indicates a device is browsable, this method will return the BrowserItems. If no parameter besides the deviceId is used, the root node of this device will be returned. Any returned item which is browsable can be passed as node. Results will be children of the given node.");
    params.insert("deviceId", JsonTypes::basicTypeToString(JsonTypes::Uuid));
    params.insert("o:itemId", JsonTypes::basicTypeToString(JsonTypes::String));
    setParams("BrowseDevice", params);
    returns.insert("deviceError", JsonTypes::deviceErrorRef());
    returns.insert("items", QVariantList() << JsonTypes::browserItemRef());
    setReturns("BrowseDevice", returns);

    params.clear(); returns.clear();
    setDescription("GetBrowserItem", "Get a single item from the browser. This won't give any more info on an item than a regular browseDevice call, but it allows to fetch details of an item if only the ID is known.");
    params.insert("deviceId", JsonTypes::basicTypeToString(JsonTypes::Uuid));
    params.insert("o:itemId", JsonTypes::basicTypeToString(JsonTypes::String));
    setParams("GetBrowserItem", params);
    returns.insert("deviceError", JsonTypes::deviceErrorRef());
    returns.insert("o:item", JsonTypes::browserItemRef());
    setReturns("GetBrowserItem", returns);

    // Notifications
    params.clear(); returns.clear();
    setDescription("StateChanged", "Emitted whenever a State of a device changes.");
    params.insert("deviceId", JsonTypes::basicTypeToString(JsonTypes::Uuid));
    params.insert("stateTypeId", JsonTypes::basicTypeToString(JsonTypes::Uuid));
    params.insert("value", JsonTypes::basicTypeToString(JsonTypes::Variant));
    setParams("StateChanged", params);

    params.clear(); returns.clear();
    setDescription("DeviceRemoved", "Emitted whenever a Device was removed.");
    params.insert("deviceId", JsonTypes::basicTypeToString(JsonTypes::Uuid));
    setParams("DeviceRemoved", params);

    params.clear(); returns.clear();
    setDescription("DeviceAdded", "Emitted whenever a Device was added.");
    params.insert("device", JsonTypes::deviceRef());
    setParams("DeviceAdded", params);

    params.clear(); returns.clear();
    setDescription("DeviceChanged", "Emitted whenever the params or name of a Device are changed (by EditDevice or ReconfigureDevice).");
    params.insert("device", JsonTypes::deviceRef());
    setParams("DeviceChanged", params);

    params.clear(); returns.clear();
    setDescription("DeviceSettingChanged", "Emitted whenever the setting of a Device is changed.");
    params.insert("deviceId", JsonTypes::basicTypeToString(JsonTypes::Uuid));
    params.insert("paramTypeId", JsonTypes::basicTypeToString(JsonTypes::Uuid));
    params.insert("value", JsonTypes::basicTypeToString(JsonTypes::Variant));
    setParams("DeviceSettingChanged", params);

    params.clear(); returns.clear();
    setDescription("PluginConfigurationChanged", "Emitted whenever a plugin's configuration is changed.");
    params.insert("pluginId", JsonTypes::basicTypeToString(JsonTypes::Uuid));
    params.insert("configuration", QVariantList() << JsonTypes::paramRef());
    setParams("PluginConfigurationChanged", params);

    connect(NymeaCore::instance(), &NymeaCore::pluginConfigChanged, this, &DeviceHandler::pluginConfigChanged);
    connect(NymeaCore::instance(), &NymeaCore::deviceStateChanged, this, &DeviceHandler::deviceStateChanged);
    connect(NymeaCore::instance(), &NymeaCore::deviceRemoved, this, &DeviceHandler::deviceRemovedNotification);
    connect(NymeaCore::instance(), &NymeaCore::deviceAdded, this, &DeviceHandler::deviceAddedNotification);
    connect(NymeaCore::instance(), &NymeaCore::deviceChanged, this, &DeviceHandler::deviceChangedNotification);
    connect(NymeaCore::instance(), &NymeaCore::deviceSettingChanged, this, &DeviceHandler::deviceSettingChangedNotification);
    connect(NymeaCore::instance(), &NymeaCore::devicesDiscovered, this, &DeviceHandler::devicesDiscovered, Qt::QueuedConnection);
    connect(NymeaCore::instance(), &NymeaCore::deviceSetupFinished, this, &DeviceHandler::deviceSetupFinished);
    connect(NymeaCore::instance(), &NymeaCore::deviceReconfigurationFinished, this, &DeviceHandler::deviceReconfigurationFinished);
    connect(NymeaCore::instance(), &NymeaCore::pairingFinished, this, &DeviceHandler::pairingFinished);
    connect(NymeaCore::instance()->deviceManager(), &DeviceManager::browseRequestFinished, this, &DeviceHandler::browseRequestFinished);
    connect(NymeaCore::instance()->deviceManager(), &DeviceManager::browserItemRequestFinished, this, &DeviceHandler::browserItemRequestFinished);
}

/*! Returns the name of the \l{DeviceHandler}. In this case \b Devices.*/
QString DeviceHandler::name() const
{
    return "Devices";
}

JsonReply* DeviceHandler::GetSupportedVendors(const QVariantMap &params) const
{
    Q_UNUSED(params)

    QVariantMap returns;
    returns.insert("vendors", JsonTypes::packSupportedVendors(params.value("locale").toLocale()));
    return createReply(returns);
}

JsonReply* DeviceHandler::GetSupportedDevices(const QVariantMap &params) const
{
    QVariantMap returns;
    returns.insert("deviceClasses", JsonTypes::packSupportedDevices(VendorId(params.value("vendorId").toString()), params.value("locale").toLocale()));
    return createReply(returns);
}

JsonReply *DeviceHandler::GetDiscoveredDevices(const QVariantMap &params) const
{
    QVariantMap returns;

    DeviceClassId deviceClassId = DeviceClassId(params.value("deviceClassId").toString());

    ParamList discoveryParams = JsonTypes::unpackParams(params.value("discoveryParams").toList());

    Device::DeviceError status = NymeaCore::instance()->deviceManager()->discoverDevices(deviceClassId, discoveryParams);
    if (status == Device::DeviceErrorAsync ) {
        JsonReply *reply = createAsyncReply("GetDiscoveredDevices");
        connect(reply, &JsonReply::finished, this, [this, deviceClassId](){ m_discoverRequests.remove(deviceClassId); });
        m_discoverRequests.insert(deviceClassId, reply);
        return reply;
    }
    returns.insert("deviceError", JsonTypes::deviceErrorToString(status));
    return createReply(returns);
}

JsonReply* DeviceHandler::GetPlugins(const QVariantMap &params) const
{
    Q_UNUSED(params)

    QVariantMap returns;
    returns.insert("plugins", JsonTypes::packPlugins(params.value("locale").toLocale()));
    return createReply(returns);
}

JsonReply *DeviceHandler::GetPluginConfiguration(const QVariantMap &params) const
{
    QVariantMap returns;

    DevicePlugin *plugin = NymeaCore::instance()->deviceManager()->plugins().findById(PluginId(params.value("pluginId").toString()));
    if (!plugin) {
        returns.insert("deviceError", JsonTypes::deviceErrorToString(Device::DeviceErrorPluginNotFound));
        return createReply(returns);
    }

    QVariantList paramVariantList;
    foreach (const Param &param, plugin->configuration()) {
        paramVariantList.append(JsonTypes::packParam(param));
    }
    returns.insert("configuration", paramVariantList);
    returns.insert("deviceError", JsonTypes::deviceErrorToString(Device::DeviceErrorNoError));
    return createReply(returns);
}

JsonReply* DeviceHandler::SetPluginConfiguration(const QVariantMap &params)
{
    QVariantMap returns;
    PluginId pluginId = PluginId(params.value("pluginId").toString());
    ParamList pluginParams = JsonTypes::unpackParams(params.value("configuration").toList());
    Device::DeviceError result = NymeaCore::instance()->deviceManager()->setPluginConfig(pluginId, pluginParams);
    returns.insert("deviceError", JsonTypes::deviceErrorToString(result));
    return createReply(returns);
}

JsonReply* DeviceHandler::AddConfiguredDevice(const QVariantMap &params)
{
    DeviceClassId deviceClass(params.value("deviceClassId").toString());
    QString deviceName = params.value("name").toString();
    ParamList deviceParams = JsonTypes::unpackParams(params.value("deviceParams").toList());
    DeviceDescriptorId deviceDescriptorId(params.value("deviceDescriptorId").toString());
    DeviceId newDeviceId = DeviceId::createDeviceId();
    Device::DeviceError status;
    if (deviceDescriptorId.isNull()) {
        status = NymeaCore::instance()->deviceManager()->addConfiguredDevice(deviceClass, deviceName, deviceParams, newDeviceId);
    } else {
        status = NymeaCore::instance()->deviceManager()->addConfiguredDevice(deviceClass, deviceName, deviceDescriptorId, deviceParams, newDeviceId);
    }
    QVariantMap returns;
    switch (status) {
    case Device::DeviceErrorAsync: {
        JsonReply *asyncReply = createAsyncReply("AddConfiguredDevice");
        connect(asyncReply, &JsonReply::finished, [this, newDeviceId](){ m_asynDeviceAdditions.remove(newDeviceId); });
        m_asynDeviceAdditions.insert(newDeviceId, asyncReply);
        return asyncReply;
    }
    case Device::DeviceErrorNoError:
        returns.insert("deviceId", newDeviceId);
        returns.insert("deviceError", JsonTypes::deviceErrorToString(status));
        break;
    default:
        returns.insert("deviceError", JsonTypes::deviceErrorToString(status));
    }
    return createReply(returns);
}

JsonReply *DeviceHandler::PairDevice(const QVariantMap &params)
{
    DeviceClassId deviceClassId(params.value("deviceClassId").toString());
    QString deviceName = params.value("name").toString();
    DeviceClass deviceClass = NymeaCore::instance()->deviceManager()->findDeviceClass(deviceClassId);

    DevicePairingInfo pairingInfo;
    if (params.contains("deviceDescriptorId")) {
        DeviceDescriptorId deviceDescriptorId(params.value("deviceDescriptorId").toString());
        pairingInfo = NymeaCore::instance()->deviceManager()->pairDevice(deviceClassId, deviceName, deviceDescriptorId);
    } else {
        ParamList deviceParams = JsonTypes::unpackParams(params.value("deviceParams").toList());
        pairingInfo = NymeaCore::instance()->deviceManager()->pairDevice(deviceClassId, deviceName, deviceParams);
    }

    QVariantMap returns;
    returns.insert("deviceError", JsonTypes::deviceErrorToString(pairingInfo.status()));
    if (pairingInfo.status() == Device::DeviceErrorNoError) {
        returns.insert("displayMessage", NymeaCore::instance()->deviceManager()->translate(deviceClass.pluginId(), pairingInfo.message(), params.value("locale").toLocale()));
        returns.insert("pairingTransactionId", pairingInfo.transactionId().toString());
        returns.insert("setupMethod", JsonTypes::setupMethod().at(deviceClass.setupMethod()));
        if (deviceClass.setupMethod() == DeviceClass::SetupMethodOAuth) {
            returns.insert("oAuthUrl", pairingInfo.oAuthUrl());
        }
    }
    return createReply(returns);
}

JsonReply *DeviceHandler::ConfirmPairing(const QVariantMap &params)
{
    PairingTransactionId pairingTransactionId = PairingTransactionId(params.value("pairingTransactionId").toString());
    QString username = params.value("username").toString();
    QString secret = params.value("secret").toString();
    DevicePairingInfo devicePairingInfo = NymeaCore::instance()->deviceManager()->confirmPairing(pairingTransactionId, secret, username);

    JsonReply *reply = nullptr;
    if (devicePairingInfo.status() == Device::DeviceErrorAsync) {
        reply = createAsyncReply("ConfirmPairing");
        connect(reply, &JsonReply::finished, [this, pairingTransactionId](){ m_asyncPairingRequests.remove(pairingTransactionId); });
        m_asyncPairingRequests.insert(pairingTransactionId, reply);
        return reply;
    }
    QVariantMap returns;
    returns.insert("deviceError", JsonTypes::deviceErrorToString(devicePairingInfo.status()));
    returns.insert("displayMessage", devicePairingInfo.message());
    reply = createReply(returns);
    return reply;
}

JsonReply* DeviceHandler::GetConfiguredDevices(const QVariantMap &params) const
{
    QVariantMap returns;
    QVariantList configuredDeviceList;
    if (params.contains("deviceId")) {
        Device *device = NymeaCore::instance()->deviceManager()->findConfiguredDevice(DeviceId(params.value("deviceId").toString()));
        if (!device) {
            returns.insert("deviceError", JsonTypes::deviceErrorToString(Device::DeviceErrorDeviceNotFound));
            return createReply(returns);
        } else {
            configuredDeviceList.append(JsonTypes::packDevice(device));
        }
    } else {
        foreach (Device *device, NymeaCore::instance()->deviceManager()->configuredDevices()) {
            configuredDeviceList.append(JsonTypes::packDevice(device));
        }
    }
    returns.insert("devices", configuredDeviceList);
    return createReply(returns);
}

JsonReply *DeviceHandler::ReconfigureDevice(const QVariantMap &params)
{
    Q_UNUSED(params);
    DeviceId deviceId = DeviceId(params.value("deviceId").toString());
    ParamList deviceParams = JsonTypes::unpackParams(params.value("deviceParams").toList());

    Device::DeviceError status;
    DeviceDescriptorId deviceDescriptorId(params.value("deviceDescriptorId").toString());
    if (deviceDescriptorId.isNull()) {
        status = NymeaCore::instance()->deviceManager()->reconfigureDevice(deviceId, deviceParams);
    } else {
        status = NymeaCore::instance()->deviceManager()->reconfigureDevice(deviceId, deviceDescriptorId);
    }

    if (status == Device::DeviceErrorAsync) {
        JsonReply *asyncReply = createAsyncReply("ReconfigureDevice");
        connect(asyncReply, &JsonReply::finished, [this, deviceId](){ m_asynDeviceEditAdditions.remove(deviceId); });
        m_asynDeviceEditAdditions.insert(deviceId, asyncReply);
        return asyncReply;
    }

    QVariantMap returns;
    returns.insert("deviceError", JsonTypes::deviceErrorToString(status));
    return createReply(returns);
}

JsonReply *DeviceHandler::EditDevice(const QVariantMap &params)
{
    DeviceId deviceId = DeviceId(params.value("deviceId").toString());
    QString name = params.value("name").toString();

    qCDebug(dcJsonRpc()) << "Edit device" << deviceId << name;

    Device::DeviceError status = NymeaCore::instance()->deviceManager()->editDevice(deviceId, name);

    QVariantMap returns;
    returns.insert("deviceError", JsonTypes::deviceErrorToString(status));
    return createReply(returns);
}

JsonReply* DeviceHandler::RemoveConfiguredDevice(const QVariantMap &params)
{
    QVariantMap returns;
    DeviceId deviceId = DeviceId(params.value("deviceId").toString());

    // global removePolicy has priority
    if (params.contains("removePolicy")) {
        RuleEngine::RemovePolicy removePolicy = params.value("removePolicy").toString() == "RemovePolicyCascade" ? RuleEngine::RemovePolicyCascade : RuleEngine::RemovePolicyUpdate;
        Device::DeviceError status = NymeaCore::instance()->removeConfiguredDevice(deviceId, removePolicy);
        returns.insert("deviceError", JsonTypes::deviceErrorToString(status));
        return createReply(returns);
    }

    QHash<RuleId, RuleEngine::RemovePolicy> removePolicyList;
    foreach (const QVariant &variant, params.value("removePolicyList").toList()) {
        RuleId ruleId = RuleId(variant.toMap().value("ruleId").toString());
        RuleEngine::RemovePolicy policy = variant.toMap().value("policy").toString() == "RemovePolicyCascade" ? RuleEngine::RemovePolicyCascade : RuleEngine::RemovePolicyUpdate;
        removePolicyList.insert(ruleId, policy);
    }

    QPair<Device::DeviceError, QList<RuleId> > status = NymeaCore::instance()->removeConfiguredDevice(deviceId, removePolicyList);
    returns.insert("deviceError", JsonTypes::deviceErrorToString(status.first));

    if (!status.second.isEmpty()) {
        QVariantList ruleIdList;
        foreach (const RuleId &ruleId, status.second) {
            ruleIdList.append(ruleId.toString());
        }
        returns.insert("ruleIds", ruleIdList);
    }

    return createReply(returns);
}

JsonReply *DeviceHandler::SetDeviceSettings(const QVariantMap &params)
{
    QVariantMap returns;
    DeviceId deviceId = DeviceId(params.value("deviceId").toString());
    ParamList settings = JsonTypes::unpackParams(params.value("settings").toList());
    Device::DeviceError status = NymeaCore::instance()->deviceManager()->setDeviceSettings(deviceId, settings);
    returns.insert("deviceError", JsonTypes::deviceErrorToString(status));
    return createReply(returns);
}

JsonReply* DeviceHandler::GetEventTypes(const QVariantMap &params) const
{
    QVariantMap returns;

    QVariantList eventList;
    DeviceClass deviceClass = NymeaCore::instance()->deviceManager()->findDeviceClass(DeviceClassId(params.value("deviceClassId").toString()));
    foreach (const EventType &eventType, deviceClass.eventTypes()) {
        eventList.append(JsonTypes::packEventType(eventType, deviceClass.pluginId(), params.value("locale").toLocale()));
    }
    returns.insert("eventTypes", eventList);
    return createReply(returns);
}

JsonReply* DeviceHandler::GetActionTypes(const QVariantMap &params) const
{
    QVariantMap returns;

    QVariantList actionList;
    DeviceClass deviceClass = NymeaCore::instance()->deviceManager()->findDeviceClass(DeviceClassId(params.value("deviceClassId").toString()));
    foreach (const ActionType &actionType, deviceClass.actionTypes()) {
        actionList.append(JsonTypes::packActionType(actionType, deviceClass.pluginId(), params.value("locale").toLocale()));
    }
    returns.insert("actionTypes", actionList);
    return createReply(returns);
}

JsonReply* DeviceHandler::GetStateTypes(const QVariantMap &params) const
{
    QVariantMap returns;

    QVariantList stateList;
    DeviceClass deviceClass = NymeaCore::instance()->deviceManager()->findDeviceClass(DeviceClassId(params.value("deviceClassId").toString()));
    foreach (const StateType &stateType, deviceClass.stateTypes()) {
        stateList.append(JsonTypes::packStateType(stateType, deviceClass.pluginId(), NymeaCore::instance()->configuration()->locale()));
    }
    returns.insert("stateTypes", stateList);
    return createReply(returns);
}

JsonReply* DeviceHandler::GetStateValue(const QVariantMap &params) const
{
    QVariantMap returns;

    Device *device = NymeaCore::instance()->deviceManager()->findConfiguredDevice(DeviceId(params.value("deviceId").toString()));
    if (!device) {
        returns.insert("deviceError", JsonTypes::deviceErrorToString(Device::DeviceErrorDeviceNotFound));
        return createReply(returns);
    }
    StateTypeId stateTypeId = StateTypeId(params.value("stateTypeId").toString());
    if (!device->hasState(stateTypeId)) {
        returns.insert("deviceError", JsonTypes::deviceErrorToString(Device::DeviceErrorStateTypeNotFound));
        return createReply(returns);
    }

    returns.insert("deviceError", JsonTypes::deviceErrorToString(Device::DeviceErrorNoError));
    returns.insert("value", device->state(stateTypeId).value());
    return createReply(returns);
}

JsonReply *DeviceHandler::GetStateValues(const QVariantMap &params) const
{
    QVariantMap returns;

    Device *device = NymeaCore::instance()->deviceManager()->findConfiguredDevice(DeviceId(params.value("deviceId").toString()));
    if (!device) {
        returns.insert("deviceError", JsonTypes::deviceErrorToString(Device::DeviceErrorDeviceNotFound));
        return createReply(returns);
    }

    returns.insert("deviceError", JsonTypes::deviceErrorToString(Device::DeviceErrorNoError));
    returns.insert("values", JsonTypes::packDeviceStates(device));
    return createReply(returns);
}

JsonReply *DeviceHandler::BrowseDevice(const QVariantMap &params) const
{
    QVariantMap returns;
    DeviceId deviceId = DeviceId(params.value("deviceId").toString());
    QString itemId = params.value("itemId").toString();

    Device::BrowseResult result = NymeaCore::instance()->deviceManager()->browseDevice(deviceId, itemId, params.value("locale").toLocale());

    if (result.status == Device::DeviceErrorAsync ) {
        JsonReply *reply = createAsyncReply("BrowseDevice");
        m_asyncBrowseRequests.insert(result.id(), reply);
        connect(reply, &JsonReply::finished, this, [this, result](){
            m_asyncBrowseRequests.remove(result.id());
        });
        return reply;
    }

    returns.insert("deviceError", JsonTypes::deviceErrorToString(result.status));
    returns.insert("items", JsonTypes::packBrowserItems(result.items));
    return createReply(returns);
}

JsonReply *DeviceHandler::GetBrowserItem(const QVariantMap &params) const
{
    QVariantMap returns;
    DeviceId deviceId = DeviceId(params.value("deviceId").toString());
    QString itemId = params.value("itemId").toString();

    Device::BrowserItemResult result = NymeaCore::instance()->deviceManager()->browserItemDetails(deviceId, itemId, params.value("locale").toLocale());

    if (result.status == Device::DeviceErrorAsync ) {
        JsonReply *reply = createAsyncReply("GetBrowserItem");
        m_asyncBrowseDetailsRequests.insert(result.id(), reply);
        connect(reply, &JsonReply::finished, this, [this, result](){
            m_asyncBrowseDetailsRequests.remove(result.id());
        });
        return reply;
    }

    returns.insert("deviceError", JsonTypes::deviceErrorToString(result.status));
    if (result.status == Device::DeviceErrorNoError) {
        returns.insert("item", JsonTypes::packBrowserItem(result.item));
    }
    return createReply(returns);
}

void DeviceHandler::pluginConfigChanged(const PluginId &id, const ParamList &config)
{
    QVariantMap params;
    params.insert("pluginId", id);
    QVariantList configList;
    foreach (const Param &param, config) {
        configList << JsonTypes::packParam(param);
    }
    params.insert("configuration", configList);
    emit PluginConfigurationChanged(params);
}

void DeviceHandler::deviceStateChanged(Device *device, const QUuid &stateTypeId, const QVariant &value)
{
    QVariantMap params;
    params.insert("deviceId", device->id());
    params.insert("stateTypeId", stateTypeId);
    params.insert("value", value);

    emit StateChanged(params);
}

void DeviceHandler::deviceRemovedNotification(const QUuid &deviceId)
{
    QVariantMap params;
    params.insert("deviceId", deviceId);

    emit DeviceRemoved(params);
}

void DeviceHandler::deviceAddedNotification(Device *device)
{
    QVariantMap params;
    params.insert("device", JsonTypes::packDevice(device));

    emit DeviceAdded(params);
}

void DeviceHandler::deviceChangedNotification(Device *device)
{
    QVariantMap params;
    params.insert("device", JsonTypes::packDevice(device));

    emit DeviceChanged(params);
}

void DeviceHandler::deviceSettingChangedNotification(const DeviceId deviceId, const ParamTypeId &paramTypeId, const QVariant &value)
{
    QVariantMap params;
    params.insert("deviceId", deviceId);
    params.insert("paramTypeId", paramTypeId.toString());
    params.insert("value", value);
    emit DeviceSettingChanged(params);
}

void DeviceHandler::devicesDiscovered(const DeviceClassId &deviceClassId, const QList<DeviceDescriptor> deviceDescriptors)
{
    if (!m_discoverRequests.contains(deviceClassId)) {
        return; // We didn't start this discovery... Ignore it.
    }

    JsonReply *reply = nullptr;
    reply = m_discoverRequests.take(deviceClassId);
    if (!reply)
        return;

    QVariantMap returns;
    returns.insert("deviceDescriptors", JsonTypes::packDeviceDescriptors(deviceDescriptors));
    returns.insert("deviceError", JsonTypes::deviceErrorToString(Device::DeviceErrorNoError));

    reply->setData(returns);
    reply->finished();
}

void DeviceHandler::deviceSetupFinished(Device *device, Device::DeviceError status)
{
    qCDebug(dcJsonRpc) << "Got a device setup finished" << device->name() << device->id();
    if (!m_asynDeviceAdditions.contains(device->id())) {
        return; // Not the device we're waiting for...
    }

    JsonReply *reply = m_asynDeviceAdditions.take(device->id());

    QVariantMap returns;
    returns.insert("deviceError", JsonTypes::deviceErrorToString(status));

    if(status == Device::DeviceErrorNoError) {
        returns.insert("deviceId", device->id());
    }
    reply->setData(returns);
    reply->finished();
}

void DeviceHandler::deviceReconfigurationFinished(Device *device, Device::DeviceError status)
{
    qCDebug(dcJsonRpc) << "Got async device reconfiguration finished";
    if (!m_asynDeviceEditAdditions.contains(device->id())) {
        return;
    }
    JsonReply *reply = m_asynDeviceEditAdditions.take(device->id());

    QVariantMap returns;
    returns.insert("deviceError", JsonTypes::deviceErrorToString(status));
    reply->setData(returns);
    reply->finished();
}

void DeviceHandler::pairingFinished(const DevicePairingInfo &devicePairingInfo)
{
    qCDebug(dcJsonRpc) << "Got pairing finished";
    JsonReply *reply = m_asyncPairingRequests.take(devicePairingInfo.transactionId());
    if (!reply) {
        return;
    }

    if (devicePairingInfo.status() != Device::DeviceErrorNoError) {
        QVariantMap returns;
        returns.insert("deviceError", JsonTypes::deviceErrorToString(devicePairingInfo.status()));
        returns.insert("displayMessage", devicePairingInfo.message());
        reply->setData(returns);
        reply->finished();
        return;
    }
    m_asynDeviceAdditions.insert(devicePairingInfo.deviceId(), reply);
}

void DeviceHandler::browseRequestFinished(const Device::BrowseResult &result)
{
    if (!m_asyncBrowseRequests.contains(result.id())) {
        qCWarning(dcJsonRpc()) << "No pending JsonRpc reply. Did it time out?";
        return;
    }

    JsonReply *reply = m_asyncBrowseRequests.take(result.id());
    QVariantMap params;
    params.insert("items", JsonTypes::packBrowserItems(result.items));
    params.insert("deviceError", JsonTypes::deviceErrorToString(result.status));
    reply->setData(params);
    reply->finished();
}

void DeviceHandler::browserItemRequestFinished(const Device::BrowserItemResult &result)
{
    if (!m_asyncBrowseDetailsRequests.contains(result.id())) {
        qCWarning(dcJsonRpc()) << "No pending JsonRpc reply for result" << result.id() << ". Did it time out?";
        return;
    }
    JsonReply *reply = m_asyncBrowseDetailsRequests.take(result.id());
    QVariantMap params;
    if (result.status == Device::DeviceErrorNoError) {
        params.insert("item", JsonTypes::packBrowserItem(result.item));
    }
    params.insert("deviceError", JsonTypes::deviceErrorToString(result.status));
    reply->setData(params);
    reply->finished();
}

}
