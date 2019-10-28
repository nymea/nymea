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
#include "types/browseritem.h"
#include "types/mediabrowseritem.h"
#include "devices/translator.h"
#include "devices/devicediscoveryinfo.h"
#include "devices/devicepairinginfo.h"
#include "devices/devicesetupinfo.h"
#include "devices/browseresult.h"
#include "devices/browseritemresult.h"

#include <QDebug>

namespace nymeaserver {

/*! Constructs a new \l DeviceHandler with the given \a parent. */
DeviceHandler::DeviceHandler(QObject *parent) :
    JsonHandler(parent)
{
    // Enums
    registerEnum<Device::DeviceError>();
    registerEnum<DeviceClass::SetupMethod>();
    registerEnum<DeviceClass::CreateMethod, DeviceClass::CreateMethods>();
    registerEnum<Types::Unit>();
    registerEnum<Types::InputType>();
    registerEnum<RuleEngine::RemovePolicy>();
    registerEnum<BrowserItem::BrowserIcon>();
    registerEnum<MediaBrowserItem::MediaBrowserIcon>();

    // Objects
    registerObject<ParamType, ParamTypes>();
    registerObject<Param, ParamList>();
    registerObject<DevicePlugin>();
    registerObject<Vendor>();
    registerObject<EventType, EventTypes>();
    registerObject<StateType, StateTypes>();
    registerObject<ActionType, ActionTypes>();
    registerObject<DeviceClass>();
    registerObject<DeviceDescriptor>();
    registerObject<State, States>();
    registerObject<Device>();

    // Regsitering browseritem manually for now. Not sure how to deal with the
    // polymorphism in int (e.g MediaBrowserItem)
    QVariantMap browserItem;
    browserItem.insert("id", enumValueName(String));
    browserItem.insert("displayName", enumValueName(String));
    browserItem.insert("description", enumValueName(String));
    browserItem.insert("icon", enumRef<BrowserItem::BrowserIcon>());
    browserItem.insert("thumbnail", enumValueName(String));
    browserItem.insert("executable", enumValueName(Bool));
    browserItem.insert("browsable", enumValueName(Bool));
    browserItem.insert("disabled", enumValueName(Bool));
    browserItem.insert("actionTypeIds", QVariantList() << enumValueName(Uuid));
    browserItem.insert("o:mediaIcon", enumRef<MediaBrowserItem::MediaBrowserIcon>());
    registerObject("BrowserItem", browserItem);


    // Methods
    QString description; QVariantMap returns; QVariantMap params;
    description = "Returns a list of supported Vendors.";
    returns.insert("vendors", QVariantList() << objectRef("Vendor"));
    registerMethod("GetSupportedVendors", description, params, returns);

    params.clear(); returns.clear();
    description = "Returns a list of supported Device classes, optionally filtered by vendorId.";
    params.insert("o:vendorId", enumValueName(Uuid));
    returns.insert("deviceClasses", QVariantList() << objectRef("DeviceClass"));
    registerMethod("GetSupportedDevices", description, params, returns);

    params.clear(); returns.clear();
    description = "Returns a list of loaded plugins.";
    returns.insert("plugins", QVariantList() << objectRef("DevicePlugin"));
    registerMethod("GetPlugins", description, params, returns);

    params.clear(); returns.clear();
    description = "Get a plugin's params.";
    params.insert("pluginId", enumValueName(Uuid));
    returns.insert("deviceError", enumRef<Device::DeviceError>());
    returns.insert("o:configuration", QVariantList() << objectRef("Param"));
    registerMethod("GetPluginConfiguration", description, params, returns);

    params.clear(); returns.clear();
    description = "Set a plugin's params.";
    params.insert("pluginId", enumValueName(Uuid));
    params.insert("configuration", QVariantList() << objectRef("Param"));
    returns.insert("deviceError", enumRef<Device::DeviceError>());
    registerMethod("SetPluginConfiguration", description, params, returns);

    params.clear(); returns.clear();
    description = "Add a configured device with a setupMethod of SetupMethodJustAdd. "
                                          "For devices with a setupMethod different than SetupMethodJustAdd, use PairDevice. "
                                          "Devices with CreateMethodJustAdd require all parameters to be supplied here. "
                                          "Devices with CreateMethodDiscovery require the use of a deviceDescriptorId. For discovered "
                                          "devices params are not required and will be taken from the DeviceDescriptor, however, they "
                                          "may be overridden by supplying deviceParams.";
    params.insert("deviceClassId", enumValueName(Uuid));
    params.insert("name", enumValueName(String));
    params.insert("o:deviceDescriptorId", enumValueName(Uuid));
    params.insert("o:deviceParams", QVariantList() << objectRef("Param"));
    returns.insert("deviceError", enumRef<Device::DeviceError>());
    returns.insert("o:deviceId", enumValueName(Uuid));
    returns.insert("o:displayMessage", enumValueName(String));
    registerMethod("AddConfiguredDevice", description, params, returns);

    params.clear(); returns.clear();
    description = "Pair a device. "
                                 "Use this to set up or reconfigure devices for DeviceClasses with a setupMethod different than SetupMethodJustAdd. "
                                 "Depending on the CreateMethod and whether a new devices is set up or an existing one is reconfigured, different parameters "
                                 "are required:\n"
                                 "CreateMethodJustAdd takes the deviceClassId and the parameters you want to have with that device.\n"
                                 "CreateMethodDiscovery requires the use of a deviceDescriptorId, previously obtained with DiscoverDevices. Optionally, "
                                 "parameters can be overridden with the give deviceParams.\n"
                                 "If an existing device should be reconfigured, the deviceId of said device should be given additionally.\n"
                                 "If success is true, the return values will contain a pairingTransactionId, a displayMessage and "
                                 "the setupMethod. Depending on the setupMethod, the application should present the use an appropriate login mask, "
                                 "that is, For SetupMethodDisplayPin the user should enter a pin that is displayed on the device, for SetupMethodEnterPin the "
                                 "application should present the given PIN so the user can enter it on the device. For SetupMethodPushButton, the displayMessage "
                                 "shall be presented to the user as informational hints to press a button on the device. For SetupMethodUserAndPassword a login "
                                 "mask for a user and password login should be presented to the user. In case of SetupMethodOAuth, an OAuth URL will be returned "
                                 "which shall be opened in a web view to allow the user logging in.\n"
                                 "Once the login procedure has completed, the application shall proceed with ConfirmPairing, providing the results of the pairing "
                                 "procedure.";
    params.insert("o:deviceClassId", enumValueName(Uuid));
    params.insert("o:name", enumValueName(String));
    params.insert("o:deviceDescriptorId", enumValueName(Uuid));
    params.insert("o:deviceParams", QVariantList() << objectRef("Param"));
    params.insert("o:deviceId", enumValueName(Uuid));
    returns.insert("deviceError", enumRef<Device::DeviceError>());
    returns.insert("o:setupMethod", enumRef<DeviceClass::SetupMethod>());
    returns.insert("o:pairingTransactionId", enumValueName(Uuid));
    returns.insert("o:displayMessage", enumValueName(String));
    returns.insert("o:oAuthUrl", enumValueName(String));
    returns.insert("o:pin", enumValueName(String));
    registerMethod("PairDevice", description, params, returns);

    params.clear(); returns.clear();
    description = "Confirm an ongoing pairing. For SetupMethodUserAndPassword, provide the username in the \"username\" field "
                                     "and the password in the \"secret\" field. For SetupMethodEnterPin and provide the PIN in the \"secret\" "
                                     "field. In case of SetupMethodOAuth, the previously opened web view will eventually be redirected to http://128.0.0.1:8888 "
                                     "and the OAuth code as query parameters to this url. Provide the entire unmodified URL in the secret field.";
    params.insert("pairingTransactionId", enumValueName(Uuid));
    params.insert("o:username", enumValueName(String));
    params.insert("o:secret", enumValueName(String));
    returns.insert("deviceError", enumRef<Device::DeviceError>());
    returns.insert("o:displayMessage", enumValueName(String));
    returns.insert("o:deviceId", enumValueName(Uuid));
    registerMethod("ConfirmPairing", description, params, returns);

    params.clear(); returns.clear();
    description = "Returns a list of configured devices, optionally filtered by deviceId.";
    params.insert("o:deviceId", enumValueName(Uuid));
    returns.insert("devices", QVariantList() << objectRef("Device"));
    registerMethod("GetConfiguredDevices", description, params, returns);

    params.clear(); returns.clear();
    description = "Performs a device discovery and returns the results. This function may take a while to return. "
                                           "Note that this method will include all the found devices, that is, including devices that may "
                                           "already have been added. Those devices will have deviceId set to the device id of the already "
                                           "added device. Such results may be used to reconfigure existing devices and might be filtered "
                                           "in cases where only unknown devices are of interest.";
    params.insert("deviceClassId", enumValueName(Uuid));
    params.insert("o:discoveryParams", QVariantList() << objectRef("Param"));
    returns.insert("deviceError", enumRef<Device::DeviceError>());
    returns.insert("o:displayMessage", enumValueName(String));
    returns.insert("o:deviceDescriptors", QVariantList() << objectRef("DeviceDescriptor"));
    registerMethod("GetDiscoveredDevices", description, params, returns);

    params.clear(); returns.clear();
    description = "Reconfigure a device. This comes down to removing and recreating a device with new parameters "
                  "but keeping its device id the same (and with that keeping rules, tags etc). For devices with "
                  "create method CreateMethodDiscovery, a discovery (GetDiscoveredDevices) shall be performed first "
                  "and this method is to be called with a deviceDescriptorId of the re-discovered device instead of "
                  "the deviceId directly. Device parameters will be taken from the discovery, but can be overridden "
                  "individually here by providing them in the deviceParams parameter. Only writable parameters can "
                   "be changed.";
    params.insert("o:deviceId", enumValueName(Uuid));
    params.insert("o:deviceDescriptorId", enumValueName(Uuid));
    params.insert("o:deviceParams", QVariantList() << objectRef("Param"));
    returns.insert("deviceError", enumRef<Device::DeviceError>());
    returns.insert("o:displayMessage", enumValueName(String));
    registerMethod("ReconfigureDevice", description, params, returns);

    params.clear(); returns.clear();
    description = "Edit the name of a device. This method does not change the "
                                 "configuration of the device.";
    params.insert("deviceId", enumValueName(Uuid));
    params.insert("name", enumValueName(String));
    returns.insert("deviceError", enumRef<Device::DeviceError>());
    registerMethod("EditDevice", description, params, returns);

    params.clear(); returns.clear();
    description = "Change the settings of a device.";
    params.insert("deviceId", enumValueName(Uuid));
    params.insert("settings", QVariantList() << objectRef("Param"));
    returns.insert("deviceError", enumRef<Device::DeviceError>());
    registerMethod("SetDeviceSettings", description, params, returns);

    params.clear(); returns.clear();
    description = "Remove a device from the system.";
    params.insert("deviceId", enumValueName(Uuid));
    params.insert("o:removePolicy", enumRef<RuleEngine::RemovePolicy>());
    QVariantMap policy;
    policy.insert("ruleId", enumValueName(Uuid));
    policy.insert("policy", enumRef<RuleEngine::RemovePolicy>());
    QVariantList removePolicyList;
    removePolicyList.append(policy);
    params.insert("o:removePolicyList", removePolicyList);
    returns.insert("deviceError", enumRef<Device::DeviceError>());
    returns.insert("o:ruleIds", QVariantList() << enumValueName(Uuid));
    registerMethod("RemoveConfiguredDevice", description, params, returns);

    params.clear(); returns.clear();
    description = "Get event types for a specified deviceClassId.";
    params.insert("deviceClassId", enumValueName(Uuid));
    returns.insert("eventTypes", QVariantList() << objectRef("EventType"));
    registerMethod("GetEventTypes", description, params, returns);

    params.clear(); returns.clear();
    description = "Get action types for a specified deviceClassId.";
    params.insert("deviceClassId", enumValueName(Uuid));
    returns.insert("actionTypes", QVariantList() << objectRef("ActionType"));
    registerMethod("GetActionTypes", description, params, returns);

    params.clear(); returns.clear();
    description = "Get state types for a specified deviceClassId.";
    params.insert("deviceClassId", enumValueName(Uuid));
    returns.insert("stateTypes", QVariantList() << objectRef("StateType"));
    registerMethod("GetStateTypes", description, params, returns);

    params.clear(); returns.clear();
    description = "Get the value of the given device and the given stateType";
    params.insert("deviceId", enumValueName(Uuid));
    params.insert("stateTypeId", enumValueName(Uuid));
    returns.insert("deviceError", enumRef<Device::DeviceError>());
    returns.insert("o:value", enumValueName(Variant));
    registerMethod("GetStateValue", description, params, returns);

    params.clear(); returns.clear();
    description = "Get all the state values of the given device.";
    params.insert("deviceId", enumValueName(Uuid));
    returns.insert("deviceError", enumRef<Device::DeviceError>());
    QVariantMap state;
    state.insert("stateTypeId", enumValueName(Uuid));
    state.insert("value", enumValueName(Variant));
    returns.insert("o:values", QVariantList() << state);
    registerMethod("GetStateValues", description, params, returns);

    params.clear(); returns.clear();
    description = "Browse a device. If a DeviceClass indicates a device is browsable, this method will return the BrowserItems. If no parameter besides the deviceId is used, the root node of this device will be returned. Any returned item which is browsable can be passed as node. Results will be children of the given node.";
    params.insert("deviceId", enumValueName(Uuid));
    params.insert("o:itemId", enumValueName(String));
    returns.insert("deviceError", enumRef<Device::DeviceError>());
    returns.insert("items", QVariantList() << objectRef("BrowserItem"));
    registerMethod("BrowseDevice", description, params, returns);

    params.clear(); returns.clear();
    description = "Get a single item from the browser. This won't give any more info on an item than a regular browseDevice call, but it allows to fetch details of an item if only the ID is known.";
    params.insert("deviceId", enumValueName(Uuid));
    params.insert("o:itemId", enumValueName(String));
    returns.insert("deviceError", enumRef<Device::DeviceError>());
    returns.insert("o:item", objectRef("BrowserItem"));
    registerMethod("GetBrowserItem", description, params, returns);

    // Notifications
    params.clear(); returns.clear();
    description = "Emitted whenever a State of a device changes.";
    params.insert("deviceId", enumValueName(Uuid));
    params.insert("stateTypeId", enumValueName(Uuid));
    params.insert("value", enumValueName(Variant));
    registerNotification("StateChanged", description, params);

    params.clear(); returns.clear();
    description = "Emitted whenever a Device was removed.";
    params.insert("deviceId", enumValueName(Uuid));
    registerNotification("DeviceRemoved", description, params);

    params.clear(); returns.clear();
    description = "Emitted whenever a Device was added.";
    params.insert("device", objectRef("Device"));
    registerNotification("DeviceAdded", description, params);

    params.clear(); returns.clear();
    description = "Emitted whenever the params or name of a Device are changed (by EditDevice or ReconfigureDevice).";
    params.insert("device", objectRef("Device"));
    registerNotification("DeviceChanged", description, params);

    params.clear(); returns.clear();
    description = "Emitted whenever the setting of a Device is changed.";
    params.insert("deviceId", enumValueName(Uuid));
    params.insert("paramTypeId", enumValueName(Uuid));
    params.insert("value", enumValueName(Variant));
    registerNotification("DeviceSettingChanged", description, params);

    params.clear(); returns.clear();
    description = "Emitted whenever a plugin's configuration is changed.";
    params.insert("pluginId", enumValueName(Uuid));
    params.insert("configuration", QVariantList() << objectRef("Param"));
    registerNotification("PluginConfigurationChanged", description, params);

    connect(NymeaCore::instance(), &NymeaCore::pluginConfigChanged, this, &DeviceHandler::pluginConfigChanged);
    connect(NymeaCore::instance(), &NymeaCore::deviceStateChanged, this, &DeviceHandler::deviceStateChanged);
    connect(NymeaCore::instance(), &NymeaCore::deviceRemoved, this, &DeviceHandler::deviceRemovedNotification);
    connect(NymeaCore::instance(), &NymeaCore::deviceAdded, this, &DeviceHandler::deviceAddedNotification);
    connect(NymeaCore::instance(), &NymeaCore::deviceChanged, this, &DeviceHandler::deviceChangedNotification);
    connect(NymeaCore::instance(), &NymeaCore::deviceSettingChanged, this, &DeviceHandler::deviceSettingChangedNotification);
}

/*! Returns the name of the \l{DeviceHandler}. In this case \b Devices.*/
QString DeviceHandler::name() const
{
    return "Devices";
}

JsonReply* DeviceHandler::GetSupportedVendors(const QVariantMap &params) const
{
    QLocale locale = params.value("locale").toLocale();

    QVariantList vendors;
    foreach (const Vendor &vendor, NymeaCore::instance()->deviceManager()->supportedVendors()) {
        Vendor translatedVendor = NymeaCore::instance()->deviceManager()->translateVendor(vendor, locale);
        vendors.append(pack(translatedVendor));
    }

    QVariantMap returns;
    returns.insert("vendors", vendors);
    return createReply(returns);
}

JsonReply* DeviceHandler::GetSupportedDevices(const QVariantMap &params) const
{
    QLocale locale = params.value("locale").toLocale();
    VendorId vendorId = VendorId(params.value("vendorId").toString());
    QVariantMap returns;
    QVariantList deviceClasses;
    foreach (const DeviceClass &deviceClass, NymeaCore::instance()->deviceManager()->supportedDevices(vendorId)) {
        DeviceClass translatedDeviceClass = NymeaCore::instance()->deviceManager()->translateDeviceClass(deviceClass, locale);
        deviceClasses.append(pack(translatedDeviceClass));
    }

    returns.insert("deviceClasses", deviceClasses);
    return createReply(returns);
}

JsonReply *DeviceHandler::GetDiscoveredDevices(const QVariantMap &params) const
{
    QLocale locale = params.value("locale").toLocale();

    QVariantMap returns;

    DeviceClassId deviceClassId = DeviceClassId(params.value("deviceClassId").toString());

    ParamList discoveryParams = unpackParams(params.value("discoveryParams").toList());

    JsonReply *reply = createAsyncReply("GetDiscoveredDevices");
    DeviceDiscoveryInfo *info = NymeaCore::instance()->deviceManager()->discoverDevices(deviceClassId, discoveryParams);
    connect(info, &DeviceDiscoveryInfo::finished, reply, [this, reply, info, locale](){
        QVariantMap returns;
        returns.insert("deviceError", enumValueName<Device::DeviceError>(info->status()));

        if (info->status() == Device::DeviceErrorNoError) {
            QVariantList deviceDescriptorList;
            foreach (const DeviceDescriptor &deviceDescriptor, info->deviceDescriptors()) {
                deviceDescriptorList.append(pack(deviceDescriptor));
            }
            returns.insert("deviceDescriptors", deviceDescriptorList);
        }

        if (!info->displayMessage().isEmpty()) {
            returns.insert("displayMessage", info->translatedDisplayMessage(locale));
        }

        reply->setData(returns);
        reply->finished();

    });
    return reply;
}

JsonReply* DeviceHandler::GetPlugins(const QVariantMap &params) const
{
    QLocale locale = params.value("locale").toLocale();

    QVariantList plugins;
    foreach (DevicePlugin* plugin, NymeaCore::instance()->deviceManager()->plugins()) {
        QVariantMap packedPlugin = pack(*plugin).toMap();
        packedPlugin["displayName"] = NymeaCore::instance()->deviceManager()->translate(plugin->pluginId(), plugin->pluginDisplayName(), locale);
        plugins.append(packedPlugin);
    }

    QVariantMap returns;
    returns.insert("plugins", plugins);
    return createReply(returns);
}

JsonReply *DeviceHandler::GetPluginConfiguration(const QVariantMap &params) const
{
    QVariantMap returns;

    DevicePlugin *plugin = NymeaCore::instance()->deviceManager()->plugins().findById(PluginId(params.value("pluginId").toString()));
    if (!plugin) {
        returns.insert("deviceError", enumValueName<Device::DeviceError>(Device::DeviceErrorPluginNotFound));
        return createReply(returns);
    }

    QVariantList paramVariantList;
    foreach (const Param &param, plugin->configuration()) {
        paramVariantList.append(pack(param));
    }
    returns.insert("configuration", paramVariantList);
    returns.insert("deviceError", enumValueName<Device::DeviceError>(Device::DeviceErrorNoError));
    return createReply(returns);
}

JsonReply* DeviceHandler::SetPluginConfiguration(const QVariantMap &params)
{
    QVariantMap returns;
    PluginId pluginId = PluginId(params.value("pluginId").toString());
    ParamList pluginParams = unpackParams(params.value("configuration").toList());
    Device::DeviceError result = NymeaCore::instance()->deviceManager()->setPluginConfig(pluginId, pluginParams);
    returns.insert("deviceError",enumValueName<Device::DeviceError>(result));
    return createReply(returns);
}

JsonReply* DeviceHandler::AddConfiguredDevice(const QVariantMap &params)
{
    DeviceClassId deviceClassId(params.value("deviceClassId").toString());
    QString deviceName = params.value("name").toString();
    ParamList deviceParams = unpackParams(params.value("deviceParams").toList());
    DeviceDescriptorId deviceDescriptorId(params.value("deviceDescriptorId").toString());
    QLocale locale = params.value("locale").toLocale();

    JsonReply *jsonReply = createAsyncReply("AddConfiguredDevice");

    DeviceSetupInfo *info;
    if (deviceDescriptorId.isNull()) {
        info = NymeaCore::instance()->deviceManager()->addConfiguredDevice(deviceClassId, deviceParams, deviceName);
    } else {
        info = NymeaCore::instance()->deviceManager()->addConfiguredDevice(deviceDescriptorId, deviceParams, deviceName);
    }
    connect(info, &DeviceSetupInfo::finished, jsonReply, [info, jsonReply, locale](){
        QVariantMap returns;
        returns.insert("deviceError", enumValueName<Device::DeviceError>(info->status()));

        if (!info->displayMessage().isEmpty()) {
            returns.insert("displayMessage", info->translatedDisplayMessage(locale));
        }

        if(info->status() == Device::DeviceErrorNoError) {
            returns.insert("deviceId", info->device()->id());
        }
        jsonReply->setData(returns);
        jsonReply->finished();

    });
    return jsonReply;
}

JsonReply *DeviceHandler::PairDevice(const QVariantMap &params)
{
    QString deviceName = params.value("name").toString();
    ParamList deviceParams = unpackParams(params.value("deviceParams").toList());
    QLocale locale = params.value("locale").toLocale();

    DevicePairingInfo *info;
    if (params.contains("deviceDescriptorId")) {
        DeviceDescriptorId deviceDescriptorId = DeviceDescriptorId(params.value("deviceDescriptorId").toString());
        info = NymeaCore::instance()->deviceManager()->pairDevice(deviceDescriptorId, deviceParams, deviceName);
    } else if (params.contains("deviceId")) {
        DeviceId deviceId = DeviceId(params.value("deviceId").toString());
        info = NymeaCore::instance()->deviceManager()->pairDevice(deviceId, deviceParams, deviceName);
    } else {
        DeviceClassId deviceClassId(params.value("deviceClassId").toString());
        info = NymeaCore::instance()->deviceManager()->pairDevice(deviceClassId, deviceParams, deviceName);
    }

    JsonReply *jsonReply = createAsyncReply("PairDevice");

    connect(info, &DevicePairingInfo::finished, jsonReply, [jsonReply, info, locale](){
        QVariantMap returns;
        returns.insert("deviceError", enumValueName<Device::DeviceError>(info->status()));
        returns.insert("pairingTransactionId", info->transactionId().toString());

        if (info->status() == Device::DeviceErrorNoError) {
            DeviceClass deviceClass = NymeaCore::instance()->deviceManager()->findDeviceClass(info->deviceClassId());
            returns.insert("setupMethod", enumValueName<DeviceClass::SetupMethod>(deviceClass.setupMethod()));
        }

        if (!info->displayMessage().isEmpty()) {
            returns.insert("displayMessage", info->translatedDisplayMessage(locale));
        }

        if (!info->oAuthUrl().isEmpty()) {
            returns.insert("oAuthUrl", info->oAuthUrl());
        }

        jsonReply->setData(returns);
        jsonReply->finished();
    });

    return jsonReply;
}

JsonReply *DeviceHandler::ConfirmPairing(const QVariantMap &params)
{
    PairingTransactionId pairingTransactionId = PairingTransactionId(params.value("pairingTransactionId").toString());
    QString secret = params.value("secret").toString();
    QString username = params.value("username").toString();
    QLocale locale = params.value("locale").toLocale();

    JsonReply *jsonReply = createAsyncReply("ConfirmPairing");

    DevicePairingInfo *info = NymeaCore::instance()->deviceManager()->confirmPairing(pairingTransactionId, username, secret);
    connect(info, &DevicePairingInfo::finished, jsonReply, [info, jsonReply, locale](){

        QVariantMap returns;
        returns.insert("deviceError", enumValueName<Device::DeviceError>(info->status()));
        if (!info->displayMessage().isEmpty()) {
            returns.insert("displayMessage", info->translatedDisplayMessage(locale));
        }
        if (info->status() == Device::DeviceErrorNoError) {
            returns.insert("deviceId", info->deviceId().toString());
        }
        jsonReply->setData(returns);
        jsonReply->finished();
    });

    return jsonReply;
}

JsonReply* DeviceHandler::GetConfiguredDevices(const QVariantMap &params) const
{
    QVariantMap returns;
    QVariantList configuredDeviceList;
    if (params.contains("deviceId")) {
        Device *device = NymeaCore::instance()->deviceManager()->findConfiguredDevice(DeviceId(params.value("deviceId").toString()));
        if (!device) {
            returns.insert("deviceError", enumValueName<Device::DeviceError>(Device::DeviceErrorDeviceNotFound));
            return createReply(returns);
        } else {
            configuredDeviceList.append(packDevice(device));
        }
    } else {
        foreach (Device *device, NymeaCore::instance()->deviceManager()->configuredDevices()) {
            configuredDeviceList.append(packDevice(device));
        }
    }
    returns.insert("devices", configuredDeviceList);
    return createReply(returns);
}

JsonReply *DeviceHandler::ReconfigureDevice(const QVariantMap &params)
{
    DeviceId deviceId = DeviceId(params.value("deviceId").toString());
    ParamList deviceParams = unpackParams(params.value("deviceParams").toList());
    DeviceDescriptorId deviceDescriptorId(params.value("deviceDescriptorId").toString());
    QLocale locale = params.value("locale").toLocale();

    JsonReply *jsonReply = createAsyncReply("ReconfigureDevice");

    DeviceSetupInfo *info;
    if (!deviceDescriptorId.isNull()) {
        info = NymeaCore::instance()->deviceManager()->reconfigureDevice(deviceDescriptorId, deviceParams);
    } else if (!deviceId.isNull()){
        info = NymeaCore::instance()->deviceManager()->reconfigureDevice(deviceId, deviceParams);
    } else {
        qCWarning(dcJsonRpc()) << "Either deviceId or deviceDescriptorId are required";
        QVariantMap ret;
        ret.insert("deviceError", enumValueName(Device::DeviceErrorMissingParameter));
        return createReply(ret);
    }

    connect(info, &DeviceSetupInfo::finished, jsonReply, [info, jsonReply, locale](){

        QVariantMap returns;
        returns.insert("deviceError", enumValueName<Device::DeviceError>(info->status()));
        returns.insert("displayMessage", info->translatedDisplayMessage(locale));
        jsonReply->setData(returns);
        jsonReply->finished();

    });

    return jsonReply;
}

JsonReply *DeviceHandler::EditDevice(const QVariantMap &params)
{
    DeviceId deviceId = DeviceId(params.value("deviceId").toString());
    QString name = params.value("name").toString();

    qCDebug(dcJsonRpc()) << "Edit device" << deviceId << name;

    Device::DeviceError status = NymeaCore::instance()->deviceManager()->editDevice(deviceId, name);

    return createReply(statusToReply(status));
}

JsonReply* DeviceHandler::RemoveConfiguredDevice(const QVariantMap &params)
{
    QVariantMap returns;
    DeviceId deviceId = DeviceId(params.value("deviceId").toString());

    // global removePolicy has priority
    if (params.contains("removePolicy")) {
        RuleEngine::RemovePolicy removePolicy = params.value("removePolicy").toString() == "RemovePolicyCascade" ? RuleEngine::RemovePolicyCascade : RuleEngine::RemovePolicyUpdate;
        Device::DeviceError status = NymeaCore::instance()->removeConfiguredDevice(deviceId, removePolicy);
        returns.insert("deviceError", enumValueName<Device::DeviceError>(status));
        return createReply(returns);
    }

    QHash<RuleId, RuleEngine::RemovePolicy> removePolicyList;
    foreach (const QVariant &variant, params.value("removePolicyList").toList()) {
        RuleId ruleId = RuleId(variant.toMap().value("ruleId").toString());
        RuleEngine::RemovePolicy policy = variant.toMap().value("policy").toString() == "RemovePolicyCascade" ? RuleEngine::RemovePolicyCascade : RuleEngine::RemovePolicyUpdate;
        removePolicyList.insert(ruleId, policy);
    }

    QPair<Device::DeviceError, QList<RuleId> > status = NymeaCore::instance()->removeConfiguredDevice(deviceId, removePolicyList);
    returns.insert("deviceError", enumValueName<Device::DeviceError>(status.first));

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
    DeviceId deviceId = DeviceId(params.value("deviceId").toString());
    ParamList settings = unpackParams(params.value("settings").toList());
    Device::DeviceError status = NymeaCore::instance()->deviceManager()->setDeviceSettings(deviceId, settings);
    return createReply(statusToReply(status));
}

JsonReply* DeviceHandler::GetEventTypes(const QVariantMap &params) const
{
    QVariantMap returns;

    QVariantList eventList;
    DeviceClass deviceClass = NymeaCore::instance()->deviceManager()->findDeviceClass(DeviceClassId(params.value("deviceClassId").toString()));
    foreach (const EventType &eventType, deviceClass.eventTypes()) {
        eventList.append(packEventType(eventType, deviceClass.pluginId(), params.value("locale").toLocale()));
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
        actionList.append(packActionType(actionType, deviceClass.pluginId(), params.value("locale").toLocale()));
    }
    returns.insert("actionTypes", actionList);
    return createReply(returns);
}

JsonReply* DeviceHandler::GetStateTypes(const QVariantMap &params) const
{
    QLocale locale = params.value("locale").toLocale();

    QVariantMap returns;

    QVariantList stateList;
    DeviceClass deviceClass = NymeaCore::instance()->deviceManager()->findDeviceClass(DeviceClassId(params.value("deviceClassId").toString()));
    foreach (StateType stateType, deviceClass.stateTypes()) {
        stateType.setDisplayName(NymeaCore::instance()->deviceManager()->translate(deviceClass.pluginId(), stateType.displayName(), locale));
        stateList.append(pack(stateType));
    }
    returns.insert("stateTypes", stateList);
    return createReply(returns);
}

JsonReply* DeviceHandler::GetStateValue(const QVariantMap &params) const
{
    Device *device = NymeaCore::instance()->deviceManager()->findConfiguredDevice(DeviceId(params.value("deviceId").toString()));
    if (!device) {
        return createReply(statusToReply(Device::DeviceErrorDeviceNotFound));
    }
    StateTypeId stateTypeId = StateTypeId(params.value("stateTypeId").toString());
    if (!device->hasState(stateTypeId)) {
        return createReply(statusToReply(Device::DeviceErrorStateTypeNotFound));
    }

    QVariantMap returns = statusToReply(Device::DeviceErrorNoError);
    returns.insert("value", device->state(stateTypeId).value());
    return createReply(returns);
}

JsonReply *DeviceHandler::GetStateValues(const QVariantMap &params) const
{
    Device *device = NymeaCore::instance()->deviceManager()->findConfiguredDevice(DeviceId(params.value("deviceId").toString()));
    if (!device) {
        return createReply(statusToReply(Device::DeviceErrorDeviceNotFound));
    }

    QVariantMap returns = statusToReply(Device::DeviceErrorNoError);
    returns.insert("values", packDeviceStates(device));
    return createReply(returns);
}

JsonReply *DeviceHandler::BrowseDevice(const QVariantMap &params) const
{
    DeviceId deviceId = DeviceId(params.value("deviceId").toString());
    QString itemId = params.value("itemId").toString();

    JsonReply *jsonReply = createAsyncReply("BrowseDevice");

    BrowseResult *result = NymeaCore::instance()->deviceManager()->browseDevice(deviceId, itemId, params.value("locale").toLocale());
    connect(result, &BrowseResult::finished, jsonReply, [this, jsonReply, result](){

        QVariantMap returns = statusToReply(result->status());
        QVariantList list;
        foreach (const BrowserItem &item, result->items()) {
            list.append(packBrowserItem(item));
        }
        returns.insert("items", list);
        jsonReply->setData(returns);
        jsonReply->finished();
    });

    return jsonReply;
}

JsonReply *DeviceHandler::GetBrowserItem(const QVariantMap &params) const
{
    QVariantMap returns;
    DeviceId deviceId = DeviceId(params.value("deviceId").toString());
    QString itemId = params.value("itemId").toString();

    JsonReply *jsonReply = createAsyncReply("GetBrowserItem");

    BrowserItemResult *result = NymeaCore::instance()->deviceManager()->browserItemDetails(deviceId, itemId, params.value("locale").toLocale());
    connect(result, &BrowserItemResult::finished, jsonReply, [this, jsonReply, result](){
        QVariantMap params = statusToReply(result->status());
        if (result->status() == Device::DeviceErrorNoError) {
            params.insert("item", packBrowserItem(result->item()));
        }
        jsonReply->setData(params);
        jsonReply->finished();
    });

    return jsonReply;
}

Param DeviceHandler::unpackParam(const QVariantMap &param)
{
    if (param.keys().count() == 0)
        return Param();

    ParamTypeId paramTypeId = param.value("paramTypeId").toUuid();
    QVariant value = param.value("value");
    return Param(paramTypeId, value);
}

ParamList DeviceHandler::unpackParams(const QVariantList &params)
{
    ParamList paramList;
    foreach (const QVariant &paramVariant, params) {
        paramList.append(unpackParam(paramVariant.toMap()));
    }

    return paramList;
}

QVariantMap DeviceHandler::packParam(const Param &param)
{
    QVariantMap variantMap;
    variantMap.insert("paramTypeId", param.paramTypeId().toString());
    variantMap.insert("value", param.value());
    return variantMap;
}

QVariantList DeviceHandler::packParams(const ParamList &paramList)
{
    QVariantList ret;
    foreach (const Param &param, paramList) {
        ret << packParam(param);
    }
    return ret;
}

QVariantMap DeviceHandler::packDevice(Device *device)
{
    QVariantMap variant;
    variant.insert("id", device->id().toString());
    variant.insert("deviceClassId", device->deviceClassId().toString());
    variant.insert("name", device->name());
    variant.insert("params", packParams(device->params()));
    variant.insert("settings", packParams(device->settings()));

    if (!device->parentId().isNull())
        variant.insert("parentId", device->parentId().toString());

    variant.insert("states", packDeviceStates(device));
    variant.insert("setupComplete", device->setupComplete());
    return variant;
}

QVariantList DeviceHandler::packDeviceStates(Device *device)
{
    DeviceClass deviceClass = NymeaCore::instance()->deviceManager()->findDeviceClass(device->deviceClassId());
    QVariantList stateValues;
    foreach (const StateType &stateType, deviceClass.stateTypes()) {
        QVariantMap stateValue;
        stateValue.insert("stateTypeId", stateType.id().toString());
        stateValue.insert("value", device->stateValue(stateType.id()));
        stateValues.append(stateValue);
    }
    return stateValues;
}

QVariantMap DeviceHandler::packBrowserItem(const BrowserItem &item)
{
    QVariantMap ret;
    ret.insert("id", item.id());
    ret.insert("displayName", item.displayName());
    ret.insert("description", item.description());
    ret.insert("icon", enumValueName<BrowserItem::BrowserIcon>(item.icon()));
    if (item.extendedPropertiesFlags().testFlag(BrowserItem::ExtendedPropertiesMedia)) {
        ret.insert("mediaIcon", enumValueName<MediaBrowserItem::MediaBrowserIcon>(static_cast<MediaBrowserItem::MediaBrowserIcon>(item.extendedProperty("mediaIcon").toInt())));
    }
    ret.insert("thumbnail", item.thumbnail());
    ret.insert("executable", item.executable());
    ret.insert("browsable", item.browsable());
    ret.insert("disabled", item.disabled());
    QVariantList actionTypeIds;
    foreach (const ActionTypeId &id, item.actionTypeIds()) {
        actionTypeIds.append(id.toString());
    }
    ret.insert("actionTypeIds", actionTypeIds);
    return ret;
}

QVariantMap DeviceHandler::packParamType(const ParamType &paramType, const PluginId &pluginId, const QLocale &locale) const
{
    ParamType translatedParamType = paramType;
    translatedParamType.setDisplayName(NymeaCore::instance()->deviceManager()->translate(pluginId, paramType.displayName(), locale));
    return pack(translatedParamType).toMap();
}

QVariantMap DeviceHandler::packEventType(const EventType &eventType, const PluginId &pluginId, const QLocale &locale) const
{
    QVariantMap variant;
    variant.insert("id", eventType.id().toString());
    variant.insert("name", eventType.name());
    variant.insert("displayName", NymeaCore::instance()->deviceManager()->translate(pluginId, eventType.displayName(), locale));
    variant.insert("index", eventType.index());

    QVariantList paramTypes;
    foreach (const ParamType &paramType, eventType.paramTypes())
        paramTypes.append(packParamType(paramType, pluginId, locale));

    variant.insert("paramTypes", paramTypes);
    return variant;
}

QVariantMap DeviceHandler::packActionType(const ActionType &actionType, const PluginId &pluginId, const QLocale &locale) const
{
    QVariantMap variantMap;
    variantMap.insert("id", actionType.id().toString());
    variantMap.insert("name", actionType.name());
    variantMap.insert("displayName", NymeaCore::instance()->deviceManager()->translate(pluginId, actionType.displayName(), locale));
    variantMap.insert("index", actionType.index());
    QVariantList paramTypes;
    foreach (const ParamType &paramType, actionType.paramTypes())
        paramTypes.append(packParamType(paramType, pluginId, locale));

    variantMap.insert("paramTypes", paramTypes);
    return variantMap;
}

QVariantList DeviceHandler::packCreateMethods(DeviceClass::CreateMethods createMethods) const
{
    QVariantList ret;
    if (createMethods.testFlag(DeviceClass::CreateMethodUser))
        ret << "CreateMethodUser";

    if (createMethods.testFlag(DeviceClass::CreateMethodAuto))
        ret << "CreateMethodAuto";

    if (createMethods.testFlag(DeviceClass::CreateMethodDiscovery))
        ret << "CreateMethodDiscovery";

    return ret;
}


void DeviceHandler::pluginConfigChanged(const PluginId &id, const ParamList &config)
{
    QVariantMap params;
    params.insert("pluginId", id);
    QVariantList configList;
    foreach (const Param &param, config) {
        configList << packParam(param);
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
    params.insert("device", packDevice(device));
    emit DeviceAdded(params);
}

void DeviceHandler::deviceChangedNotification(Device *device)
{
    QVariantMap params;
    params.insert("device", packDevice(device));
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

QVariantMap DeviceHandler::statusToReply(Device::DeviceError status) const
{
    QVariantMap returns;
    returns.insert("deviceError", enumValueName<Device::DeviceError>(status));
    return returns;
}

}
