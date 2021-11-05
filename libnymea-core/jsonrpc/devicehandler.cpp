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

#include "devicehandler.h"
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
#include <QJsonDocument>

namespace nymeaserver {

DeviceHandler::DeviceHandler(QObject *parent) :
    JsonHandler(parent)
{
    // Enums
    registerEnum<Device::DeviceError>();
    registerEnum<Thing::ThingError>();
    registerEnum<Device::DeviceSetupStatus>();
    registerEnum<Thing::ThingSetupStatus>();
    registerEnum<DeviceClass::SetupMethod>();
    registerFlag<DeviceClass::CreateMethod, DeviceClass::CreateMethods>();
    registerEnum<Types::Unit>();
    registerEnum<Types::InputType>();
    registerEnum<Types::IOType>();
    registerEnum<Types::StateValueFilter>();
    registerEnum<RuleEngine::RemovePolicy>();
    registerEnum<BrowserItem::BrowserIcon>();
    registerEnum<MediaBrowserItem::MediaBrowserIcon>();

    // Objects
    registerObject<ParamType, ParamTypes>();
    registerObject<Param, ParamList>();
    registerUncreatableObject<DevicePlugin, DevicePlugins>();
    registerUncreatableObject<IntegrationPlugin, IntegrationPlugins>();
    registerObject<Vendor, Vendors>();
    registerObject<EventType, EventTypes>();
    registerObject<StateType, StateTypes>();
    registerObject<ActionType, ActionTypes>();
    registerObject<DeviceClass, DeviceClasses>();
    registerObject<DeviceDescriptor, DeviceDescriptors>();
    registerObject<ThingDescriptor, ThingDescriptors>();
    registerObject<Event>();
    registerObject<Action>();
    registerObject<State, States>();
    registerUncreatableObject<Device, Devices>();
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
    returns.insert("deviceClasses", objectRef<DeviceClasses>());
    registerMethod("GetSupportedDevices", description, params, returns);

    params.clear(); returns.clear();
    description = "Returns a list of loaded plugins.";
    returns.insert("plugins", objectRef<DevicePlugins>());
    registerMethod("GetPlugins", description, params, returns);

    params.clear(); returns.clear();
    description = "Get a plugin's params.";
    params.insert("pluginId", enumValueName(Uuid));
    returns.insert("deviceError", enumRef<Device::DeviceError>());
    returns.insert("o:configuration", objectRef<ParamList>());
    registerMethod("GetPluginConfiguration", description, params, returns);

    params.clear(); returns.clear();
    description = "Set a plugin's params.";
    params.insert("pluginId", enumValueName(Uuid));
    params.insert("configuration", objectRef<ParamList>());
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
    params.insert("o:deviceParams", objectRef<ParamList>());
    returns.insert("deviceError", enumRef<Device::DeviceError>());
    returns.insert("o:deviceId", enumValueName(Uuid));
    returns.insert("o:displayMessage", enumValueName(String));
    registerMethod("AddConfiguredDevice", description, params, returns);

    params.clear(); returns.clear();
    description = "Pair a device. "
                                 "Use this to set up or reconfigure devices for DeviceClasses with a setupMethod different than SetupMethodJustAdd. "
                                 "Depending on the CreateMethod and whether a new devices is set up or an existing one is reconfigured, different parameters "
                                 "are required:\n"
                                 "CreateMethodJustAdd takes the deviceClassId and the parameters to be used with that device. "
                                 "If an existing device should be reconfigured, the deviceId of said device should be given additionally.\n"
                                 "CreateMethodDiscovery requires the use of a deviceDescriptorId, previously obtained with DiscoverDevices. Optionally, "
                                 "parameters can be overridden with the give deviceParams. DeviceDescriptors containing a deviceId will reconfigure that "
                                 "device, descriptors without deviceId will add a new one.\n"
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
    params.insert("o:deviceParams", objectRef<ParamList>());
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
    returns.insert("devices", objectRef<Devices>());
    registerMethod("GetConfiguredDevices", description, params, returns);

    params.clear(); returns.clear();
    description = "Performs a device discovery and returns the results. This function may take a while to return. "
                                           "Note that this method will include all the found devices, that is, including devices that "
                                           "already have been added. Those devices will have deviceId set to the device id of the already "
                                           "added device. Such results may be used to reconfigure existing devices and might be filtered "
                                           "in cases where only unknown devices are of interest.";
    params.insert("deviceClassId", enumValueName(Uuid));
    params.insert("o:discoveryParams", objectRef<ParamList>());
    returns.insert("deviceError", enumRef<Device::DeviceError>());
    returns.insert("o:displayMessage", enumValueName(String));
    returns.insert("o:deviceDescriptors", objectRef<DeviceDescriptors>());
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
    params.insert("settings", objectRef<ParamList>());
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
    returns.insert("eventTypes", objectRef<EventTypes>());
    registerMethod("GetEventTypes", description, params, returns);

    params.clear(); returns.clear();
    description = "Get action types for a specified deviceClassId.";
    params.insert("deviceClassId", enumValueName(Uuid));
    returns.insert("actionTypes", objectRef<ActionTypes>());
    registerMethod("GetActionTypes", description, params, returns);

    params.clear(); returns.clear();
    description = "Get state types for a specified deviceClassId.";
    params.insert("deviceClassId", enumValueName(Uuid));
    returns.insert("stateTypes", objectRef<StateTypes>());
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
    returns.insert("o:values", objectRef<States>());
    registerMethod("GetStateValues", description, params, returns);

    params.clear(); returns.clear();
    description = "Browse a device. If a DeviceClass indicates a device is browsable, this method will return "
                  "the BrowserItems. If no parameter besides the deviceId is used, the root node of this device "
                  "will be returned. Any returned item which is browsable can be passed as node. Results will be "
                  "children of the given node.\n"
                  "In case of an error during browsing, the error will be indicated and the displayMessage may contain "
                  "additional information for the user. The displayMessage will be translated. A client UI showing this "
                  "message to the user should be prepared for empty, but also longer strings.";
    params.insert("deviceId", enumValueName(Uuid));
    params.insert("o:itemId", enumValueName(String));
    returns.insert("deviceError", enumRef<Device::DeviceError>());
    returns.insert("o:displayMessage", enumValueName(String));
    returns.insert("items", QVariantList() << objectRef("BrowserItem"));
    registerMethod("BrowseDevice", description, params, returns);

    params.clear(); returns.clear();
    description = "Get a single item from the browser. This won't give any more info on an item than a regular browseDevice "
                  "call, but it allows to fetch details of an item if only the ID is known.\n"
                  "In case of an error during browsing, the error will be indicated and the displayMessage may contain "
                  "additional information for the user. The displayMessage will be translated. A client UI showing this "
                  "message to the user should be prepared for empty, but also longer strings.";
    params.insert("deviceId", enumValueName(Uuid));
    params.insert("o:itemId", enumValueName(String));
    returns.insert("deviceError", enumRef<Device::DeviceError>());
    returns.insert("o:displayMessage", enumValueName(String));
    returns.insert("o:item", objectRef("BrowserItem"));
    registerMethod("GetBrowserItem", description, params, returns);

    params.clear(); returns.clear();
    description = "Execute a single action.";
    params.insert("actionTypeId", enumValueName(Uuid));
    params.insert("deviceId", enumValueName(Uuid));
    params.insert("o:params", objectRef<ParamList>());
    returns.insert("deviceError", enumRef<Device::DeviceError>());
    returns.insert("o:displayMessage", enumValueName(String));
    registerMethod("ExecuteAction", description, params, returns);

    params.clear(); returns.clear();
    description = "Execute the item identified by itemId on the given device.\n"
                  "In case of an error during execution, the error will be indicated and the displayMessage may contain "
                  "additional information for the user. The displayMessage will be translated. A client UI showing this "
                  "message to the user should be prepared for empty, but also longer strings.";
    params.insert("deviceId", enumValueName(Uuid));
    params.insert("itemId", enumValueName(String));
    returns.insert("deviceError", enumRef<Device::DeviceError>());
    returns.insert("o:displayMessage", enumValueName(String));
    registerMethod("ExecuteBrowserItem", description, params, returns);

    params.clear(); returns.clear();
    description = "Execute the action for the browser item identified by actionTypeId and the itemId on the given device.\n"
                  "In case of an error during execution, the error will be indicated and the displayMessage may contain "
                  "additional information for the user. The displayMessage will be translated. A client UI showing this "
                  "message to the user should be prepared for empty, but also longer strings.";
    params.insert("deviceId", enumValueName(Uuid));
    params.insert("itemId", enumValueName(String));
    params.insert("actionTypeId", enumValueName(Uuid));
    params.insert("o:params", objectRef<ParamList>());
    returns.insert("deviceError", enumRef<Device::DeviceError>());
    returns.insert("o:displayMessage", enumValueName(String));
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
    params.insert("device", objectRef<Device>());
    registerNotification("DeviceAdded", description, params);

    params.clear(); returns.clear();
    description = "Emitted whenever the params, name or setupStatus of a Device changes.";
    params.insert("device", objectRef<Device>());
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

    connect(NymeaCore::instance(), &NymeaCore::pluginConfigChanged, this, &DeviceHandler::pluginConfigChanged);
    connect(NymeaCore::instance(), &NymeaCore::thingStateChanged, this, &DeviceHandler::deviceStateChanged);
    connect(NymeaCore::instance(), &NymeaCore::thingRemoved, this, &DeviceHandler::deviceRemovedNotification);
    connect(NymeaCore::instance(), &NymeaCore::thingAdded, this, &DeviceHandler::deviceAddedNotification);
    connect(NymeaCore::instance(), &NymeaCore::thingChanged, this, &DeviceHandler::deviceChangedNotification);
    connect(NymeaCore::instance(), &NymeaCore::thingSettingChanged, this, &DeviceHandler::deviceSettingChangedNotification);

    connect(NymeaCore::instance(), &NymeaCore::initialized, this, [=](){
        // Generating cache hashes.
        // NOTE: We need to sort the lists to get a stable result
        QHash<ThingClassId, ThingClass> thingClassesMap;
        foreach (const ThingClass &tc, NymeaCore::instance()->thingManager()->supportedThings()) {
            thingClassesMap.insert(tc.id(), tc);
        }
        QList<ThingClassId> thingClassIds = thingClassesMap.keys();
        std::sort(thingClassIds.begin(), thingClassIds.end());
        DeviceClasses thingClasses;
        foreach (const ThingClassId &id, thingClassIds) {
            thingClasses.append(thingClassesMap.value(id));
        }
        QByteArray hash = QCryptographicHash::hash(QJsonDocument::fromVariant(pack(thingClasses)).toJson(), QCryptographicHash::Md5).toHex();
        m_cacheHashes.insert("GetSupportedDevices", hash);

        QHash<VendorId, Vendor> vendorsMap;
        foreach (const Vendor &v, NymeaCore::instance()->thingManager()->supportedVendors()) {
            vendorsMap.insert(v.id(), v);
        }
        QList<VendorId> vendorIds = vendorsMap.keys();
        std::sort(vendorIds.begin(), vendorIds.end());
        Vendors vendors;
        foreach (const VendorId &id, vendorIds) {
            vendors.append(vendorsMap.value(id));
        }
        hash = QCryptographicHash::hash(QJsonDocument::fromVariant(pack(vendors)).toJson(), QCryptographicHash::Md5).toHex();
        m_cacheHashes.insert("GetSupportedVendors", hash);
    });

}

/*! Returns the name of the \l{DeviceHandler}. In this case \b Devices.*/
QString DeviceHandler::name() const
{
    return "Devices";
}

QHash<QString, QString> DeviceHandler::cacheHashes() const
{
    return m_cacheHashes;
}

JsonReply* DeviceHandler::GetSupportedVendors(const QVariantMap &params, const JsonContext &context) const
{
    Q_UNUSED(params)
    QVariantList vendors;
    foreach (const Vendor &vendor, NymeaCore::instance()->thingManager()->supportedVendors()) {
        Vendor translatedVendor = NymeaCore::instance()->thingManager()->translateVendor(vendor, context.locale());
        vendors.append(pack(translatedVendor));
    }

    QVariantMap returns;
    returns.insert("vendors", vendors);
    return createReply(returns);
}

JsonReply* DeviceHandler::GetSupportedDevices(const QVariantMap &params, const JsonContext &context) const
{
    VendorId vendorId = VendorId(params.value("vendorId").toString());
    QVariantMap returns;
    QVariantList deviceClasses;
    foreach (const DeviceClass &deviceClass, NymeaCore::instance()->thingManager()->supportedThings(vendorId)) {
        DeviceClass translatedDeviceClass = NymeaCore::instance()->thingManager()->translateThingClass(deviceClass, context.locale());
        deviceClasses.append(pack(translatedDeviceClass));
    }

    returns.insert("deviceClasses", deviceClasses);
    return createReply(returns);
}

JsonReply *DeviceHandler::GetDiscoveredDevices(const QVariantMap &params, const JsonContext &context) const
{
    QLocale locale = context.locale();

    QVariantMap returns;

    ThingClassId thingClassId = ThingClassId(params.value("deviceClassId").toString());
    ParamList discoveryParams = unpack<ParamList>(params.value("discoveryParams"));

    JsonReply *reply = createAsyncReply("GetDiscoveredDevices");
    ThingDiscoveryInfo *info = NymeaCore::instance()->thingManager()->discoverThings(thingClassId, discoveryParams);
    connect(info, &ThingDiscoveryInfo::finished, reply, [this, reply, info, locale](){
        QVariantMap returns;
        returns.insert("deviceError", enumValueName<Device::ThingError>(info->status()).replace("Thing", "Device"));

        if (info->status() == Device::ThingErrorNoError) {
            QVariantList deviceDescriptorList;
            foreach (const ThingDescriptor &thingDescriptor, info->thingDescriptors()) {
                QVariantMap packedDescriptor = pack(thingDescriptor).toMap();
                if (packedDescriptor.contains("thingId")) {
                    packedDescriptor.insert("deviceId", packedDescriptor.value("thingId"));
                }
                deviceDescriptorList.append(packedDescriptor);
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

JsonReply* DeviceHandler::GetPlugins(const QVariantMap &params, const JsonContext &context) const
{
    Q_UNUSED(params)
    QVariantList plugins;
    foreach (IntegrationPlugin* plugin, NymeaCore::instance()->thingManager()->plugins()) {
        QVariantMap packedPlugin = pack(*plugin).toMap();
        packedPlugin["displayName"] = NymeaCore::instance()->thingManager()->translate(plugin->pluginId(), plugin->pluginDisplayName(), context.locale());
        plugins.append(packedPlugin);
    }

    QVariantMap returns;
    returns.insert("plugins", plugins);
    return createReply(returns);
}

JsonReply *DeviceHandler::GetPluginConfiguration(const QVariantMap &params) const
{
    QVariantMap returns;

    IntegrationPlugin *plugin = NymeaCore::instance()->thingManager()->plugins().findById(PluginId(params.value("pluginId").toString()));
    if (!plugin) {
        returns.insert("deviceError", enumValueName<Device::ThingError>(Device::ThingErrorPluginNotFound).replace("Thing", "Device"));
        return createReply(returns);
    }

    QVariantList paramVariantList;
    foreach (const Param &param, plugin->configuration()) {
        paramVariantList.append(pack(param));
    }
    returns.insert("configuration", paramVariantList);
    returns.insert("deviceError", enumValueName<Device::ThingError>(Device::ThingErrorNoError).replace("Thing", "Device"));
    return createReply(returns);
}

JsonReply* DeviceHandler::SetPluginConfiguration(const QVariantMap &params)
{
    QVariantMap returns;
    PluginId pluginId = PluginId(params.value("pluginId").toString());
    ParamList pluginParams = unpack<ParamList>(params.value("configuration"));
    Device::ThingError result = NymeaCore::instance()->thingManager()->setPluginConfig(pluginId, pluginParams);
    returns.insert("deviceError",enumValueName<Device::ThingError>(result).replace("Thing", "Device"));
    return createReply(returns);
}

JsonReply* DeviceHandler::AddConfiguredDevice(const QVariantMap &params, const JsonContext &context)
{
    ThingClassId thingClassId(params.value("deviceClassId").toString());
    QString deviceName = params.value("name").toString();
    ParamList deviceParams = unpack<ParamList>(params.value("deviceParams"));
    ThingDescriptorId deviceDescriptorId(params.value("deviceDescriptorId").toString());
    QLocale locale = context.locale();

    JsonReply *jsonReply = createAsyncReply("AddConfiguredDevice");

    ThingSetupInfo *info;
    if (deviceDescriptorId.isNull()) {
        info = NymeaCore::instance()->thingManager()->addConfiguredThing(thingClassId, deviceParams, deviceName);
    } else {
        info = NymeaCore::instance()->thingManager()->addConfiguredThing(deviceDescriptorId, deviceParams, deviceName);
    }
    connect(info, &ThingSetupInfo::finished, jsonReply, [info, jsonReply, locale](){
        QVariantMap returns;
        returns.insert("deviceError", enumValueName<Device::ThingError>(info->status()).replace("Thing", "Device"));

        if (!info->displayMessage().isEmpty()) {
            returns.insert("displayMessage", info->translatedDisplayMessage(locale));
        }

        if(info->status() == Device::ThingErrorNoError) {
            returns.insert("deviceId", info->thing()->id());
        }
        jsonReply->setData(returns);
        jsonReply->finished();

    });
    return jsonReply;
}

JsonReply *DeviceHandler::PairDevice(const QVariantMap &params, const JsonContext &context)
{
    QString deviceName = params.value("name").toString();
    ParamList deviceParams = unpack<ParamList>(params.value("deviceParams"));
    QLocale locale = context.locale();

    ThingPairingInfo *info;
    if (params.contains("deviceDescriptorId")) {
        ThingDescriptorId deviceDescriptorId = ThingDescriptorId(params.value("deviceDescriptorId").toString());
        info = NymeaCore::instance()->thingManager()->pairThing(deviceDescriptorId, deviceParams, deviceName);
    } else if (params.contains("deviceId")) {
        ThingId thingId = ThingId(params.value("deviceId").toString());
        info = NymeaCore::instance()->thingManager()->pairThing(thingId, deviceParams, deviceName);
    } else {
        ThingClassId thingClassId(params.value("deviceClassId").toString());
        info = NymeaCore::instance()->thingManager()->pairThing(thingClassId, deviceParams, deviceName);
    }

    JsonReply *jsonReply = createAsyncReply("PairDevice");

    connect(info, &ThingPairingInfo::finished, jsonReply, [jsonReply, info, locale](){
        QVariantMap returns;
        returns.insert("deviceError", enumValueName<Device::ThingError>(info->status()).replace("Thing", "Device"));
        returns.insert("pairingTransactionId", info->transactionId().toString());

        if (info->status() == Device::ThingErrorNoError) {
            ThingClass thingClass = NymeaCore::instance()->thingManager()->findThingClass(info->thingClassId());
            returns.insert("setupMethod", enumValueName<DeviceClass::SetupMethod>(thingClass.setupMethod()));
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

JsonReply *DeviceHandler::ConfirmPairing(const QVariantMap &params, const JsonContext &context)
{
    PairingTransactionId pairingTransactionId = PairingTransactionId(params.value("pairingTransactionId").toString());
    QString secret = params.value("secret").toString();
    QString username = params.value("username").toString();
    QLocale locale = context.locale();

    JsonReply *jsonReply = createAsyncReply("ConfirmPairing");

    ThingPairingInfo *info = NymeaCore::instance()->thingManager()->confirmPairing(pairingTransactionId, username, secret);
    connect(info, &ThingPairingInfo::finished, jsonReply, [info, jsonReply, locale](){

        QVariantMap returns;
        returns.insert("deviceError", enumValueName<Device::ThingError>(info->status()).replace("Thing", "Device"));
        if (!info->displayMessage().isEmpty()) {
            returns.insert("displayMessage", info->translatedDisplayMessage(locale));
        }
        if (info->status() == Device::ThingErrorNoError) {
            returns.insert("deviceId", info->thingId().toString());
        }
        jsonReply->setData(returns);
        jsonReply->finished();
    });

    return jsonReply;
}

JsonReply* DeviceHandler::GetConfiguredDevices(const QVariantMap &params, const JsonContext &context) const
{
    QVariantMap returns;
    QVariantList configuredDeviceList;
    if (params.contains("deviceId")) {
        Thing *thing = NymeaCore::instance()->thingManager()->findConfiguredThing(ThingId(params.value("deviceId").toString()));
        if (!thing) {
            returns.insert("devices", QVariantList());
            return createReply(returns);
        } else {
            QVariantMap deviceMap = pack(thing).toMap();
            // Patch in deviceClassId
            deviceMap.insert("deviceClassId", deviceMap.value("thingClassId"));
            configuredDeviceList.append(deviceMap);
        }
    } else {
        foreach (Thing *thing, NymeaCore::instance()->thingManager()->configuredThings()) {
            QVariantMap packedThing = pack(thing).toMap();
            QString translatedSetupStatus = NymeaCore::instance()->thingManager()->translate(thing->pluginId(), thing->setupDisplayMessage(), context.locale());
            if (!translatedSetupStatus.isEmpty()) {
                packedThing["setupDisplayMessage"] = translatedSetupStatus;
            }
            // Patch in deviceClassId
            packedThing.insert("deviceClassId", packedThing.value("thingClassId"));
            configuredDeviceList.append(packedThing);
        }
    }
    returns.insert("devices", configuredDeviceList);
    return createReply(returns);
}

JsonReply *DeviceHandler::ReconfigureDevice(const QVariantMap &params, const JsonContext &context)
{
    ThingId thingId = ThingId(params.value("deviceId").toString());
    ParamList deviceParams = unpack<ParamList>(params.value("deviceParams"));
    ThingDescriptorId deviceDescriptorId(params.value("deviceDescriptorId").toString());
    QLocale locale = context.locale();

    JsonReply *jsonReply = createAsyncReply("ReconfigureDevice");

    ThingSetupInfo *info;
    if (!deviceDescriptorId.isNull()) {
        info = NymeaCore::instance()->thingManager()->reconfigureThing(deviceDescriptorId, deviceParams);
    } else if (!thingId.isNull()){
        info = NymeaCore::instance()->thingManager()->reconfigureThing(thingId, deviceParams);
    } else {
        qCWarning(dcJsonRpc()) << "Either deviceId or deviceDescriptorId are required";
        QVariantMap ret;
        ret.insert("deviceError", enumValueName(Device::ThingErrorMissingParameter));
        return createReply(ret);
    }

    connect(info, &ThingSetupInfo::finished, jsonReply, [info, jsonReply, locale](){

        QVariantMap returns;
        returns.insert("deviceError", enumValueName<Device::ThingError>(info->status()).replace("Thing", "Device"));
        returns.insert("displayMessage", info->translatedDisplayMessage(locale));
        jsonReply->setData(returns);
        jsonReply->finished();

    });

    return jsonReply;
}

JsonReply *DeviceHandler::EditDevice(const QVariantMap &params)
{
    ThingId thingId = ThingId(params.value("deviceId").toString());
    QString name = params.value("name").toString();

    qCDebug(dcJsonRpc()) << "Edit device" << thingId << name;

    Device::ThingError status = NymeaCore::instance()->thingManager()->editThing(thingId, name);

    return createReply(statusToReply(status));
}

JsonReply* DeviceHandler::RemoveConfiguredDevice(const QVariantMap &params)
{
    QVariantMap returns;
    ThingId thingId = ThingId(params.value("deviceId").toString());

    // global removePolicy has priority
    if (params.contains("removePolicy")) {
        RuleEngine::RemovePolicy removePolicy = params.value("removePolicy").toString() == "RemovePolicyCascade" ? RuleEngine::RemovePolicyCascade : RuleEngine::RemovePolicyUpdate;
        Device::ThingError status = NymeaCore::instance()->removeConfiguredThing(thingId, removePolicy);
        returns.insert("deviceError", enumValueName<Device::ThingError>(status).replace("Thing", "Device"));
        return createReply(returns);
    }

    QHash<RuleId, RuleEngine::RemovePolicy> removePolicyList;
    foreach (const QVariant &variant, params.value("removePolicyList").toList()) {
        RuleId ruleId = RuleId(variant.toMap().value("ruleId").toString());
        RuleEngine::RemovePolicy policy = variant.toMap().value("policy").toString() == "RemovePolicyCascade" ? RuleEngine::RemovePolicyCascade : RuleEngine::RemovePolicyUpdate;
        removePolicyList.insert(ruleId, policy);
    }

    QPair<Device::ThingError, QList<RuleId> > status = NymeaCore::instance()->removeConfiguredThing(thingId, removePolicyList);
    returns.insert("deviceError", enumValueName<Device::ThingError>(status.first).replace("Thing", "Device"));

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
    ThingId thingId = ThingId(params.value("deviceId").toString());
    ParamList settings = unpack<ParamList>(params.value("settings"));
    Device::ThingError status = NymeaCore::instance()->thingManager()->setThingSettings(thingId, settings);
    return createReply(statusToReply(status));
}

JsonReply* DeviceHandler::GetEventTypes(const QVariantMap &params, const JsonContext &context) const
{
    ThingClass deviceClass = NymeaCore::instance()->thingManager()->findThingClass(ThingClassId(params.value("deviceClassId").toString()));
    ThingClass translatedDeviceClass = NymeaCore::instance()->thingManager()->translateThingClass(deviceClass, context.locale());

    QVariantMap returns;
    returns.insert("eventTypes", pack(translatedDeviceClass.eventTypes()));
    return createReply(returns);
}

JsonReply* DeviceHandler::GetActionTypes(const QVariantMap &params, const JsonContext &context) const
{
    ThingClass deviceClass = NymeaCore::instance()->thingManager()->findThingClass(ThingClassId(params.value("deviceClassId").toString()));
    ThingClass translatedDeviceClass = NymeaCore::instance()->thingManager()->translateThingClass(deviceClass, context.locale());

    QVariantMap returns;
    returns.insert("actionTypes", pack(translatedDeviceClass.actionTypes()));
    return createReply(returns);
}

JsonReply* DeviceHandler::GetStateTypes(const QVariantMap &params, const JsonContext &context) const
{
    ThingClass deviceClass = NymeaCore::instance()->thingManager()->findThingClass(ThingClassId(params.value("deviceClassId").toString()));
    ThingClass translatedDeviceClass = NymeaCore::instance()->thingManager()->translateThingClass(deviceClass, context.locale());

    QVariantMap returns;
    returns.insert("stateTypes", pack(translatedDeviceClass.stateTypes()));
    return createReply(returns);
}

JsonReply* DeviceHandler::GetStateValue(const QVariantMap &params) const
{
    Thing *thing = NymeaCore::instance()->thingManager()->findConfiguredThing(ThingId(params.value("deviceId").toString()));
    if (!thing) {
        return createReply(statusToReply(Device::ThingErrorThingNotFound));
    }
    StateTypeId stateTypeId = StateTypeId(params.value("stateTypeId").toString());
    if (!thing->hasState(stateTypeId)) {
        return createReply(statusToReply(Device::ThingErrorStateTypeNotFound));
    }

    QVariantMap returns = statusToReply(Device::ThingErrorNoError);
    returns.insert("value", thing->state(stateTypeId).value());
    return createReply(returns);
}

JsonReply *DeviceHandler::GetStateValues(const QVariantMap &params) const
{
    Thing *thing = NymeaCore::instance()->thingManager()->findConfiguredThing(ThingId(params.value("deviceId").toString()));
    if (!thing) {
        return createReply(statusToReply(Device::ThingErrorThingNotFound));
    }

    QVariantMap returns = statusToReply(Device::ThingErrorNoError);
    returns.insert("values", pack(thing->states()));
    return createReply(returns);
}

JsonReply *DeviceHandler::BrowseDevice(const QVariantMap &params, const JsonContext &context) const
{
    ThingId thingId = ThingId(params.value("deviceId").toString());
    QString itemId = params.value("itemId").toString();

    JsonReply *jsonReply = createAsyncReply("BrowseDevice");

    BrowseResult *result = NymeaCore::instance()->thingManager()->browseThing(thingId, itemId, context.locale());
    connect(result, &BrowseResult::finished, jsonReply, [this, jsonReply, result, context](){

        QVariantMap returns = statusToReply(result->status());
        QVariantList list;
        foreach (const BrowserItem &item, result->items()) {
            list.append(packBrowserItem(item));
        }
        returns.insert("items", list);
        if (!result->displayMessage().isEmpty()) {
            returns.insert("displayMessage", result->translatedDisplayMessage(context.locale()));
        }
        jsonReply->setData(returns);
        jsonReply->finished();
    });

    return jsonReply;
}

JsonReply *DeviceHandler::GetBrowserItem(const QVariantMap &params, const JsonContext &context) const
{
    QVariantMap returns;
    ThingId thingId = ThingId(params.value("deviceId").toString());
    QString itemId = params.value("itemId").toString();

    JsonReply *jsonReply = createAsyncReply("GetBrowserItem");

    BrowserItemResult *result = NymeaCore::instance()->thingManager()->browserItemDetails(thingId, itemId, context.locale());
    connect(result, &BrowserItemResult::finished, jsonReply, [this, jsonReply, result, context](){
        QVariantMap params = statusToReply(result->status());
        if (result->status() == Device::ThingErrorNoError) {
            params.insert("item", packBrowserItem(result->item()));
        }
        if (!result->displayMessage().isEmpty()) {
            params.insert("displayMessage", result->translatedDisplayMessage(context.locale()));
        }
        jsonReply->setData(params);
        jsonReply->finished();
    });

    return jsonReply;
}

JsonReply *DeviceHandler::ExecuteAction(const QVariantMap &params, const JsonContext &context)
{
    ThingId thingId(params.value("deviceId").toString());
    ActionTypeId actionTypeId(params.value("actionTypeId").toString());
    ParamList actionParams = unpack<ParamList>(params.value("params"));
    QLocale locale = context.locale();

    Action action(actionTypeId, thingId);
    action.setParams(actionParams);

    JsonReply *jsonReply = createAsyncReply("ExecuteAction");

    ThingActionInfo *info = NymeaCore::instance()->thingManager()->executeAction(action);
    connect(info, &ThingActionInfo::finished, jsonReply, [info, jsonReply, locale](){
        QVariantMap data;
        data.insert("deviceError", enumValueName(info->status()).replace("Thing", "Device"));
        if (!info->displayMessage().isEmpty()) {
            data.insert("displayMessage", info->translatedDisplayMessage(locale));
        }
        jsonReply->setData(data);
        jsonReply->finished();
    });

    return jsonReply;
}

JsonReply *DeviceHandler::ExecuteBrowserItem(const QVariantMap &params, const JsonContext &context)
{
    ThingId thingId = ThingId(params.value("deviceId").toString());
    QString itemId = params.value("itemId").toString();
    BrowserAction action(thingId, itemId);

    JsonReply *jsonReply = createAsyncReply("ExecuteBrowserItem");

    BrowserActionInfo *info = NymeaCore::instance()->executeBrowserItem(action);
    connect(info, &BrowserActionInfo::finished, jsonReply, [info, jsonReply, context](){
        QVariantMap data;
        data.insert("deviceError", enumValueName<Device::ThingError>(info->status()).replace("Thing", "Device"));
        if (!info->displayMessage().isEmpty()) {
            data.insert("displayMessage", info->translatedDisplayMessage(context.locale()));
        }
        jsonReply->setData(data);
        jsonReply->finished();
    });

    return jsonReply;
}

JsonReply *DeviceHandler::ExecuteBrowserItemAction(const QVariantMap &params, const JsonContext &context)
{
    ThingId thingId = ThingId(params.value("deviceId").toString());
    QString itemId = params.value("itemId").toString();
    ActionTypeId actionTypeId = ActionTypeId(params.value("actionTypeId").toString());
    ParamList paramList = unpack<ParamList>(params.value("params"));
    BrowserItemAction browserItemAction(thingId, itemId, actionTypeId, paramList);

    JsonReply *jsonReply = createAsyncReply("ExecuteBrowserItemAction");

    BrowserItemActionInfo *info = NymeaCore::instance()->executeBrowserItemAction(browserItemAction);
    connect(info, &BrowserItemActionInfo::finished, jsonReply, [info, jsonReply, context](){
        QVariantMap data;
        data.insert("deviceError", enumValueName<Device::ThingError>(info->status()).replace("Thing", "Device"));
        if (!info->displayMessage().isEmpty()) {
            data.insert("displayMessage", info->translatedDisplayMessage(context.locale()));
        }
        jsonReply->setData(data);
        jsonReply->finished();
    });

    return jsonReply;
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

void DeviceHandler::pluginConfigChanged(const PluginId &id, const ParamList &config)
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

void DeviceHandler::deviceStateChanged(Thing *device, const QUuid &stateTypeId, const QVariant &value)
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

void DeviceHandler::deviceAddedNotification(Thing *thing)
{
    QVariantMap params;
    QVariantMap deviceMap = pack(thing).toMap();
    // Patch in deviceClassId
    deviceMap.insert("deviceClassId", deviceMap.value("thingClassId"));
    params.insert("device", deviceMap);
    emit DeviceAdded(params);
}

void DeviceHandler::deviceChangedNotification(Thing *thing)
{
    QVariantMap params;
    QVariantMap deviceMap = pack(thing).toMap();
    // Patch in deviceClassId
    deviceMap.insert("deviceClassId", deviceMap.value("thingClassId"));
    params.insert("device", deviceMap);
    emit DeviceChanged(params);
}

void DeviceHandler::deviceSettingChangedNotification(const ThingId &thingId, const ParamTypeId &paramTypeId, const QVariant &value)
{
    QVariantMap params;
    params.insert("deviceId", thingId);
    params.insert("paramTypeId", paramTypeId.toString());
    params.insert("value", value);
    emit DeviceSettingChanged(params);
}

QVariantMap DeviceHandler::translateNotification(const QString &notification, const QVariantMap &params, const QLocale &locale)
{
    if (notification == "DeviceChanged") {
        QVariantMap deviceMap = params.value("device").toMap();
        DeviceId deviceId = params.value("device").toMap().value("id").toUuid();
        Thing *device = NymeaCore::instance()->thingManager()->findConfiguredThing(deviceId);
        QString setupDisplayMessage = params.value("device").toMap().value("setupDisplayMessage").toString();
        QString translatedSetupDisplayMessage = NymeaCore::instance()->thingManager()->translate(device->pluginId(), setupDisplayMessage, locale);
        if (!translatedSetupDisplayMessage.isEmpty()) {
            deviceMap["setupDisplayMessage"] = translatedSetupDisplayMessage;
        }
        QVariantMap translatedParams = params;
        translatedParams["device"] = deviceMap;
        return translatedParams;
    }

    return params;
}

QVariantMap DeviceHandler::statusToReply(Thing::ThingError status) const
{
    QVariantMap returns;
    returns.insert("deviceError", enumValueName<Device::ThingError>(status).replace("Thing", "Device"));
    return returns;
}

DeviceClass::DeviceClass(const ThingClass &other):
    ThingClass(other.pluginId(), other.vendorId(), other.id())
{
    setName(other.name());
    setBrowsable(other.browsable());
    setEventTypes(other.eventTypes());
    setInterfaces(other.interfaces());
    setParamTypes(other.paramTypes());
    setStateTypes(other.stateTypes());
    setActionTypes(other.actionTypes());
    setDisplayName(other.displayName());
    setSetupMethod(other.setupMethod());
    setCreateMethods(other.createMethods());
    setSettingsTypes(other.settingsTypes());
    setDiscoveryParamTypes(other.discoveryParamTypes());
    setBrowserItemActionTypes(other.browserItemActionTypes());
}

DeviceClassId Device::deviceClassId() const
{
    return thingClassId();
}

}
