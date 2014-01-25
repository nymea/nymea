#include "devicehandler.h"

#include "deviceclass.h"
#include "hivecore.h"
#include "devicemanager.h"
#include "deviceplugin.h"

DeviceHandler::DeviceHandler(QObject *parent) :
    JsonHandler(parent)
{
    QVariantMap returns;
    QVariantMap params;

    params.clear(); returns.clear();
    setDescription("GetSupportedDevices", "Returns a list of supported Device classes.");
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
    setDescription("SetPluginParams", "Set a plugin's params.");
    params.insert("pluginId", "uuid");
    QVariantList pluginParams;
    pluginParams.append(JsonTypes::paramTypeRef());
    params.insert("pluginParams", pluginParams);
    setParams("SetPluginParams", params);
    setReturns("SetPluginParams", returns);

    params.clear(); returns.clear();
    setDescription("AddConfiguredDevice", "Add a configured device.");
    params.insert("deviceClassId", "uuid");
    QVariantList deviceParams;
    deviceParams.append(JsonTypes::paramRef());
    params.insert("deviceParams", deviceParams);
    setParams("AddConfiguredDevice", params);
    returns.insert("success", "bool");
    returns.insert("errorMessage", "string");
    setReturns("AddConfiguredDevice", returns);

    params.clear(); returns.clear();
    setDescription("GetConfiguredDevices", "Returns a list of configured devices.");
    setParams("GetConfiguredDevices", params);
    QVariantList devices;
    devices.append(JsonTypes::deviceRef());
    returns.insert("devices", devices);
    setReturns("GetConfiguredDevices", returns);

    params.clear(); returns.clear();
    setDescription("GetTriggerTypes", "Get trigger types for a specified deviceClassId.");
    params.insert("deviceClassId", "uuid");
    setParams("GetTriggerTypes", params);
    QVariantList triggers;
    triggers.append(JsonTypes::triggerTypeRef());
    returns.insert("triggerTypes", triggers);
    setReturns("GetTriggerTypes", returns);

    params.clear(); returns.clear();
    setDescription("GetActionTypes", "Get action types for a specified deviceClassId.");
    params.insert("deviceClassId", "uuid");
    setParams("GetActionTypes", params);
    QVariantList actions;
    actions.append(JsonTypes::actionTypeRef());
    returns.insert("actionTypes", actions);
    setReturns("GetActionTypes", returns);
}

QString DeviceHandler::name() const
{
    return "Devices";
}

QVariantMap DeviceHandler::GetSupportedDevices(const QVariantMap &params) const
{
    Q_UNUSED(params)
    QVariantMap returns;
    QVariantList supportedDeviceList;
    foreach (const DeviceClass &deviceClass, HiveCore::instance()->deviceManager()->supportedDevices()) {
        supportedDeviceList.append(JsonTypes::packDeviceClass(deviceClass));
    }
    returns.insert("deviceClasses", supportedDeviceList);
    return returns;
}

QVariantMap DeviceHandler::GetPlugins(const QVariantMap &params) const
{
    Q_UNUSED(params)
    QVariantMap returns;
    QVariantList plugins;
    foreach (DevicePlugin *plugin, HiveCore::instance()->deviceManager()->plugins()) {
        QVariantMap pluginMap;
        pluginMap.insert("id", plugin->pluginId());
        pluginMap.insert("name", plugin->pluginName());
        pluginMap.insert("params", plugin->configuration());
        plugins.append(pluginMap);
    }
    returns.insert("plugins", plugins);
    return returns;
}

QVariantMap DeviceHandler::SetPluginParams(const QVariantMap &params)
{
    QUuid pluginId = params.value("pluginId").toUuid();
    QVariantMap pluginParams = params.value("pluginParams").toMap();
    HiveCore::instance()->deviceManager()->plugin(pluginId)->setConfiguration(pluginParams);
    return QVariantMap();
}

QVariantMap DeviceHandler::AddConfiguredDevice(const QVariantMap &params)
{
    QUuid deviceClass = params.value("deviceClassId").toUuid();
    QVariantMap deviceParams = params.value("deviceParams").toMap();
    DeviceManager::DeviceError status = HiveCore::instance()->deviceManager()->addConfiguredDevice(deviceClass, deviceParams);
    QVariantMap returns;
    switch(status) {
    case DeviceManager::DeviceErrorNoError:
        returns.insert("success", true);
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
    default:
        returns.insert("errorMessage", "Unknown error.");
        returns.insert("success", false);
    }
    return returns;
}

QVariantMap DeviceHandler::GetConfiguredDevices(const QVariantMap &params) const
{
    Q_UNUSED(params)
    QVariantMap returns;
    QVariantList configuredDeviceList;
    foreach (Device *device, HiveCore::instance()->deviceManager()->configuredDevices()) {
        configuredDeviceList.append(JsonTypes::packDevice(device));
    }
    returns.insert("devices", configuredDeviceList);
    return returns;
}

QVariantMap DeviceHandler::GetTriggerTypes(const QVariantMap &params) const
{
    QVariantMap returns;

    QVariantList triggerList;
    DeviceClass deviceClass = HiveCore::instance()->deviceManager()->findDeviceClass(params.value("deviceClassId").toUuid());
    foreach (const TriggerType &triggerType, deviceClass.triggers()) {
        triggerList.append(JsonTypes::packTriggerType(triggerType));
    }
    returns.insert("triggerTypes", triggerList);
    return returns;
}

QVariantMap DeviceHandler::GetActionTypes(const QVariantMap &params) const
{
    QVariantMap returns;

    QVariantList actionList;
    DeviceClass deviceClass = HiveCore::instance()->deviceManager()->findDeviceClass(params.value("deviceClassId").toUuid());
    foreach (const ActionType &actionType, deviceClass.actions()) {
        actionList.append(JsonTypes::packActionType(actionType));
    }
    returns.insert("actionTypes", actionList);
    return returns;
}
