/****************************************************************************
 *                                                                          *
 *  This file is part of guh.                                               *
 *                                                                          *
 *  Guh is free software: you can redistribute it and/or modify             *
 *  it under the terms of the GNU General Public License as published by    *
 *  the Free Software Foundation, version 2 of the License.                 *
 *                                                                          *
 *  Guh is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *  GNU General Public License for more details.                            *
 *                                                                          *
 *  You should have received a copy of the GNU General Public License       *
 *  along with guh.  If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                          *
 ***************************************************************************/

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
    setDescription("SetPluginConfiguration", "Set a plugin's params.");
    params.insert("pluginId", "uuid");
    QVariantList pluginParams;
    pluginParams.append(JsonTypes::paramTypeRef());
    params.insert("pluginParams", pluginParams);
    setParams("SetPluginConfiguration", params);
    setReturns("SetPluginConfiguration", returns);

    params.clear(); returns.clear();
    setDescription("AddConfiguredDevice", "Add a configured device. Use deviceDescriptorId or deviceParams, depending on the createMethod of the device class.");
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
    connect(GuhCore::instance()->deviceManager(), &DeviceManager::devicesDiscovered, this, &DeviceHandler::devicesDiscovered);
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

    DeviceManager::DeviceError status = GuhCore::instance()->deviceManager()->discoverDevices(deviceClassId, params.value("discoveryParams").toMap());
    switch (status) {
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
        pluginMap.insert("params", plugin->configuration());
        plugins.append(pluginMap);
    }
    returns.insert("plugins", plugins);
    return createReply(returns);
}

JsonReply* DeviceHandler::SetPluginConfiguration(const QVariantMap &params)
{
    QVariantMap returns;
    PluginId pluginId = PluginId(params.value("pluginId").toString());
    QVariantMap pluginParams = params.value("pluginParams").toMap();
    GuhCore::instance()->deviceManager()->setPluginConfig(pluginId, pluginParams);

    // TODO: handle return values
    returns.insert("errorMessage", QString());
    returns.insert("success", true);
    return createReply(returns);
}

JsonReply* DeviceHandler::AddConfiguredDevice(const QVariantMap &params)
{
    DeviceClassId deviceClass(params.value("deviceClassId").toString());
    QVariantMap deviceParams = params.value("deviceParams").toMap();
    DeviceDescriptorId deviceDescriptorId(params.value("deviceDescriptorId").toString());
    DeviceId newDeviceId = DeviceId::createDeviceId();
    DeviceManager::DeviceError status;
    if (deviceDescriptorId.isNull()) {
        qDebug() << "adding a manual device.";
        status = GuhCore::instance()->deviceManager()->addConfiguredDevice(deviceClass, deviceParams, newDeviceId);
    } else {
        qDebug() << "adding a discovered device.";
        status = GuhCore::instance()->deviceManager()->addConfiguredDevice(deviceClass, deviceDescriptorId, newDeviceId);
    }
    QVariantMap returns;
    switch(status) {
    case DeviceManager::DeviceErrorNoError:
        returns.insert("success", true);
        returns.insert("errorMessage", "");
        returns.insert("deviceId", newDeviceId);
        break;
    case DeviceManager::DeviceErrorDeviceClassNotFound:
        returns.insert("errorMessage", "Error creating device. Device class not found.");
        returns.insert("success", false);
        break;
    case DeviceManager::DeviceErrorMissingParameter:
        returns.insert("errorMessage", "Error creating device. Missing parameter.");
        returns.insert("success", false);
        break;
    case DeviceManager::DeviceErrorSetupFailed:
        returns.insert("errorMessage", "Error creating device. Device setup failed.");
        returns.insert("success", false);
        break;
    case DeviceManager::DeviceErrorCreationMethodNotSupported:
        returns.insert("errorMessage", "Error creating device. This device can't be created this way.");
        returns.insert("success", false);
        break;
    case DeviceManager::DeviceErrorDeviceParameterError:
        returns.insert("errorMessage", "Error creating device. Invalid device parameter.");
        returns.insert("success", false);
        break;
    default:
        returns.insert("errorMessage", "Unknown error.");
        returns.insert("success", false);
    }
    return createReply(returns);
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
    switch(GuhCore::instance()->deviceManager()->removeConfiguredDevice(DeviceId(params.value("deviceId").toString()))) {
    case DeviceManager::DeviceErrorNoError:
        returns.insert("success", true);
        returns.insert("errorMessage", "");
        break;
    case DeviceManager::DeviceErrorDeviceNotFound:
        returns.insert("success", false);
        returns.insert("errorMessage", "No such device.");
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
    DeviceClass deviceClass = GuhCore::instance()->deviceManager()->findDeviceClass(params.value("deviceClassId").toUuid());
    foreach (const EventType &eventType, deviceClass.events()) {
        eventList.append(JsonTypes::packEventType(eventType));
    }
    returns.insert("eventTypes", eventList);
    return createReply(returns);
}

JsonReply* DeviceHandler::GetActionTypes(const QVariantMap &params) const
{
    QVariantMap returns;

    QVariantList actionList;
    DeviceClass deviceClass = GuhCore::instance()->deviceManager()->findDeviceClass(params.value("deviceClassId").toUuid());
    foreach (const ActionType &actionType, deviceClass.actions()) {
        actionList.append(JsonTypes::packActionType(actionType));
    }
    returns.insert("actionTypes", actionList);
    return createReply(returns);
}

JsonReply* DeviceHandler::GetStateTypes(const QVariantMap &params) const
{
    QVariantMap returns;

    QVariantList stateList;
    DeviceClass deviceClass = GuhCore::instance()->deviceManager()->findDeviceClass(params.value("deviceClassId").toUuid());
    foreach (const StateType &stateType, deviceClass.states()) {
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
    if (!device->hasState(params.value("stateTypeId").toUuid())) {
        returns.insert("success", false);
        returns.insert("errorMessage", QString("Device %1 %2 doesn't have such a state.").arg(device->name()).arg(device->id().toString()));
        return createReply(returns);
    }
    QVariant stateValue = device->stateValue(params.value("stateTypeId").toUuid());

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
