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
    params.insert("o:vendorId", "uuid");
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
    params.insert("pluginId", "uuid");
    setParams("GetPluginConfiguration", params);
    QVariantList pluginParams;
    pluginParams.append(JsonTypes::paramRef());
    returns.insert("success", "bool");
    returns.insert("errorMessage", "string");
    returns.insert("o:configuration", pluginParams);
    setReturns("GetPluginConfiguration", returns);

    params.clear(); returns.clear();
    setDescription("SetPluginConfiguration", "Set a plugin's params.");
    params.insert("pluginId", "uuid");
    params.insert("configuration", pluginParams);
    setParams("SetPluginConfiguration", params);
    returns.insert("success", "bool");
    returns.insert("errorMessage", "string");
    setReturns("SetPluginConfiguration", returns);

    params.clear(); returns.clear();
    setDescription("AddConfiguredDevice", "Add a configured device with a setupMethod of SetupMethodJustAdd. "
                   "For devices with a setupMethod different than SetupMethodJustAdd, use PairDevice. "
                   "Use deviceDescriptorId or deviceParams, depending on the createMethod of the device class. "
                   "CreateMethodJustAdd takes the parameters you want to have with that device. "
                   "CreateMethodDiscovery requires the use of a deviceDescriptorId."
                   );
    params.insert("deviceClassId", "uuid");
    params.insert("o:deviceDescriptorId", "uuid");
    QVariantList deviceParams;
    deviceParams.append(JsonTypes::paramRef());
    params.insert("o:deviceParams", deviceParams);
    setParams("AddConfiguredDevice", params);
    returns.insert("success", "bool");
    returns.insert("errorMessage", "string");
    returns.insert("o:deviceId", "uuid");
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
    returns.insert("success", "bool");
    returns.insert("errorMessage", "string");
    returns.insert("o:pairingTransactionId", "uuid");
    returns.insert("o:displayMessage", "string");
    returns.insert("o:setupMethod", JsonTypes::setupMethodTypesRef());
    setReturns("PairDevice", returns);

    params.clear(); returns.clear();
    setDescription("ConfirmPairing", "Confirm an ongoing pairing. In case of SetupMethodEnterPin also provide the pin in the params.");
    params.insert("pairingTransactionId", "uuid");
    params.insert("o:secret", "string");
    setParams("ConfirmPairing", params);
    returns.insert("success", "bool");
    returns.insert("errorMessage", "string");
    returns.insert("o:deviceId", "uuid");
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
    params.insert("deviceClassId", "uuid");
    QVariantList discoveryParams;
    discoveryParams.append(JsonTypes::paramRef());
    params.insert("o:discoveryParams", discoveryParams);
    setParams("GetDiscoveredDevices", params);
    returns.insert("success", "bool");
    returns.insert("errorMessage", "string");
    QVariantList deviceDescriptors;
    deviceDescriptors.append(JsonTypes::deviceDescriptorRef());
    returns.insert("o:deviceDescriptors", deviceDescriptors);
    setReturns("GetDiscoveredDevices", returns);

    params.clear(); returns.clear();
    setDescription("RemoveConfiguredDevice", "Remove a device from the system.");
    params.insert("deviceId", "uuid");
    setParams("RemoveConfiguredDevice", params);
    returns.insert("success", "bool");
    returns.insert("errorMessage", "string");
    setReturns("RemoveConfiguredDevice", returns);

    params.clear(); returns.clear();
    setDescription("GetEventTypes", "Get event types for a specified deviceClassId.");
    params.insert("deviceClassId", "uuid");
    setParams("GetEventTypes", params);
    QVariantList events;
    events.append(JsonTypes::eventTypeRef());
    returns.insert("eventTypes", events);
    setReturns("GetEventTypes", returns);

    params.clear(); returns.clear();
    setDescription("GetActionTypes", "Get action types for a specified deviceClassId.");
    params.insert("deviceClassId", "uuid");
    setParams("GetActionTypes", params);
    QVariantList actions;
    actions.append(JsonTypes::actionTypeRef());
    returns.insert("actionTypes", actions);
    setReturns("GetActionTypes", returns);

    params.clear(); returns.clear();
    setDescription("GetStateTypes", "Get state types for a specified deviceClassId.");
    params.insert("deviceClassId", "uuid");
    setParams("GetStateTypes", params);
    QVariantList states;
    states.append(JsonTypes::stateTypeRef());
    returns.insert("stateTypes", states);
    setReturns("GetStateTypes", returns);

    params.clear(); returns.clear();
    setDescription("GetStateValue", "Get the value of the given device and the given stateType");
    params.insert("deviceId", "uuid");
    params.insert("stateTypeId", "uuid");
    setParams("GetStateValue", params);
    returns.insert("success", "bool");
    returns.insert("errorMessage", "string");
    returns.insert("o:value", "variant");
    setReturns("GetStateValue", returns);

    // Notifications
    params.clear(); returns.clear();
    setDescription("StateChanged", "Emitted whenever a State of a device changes.");
    params.insert("deviceId", "uuid");
    params.insert("stateTypeId", "uuid");
    params.insert("variant", "value");
    setParams("StateChanged", params);

    connect(GuhCore::instance()->deviceManager(), &DeviceManager::deviceStateChanged, this, &DeviceHandler::deviceStateChanged);
    connect(GuhCore::instance()->deviceManager(), &DeviceManager::devicesDiscovered, this, &DeviceHandler::devicesDiscovered, Qt::QueuedConnection);
    connect(GuhCore::instance()->deviceManager(), &DeviceManager::deviceSetupFinished, this, &DeviceHandler::deviceSetupFinished);
    connect(GuhCore::instance()->deviceManager(), &DeviceManager::pairingFinished, this, &DeviceHandler::pairingFinished);
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
    foreach (const Vendor &vendor, GuhCore::instance()->deviceManager()->supportedVendors()) {
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
        supportedDevices = GuhCore::instance()->deviceManager()->supportedDevices(VendorId(params.value("vendorId").toString()));
    } else {
        supportedDevices = GuhCore::instance()->deviceManager()->supportedDevices();
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

    QList<Param> discoveryParams;
    foreach (const QVariant &discoveryParam, params.value("discoveryParams").toList()) {
        Param dp(discoveryParam.toMap().keys().first(), discoveryParam.toMap().values().first());
        discoveryParams.append(dp);
    }

    DeviceManager::DeviceError status = GuhCore::instance()->deviceManager()->discoverDevices(deviceClassId, discoveryParams);
    switch (status) {
    case DeviceManager::DeviceErrorAsync:
    case DeviceManager::DeviceErrorNoError: {
        JsonReply *reply = createAsyncReply("GetDiscoveredDevices");
        m_discoverRequests.insert(deviceClassId, reply);
        return reply;
    }
    case DeviceManager::DeviceErrorDeviceClassNotFound:
        returns.insert("errorMessage", "Cannot discover devices. Unknown DeviceClassId.");
        break;
    case DeviceManager::DeviceErrorPluginNotFound:
        returns.insert("errorMessage", "Cannot discover devices. Plugin for DeviceClass not found.");
        break;
    case DeviceManager::DeviceErrorCreationMethodNotSupported:
        returns.insert("errorMessage", "This device can't be discovered.");
        break;
    default:
        returns.insert("errorMessage", QString("Unknown error %1").arg(status));
    }

    returns.insert("success", false);
    return createReply(returns);
}

JsonReply* DeviceHandler::GetPlugins(const QVariantMap &params) const
{
    Q_UNUSED(params)
    QVariantMap returns;
    QVariantList plugins;
    foreach (DevicePlugin *plugin, GuhCore::instance()->deviceManager()->plugins()) {
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
    foreach (DevicePlugin *p, GuhCore::instance()->deviceManager()->plugins()) {
        if (p->pluginId() == PluginId(params.value("pluginId").toString())) {
            plugin = p;
        }
    }

    QVariantMap returns;
    if (!plugin) {
        returns.insert("success", false);
        returns.insert("errorMessage", QString("Plugin not found: %1").arg(params.value("pluginId").toString()));
        return createReply(returns);
    }

    QVariantList paramVariantList;
    foreach (const Param &param, plugin->configuration()) {
        paramVariantList.append(JsonTypes::packParam(param));
    }
    returns.insert("configuration", paramVariantList);
    returns.insert("success", true);
    returns.insert("errorMessage", QString());
    return createReply(returns);
}

JsonReply* DeviceHandler::SetPluginConfiguration(const QVariantMap &params)
{
    QVariantMap returns;
    PluginId pluginId = PluginId(params.value("pluginId").toString());
    QList<Param> pluginParams;
    foreach (const QVariant &param, params.value("configuration").toList()) {
        qDebug() << "got param" << param;
        pluginParams.append(JsonTypes::unpackParam(param.toMap()));
    }
    QPair<DeviceManager::DeviceError, QString> result = GuhCore::instance()->deviceManager()->setPluginConfig(pluginId, pluginParams);
    returns.insert("success", result.first == DeviceManager::DeviceErrorNoError);
    returns.insert("errorMessage", result.second);
    return createReply(returns);
}

JsonReply* DeviceHandler::AddConfiguredDevice(const QVariantMap &params)
{
    DeviceClassId deviceClass(params.value("deviceClassId").toString());
    QList<Param> deviceParams;
    foreach (const QString &paramName, params.value("deviceParams").toMap().keys()) {
         Param param(paramName);
         param.setValue(params.value("deviceParams").toMap().value(paramName));
         deviceParams.append(param);
    }
    DeviceDescriptorId deviceDescriptorId(params.value("deviceDescriptorId").toString());
    DeviceId newDeviceId = DeviceId::createDeviceId();
    QPair<DeviceManager::DeviceError, QString> status;
    if (deviceDescriptorId.isNull()) {
        qDebug() << "adding a manual device.";
        status = GuhCore::instance()->deviceManager()->addConfiguredDevice(deviceClass, deviceParams, newDeviceId);
    } else {
        qDebug() << "adding a discovered device.";
        status = GuhCore::instance()->deviceManager()->addConfiguredDevice(deviceClass, deviceDescriptorId, newDeviceId);
    }
    QVariantMap returns;
    switch(status.first) {
    case DeviceManager::DeviceErrorAsync: {
        JsonReply *asyncReply = createAsyncReply("AddConfiguredDevice");
        m_asynDeviceAdditions.insert(newDeviceId, asyncReply);
        return asyncReply;
    }
    case DeviceManager::DeviceErrorNoError:
        returns.insert("success", true);
        returns.insert("errorMessage", "");
        returns.insert("deviceId", newDeviceId);
        break;
    case DeviceManager::DeviceErrorDeviceClassNotFound:
        returns.insert("errorMessage", QString("Error creating device. Device class not found: %1").arg(status.second));
        returns.insert("success", false);
        break;
    case DeviceManager::DeviceErrorMissingParameter:
        returns.insert("errorMessage", QString("Error creating device. Missing parameter: %1").arg(status.second));
        returns.insert("success", false);
        break;
    case DeviceManager::DeviceErrorSetupFailed:
        returns.insert("errorMessage", QString("Error creating device. Device setup failed: %1").arg(status.second));
        returns.insert("success", false);
        break;
    case DeviceManager::DeviceErrorCreationMethodNotSupported:
        returns.insert("errorMessage", QString("Error creating device. This device can't be created this way: %1").arg(status.second));
        returns.insert("success", false);
        break;
    case DeviceManager::DeviceErrorInvalidParameter:
        returns.insert("errorMessage", QString("Error creating device. Invalid device parameter: %1").arg(status.second));
        returns.insert("success", false);
        break;
    default:
        returns.insert("errorMessage", "Unknown error. Please report a bug describing what you did.");
        returns.insert("success", false);
    }
    return createReply(returns);
}

JsonReply *DeviceHandler::PairDevice(const QVariantMap &params)
{
    DeviceClassId deviceClassId(params.value("deviceClassId").toString());
    DeviceClass deviceClass = GuhCore::instance()->deviceManager()->findDeviceClass(deviceClassId);

    QPair<DeviceManager::DeviceError, QString> status;
    if (params.contains("deviceDescriptorId")) {
        DeviceDescriptorId deviceDescriptorId(params.value("deviceDescriptorId").toString());
        status = GuhCore::instance()->deviceManager()->pairDevice(deviceClassId, deviceDescriptorId);
    } else {
        QList<Param> deviceParams;
        foreach (const QString &paramName, params.value("deviceParams").toMap().keys()) {
             Param param(paramName);
             param.setValue(params.value("deviceParams").toMap().value(paramName));
             deviceParams.append(param);
        }
        status = GuhCore::instance()->deviceManager()->pairDevice(deviceClassId, deviceParams);
    }

    QVariantMap returns;
    switch (status.first) {
    case DeviceManager::DeviceErrorNoError:
        returns.insert("success", true);
        returns.insert("errorMessage", "");
        returns.insert("displayMessage", deviceClass.pairingInfo());
        returns.insert("pairingTransactionId", status.second);
        returns.insert("setupMethod", JsonTypes::setupMethodTypes().at(deviceClass.setupMethod()));
        break;
    case DeviceManager::DeviceErrorDeviceClassNotFound:
        returns.insert("errorMessage", QString("Error pairing device. Device class not found: %1").arg(status.second));
        returns.insert("success", false);
        break;
    case DeviceManager::DeviceErrorDeviceDescriptorNotFound:
        returns.insert("errorMessage", QString("Error pairing device. Device descriptor not found: %1").arg(status.second));
        returns.insert("success", false);
        break;
    case DeviceManager::DeviceErrorCreationMethodNotSupported:
        returns.insert("errorMessage", QString("Error pairing device. This device can't be created this way: %1").arg(status.second));
        returns.insert("success", false);
        break;
    case DeviceManager::DeviceErrorPairingTransactionIdNotFound:
        returns.insert("errorMessage", QString("Error pairing device. PairingTransactionId not found: %1").arg(status.second));
        returns.insert("success", false);
        break;
    }
    return createReply(returns);
}

JsonReply *DeviceHandler::ConfirmPairing(const QVariantMap &params)
{
    QUuid pairingTransactionId = params.value("pairingTransactionId").toUuid();
    QString secret = params.value("secret").toString();
    QPair<DeviceManager::DeviceError, QString> status = GuhCore::instance()->deviceManager()->confirmPairing(pairingTransactionId, secret);

    JsonReply *reply = 0;
    QVariantMap returns;
    switch (status.first) {
    case DeviceManager::DeviceErrorAsync:
        reply = createAsyncReply("ConfirmPairing");
        m_asyncPairingRequests.insert(pairingTransactionId, reply);
        return reply;
    case DeviceManager::DeviceErrorNoError:
        returns.insert("success", true);
        returns.insert("errorMessage", QString());
    case DeviceManager::DeviceErrorSetupFailed:
    default:
        returns.insert("success", false);
        returns.insert("errorMessage", status.second);
    }
    reply = createReply(returns);
    return reply;
}

JsonReply* DeviceHandler::GetConfiguredDevices(const QVariantMap &params) const
{
    Q_UNUSED(params)
    QVariantMap returns;
    QVariantList configuredDeviceList;
    foreach (Device *device, GuhCore::instance()->deviceManager()->configuredDevices()) {
        configuredDeviceList.append(JsonTypes::packDevice(device));
    }
    returns.insert("devices", configuredDeviceList);
    return createReply(returns);
}

JsonReply* DeviceHandler::RemoveConfiguredDevice(const QVariantMap &params)
{
    QVariantMap returns;
    QPair<DeviceManager::DeviceError, QString> status = GuhCore::instance()->deviceManager()->removeConfiguredDevice(DeviceId(params.value("deviceId").toString()));
    switch(status.first) {
    case DeviceManager::DeviceErrorNoError:
        returns.insert("success", true);
        returns.insert("errorMessage", "");
        break;
    case DeviceManager::DeviceErrorDeviceNotFound:
        returns.insert("success", false);
        returns.insert("errorMessage", QString("No such device: %1").arg(status.second));
        break;
    default:
        returns.insert("success", false);
        returns.insert("errorMessage", "Unknown error.");
        break;
    }
    return createReply(returns);
}

JsonReply* DeviceHandler::GetEventTypes(const QVariantMap &params) const
{
    QVariantMap returns;

    QVariantList eventList;
    DeviceClass deviceClass = GuhCore::instance()->deviceManager()->findDeviceClass(DeviceClassId(params.value("deviceClassId").toString()));
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
    DeviceClass deviceClass = GuhCore::instance()->deviceManager()->findDeviceClass(DeviceClassId(params.value("deviceClassId").toString()));
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
    DeviceClass deviceClass = GuhCore::instance()->deviceManager()->findDeviceClass(DeviceClassId(params.value("deviceClassId").toString()));
    foreach (const StateType &stateType, deviceClass.stateTypes()) {
        stateList.append(JsonTypes::packStateType(stateType));
    }
    returns.insert("stateTypes", stateList);
    return createReply(returns);
}

JsonReply* DeviceHandler::GetStateValue(const QVariantMap &params) const
{
    QVariantMap returns;

    Device *device = GuhCore::instance()->deviceManager()->findConfiguredDevice(DeviceId(params.value("deviceId").toString()));
    if (!device) {
        returns.insert("success", false);
        returns.insert("errorMessage", "No such device");
        return createReply(returns);
    }
    if (!device->hasState(StateTypeId(params.value("stateTypeId").toString()))) {
        returns.insert("success", false);
        returns.insert("errorMessage", QString("Device %1 %2 doesn't have such a state.").arg(device->name()).arg(device->id().toString()));
        return createReply(returns);
    }
    QVariant stateValue = device->stateValue(StateTypeId(params.value("stateTypeId").toString()));

    returns.insert("success", true);
    returns.insert("errorMessage", "");
    returns.insert("value", stateValue);
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
    returns.insert("success", true);
    returns.insert("errorMessage", "");

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

    if(status == DeviceManager::DeviceErrorNoError) {
        returns.insert("success", true);
        returns.insert("errorMessage", "");
        returns.insert("deviceId", device->id());
    } else if (status == DeviceManager::DeviceErrorSetupFailed) {
        returns.insert("errorMessage", QString("Error creating device. Device setup failed."));
        returns.insert("success", false);
    } else {
        Q_ASSERT_X(false, "DeviceHandler", "Unhandled status code for deviceSetupFinished");
        returns.insert("errorMessage", "Unknown error.");
        returns.insert("success", false);
    }
    reply->setData(returns);
    reply->finished();
}

void DeviceHandler::pairingFinished(const QUuid &pairingTransactionId, DeviceManager::DeviceError status, const QString &errorMessage, const DeviceId &deviceId)
{
    qDebug() << "handler: pairing finished";
    JsonReply *reply = m_asyncPairingRequests.take(pairingTransactionId);
    if (!reply) {
        qDebug() << "not for me";
        return;
    }

    if (status != DeviceManager::DeviceErrorNoError) {
        QVariantMap returns;
        returns.insert("success", false);
        returns.insert("errorMessage", errorMessage);
        reply->setData(returns);
        reply->finished();
        return;
    }

    m_asynDeviceAdditions.insert(deviceId, reply);
}
