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

#include "devicehandler.h"
#include "guhcore.h"
#include "devicemanager.h"
#include "plugin/device.h"
#include "plugin/deviceclass.h"
#include "plugin/deviceplugin.h"

#include <QDebug>

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
                   "Use deviceDescriptorId or deviceParams, depending on the createMethod of the device class. "
                   "CreateMethodJustAdd takes the parameters you want to have with that device. "
                   "CreateMethodDiscovery requires the use of a deviceDescriptorId."
                   );
    params.insert("deviceClassId", JsonTypes::basicTypeToString(JsonTypes::Uuid));
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
                   "Use this for DeviceClasses with a setupMethod different than SetupMethodJustAdd."
                   "Use deviceDescriptorId or deviceParams, depending on the createMethod of the device class. "
                   "CreateMethodJustAdd takes the parameters you want to have with that device. "
                   "CreateMethodDiscovery requires the use of a deviceDescriptorId. "
                   "If success is true, the return values will contain a pairingTransactionId, a displayMessage and "
                   "the setupMethod. Depending on the setupMethod you should either proceed with AddConfiguredDevice "
                   " or PairDevice."
                   );
    setParams("PairDevice", params);
    returns.insert("deviceError", JsonTypes::deviceErrorRef());
    returns.insert("o:pairingTransactionId", JsonTypes::basicTypeToString(JsonTypes::Uuid));
    returns.insert("o:displayMessage", JsonTypes::basicTypeToString(JsonTypes::String));
    returns.insert("o:setupMethod", JsonTypes::setupMethodRef());
    setReturns("PairDevice", returns);

    params.clear(); returns.clear();
    setDescription("ConfirmPairing", "Confirm an ongoing pairing. In case of SetupMethodEnterPin also provide the pin in the params.");
    params.insert("pairingTransactionId", JsonTypes::basicTypeToString(JsonTypes::Uuid));
    params.insert("o:secret", JsonTypes::basicTypeToString(JsonTypes::String));
    setParams("ConfirmPairing", params);
    returns.insert("deviceError", JsonTypes::deviceErrorRef());
    returns.insert("o:deviceId", JsonTypes::basicTypeToString(JsonTypes::Uuid));
    setReturns("ConfirmPairing", returns);

    params.clear(); returns.clear();
    setDescription("GetConfiguredDevices", "Returns a list of configured devices.");
    setParams("GetConfiguredDevices", params);
    QVariantList devices;
    devices.append(JsonTypes::deviceRef());
    returns.insert("devices", devices);
    setReturns("GetConfiguredDevices", returns);

    params.clear(); returns.clear();
    setDescription("GetDiscoveredDevices", "Performs a device discovery and returns the results. This function may take a while to return.");
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
    setDescription("RemoveConfiguredDevice", "Remove a device from the system.");
    params.insert("deviceId", JsonTypes::basicTypeToString(JsonTypes::Uuid));
    QVariantList removePolicyList;
    QVariantMap policy;
    policy.insert("ruleId", JsonTypes::basicTypeToString(JsonTypes::Uuid));
    policy.insert("policy", JsonTypes::removePolicyRef());
    removePolicyList.append(policy);
    params.insert("o:removePolicyList", removePolicyList);
    setParams("RemoveConfiguredDevice", params);
    returns.insert("deviceError", JsonTypes::deviceErrorRef());
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
    setDescription("GetStateValues", "Get all the state values of the give device.");
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

    // Notifications
    params.clear(); returns.clear();
    setDescription("StateChanged", "Emitted whenever a State of a device changes.");
    params.insert("deviceId", JsonTypes::basicTypeToString(JsonTypes::Uuid));
    params.insert("stateTypeId", JsonTypes::basicTypeToString(JsonTypes::Uuid));
    params.insert("value", JsonTypes::basicTypeToString(JsonTypes::Variant));
    setParams("StateChanged", params);

    connect(GuhCore::instance(), &GuhCore::deviceStateChanged, this, &DeviceHandler::deviceStateChanged);
    connect(GuhCore::instance(), &GuhCore::devicesDiscovered, this, &DeviceHandler::devicesDiscovered, Qt::QueuedConnection);
    connect(GuhCore::instance(), &GuhCore::deviceSetupFinished, this, &DeviceHandler::deviceSetupFinished);
    connect(GuhCore::instance(), &GuhCore::pairingFinished, this, &DeviceHandler::pairingFinished);
}

QString DeviceHandler::name() const
{
    return "Devices";
}

JsonReply* DeviceHandler::GetSupportedVendors(const QVariantMap &params) const
{
    Q_UNUSED(params)
    QVariantMap returns;
    QVariantList supportedVendors;
    foreach (const Vendor &vendor, GuhCore::instance()->supportedVendors()) {
        supportedVendors.append(JsonTypes::packVendor(vendor));
    }
    returns.insert("vendors", supportedVendors);
    return createReply(returns);
}

JsonReply* DeviceHandler::GetSupportedDevices(const QVariantMap &params) const
{
    QVariantMap returns;
    QVariantList supportedDeviceList;
    QList<DeviceClass> supportedDevices;
    if (params.contains("vendorId")) {
        supportedDevices = GuhCore::instance()->supportedDevices(VendorId(params.value("vendorId").toString()));
    } else {
        supportedDevices = GuhCore::instance()->supportedDevices();
    }
    foreach (const DeviceClass &deviceClass, supportedDevices) {
        supportedDeviceList.append(JsonTypes::packDeviceClass(deviceClass));
    }
    returns.insert("deviceClasses", supportedDeviceList);
    return createReply(returns);
}

JsonReply *DeviceHandler::GetDiscoveredDevices(const QVariantMap &params) const
{
    QVariantMap returns;

    DeviceClassId deviceClassId = DeviceClassId(params.value("deviceClassId").toString());

    ParamList discoveryParams = JsonTypes::unpackParams(params.value("discoveryParams").toList());

    DeviceManager::DeviceError status = GuhCore::instance()->discoverDevices(deviceClassId, discoveryParams);
    if (status == DeviceManager::DeviceErrorAsync ) {
        JsonReply *reply = createAsyncReply("GetDiscoveredDevices");
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
    QVariantList plugins;
    foreach (DevicePlugin *plugin, GuhCore::instance()->plugins()) {
        QVariantMap pluginMap;
        pluginMap.insert("id", plugin->pluginId());
        pluginMap.insert("name", plugin->pluginName());
        QVariantList params;
        foreach (const ParamType &param, plugin->configurationDescription()) {
            params.append(JsonTypes::packParamType(param));
        }
        pluginMap.insert("params", params);
        plugins.append(pluginMap);
    }
    returns.insert("plugins", plugins);
    return createReply(returns);
}

JsonReply *DeviceHandler::GetPluginConfiguration(const QVariantMap &params) const
{
    DevicePlugin *plugin = 0;
    foreach (DevicePlugin *p, GuhCore::instance()->plugins()) {
        if (p->pluginId() == PluginId(params.value("pluginId").toString())) {
            plugin = p;
        }
    }

    QVariantMap returns;
    if (!plugin) {
        returns.insert("deviceError", JsonTypes::deviceErrorToString(DeviceManager::DeviceErrorPluginNotFound));
        return createReply(returns);
    }

    QVariantList paramVariantList;
    foreach (const Param &param, plugin->configuration()) {
        paramVariantList.append(JsonTypes::packParam(param));
    }
    returns.insert("configuration", paramVariantList);
    returns.insert("deviceError", JsonTypes::deviceErrorToString(DeviceManager::DeviceErrorNoError));
    return createReply(returns);
}

JsonReply* DeviceHandler::SetPluginConfiguration(const QVariantMap &params)
{
    QVariantMap returns;
    PluginId pluginId = PluginId(params.value("pluginId").toString());
    ParamList pluginParams = JsonTypes::unpackParams(params.value("configuration").toList());
    DeviceManager::DeviceError result = GuhCore::instance()->setPluginConfig(pluginId, pluginParams);
    returns.insert("deviceError", JsonTypes::deviceErrorToString(result));
    return createReply(returns);
}

JsonReply* DeviceHandler::AddConfiguredDevice(const QVariantMap &params)
{
    DeviceClassId deviceClass(params.value("deviceClassId").toString());
    ParamList deviceParams = JsonTypes::unpackParams(params.value("deviceParams").toList());
    DeviceDescriptorId deviceDescriptorId(params.value("deviceDescriptorId").toString());
    DeviceId newDeviceId = DeviceId::createDeviceId();
    DeviceManager::DeviceError status;
    if (deviceDescriptorId.isNull()) {
        status = GuhCore::instance()->addConfiguredDevice(deviceClass, deviceParams, newDeviceId);
    } else {
        status = GuhCore::instance()->addConfiguredDevice(deviceClass, deviceDescriptorId, newDeviceId);
    }
    QVariantMap returns;
    switch (status) {
    case DeviceManager::DeviceErrorAsync: {
        JsonReply *asyncReply = createAsyncReply("AddConfiguredDevice");
        m_asynDeviceAdditions.insert(newDeviceId, asyncReply);
        return asyncReply;
    }
    case DeviceManager::DeviceErrorNoError:
        returns.insert("deviceId", newDeviceId);
    default:
        returns.insert("deviceError", JsonTypes::deviceErrorToString(status));
    }
    return createReply(returns);
}

JsonReply *DeviceHandler::PairDevice(const QVariantMap &params)
{
    DeviceClassId deviceClassId(params.value("deviceClassId").toString());
    DeviceClass deviceClass = GuhCore::instance()->findDeviceClass(deviceClassId);

    DeviceManager::DeviceError status;
    PairingTransactionId pairingTransactionId = PairingTransactionId::createPairingTransactionId();
    if (params.contains("deviceDescriptorId")) {
        DeviceDescriptorId deviceDescriptorId(params.value("deviceDescriptorId").toString());
        status = GuhCore::instance()->pairDevice(pairingTransactionId, deviceClassId, deviceDescriptorId);
    } else {
        ParamList deviceParams = JsonTypes::unpackParams(params.value("deviceParams").toList());
        status = GuhCore::instance()->pairDevice(pairingTransactionId, deviceClassId, deviceParams);
    }

    QVariantMap returns;
    returns.insert("deviceError", JsonTypes::deviceErrorToString(status));
    if (status == DeviceManager::DeviceErrorNoError) {
        returns.insert("displayMessage", deviceClass.pairingInfo());
        returns.insert("pairingTransactionId", pairingTransactionId.toString());
        returns.insert("setupMethod", JsonTypes::setupMethod().at(deviceClass.setupMethod()));
    }
    return createReply(returns);
}

JsonReply *DeviceHandler::ConfirmPairing(const QVariantMap &params)
{
    PairingTransactionId pairingTransactionId = PairingTransactionId(params.value("pairingTransactionId").toString());
    QString secret = params.value("secret").toString();
    DeviceManager::DeviceError status = GuhCore::instance()->confirmPairing(pairingTransactionId, secret);

    JsonReply *reply = 0;
    if (status == DeviceManager::DeviceErrorAsync) {
        reply = createAsyncReply("ConfirmPairing");
        m_asyncPairingRequests.insert(pairingTransactionId, reply);
        return reply;
    }
    QVariantMap returns;
    returns.insert("deviceError", JsonTypes::deviceErrorToString(status));
    reply = createReply(returns);
    return reply;
}

JsonReply* DeviceHandler::GetConfiguredDevices(const QVariantMap &params) const
{
    Q_UNUSED(params)
    QVariantMap returns;
    QVariantList configuredDeviceList;
    foreach (Device *device, GuhCore::instance()->configuredDevices()) {
        configuredDeviceList.append(JsonTypes::packDevice(device));
    }
    returns.insert("devices", configuredDeviceList);
    return createReply(returns);
}

JsonReply* DeviceHandler::RemoveConfiguredDevice(const QVariantMap &params)
{
    DeviceId deviceId = DeviceId(params.value("deviceId").toString());
    QHash<RuleId, RuleEngine::RemovePolicy> removePolicyList;
    foreach (const QVariant &variant, params.value("removePolicyList").toList()) {
        RuleId ruleId = RuleId(variant.toMap().value("ruleId").toString());
        RuleEngine::RemovePolicy policy = variant.toMap().value("policy").toString() == "RemovePolicyCascade" ? RuleEngine::RemovePolicyCascade : RuleEngine::RemovePolicyUpdate;
        removePolicyList.insert(ruleId, policy);
    }

    QVariantMap returns;
    DeviceManager::DeviceError status = GuhCore::instance()->removeConfiguredDevice(deviceId, removePolicyList);
    returns.insert("deviceError", JsonTypes::deviceErrorToString(status));
    return createReply(returns);
}

JsonReply* DeviceHandler::GetEventTypes(const QVariantMap &params) const
{
    QVariantMap returns;

    QVariantList eventList;
    DeviceClass deviceClass = GuhCore::instance()->findDeviceClass(DeviceClassId(params.value("deviceClassId").toString()));
    foreach (const EventType &eventType, deviceClass.eventTypes()) {
        eventList.append(JsonTypes::packEventType(eventType));
    }
    returns.insert("eventTypes", eventList);
    return createReply(returns);
}

JsonReply* DeviceHandler::GetActionTypes(const QVariantMap &params) const
{
    QVariantMap returns;

    QVariantList actionList;
    DeviceClass deviceClass = GuhCore::instance()->findDeviceClass(DeviceClassId(params.value("deviceClassId").toString()));
    foreach (const ActionType &actionType, deviceClass.actionTypes()) {
        actionList.append(JsonTypes::packActionType(actionType));
    }
    returns.insert("actionTypes", actionList);
    return createReply(returns);
}

JsonReply* DeviceHandler::GetStateTypes(const QVariantMap &params) const
{
    QVariantMap returns;

    QVariantList stateList;
    DeviceClass deviceClass = GuhCore::instance()->findDeviceClass(DeviceClassId(params.value("deviceClassId").toString()));
    foreach (const StateType &stateType, deviceClass.stateTypes()) {
        stateList.append(JsonTypes::packStateType(stateType));
    }
    returns.insert("stateTypes", stateList);
    return createReply(returns);
}

JsonReply* DeviceHandler::GetStateValue(const QVariantMap &params) const
{
    QVariantMap returns;

    Device *device = GuhCore::instance()->findConfiguredDevice(DeviceId(params.value("deviceId").toString()));
    if (!device) {
        returns.insert("deviceError", JsonTypes::deviceErrorToString(DeviceManager::DeviceErrorDeviceNotFound));
        return createReply(returns);
    }
    if (!device->hasState(StateTypeId(params.value("stateTypeId").toString()))) {
        returns.insert("deviceError", JsonTypes::deviceErrorToString(DeviceManager::DeviceErrorStateTypeNotFound));
        return createReply(returns);
    }
    QVariant stateValue = device->stateValue(StateTypeId(params.value("stateTypeId").toString()));

    returns.insert("deviceError", JsonTypes::deviceErrorToString(DeviceManager::DeviceErrorNoError));
    returns.insert("value", stateValue);
    return createReply(returns);
}

JsonReply *DeviceHandler::GetStateValues(const QVariantMap &params) const
{
    QVariantMap returns;

    Device *device = GuhCore::instance()->findConfiguredDevice(DeviceId(params.value("deviceId").toString()));
    if (!device) {
        returns.insert("deviceError", JsonTypes::deviceErrorToString(DeviceManager::DeviceErrorDeviceNotFound));
        return createReply(returns);
    }

    DeviceClass deviceClass = GuhCore::instance()->findDeviceClass(device->deviceClassId());
    if (!deviceClass.isValid()) {
        returns.insert("deviceError", JsonTypes::deviceErrorToString(DeviceManager::DeviceErrorDeviceClassNotFound));
        return createReply(returns);
    }
    QVariantList values;
    foreach (const StateType &stateType, deviceClass.stateTypes()) {
        QVariantMap stateValue;
        stateValue.insert("stateTypeId", stateType.id().toString());
        stateValue.insert("value", device->stateValue(stateType.id()));
        values.append(stateValue);
    }
    returns.insert("deviceError", JsonTypes::deviceErrorToString(DeviceManager::DeviceErrorNoError));
    returns.insert("values", values);
    return createReply(returns);
}

void DeviceHandler::deviceStateChanged(Device *device, const QUuid &stateTypeId, const QVariant &value)
{
    QVariantMap params;
    params.insert("deviceId", device->id());
    params.insert("stateTypeId", stateTypeId);
    params.insert("value", value);

    emit StateChanged(params);
}

void DeviceHandler::devicesDiscovered(const DeviceClassId &deviceClassId, const QList<DeviceDescriptor> deviceDescriptors)
{
    if (!m_discoverRequests.contains(deviceClassId)) {
        return; // We didn't start this discovery... Ignore it.
    }

    JsonReply *reply = m_discoverRequests.take(deviceClassId);
    QVariantList list;
    foreach (const DeviceDescriptor &descriptor, deviceDescriptors) {
        list.append(JsonTypes::packDeviceDescriptor(descriptor));
    }
    QVariantMap returns;
    returns.insert("deviceDescriptors", list);
    returns.insert("deviceError", JsonTypes::deviceErrorToString(DeviceManager::DeviceErrorNoError));

    reply->setData(returns);
    reply->finished();
}

void DeviceHandler::deviceSetupFinished(Device *device, DeviceManager::DeviceError status)
{
    qDebug() << "got a device setup finished";
    if (!m_asynDeviceAdditions.contains(device->id())) {
        return; // Not the device we're waiting for...
    }

    JsonReply *reply = m_asynDeviceAdditions.take(device->id());

    QVariantMap returns;
    returns.insert("deviceError", JsonTypes::deviceErrorToString(status));

    if(status == DeviceManager::DeviceErrorNoError) {
        returns.insert("deviceId", device->id());
    }
    reply->setData(returns);
    reply->finished();
}

void DeviceHandler::pairingFinished(const PairingTransactionId &pairingTransactionId, DeviceManager::DeviceError status, const DeviceId &deviceId)
{
    qDebug() << "handler: pairing finished";
    JsonReply *reply = m_asyncPairingRequests.take(pairingTransactionId);
    if (!reply) {
        qDebug() << "not for me";
        return;
    }

    if (status != DeviceManager::DeviceErrorNoError) {
        QVariantMap returns;
        returns.insert("deviceError", JsonTypes::deviceErrorToString(status));
        reply->setData(returns);
        reply->finished();
        return;
    }

    m_asynDeviceAdditions.insert(deviceId, reply);
}
