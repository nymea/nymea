/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2020, nymea GmbH
* Contact: contact@nymea.io
*
* This file is part of nymea.
* This project including source code and documentation is protected by
* copyright law, and remains the property of nymea GmbH. All rights, including
* reproduction, publication, editing and translation, are reserved. The use of
* this project is subject to the terms of a license agreement to be concluded
* with nymea GmbH in accordance with the terms of use of nymea GmbH, available
* under https://nymea.io/license
*
* GNU General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU General Public License as published by the Free Software
* Foundation, GNU version 3. This project is distributed in the hope that it
* will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
* of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
* Public License for more details.
*
* You should have received a copy of the GNU General Public License along with
* this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "integrationshandler.h"
#include "nymeacore.h"
#include "integrations/thingmanager.h"
#include "integrations/thing.h"
#include "integrations/integrationplugin.h"
#include "loggingcategories.h"
#include "types/thingclass.h"
#include "types/browseritem.h"
#include "types/mediabrowseritem.h"
#include "integrations/translator.h"
#include "integrations/thingdiscoveryinfo.h"
#include "integrations/thingpairinginfo.h"
#include "integrations/thingsetupinfo.h"
#include "integrations/browseresult.h"
#include "integrations/browseritemresult.h"

#include <QDebug>

namespace nymeaserver {

/*! Constructs a new \l DeviceHandler with the given \a parent. */
IntegrationsHandler::IntegrationsHandler(ThingManager *deviceManager, QObject *parent) :
    JsonHandler(parent),
    m_deviceManager(deviceManager)
{
    // Enums
    registerEnum<Thing::ThingError>();
    registerEnum<Thing::ThingSetupStatus>();
    registerEnum<ThingClass::SetupMethod>();
    registerEnum<ThingClass::CreateMethod, ThingClass::CreateMethods>();
    registerEnum<Types::Unit>();
    registerEnum<Types::InputType>();
    registerEnum<RuleEngine::RemovePolicy>();
    registerEnum<BrowserItem::BrowserIcon>();
    registerEnum<MediaBrowserItem::MediaBrowserIcon>();

    // Objects
    registerObject<ParamType, ParamTypes>();
    registerObject<Param, ParamList>();
    registerUncreatableObject<IntegrationPlugin, IntegrationPlugins>();
    registerObject<Vendor, Vendors>();
    registerObject<EventType, EventTypes>();
    registerObject<StateType, StateTypes>();
    registerObject<ActionType, ActionTypes>();
    registerObject<ThingClass, ThingClasses>();
    registerObject<ThingDescriptor, ThingDescriptors>();
    registerObject<Event>();
    registerObject<Action>();
    registerObject<State, States>();
    registerUncreatableObject<Thing, Things>();

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
    returns.insert("vendors", objectRef<Vendors>());
    registerMethod("GetSupportedVendors", description, params, returns);

    params.clear(); returns.clear();
    description = "Returns a list of supported Device classes, optionally filtered by vendorId.";
    params.insert("o:vendorId", enumValueName(Uuid));
    returns.insert("deviceClasses", objectRef<ThingClass>());
    registerMethod("GetSupportedDevices", description, params, returns);

    params.clear(); returns.clear();
    description = "Returns a list of loaded plugins.";
    returns.insert("plugins", objectRef<IntegrationPlugins>());
    registerMethod("GetPlugins", description, params, returns);

    params.clear(); returns.clear();
    description = "Get a plugin's params.";
    params.insert("pluginId", enumValueName(Uuid));
    returns.insert("deviceError", enumRef<Thing::ThingError>());
    returns.insert("o:configuration", objectRef<ParamList>());
    registerMethod("GetPluginConfiguration", description, params, returns);

    params.clear(); returns.clear();
    description = "Set a plugin's params.";
    params.insert("pluginId", enumValueName(Uuid));
    params.insert("configuration", objectRef<ParamList>());
    returns.insert("deviceError", enumRef<Thing::ThingError>());
    registerMethod("SetPluginConfiguration", description, params, returns);

    params.clear(); returns.clear();
    description = "Add a configured device with a setupMethod of SetupMethodJustAdd. "
                                          "For devices with a setupMethod different than SetupMethodJustAdd, use PairDevice. "
                                          "Devices with CreateMethodJustAdd require all parameters to be supplied here. "
                                          "Devices with CreateMethodDiscovery require the use of a deviceDescriptorId. For discovered "
                                          "devices params are not required and will be taken from the DeviceDescriptor, however, they "
                                          "may be overridden by supplying deviceParams.";
    params.insert("thingClassId", enumValueName(Uuid));
    params.insert("name", enumValueName(String));
    params.insert("o:deviceDescriptorId", enumValueName(Uuid));
    params.insert("o:deviceParams", objectRef<ParamList>());
    returns.insert("deviceError", enumRef<Thing::ThingError>());
    returns.insert("o:deviceId", enumValueName(Uuid));
    returns.insert("o:displayMessage", enumValueName(String));
    registerMethod("AddConfiguredDevice", description, params, returns);

    params.clear(); returns.clear();
    description = "Pair a device. "
                                 "Use this to set up or reconfigure devices for DeviceClasses with a setupMethod different than SetupMethodJustAdd. "
                                 "Depending on the CreateMethod and whether a new devices is set up or an existing one is reconfigured, different parameters "
                                 "are required:\n"
                                 "CreateMethodJustAdd takes the thingClassId and the parameters you want to have with that device.\n"
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
    params.insert("o:ThingClassId", enumValueName(Uuid));
    params.insert("o:name", enumValueName(String));
    params.insert("o:deviceDescriptorId", enumValueName(Uuid));
    params.insert("o:deviceParams", objectRef<ParamList>());
    params.insert("o:deviceId", enumValueName(Uuid));
    returns.insert("deviceError", enumRef<Thing::ThingError>());
    returns.insert("o:setupMethod", enumRef<ThingClass::SetupMethod>());
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
    returns.insert("deviceError", enumRef<Thing::ThingError>());
    returns.insert("o:displayMessage", enumValueName(String));
    returns.insert("o:deviceId", enumValueName(Uuid));
    registerMethod("ConfirmPairing", description, params, returns);

    // FIXME: Add thingError!!!
    params.clear(); returns.clear();
    description = "Returns a list of configured devices, optionally filtered by deviceId.";
    params.insert("o:deviceId", enumValueName(Uuid));
    returns.insert("devices", objectRef<Things>());
    registerMethod("GetConfiguredDevices", description, params, returns);

    params.clear(); returns.clear();
    description = "Performs a device discovery and returns the results. This function may take a while to return. "
                                           "Note that this method will include all the found devices, that is, including devices that may "
                                           "already have been added. Those devices will have deviceId set to the device id of the already "
                                           "added device. Such results may be used to reconfigure existing devices and might be filtered "
                                           "in cases where only unknown devices are of interest.";
    params.insert("thingClassId", enumValueName(Uuid));
    params.insert("o:discoveryParams", objectRef<ParamList>());
    returns.insert("deviceError", enumRef<Thing::ThingError>());
    returns.insert("o:displayMessage", enumValueName(String));
    returns.insert("o:deviceDescriptors", objectRef<ThingDescriptors>());
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
    params.insert("o:deviceParams", objectRef<ParamList>());
    returns.insert("deviceError", enumRef<Thing::ThingError>());
    returns.insert("o:displayMessage", enumValueName(String));
    registerMethod("ReconfigureDevice", description, params, returns);

    params.clear(); returns.clear();
    description = "Edit the name of a device. This method does not change the "
                                 "configuration of the device.";
    params.insert("deviceId", enumValueName(Uuid));
    params.insert("name", enumValueName(String));
    returns.insert("deviceError", enumRef<Thing::ThingError>());
    registerMethod("EditDevice", description, params, returns);

    params.clear(); returns.clear();
    description = "Change the settings of a device.";
    params.insert("deviceId", enumValueName(Uuid));
    params.insert("settings", objectRef<ParamList>());
    returns.insert("deviceError", enumRef<Thing::ThingError>());
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
    returns.insert("deviceError", enumRef<Thing::ThingError>());
    returns.insert("o:ruleIds", QVariantList() << enumValueName(Uuid));
    registerMethod("RemoveConfiguredDevice", description, params, returns);

    params.clear(); returns.clear();
    description = "Get event types for a specified thingClassId.";
    params.insert("thingClassId", enumValueName(Uuid));
    returns.insert("eventTypes", objectRef<EventTypes>());
    registerMethod("GetEventTypes", description, params, returns);

    params.clear(); returns.clear();
    description = "Get action types for a specified thingClassId.";
    params.insert("thingClassId", enumValueName(Uuid));
    returns.insert("actionTypes", objectRef<ActionTypes>());
    registerMethod("GetActionTypes", description, params, returns);

    params.clear(); returns.clear();
    description = "Get state types for a specified thingClassId.";
    params.insert("thingClassId", enumValueName(Uuid));
    returns.insert("stateTypes", objectRef<StateTypes>());
    registerMethod("GetStateTypes", description, params, returns);

    params.clear(); returns.clear();
    description = "Get the value of the given device and the given stateType";
    params.insert("deviceId", enumValueName(Uuid));
    params.insert("stateTypeId", enumValueName(Uuid));
    returns.insert("deviceError", enumRef<Thing::ThingError>());
    returns.insert("o:value", enumValueName(Variant));
    registerMethod("GetStateValue", description, params, returns);

    params.clear(); returns.clear();
    description = "Get all the state values of the given device.";
    params.insert("deviceId", enumValueName(Uuid));
    returns.insert("deviceError", enumRef<Thing::ThingError>());
    returns.insert("o:values", objectRef<States>());
    registerMethod("GetStateValues", description, params, returns);

    params.clear(); returns.clear();
    description = "Browse a device. If a DeviceClass indicates a device is browsable, this method will return the BrowserItems. If no parameter besides the deviceId is used, the root node of this device will be returned. Any returned item which is browsable can be passed as node. Results will be children of the given node.";
    params.insert("deviceId", enumValueName(Uuid));
    params.insert("o:itemId", enumValueName(String));
    returns.insert("deviceError", enumRef<Thing::ThingError>());
    returns.insert("items", QVariantList() << objectRef("BrowserItem"));
    registerMethod("BrowseDevice", description, params, returns);

    params.clear(); returns.clear();
    description = "Get a single item from the browser. This won't give any more info on an item than a regular browseDevice call, but it allows to fetch details of an item if only the ID is known.";
    params.insert("deviceId", enumValueName(Uuid));
    params.insert("o:itemId", enumValueName(String));
    returns.insert("deviceError", enumRef<Thing::ThingError>());
    returns.insert("o:item", objectRef("BrowserItem"));
    registerMethod("GetBrowserItem", description, params, returns);

    params.clear(); returns.clear();
    description = "Execute a single action.";
    params.insert("actionTypeId", enumValueName(Uuid));
    params.insert("deviceId", enumValueName(Uuid));
    params.insert("o:params", objectRef<ParamList>());
    returns.insert("deviceError", enumRef<Thing::ThingError>());
    returns.insert("o:displayMessage", enumValueName(String));
    registerMethod("ExecuteAction", description, params, returns);

    params.clear(); returns.clear();
    description = "Execute the item identified by itemId on the given device.";
    params.insert("deviceId", enumValueName(Uuid));
    params.insert("itemId", enumValueName(String));
    returns.insert("deviceError", enumRef<Thing::ThingError>());
    registerMethod("ExecuteBrowserItem", description, params, returns);

    params.clear(); returns.clear();
    description = "Execute the action for the browser item identified by actionTypeId and the itemId on the given device.";
    params.insert("deviceId", enumValueName(Uuid));
    params.insert("itemId", enumValueName(String));
    params.insert("actionTypeId", enumValueName(Uuid));
    params.insert("o:params", objectRef<ParamList>());
    returns.insert("deviceError", enumRef<Thing::ThingError>());
    registerMethod("ExecuteBrowserItemAction", description, params, returns);

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
    params.insert("device", objectRef<Thing>());
    registerNotification("DeviceAdded", description, params);

    params.clear(); returns.clear();
    description = "Emitted whenever the params or name of a Device are changed (by EditDevice or ReconfigureDevice).";
    params.insert("device", objectRef<Thing>());
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
    params.insert("configuration", objectRef<ParamList>());
    registerNotification("PluginConfigurationChanged", description, params);

    params.clear(); returns.clear();
    description = "Emitted whenever an Event is triggered.";
    params.insert("event", objectRef<Event>());
    registerNotification("EventTriggered", description, params);
    connect(NymeaCore::instance(), &NymeaCore::eventTriggered, this, [this](const Event &event){
        QVariantMap params;
        params.insert("event", pack(event));
        emit EventTriggered(params);
    });

    connect(NymeaCore::instance(), &NymeaCore::pluginConfigChanged, this, &IntegrationsHandler::pluginConfigChanged);
    connect(NymeaCore::instance(), &NymeaCore::thingStateChanged, this, &IntegrationsHandler::thingStateChanged);
    connect(NymeaCore::instance(), &NymeaCore::thingRemoved, this, &IntegrationsHandler::thingRemovedNotification);
    connect(NymeaCore::instance(), &NymeaCore::thingAdded, this, &IntegrationsHandler::deviceAddedNotification);
    connect(NymeaCore::instance(), &NymeaCore::thingChanged, this, &IntegrationsHandler::deviceChangedNotification);
    connect(NymeaCore::instance(), &NymeaCore::thingSettingChanged, this, &IntegrationsHandler::deviceSettingChangedNotification);
}

/*! Returns the name of the \l{IntegrationsHandler}. In this case \b Devices.*/
QString IntegrationsHandler::name() const
{
    return "Integrations";
}

JsonReply* IntegrationsHandler::GetSupportedVendors(const QVariantMap &params) const
{
    QLocale locale = params.value("locale").toLocale();

    QVariantList vendors;
    foreach (const Vendor &vendor, NymeaCore::instance()->thingManager()->supportedVendors()) {
        Vendor translatedVendor = NymeaCore::instance()->thingManager()->translateVendor(vendor, locale);
        vendors.append(pack(translatedVendor));
    }

    QVariantMap returns;
    returns.insert("vendors", vendors);
    return createReply(returns);
}

JsonReply* IntegrationsHandler::GetSupportedDevices(const QVariantMap &params) const
{
    QLocale locale = params.value("locale").toLocale();
    VendorId vendorId = VendorId(params.value("vendorId").toString());
    QVariantMap returns;
    QVariantList deviceClasses;
    foreach (const ThingClass &deviceClass, NymeaCore::instance()->thingManager()->supportedThings(vendorId)) {
        ThingClass translatedDeviceClass = NymeaCore::instance()->thingManager()->translateThingClass(deviceClass, locale);
        deviceClasses.append(pack(translatedDeviceClass));
    }

    returns.insert("deviceClasses", deviceClasses);
    return createReply(returns);
}

JsonReply *IntegrationsHandler::GetDiscoveredDevices(const QVariantMap &params) const
{
    QLocale locale = params.value("locale").toLocale();

    QVariantMap returns;

    ThingClassId thingClassId = ThingClassId(params.value("thingClassId").toString());
    ParamList discoveryParams = unpack<ParamList>(params.value("discoveryParams"));

    JsonReply *reply = createAsyncReply("GetDiscoveredDevices");
    ThingDiscoveryInfo *info = NymeaCore::instance()->thingManager()->discoverThings(thingClassId, discoveryParams);
    connect(info, &ThingDiscoveryInfo::finished, reply, [this, reply, info, locale](){
        QVariantMap returns;
        returns.insert("deviceError", enumValueName<Thing::ThingError>(info->status()));

        if (info->status() == Thing::ThingErrorNoError) {
            QVariantList deviceDescriptorList;
            foreach (const ThingDescriptor &deviceDescriptor, info->thingDescriptors()) {
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

JsonReply* IntegrationsHandler::GetPlugins(const QVariantMap &params) const
{
    QLocale locale = params.value("locale").toLocale();

    QVariantList plugins;
    foreach (IntegrationPlugin* plugin, NymeaCore::instance()->thingManager()->plugins()) {
        QVariantMap packedPlugin = pack(*plugin).toMap();
        packedPlugin["displayName"] = NymeaCore::instance()->thingManager()->translate(plugin->pluginId(), plugin->pluginDisplayName(), locale);
        plugins.append(packedPlugin);
    }

    QVariantMap returns;
    returns.insert("plugins", plugins);
    return createReply(returns);
}

JsonReply *IntegrationsHandler::GetPluginConfiguration(const QVariantMap &params) const
{
    QVariantMap returns;

    IntegrationPlugin *plugin = NymeaCore::instance()->thingManager()->plugins().findById(PluginId(params.value("pluginId").toString()));
    if (!plugin) {
        returns.insert("deviceError", enumValueName<Thing::ThingError>(Thing::ThingErrorPluginNotFound));
        return createReply(returns);
    }

    QVariantList paramVariantList;
    foreach (const Param &param, plugin->configuration()) {
        paramVariantList.append(pack(param));
    }
    returns.insert("configuration", paramVariantList);
    returns.insert("deviceError", enumValueName<Thing::ThingError>(Thing::ThingErrorNoError));
    return createReply(returns);
}

JsonReply* IntegrationsHandler::SetPluginConfiguration(const QVariantMap &params)
{
    QVariantMap returns;
    PluginId pluginId = PluginId(params.value("pluginId").toString());
    ParamList pluginParams = unpack<ParamList>(params.value("configuration"));
    Thing::ThingError result = NymeaCore::instance()->thingManager()->setPluginConfig(pluginId, pluginParams);
    returns.insert("deviceError",enumValueName<Thing::ThingError>(result));
    return createReply(returns);
}

JsonReply* IntegrationsHandler::AddConfiguredDevice(const QVariantMap &params)
{
    ThingClassId ThingClassId(params.value("thingClassId").toString());
    QString deviceName = params.value("name").toString();
    ParamList deviceParams = unpack<ParamList>(params.value("deviceParams"));
    ThingDescriptorId deviceDescriptorId(params.value("deviceDescriptorId").toString());
    QLocale locale = params.value("locale").toLocale();

    JsonReply *jsonReply = createAsyncReply("AddConfiguredDevice");

    ThingSetupInfo *info;
    if (deviceDescriptorId.isNull()) {
        info = NymeaCore::instance()->thingManager()->addConfiguredThing(ThingClassId, deviceParams, deviceName);
    } else {
        info = NymeaCore::instance()->thingManager()->addConfiguredThing(deviceDescriptorId, deviceParams, deviceName);
    }
    connect(info, &ThingSetupInfo::finished, jsonReply, [info, jsonReply, locale](){
        QVariantMap returns;
        returns.insert("deviceError", enumValueName<Thing::ThingError>(info->status()));

        if (!info->displayMessage().isEmpty()) {
            returns.insert("displayMessage", info->translatedDisplayMessage(locale));
        }

        if(info->status() == Thing::ThingErrorNoError) {
            returns.insert("deviceId", info->thing()->id());
        }
        jsonReply->setData(returns);
        jsonReply->finished();

    });
    return jsonReply;
}

JsonReply *IntegrationsHandler::PairDevice(const QVariantMap &params)
{
    QString deviceName = params.value("name").toString();
    ParamList deviceParams = unpack<ParamList>(params.value("deviceParams"));
    QLocale locale = params.value("locale").toLocale();

    ThingPairingInfo *info;
    if (params.contains("deviceDescriptorId")) {
        ThingDescriptorId deviceDescriptorId = ThingDescriptorId(params.value("deviceDescriptorId").toString());
        info = NymeaCore::instance()->thingManager()->pairThing(deviceDescriptorId, deviceParams, deviceName);
    } else if (params.contains("deviceId")) {
        ThingId deviceId = ThingId(params.value("deviceId").toString());
        info = NymeaCore::instance()->thingManager()->pairThing(deviceId, deviceParams, deviceName);
    } else {
        ThingClassId thingClassId(params.value("thingClassId").toString());
        info = NymeaCore::instance()->thingManager()->pairThing(thingClassId, deviceParams, deviceName);
    }

    JsonReply *jsonReply = createAsyncReply("PairDevice");

    connect(info, &ThingPairingInfo::finished, jsonReply, [jsonReply, info, locale](){
        QVariantMap returns;
        returns.insert("deviceError", enumValueName<Thing::ThingError>(info->status()));
        returns.insert("pairingTransactionId", info->transactionId().toString());

        if (info->status() == Thing::ThingErrorNoError) {
            ThingClass thingClass = NymeaCore::instance()->thingManager()->findThingClass(info->thingClassId());
            returns.insert("setupMethod", enumValueName<ThingClass::SetupMethod>(thingClass.setupMethod()));
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

JsonReply *IntegrationsHandler::ConfirmPairing(const QVariantMap &params)
{
    PairingTransactionId pairingTransactionId = PairingTransactionId(params.value("pairingTransactionId").toString());
    QString secret = params.value("secret").toString();
    QString username = params.value("username").toString();
    QLocale locale = params.value("locale").toLocale();

    JsonReply *jsonReply = createAsyncReply("ConfirmPairing");

    ThingPairingInfo *info = NymeaCore::instance()->thingManager()->confirmPairing(pairingTransactionId, username, secret);
    connect(info, &ThingPairingInfo::finished, jsonReply, [info, jsonReply, locale](){

        QVariantMap returns;
        returns.insert("deviceError", enumValueName<Thing::ThingError>(info->status()));
        if (!info->displayMessage().isEmpty()) {
            returns.insert("displayMessage", info->translatedDisplayMessage(locale));
        }
        if (info->status() == Thing::ThingErrorNoError) {
            returns.insert("deviceId", info->thingId().toString());
        }
        jsonReply->setData(returns);
        jsonReply->finished();
    });

    return jsonReply;
}

JsonReply* IntegrationsHandler::GetConfiguredDevices(const QVariantMap &params) const
{
    QVariantMap returns;
    QVariantList configuredDeviceList;
    if (params.contains("deviceId")) {
        Thing *device = NymeaCore::instance()->thingManager()->findConfiguredThing(ThingId(params.value("deviceId").toString()));
        if (!device) {
            returns.insert("deviceError", enumValueName<Thing::ThingError>(Thing::ThingErrorThingNotFound));
            return createReply(returns);
        } else {
            configuredDeviceList.append(pack(device));
        }
    } else {
        foreach (Thing *device, NymeaCore::instance()->thingManager()->configuredThings()) {
            configuredDeviceList.append(pack(device));
        }
    }
    returns.insert("devices", configuredDeviceList);
    return createReply(returns);
}

JsonReply *IntegrationsHandler::ReconfigureDevice(const QVariantMap &params)
{
    ThingId thingId = ThingId(params.value("deviceId").toString());
    ParamList deviceParams = unpack<ParamList>(params.value("deviceParams"));
    ThingDescriptorId deviceDescriptorId(params.value("deviceDescriptorId").toString());
    QLocale locale = params.value("locale").toLocale();

    JsonReply *jsonReply = createAsyncReply("ReconfigureDevice");

    ThingSetupInfo *info;
    if (!deviceDescriptorId.isNull()) {
        info = NymeaCore::instance()->thingManager()->reconfigureThing(deviceDescriptorId, deviceParams);
    } else if (!thingId.isNull()){
        info = NymeaCore::instance()->thingManager()->reconfigureThing(thingId, deviceParams);
    } else {
        qCWarning(dcJsonRpc()) << "Either deviceId or deviceDescriptorId are required";
        QVariantMap ret;
        ret.insert("deviceError", enumValueName(Thing::ThingErrorMissingParameter));
        return createReply(ret);
    }

    connect(info, &ThingSetupInfo::finished, jsonReply, [info, jsonReply, locale](){

        QVariantMap returns;
        returns.insert("deviceError", enumValueName<Thing::ThingError>(info->status()));
        returns.insert("displayMessage", info->translatedDisplayMessage(locale));
        jsonReply->setData(returns);
        jsonReply->finished();

    });

    return jsonReply;
}

JsonReply *IntegrationsHandler::EditDevice(const QVariantMap &params)
{
    ThingId thingId = ThingId(params.value("deviceId").toString());
    QString name = params.value("name").toString();

    qCDebug(dcJsonRpc()) << "Edit device" << thingId << name;

    Thing::ThingError status = NymeaCore::instance()->thingManager()->editThing(thingId, name);

    return createReply(statusToReply(status));
}

JsonReply* IntegrationsHandler::RemoveConfiguredDevice(const QVariantMap &params)
{
    QVariantMap returns;
    ThingId thingId = ThingId(params.value("deviceId").toString());

    // global removePolicy has priority
    if (params.contains("removePolicy")) {
        RuleEngine::RemovePolicy removePolicy = params.value("removePolicy").toString() == "RemovePolicyCascade" ? RuleEngine::RemovePolicyCascade : RuleEngine::RemovePolicyUpdate;
        Thing::ThingError status = NymeaCore::instance()->removeConfiguredThing(thingId, removePolicy);
        returns.insert("deviceError", enumValueName<Thing::ThingError>(status));
        return createReply(returns);
    }

    QHash<RuleId, RuleEngine::RemovePolicy> removePolicyList;
    foreach (const QVariant &variant, params.value("removePolicyList").toList()) {
        RuleId ruleId = RuleId(variant.toMap().value("ruleId").toString());
        RuleEngine::RemovePolicy policy = variant.toMap().value("policy").toString() == "RemovePolicyCascade" ? RuleEngine::RemovePolicyCascade : RuleEngine::RemovePolicyUpdate;
        removePolicyList.insert(ruleId, policy);
    }

    QPair<Thing::ThingError, QList<RuleId> > status = NymeaCore::instance()->removeConfiguredThing(thingId, removePolicyList);
    returns.insert("deviceError", enumValueName<Thing::ThingError>(status.first));

    if (!status.second.isEmpty()) {
        QVariantList ruleIdList;
        foreach (const RuleId &ruleId, status.second) {
            ruleIdList.append(ruleId.toString());
        }
        returns.insert("ruleIds", ruleIdList);
    }

    return createReply(returns);
}

JsonReply *IntegrationsHandler::SetDeviceSettings(const QVariantMap &params)
{
    ThingId thingId = ThingId(params.value("deviceId").toString());
    ParamList settings = unpack<ParamList>(params.value("settings"));
    Thing::ThingError status = NymeaCore::instance()->thingManager()->setThingSettings(thingId, settings);
    return createReply(statusToReply(status));
}

JsonReply* IntegrationsHandler::GetEventTypes(const QVariantMap &params) const
{
    QLocale locale = params.value("locale").toLocale();

    ThingClass deviceClass = NymeaCore::instance()->thingManager()->findThingClass(ThingClassId(params.value("thingClassId").toString()));
    ThingClass translatedDeviceClass = NymeaCore::instance()->thingManager()->translateThingClass(deviceClass, locale);

    QVariantMap returns;
    returns.insert("eventTypes", pack(translatedDeviceClass.eventTypes()));
    return createReply(returns);
}

JsonReply* IntegrationsHandler::GetActionTypes(const QVariantMap &params) const
{
    QLocale locale = params.value("locale").toLocale();

    ThingClass deviceClass = NymeaCore::instance()->thingManager()->findThingClass(ThingClassId(params.value("thingClassId").toString()));
    ThingClass translatedDeviceClass = NymeaCore::instance()->thingManager()->translateThingClass(deviceClass, locale);

    QVariantMap returns;
    returns.insert("actionTypes", pack(translatedDeviceClass.actionTypes()));
    return createReply(returns);
}

JsonReply* IntegrationsHandler::GetStateTypes(const QVariantMap &params) const
{
    QLocale locale = params.value("locale").toLocale();

    ThingClass deviceClass = NymeaCore::instance()->thingManager()->findThingClass(ThingClassId(params.value("thingClassId").toString()));
    ThingClass translatedDeviceClass = NymeaCore::instance()->thingManager()->translateThingClass(deviceClass, locale);

    QVariantMap returns;
    returns.insert("stateTypes", pack(translatedDeviceClass.stateTypes()));
    return createReply(returns);
}

JsonReply* IntegrationsHandler::GetStateValue(const QVariantMap &params) const
{
    Thing *thing = NymeaCore::instance()->thingManager()->findConfiguredThing(ThingId(params.value("deviceId").toString()));
    if (!thing) {
        return createReply(statusToReply(Thing::ThingErrorThingNotFound));
    }
    StateTypeId stateTypeId = StateTypeId(params.value("stateTypeId").toString());
    if (!thing->hasState(stateTypeId)) {
        return createReply(statusToReply(Thing::ThingErrorStateTypeNotFound));
    }

    QVariantMap returns = statusToReply(Thing::ThingErrorNoError);
    returns.insert("value", thing->state(stateTypeId).value());
    return createReply(returns);
}

JsonReply *IntegrationsHandler::GetStateValues(const QVariantMap &params) const
{
    Thing *thing = NymeaCore::instance()->thingManager()->findConfiguredThing(ThingId(params.value("deviceId").toString()));
    if (!thing) {
        return createReply(statusToReply(Thing::ThingErrorThingNotFound));
    }

    QVariantMap returns = statusToReply(Thing::ThingErrorNoError);
    returns.insert("values", pack(thing->states()));
    return createReply(returns);
}

JsonReply *IntegrationsHandler::BrowseDevice(const QVariantMap &params) const
{
    ThingId thingId = ThingId(params.value("deviceId").toString());
    QString itemId = params.value("itemId").toString();

    JsonReply *jsonReply = createAsyncReply("BrowseDevice");

    BrowseResult *result = NymeaCore::instance()->thingManager()->browseThing(thingId, itemId, params.value("locale").toLocale());
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

JsonReply *IntegrationsHandler::GetBrowserItem(const QVariantMap &params) const
{
    QVariantMap returns;
    ThingId thingId = ThingId(params.value("deviceId").toString());
    QString itemId = params.value("itemId").toString();

    JsonReply *jsonReply = createAsyncReply("GetBrowserItem");

    BrowserItemResult *result = NymeaCore::instance()->thingManager()->browserItemDetails(thingId, itemId, params.value("locale").toLocale());
    connect(result, &BrowserItemResult::finished, jsonReply, [this, jsonReply, result](){
        QVariantMap params = statusToReply(result->status());
        if (result->status() == Thing::ThingErrorNoError) {
            params.insert("item", packBrowserItem(result->item()));
        }
        jsonReply->setData(params);
        jsonReply->finished();
    });

    return jsonReply;
}

JsonReply *IntegrationsHandler::ExecuteAction(const QVariantMap &params)
{
    ThingId thingId(params.value("deviceId").toString());
    ActionTypeId actionTypeId(params.value("actionTypeId").toString());
    ParamList actionParams = unpack<ParamList>(params.value("params"));
    QLocale locale = params.value("locale").toLocale();

    Action action(actionTypeId, thingId);
    action.setParams(actionParams);

    JsonReply *jsonReply = createAsyncReply("ExecuteAction");

    ThingActionInfo *info = NymeaCore::instance()->executeAction(action);
    connect(info, &ThingActionInfo::finished, jsonReply, [info, jsonReply, locale](){
        QVariantMap data;
        data.insert("deviceError", enumValueName(info->status()));
        if (!info->displayMessage().isEmpty()) {
            data.insert("displayMessage", info->translatedDisplayMessage(locale));
        }
        jsonReply->setData(data);
        jsonReply->finished();
    });

    return jsonReply;
}

JsonReply *IntegrationsHandler::ExecuteBrowserItem(const QVariantMap &params)
{
    ThingId thingId = ThingId(params.value("deviceId").toString());
    QString itemId = params.value("itemId").toString();
    BrowserAction action(thingId, itemId);

    JsonReply *jsonReply = createAsyncReply("ExecuteBrowserItem");

    BrowserActionInfo *info = NymeaCore::instance()->executeBrowserItem(action);
    connect(info, &BrowserActionInfo::finished, jsonReply, [info, jsonReply](){
        QVariantMap data;
        data.insert("deviceError", enumValueName<Thing::ThingError>(info->status()));
        jsonReply->setData(data);
        jsonReply->finished();
    });

    return jsonReply;
}

JsonReply *IntegrationsHandler::ExecuteBrowserItemAction(const QVariantMap &params)
{
    ThingId thingId = ThingId(params.value("deviceId").toString());
    QString itemId = params.value("itemId").toString();
    ActionTypeId actionTypeId = ActionTypeId(params.value("actionTypeId").toString());
    ParamList paramList = unpack<ParamList>(params.value("params"));
    BrowserItemAction browserItemAction(thingId, itemId, actionTypeId, paramList);

    JsonReply *jsonReply = createAsyncReply("ExecuteBrowserItemAction");

    BrowserItemActionInfo *info = NymeaCore::instance()->executeBrowserItemAction(browserItemAction);
    connect(info, &BrowserItemActionInfo::finished, jsonReply, [info, jsonReply](){
        QVariantMap data;
        data.insert("deviceError", enumValueName<Thing::ThingError>(info->status()));
        jsonReply->setData(data);
        jsonReply->finished();
    });

    return jsonReply;
}

QVariantMap IntegrationsHandler::packBrowserItem(const BrowserItem &item)
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

void IntegrationsHandler::pluginConfigChanged(const PluginId &id, const ParamList &config)
{
    QVariantMap params;
    params.insert("pluginId", id);
    QVariantList configList;
    foreach (const Param &param, config) {
        configList << pack(param);
    }
    params.insert("configuration", configList);
    emit PluginConfigurationChanged(params);
}

void IntegrationsHandler::thingStateChanged(Thing *thing, const QUuid &stateTypeId, const QVariant &value)
{
    QVariantMap params;
    params.insert("thingId", thing->id());
    params.insert("stateTypeId", stateTypeId);
    params.insert("value", value);
    emit StateChanged(params);
}

void IntegrationsHandler::thingRemovedNotification(const ThingId &thingId)
{
    QVariantMap params;
    params.insert("thingId", thingId);
    emit DeviceRemoved(params);
}

void IntegrationsHandler::deviceAddedNotification(Thing *device)
{
    QVariantMap params;
    params.insert("device", pack(device));
    emit DeviceAdded(params);
}

void IntegrationsHandler::deviceChangedNotification(Thing *device)
{
    QVariantMap params;
    params.insert("device", pack(device));
    emit DeviceChanged(params);
}

void IntegrationsHandler::deviceSettingChangedNotification(const ThingId &thingId, const ParamTypeId &paramTypeId, const QVariant &value)
{
    QVariantMap params;
    params.insert("deviceId", thingId);
    params.insert("paramTypeId", paramTypeId.toString());
    params.insert("value", value);
    emit DeviceSettingChanged(params);
}

QVariantMap IntegrationsHandler::statusToReply(Thing::ThingError status) const
{
    QVariantMap returns;
    returns.insert("deviceError", enumValueName<Thing::ThingError>(status));
    return returns;
}

}
