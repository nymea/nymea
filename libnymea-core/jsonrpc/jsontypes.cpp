/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
 *  Copyright (C) 2017 Michael Zanetti <michael.zanetti@guh.io>            *
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
    \class nymeaserver::JsonTypes
    \brief This class represents the types for the JSON-RPC API.

    \ingroup json
    \inmodule core

    This class represents all JSON-RPC API types and allows to transform Json
    objects into c++ objects and vers visa.

*/

/*! \enum nymeaserver::JsonTypes::BasicType

    This enum type specifies the basic types of a JSON RPC API.

    \value Uuid
    \value String
    \value Int
    \value Uint
    \value Double
    \value Bool
    \value Variant
    \value Color
    \value Time
    \value Object
*/

#include "jsontypes.h"

#include "plugin/device.h"
#include "devicemanager.h"
#include "nymeacore.h"
#include "ruleengine.h"
#include "loggingcategories.h"
#include "logging/logvaluetool.h"
#include "translator.h"
#include "plugin/deviceplugin.h"

#include <QStringList>
#include <QJsonDocument>
#include <QDebug>
#include <QMetaEnum>

namespace nymeaserver {

bool JsonTypes::s_initialized = false;
QString JsonTypes::s_lastError;

QVariantList JsonTypes::s_basicType;
QVariantList JsonTypes::s_stateOperator;
QVariantList JsonTypes::s_valueOperator;
QVariantList JsonTypes::s_inputType;
QVariantList JsonTypes::s_unit;
QVariantList JsonTypes::s_createMethod;
QVariantList JsonTypes::s_setupMethod;
QVariantList JsonTypes::s_removePolicy;
QVariantList JsonTypes::s_deviceError;
QVariantList JsonTypes::s_ruleError;
QVariantList JsonTypes::s_loggingError;
QVariantList JsonTypes::s_loggingSource;
QVariantList JsonTypes::s_loggingLevel;
QVariantList JsonTypes::s_loggingEventType;
QVariantList JsonTypes::s_repeatingMode;
QVariantList JsonTypes::s_configurationError;
QVariantList JsonTypes::s_networkManagerError;
QVariantList JsonTypes::s_networkManagerState;
QVariantList JsonTypes::s_networkDeviceState;
QVariantList JsonTypes::s_userError;
QVariantList JsonTypes::s_tagError;
QVariantList JsonTypes::s_cloudConnectionState;

QVariantMap JsonTypes::s_paramType;
QVariantMap JsonTypes::s_param;
QVariantMap JsonTypes::s_ruleAction;
QVariantMap JsonTypes::s_ruleActionParam;
QVariantMap JsonTypes::s_paramDescriptor;
QVariantMap JsonTypes::s_stateType;
QVariantMap JsonTypes::s_state;
QVariantMap JsonTypes::s_stateDescriptor;
QVariantMap JsonTypes::s_stateEvaluator;
QVariantMap JsonTypes::s_eventType;
QVariantMap JsonTypes::s_event;
QVariantMap JsonTypes::s_eventDescriptor;
QVariantMap JsonTypes::s_actionType;
QVariantMap JsonTypes::s_action;
QVariantMap JsonTypes::s_plugin;
QVariantMap JsonTypes::s_vendor;
QVariantMap JsonTypes::s_deviceClass;
QVariantMap JsonTypes::s_device;
QVariantMap JsonTypes::s_deviceDescriptor;
QVariantMap JsonTypes::s_rule;
QVariantMap JsonTypes::s_ruleDescription;
QVariantMap JsonTypes::s_logEntry;
QVariantMap JsonTypes::s_timeDescriptor;
QVariantMap JsonTypes::s_calendarItem;
QVariantMap JsonTypes::s_timeEventItem;
QVariantMap JsonTypes::s_repeatingOption;
QVariantMap JsonTypes::s_wirelessAccessPoint;
QVariantMap JsonTypes::s_wiredNetworkDevice;
QVariantMap JsonTypes::s_wirelessNetworkDevice;
QVariantMap JsonTypes::s_tokenInfo;
QVariantMap JsonTypes::s_serverConfiguration;
QVariantMap JsonTypes::s_webServerConfiguration;
QVariantMap JsonTypes::s_tag;
QVariantMap JsonTypes::s_mqttPolicy;
QVariantMap JsonTypes::s_package;
QVariantMap JsonTypes::s_repository;

void JsonTypes::init()
{
    // Enums
    s_basicType = enumToStrings(JsonTypes::staticMetaObject, "BasicType");
    s_stateOperator = enumToStrings(Types::staticMetaObject, "StateOperator");
    s_valueOperator = enumToStrings(Types::staticMetaObject, "ValueOperator");
    s_inputType = enumToStrings(Types::staticMetaObject, "InputType");
    s_unit = enumToStrings(Types::staticMetaObject, "Unit");
    s_createMethod = enumToStrings(DeviceClass::staticMetaObject, "CreateMethod");
    s_setupMethod = enumToStrings(DeviceClass::staticMetaObject, "SetupMethod");
    s_removePolicy = enumToStrings(RuleEngine::staticMetaObject, "RemovePolicy");
    s_deviceError = enumToStrings(DeviceManager::staticMetaObject, "DeviceError");
    s_ruleError = enumToStrings(RuleEngine::staticMetaObject, "RuleError");
    s_loggingError = enumToStrings(Logging::staticMetaObject, "LoggingError");
    s_loggingSource = enumToStrings(Logging::staticMetaObject, "LoggingSource");
    s_loggingLevel = enumToStrings(Logging::staticMetaObject, "LoggingLevel");
    s_loggingEventType = enumToStrings(Logging::staticMetaObject, "LoggingEventType");
    s_repeatingMode = enumToStrings(RepeatingOption::staticMetaObject, "RepeatingMode");
    s_configurationError = enumToStrings(NymeaConfiguration::staticMetaObject, "ConfigurationError");
    s_networkManagerError = enumToStrings(NetworkManager::staticMetaObject, "NetworkManagerError");
    s_networkManagerState = enumToStrings(NetworkManager::staticMetaObject, "NetworkManagerState");
    s_networkDeviceState = enumToStrings(NetworkDevice::staticMetaObject, "NetworkDeviceState");
    s_userError = enumToStrings(UserManager::staticMetaObject, "UserError");
    s_tagError = enumToStrings(TagsStorage::staticMetaObject, "TagError");
    s_cloudConnectionState = enumToStrings(CloudManager::staticMetaObject, "CloudConnectionState");

    // ParamType
    s_paramType.insert("id", basicTypeToString(Uuid));
    s_paramType.insert("name", basicTypeToString(String));
    s_paramType.insert("displayName", basicTypeToString(String));
    s_paramType.insert("type", basicTypeRef());
    s_paramType.insert("index", basicTypeToString(Int));
    s_paramType.insert("o:defaultValue", basicTypeToString(Variant));
    s_paramType.insert("o:minValue", basicTypeToString(Variant));
    s_paramType.insert("o:maxValue", basicTypeToString(Variant));
    s_paramType.insert("o:allowedValues", QVariantList() << basicTypeToString(Variant));
    s_paramType.insert("o:inputType", inputTypeRef());
    s_paramType.insert("o:unit", unitRef());
    s_paramType.insert("o:readOnly", basicTypeToString(Bool));

    // Param
    s_param.insert("paramTypeId", basicTypeToString(Uuid));
    s_param.insert("value", basicTypeRef());

    // RuleAction
    s_ruleAction.insert("o:deviceId", basicTypeToString(Uuid));
    s_ruleAction.insert("o:actionTypeId", basicTypeToString(Uuid));
    s_ruleAction.insert("o:interface", basicTypeToString(String));
    s_ruleAction.insert("o:interfaceAction", basicTypeToString(String));
    s_ruleAction.insert("o:ruleActionParams", QVariantList() << ruleActionParamRef());

    // RuleActionParam
    s_ruleActionParam.insert("o:paramTypeId", basicTypeToString(Uuid));
    s_ruleActionParam.insert("o:paramName", basicTypeToString(String));
    s_ruleActionParam.insert("o:value", basicTypeRef());
    s_ruleActionParam.insert("o:eventTypeId", basicTypeToString(Uuid));
    s_ruleActionParam.insert("o:eventParamTypeId", basicTypeToString(Uuid));
    s_ruleActionParam.insert("o:stateDeviceId", basicTypeToString(Uuid));
    s_ruleActionParam.insert("o:stateTypeId", basicTypeToString(Uuid));

    // ParamDescriptor
    s_paramDescriptor.insert("o:paramTypeId", basicTypeToString(Uuid));
    s_paramDescriptor.insert("o:paramName", basicTypeToString(Uuid));
    s_paramDescriptor.insert("value", basicTypeRef());
    s_paramDescriptor.insert("operator", valueOperatorRef());

    // StateType
    s_stateType.insert("id", basicTypeToString(Uuid));
    s_stateType.insert("name", basicTypeToString(String));
    s_stateType.insert("displayName", basicTypeToString(String));
    s_stateType.insert("type", basicTypeRef());
    s_stateType.insert("index", basicTypeToString(Int));
    s_stateType.insert("defaultValue", basicTypeToString(Variant));
    s_stateType.insert("o:unit", unitRef());
    s_stateType.insert("o:minValue", basicTypeToString(Variant));
    s_stateType.insert("o:maxValue", basicTypeToString(Variant));
    s_stateType.insert("o:possibleValues", QVariantList() << basicTypeToString(Variant));

    // State
    s_state.insert("stateTypeId", basicTypeToString(Uuid));
    s_state.insert("deviceId", basicTypeToString(Uuid));
    s_state.insert("value", basicTypeToString(Variant));

    // StateDescriptor
    s_stateDescriptor.insert("o:stateTypeId", basicTypeToString(Uuid));
    s_stateDescriptor.insert("o:deviceId", basicTypeToString(Uuid));
    s_stateDescriptor.insert("o:interface", basicTypeToString(String));
    s_stateDescriptor.insert("o:interfaceState", basicTypeToString(String));
    s_stateDescriptor.insert("value", basicTypeToString(Variant));
    s_stateDescriptor.insert("operator", valueOperatorRef());

    // StateEvaluator
    s_stateEvaluator.insert("o:stateDescriptor", stateDescriptorRef());
    s_stateEvaluator.insert("o:childEvaluators", QVariantList() << stateEvaluatorRef());
    s_stateEvaluator.insert("o:operator", stateOperatorRef());

    // EventType
    s_eventType.insert("id", basicTypeToString(Uuid));
    s_eventType.insert("name", basicTypeToString(String));
    s_eventType.insert("displayName", basicTypeToString(String));
    s_eventType.insert("index", basicTypeToString(Int));
    s_eventType.insert("paramTypes", QVariantList() << paramTypeRef());

    // Event
    s_event.insert("eventTypeId", basicTypeToString(Uuid));
    s_event.insert("deviceId", basicTypeToString(Uuid));
    s_event.insert("o:params", QVariantList() << paramRef());

    // EventDescriptor
    s_eventDescriptor.insert("o:eventTypeId", basicTypeToString(Uuid));
    s_eventDescriptor.insert("o:deviceId", basicTypeToString(Uuid));
    s_eventDescriptor.insert("o:interface", basicTypeToString(String));
    s_eventDescriptor.insert("o:interfaceEvent", basicTypeToString(String));
    s_eventDescriptor.insert("o:paramDescriptors", QVariantList() << paramDescriptorRef());

    // ActionType
    s_actionType.insert("id", basicTypeToString(Uuid));
    s_actionType.insert("name", basicTypeToString(String));
    s_actionType.insert("displayName", basicTypeToString(String));
    s_actionType.insert("index", basicTypeToString(Int));
    s_actionType.insert("paramTypes", QVariantList() << paramTypeRef());

    // Action
    s_action.insert("actionTypeId", basicTypeToString(Uuid));
    s_action.insert("deviceId", basicTypeToString(Uuid));
    s_action.insert("o:params", QVariantList() << paramRef());

    // Pugin
    s_plugin.insert("id", basicTypeToString(Uuid));
    s_plugin.insert("name", basicTypeToString(String));
    s_plugin.insert("displayName", basicTypeToString(String));
    s_plugin.insert("paramTypes", QVariantList() << paramTypeRef());

    // Vendor
    s_vendor.insert("id", basicTypeToString(Uuid));
    s_vendor.insert("name", basicTypeToString(String));
    s_vendor.insert("displayName", basicTypeToString(String));

    // DeviceClass
    s_deviceClass.insert("id", basicTypeToString(Uuid));
    s_deviceClass.insert("vendorId", basicTypeToString(Uuid));
    s_deviceClass.insert("pluginId", basicTypeToString(Uuid));
    s_deviceClass.insert("name", basicTypeToString(String));
    s_deviceClass.insert("displayName", basicTypeToString(String));
    s_deviceClass.insert("interfaces", QVariantList() << basicTypeToString(String));
    s_deviceClass.insert("setupMethod", setupMethodRef());
    s_deviceClass.insert("createMethods", QVariantList() << createMethodRef());
    s_deviceClass.insert("stateTypes", QVariantList() << stateTypeRef());
    s_deviceClass.insert("eventTypes", QVariantList() << eventTypeRef());
    s_deviceClass.insert("actionTypes", QVariantList() << actionTypeRef());
    s_deviceClass.insert("paramTypes", QVariantList() << paramTypeRef());
    s_deviceClass.insert("discoveryParamTypes", QVariantList() << paramTypeRef());

    // Device
    s_device.insert("id", basicTypeToString(Uuid));
    s_device.insert("deviceClassId", basicTypeToString(Uuid));
    s_device.insert("name", basicTypeToString(String));
    s_device.insert("params", QVariantList() << paramRef());
    QVariantMap stateValues;
    stateValues.insert("stateTypeId", basicTypeToString(Uuid));
    stateValues.insert("value", basicTypeToString(Variant));
    s_device.insert("states", QVariantList() << stateValues);
    s_device.insert("setupComplete", basicTypeToString(Bool));
    s_device.insert("o:parentId", basicTypeToString(Uuid));

    // DeviceDescriptor
    s_deviceDescriptor.insert("id", basicTypeToString(Uuid));
    s_deviceDescriptor.insert("deviceId", basicTypeToString(Uuid));
    s_deviceDescriptor.insert("title", basicTypeToString(String));
    s_deviceDescriptor.insert("description", basicTypeToString(String));
    s_deviceDescriptor.insert("deviceParams", QVariantList() << paramRef());

    // Rule
    s_rule.insert("id", basicTypeToString(Uuid));
    s_rule.insert("name", basicTypeToString(String));
    s_rule.insert("enabled", basicTypeToString(Bool));
    s_rule.insert("executable", basicTypeToString(Bool));
    s_rule.insert("active", basicTypeToString(Bool));
    s_rule.insert("eventDescriptors", QVariantList() << eventDescriptorRef());
    s_rule.insert("actions", QVariantList() << ruleActionRef());
    s_rule.insert("exitActions", QVariantList() << ruleActionRef());
    s_rule.insert("stateEvaluator", stateEvaluatorRef());
    s_rule.insert("timeDescriptor", timeDescriptorRef());

    // RuleDescription
    s_ruleDescription.insert("id", basicTypeToString(Uuid));
    s_ruleDescription.insert("name", basicTypeToString(String));
    s_ruleDescription.insert("enabled", basicTypeToString(Bool));
    s_ruleDescription.insert("active", basicTypeToString(Bool));
    s_ruleDescription.insert("executable", basicTypeToString(Bool));

    // LogEntry
    s_logEntry.insert("timestamp", basicTypeToString(Int));
    s_logEntry.insert("loggingLevel", loggingLevelRef());
    s_logEntry.insert("source", loggingSourceRef());
    s_logEntry.insert("o:typeId", basicTypeToString(Uuid));
    s_logEntry.insert("o:deviceId", basicTypeToString(Uuid));
    s_logEntry.insert("o:value", basicTypeToString(String));
    s_logEntry.insert("o:active", basicTypeToString(Bool));
    s_logEntry.insert("o:eventType", loggingEventTypeRef());
    s_logEntry.insert("o:errorCode", basicTypeToString(String));

    // TimeDescriptor
    s_timeDescriptor.insert("o:calendarItems", QVariantList() << calendarItemRef());
    s_timeDescriptor.insert("o:timeEventItems", QVariantList() << timeEventItemRef());

    // CalendarItem
    s_calendarItem.insert("o:datetime", basicTypeToString(QVariant::UInt));
    s_calendarItem.insert("o:startTime", basicTypeToString(QVariant::Time));
    s_calendarItem.insert("duration", basicTypeToString(QVariant::UInt));
    s_calendarItem.insert("o:repeating", repeatingOptionRef());

    // TimeEventItem
    s_timeEventItem.insert("o:datetime", basicTypeToString(QVariant::UInt));
    s_timeEventItem.insert("o:time", basicTypeToString(QVariant::Time));
    s_timeEventItem.insert("o:repeating", repeatingOptionRef());

    // RepeatingOption
    s_repeatingOption.insert("mode", repeatingModeRef());
    s_repeatingOption.insert("o:weekDays", QVariantList() << basicTypeToString(Int));
    s_repeatingOption.insert("o:monthDays", QVariantList() << basicTypeToString(Int));

    // WirelessAccessPoint
    s_wirelessAccessPoint.insert("ssid", basicTypeToString(QVariant::String));
    s_wirelessAccessPoint.insert("macAddress", basicTypeToString(QVariant::String));
    s_wirelessAccessPoint.insert("frequency", basicTypeToString(QVariant::Double));
    s_wirelessAccessPoint.insert("signalStrength", basicTypeToString(QVariant::Int));
    s_wirelessAccessPoint.insert("protected", basicTypeToString(QVariant::Bool));

    // WiredNetworkDevice
    s_wiredNetworkDevice.insert("interface", basicTypeToString(QVariant::String));
    s_wiredNetworkDevice.insert("macAddress", basicTypeToString(QVariant::String));
    s_wiredNetworkDevice.insert("state", networkDeviceStateRef());
    s_wiredNetworkDevice.insert("bitRate", basicTypeToString(QVariant::String));
    s_wiredNetworkDevice.insert("pluggedIn", basicTypeToString(QVariant::Bool));

    // WirelessNetworkDevice
    s_wirelessNetworkDevice.insert("interface", basicTypeToString(QVariant::String));
    s_wirelessNetworkDevice.insert("macAddress", basicTypeToString(QVariant::String));
    s_wirelessNetworkDevice.insert("state", networkDeviceStateRef());
    s_wirelessNetworkDevice.insert("bitRate", basicTypeToString(QVariant::String));
    s_wirelessNetworkDevice.insert("o:currentAccessPoint", wirelessAccessPointRef());

    // TokenInfo
    s_tokenInfo.insert("id", basicTypeToString(QVariant::Uuid));
    s_tokenInfo.insert("userName", basicTypeToString(QVariant::String));
    s_tokenInfo.insert("deviceName", basicTypeToString(QVariant::String));
    s_tokenInfo.insert("creationTime", basicTypeToString(QVariant::UInt));

    // ServerConfiguration
    s_serverConfiguration.insert("id", basicTypeToString(QVariant::String));
    s_serverConfiguration.insert("address", basicTypeToString(QVariant::String));
    s_serverConfiguration.insert("port", basicTypeToString(QVariant::UInt));
    s_serverConfiguration.insert("sslEnabled", basicTypeToString(QVariant::Bool));
    s_serverConfiguration.insert("authenticationEnabled", basicTypeToString(QVariant::Bool));

    s_webServerConfiguration = s_serverConfiguration;
    s_webServerConfiguration.insert("publicFolder", basicTypeToString(QVariant::String));

    // MQTT
    s_mqttPolicy.insert("clientId", basicTypeToString(QVariant::String));
    s_mqttPolicy.insert("username", basicTypeToString(QVariant::String));
    s_mqttPolicy.insert("password", basicTypeToString(QVariant::String));
    s_mqttPolicy.insert("allowedPublishTopicFilters", basicTypeToString(QVariant::StringList));
    s_mqttPolicy.insert("allowedSubscribeTopicFilters", basicTypeToString(QVariant::StringList));

    // Tag
    s_tag.insert("o:deviceId", basicTypeToString(QVariant::Uuid));
    s_tag.insert("o:ruleId", basicTypeToString(QVariant::Uuid));
    s_tag.insert("appId", basicTypeToString(QVariant::String));
    s_tag.insert("tagId", basicTypeToString(QVariant::String));
    s_tag.insert("o:value", basicTypeToString(QVariant::String));

    s_package.insert("id", basicTypeToString(QVariant::String));
    s_package.insert("displayName", basicTypeToString(QVariant::String));
    s_package.insert("summary", basicTypeToString(QVariant::String));
    s_package.insert("installedVersion", basicTypeToString(QVariant::String));
    s_package.insert("candidateVersion", basicTypeToString(QVariant::String));
    s_package.insert("changelog", basicTypeToString(QVariant::String));
    s_package.insert("updateAvailable", basicTypeToString(QVariant::Bool));
    s_package.insert("rollbackAvailable", basicTypeToString(QVariant::Bool));
    s_package.insert("canRemove", basicTypeToString(QVariant::Bool));

    s_repository.insert("id", basicTypeToString(QVariant::String));
    s_repository.insert("displayName", basicTypeToString(QVariant::String));
    s_repository.insert("enabled", basicTypeToString(QVariant::Bool));

    s_initialized = true;
}

QPair<bool, QString> JsonTypes::report(bool status, const QString &message)
{
    return qMakePair<bool, QString>(status, message);
}

QVariantList JsonTypes::enumToStrings(const QMetaObject &metaObject, const QString &enumName)
{
    int enumIndex = metaObject.indexOfEnumerator(enumName.toLatin1().data());
    Q_ASSERT_X(enumIndex >= 0, "JsonTypes", QString("Enumerator %1 not found in %2").arg(enumName).arg(metaObject.className()).toLocal8Bit());
    QMetaEnum metaEnum = metaObject.enumerator(enumIndex);

    QVariantList enumStrings;
    for (int i = 0; i < metaEnum.keyCount(); i++)
        enumStrings << metaEnum.valueToKey(metaEnum.value(i));

    return enumStrings;
}

/*! Returns a map containing all API types. */
QVariantMap JsonTypes::allTypes()
{
    QVariantMap allTypes;
    allTypes.insert("BasicType", basicType());
    allTypes.insert("ParamType", paramTypeDescription());
    allTypes.insert("InputType", inputType());
    allTypes.insert("Unit", unit());
    allTypes.insert("CreateMethod", createMethod());
    allTypes.insert("SetupMethod", setupMethod());
    allTypes.insert("ValueOperator", valueOperator());
    allTypes.insert("StateOperator", stateOperator());
    allTypes.insert("RemovePolicy", removePolicy());
    allTypes.insert("DeviceError", deviceError());
    allTypes.insert("RuleError", ruleError());
    allTypes.insert("LoggingError", loggingError());
    allTypes.insert("LoggingLevel", loggingLevel());
    allTypes.insert("LoggingSource", loggingSource());
    allTypes.insert("LoggingEventType", loggingEventType());
    allTypes.insert("RepeatingMode", repeatingMode());
    allTypes.insert("ConfigurationError", configurationError());
    allTypes.insert("NetworkManagerError", networkManagerError());
    allTypes.insert("NetworkManagerState", networkManagerState());
    allTypes.insert("NetworkDeviceState", networkDeviceState());
    allTypes.insert("UserError", userError());
    allTypes.insert("TagError", tagError());
    allTypes.insert("CloudConnectionState", cloudConnectionState());

    allTypes.insert("StateType", stateTypeDescription());
    allTypes.insert("StateDescriptor", stateDescriptorDescription());
    allTypes.insert("StateEvaluator", stateEvaluatorDescription());
    allTypes.insert("Event", eventDescription());
    allTypes.insert("EventType", eventTypeDescription());
    allTypes.insert("EventDescriptor", eventDescriptorDescription());
    allTypes.insert("ActionType", actionTypeDescription());
    allTypes.insert("Vendor", vendorDescription());
    allTypes.insert("DeviceClass", deviceClassDescription());
    allTypes.insert("Plugin", pluginDescription());
    allTypes.insert("Param", paramDescription());
    allTypes.insert("RuleAction", ruleActionDescription());
    allTypes.insert("RuleActionParam", ruleActionParamDescription());
    allTypes.insert("ParamDescriptor", paramDescriptorDescription());
    allTypes.insert("State", stateDescription());
    allTypes.insert("Device", deviceDescription());
    allTypes.insert("DeviceDescriptor", deviceDescriptorDescription());
    allTypes.insert("Action", actionDescription());
    allTypes.insert("Rule", ruleDescription());
    allTypes.insert("RuleDescription", ruleDescriptionDescription());
    allTypes.insert("LogEntry", logEntryDescription());
    allTypes.insert("TimeDescriptor", timeDescriptorDescription());
    allTypes.insert("CalendarItem", calendarItemDescription());
    allTypes.insert("TimeEventItem", timeEventItemDescription());
    allTypes.insert("RepeatingOption", repeatingOptionDescription());
    allTypes.insert("WirelessAccessPoint", wirelessAccessPointDescription());
    allTypes.insert("WiredNetworkDevice", wiredNetworkDeviceDescription());
    allTypes.insert("WirelessNetworkDevice", wirelessNetworkDeviceDescription());
    allTypes.insert("TokenInfo", tokenInfoDescription());
    allTypes.insert("ServerConfiguration", serverConfigurationDescription());
    allTypes.insert("WebServerConfiguration", serverConfigurationDescription());
    allTypes.insert("Tag", tagDescription());
    allTypes.insert("MqttPolicy", mqttPolicyDescription());
    allTypes.insert("Package", packageDescription());
    allTypes.insert("Repository", repositoryDescription());

    return allTypes;
}

/*! Returns a variant map of the given \a eventType. */
QVariantMap JsonTypes::packEventType(const EventType &eventType, const PluginId &pluginId, const QLocale &locale)
{
    QVariantMap variant;
    variant.insert("id", eventType.id().toString());
    variant.insert("name", eventType.name());
    variant.insert("displayName", NymeaCore::instance()->deviceManager()->translator()->translate(pluginId, eventType.displayName(), locale));
    variant.insert("index", eventType.index());

    QVariantList paramTypes;
    foreach (const ParamType &paramType, eventType.paramTypes())
        paramTypes.append(packParamType(paramType, pluginId, locale));

    variant.insert("paramTypes", paramTypes);
    return variant;
}

/*! Returns a variant map of the given \a event. */
QVariantMap JsonTypes::packEvent(const Event &event)
{
    QVariantMap variant;
    variant.insert("eventTypeId", event.eventTypeId().toString());
    variant.insert("deviceId", event.deviceId().toString());
    QVariantList params;
    foreach (const Param &param, event.params())
        params.append(packParam(param));

    variant.insert("params", params);
    return variant;
}

/*! Returns a variant map of the given \a eventDescriptor. */
QVariantMap JsonTypes::packEventDescriptor(const EventDescriptor &eventDescriptor)
{
    QVariantMap variant;
    if (eventDescriptor.type() == EventDescriptor::TypeDevice) {
        variant.insert("eventTypeId", eventDescriptor.eventTypeId().toString());
        variant.insert("deviceId", eventDescriptor.deviceId().toString());
    } else {
        variant.insert("interface", eventDescriptor.interface());
        variant.insert("interfaceEvent", eventDescriptor.interfaceEvent());
    }
    QVariantList params;
    foreach (const ParamDescriptor &paramDescriptor, eventDescriptor.paramDescriptors())
        params.append(packParamDescriptor(paramDescriptor));

    variant.insert("paramDescriptors", params);
    return variant;
}

/*! Returns a variant map of the given \a actionType. */
QVariantMap JsonTypes::packActionType(const ActionType &actionType, const PluginId &pluginId, const QLocale &locale)
{
    QVariantMap variantMap;
    variantMap.insert("id", actionType.id().toString());
    variantMap.insert("name", actionType.name());
    variantMap.insert("displayName", NymeaCore::instance()->deviceManager()->translator()->translate(pluginId, actionType.displayName(), locale));
    variantMap.insert("index", actionType.index());
    QVariantList paramTypes;
    foreach (const ParamType &paramType, actionType.paramTypes())
        paramTypes.append(packParamType(paramType, pluginId, locale));

    variantMap.insert("paramTypes", paramTypes);
    return variantMap;
}

/*! Returns a variant map of the given \a action. */
QVariantMap JsonTypes::packAction(const Action &action)
{
    QVariantMap variant;
    variant.insert("actionTypeId", action.actionTypeId().toString());
    variant.insert("deviceId", action.deviceId().toString());
    QVariantList params;
    foreach (const Param &param, action.params())
        params.append(packParam(param));

    variant.insert("params", params);
    return variant;
}

/*! Returns a variant map of the given \a ruleAction. */
QVariantMap JsonTypes::packRuleAction(const RuleAction &ruleAction)
{
    QVariantMap variant;
    if (ruleAction.type() == RuleAction::TypeDevice) {
        variant.insert("actionTypeId", ruleAction.actionTypeId().toString());
        variant.insert("deviceId", ruleAction.deviceId().toString());
    } else {
        variant.insert("interface", ruleAction.interface());
        variant.insert("interfaceAction", ruleAction.interfaceAction());
    }
    QVariantList params;
    foreach (const RuleActionParam &ruleActionParam, ruleAction.ruleActionParams())
        params.append(packRuleActionParam(ruleActionParam));

    variant.insert("ruleActionParams", params);
    return variant;
}

/*! Returns a variant map of the given \a ruleActionParam. */
QVariantMap JsonTypes::packRuleActionParam(const RuleActionParam &ruleActionParam)
{
    QVariantMap variantMap;
    if (!ruleActionParam.paramTypeId().isNull()) {
        variantMap.insert("paramTypeId", ruleActionParam.paramTypeId().toString());
    } else {
        variantMap.insert("paramName", ruleActionParam.paramName());
    }

    if (ruleActionParam.isEventBased()) {
        variantMap.insert("eventTypeId", ruleActionParam.eventTypeId().toString());
        variantMap.insert("eventParamTypeId", ruleActionParam.eventParamTypeId().toString());
    } else if (ruleActionParam.isStateBased()) {
        variantMap.insert("stateDeviceId", ruleActionParam.stateDeviceId().toString());
        variantMap.insert("stateTypeId", ruleActionParam.stateTypeId().toString());
    } else {
        variantMap.insert("value", ruleActionParam.value());
    }
    return variantMap;
}

/*! Returns a variant map of the given \a state. */
QVariantMap JsonTypes::packState(const State &state)
{
    QVariantMap stateMap;
    stateMap.insert("stateTypeId", state.stateTypeId().toString());
    stateMap.insert("value", state.value());
    return stateMap;
}

/*! Returns a variant map of the given \a stateType. */
QVariantMap JsonTypes::packStateType(const StateType &stateType, const PluginId &pluginId, const QLocale &locale)
{
    QVariantMap variantMap;
    variantMap.insert("id", stateType.id().toString());
    variantMap.insert("name", stateType.name());
    variantMap.insert("displayName", NymeaCore::instance()->deviceManager()->translator()->translate(pluginId, stateType.displayName(), locale));
    variantMap.insert("index", stateType.index());
    variantMap.insert("type", basicTypeToString(stateType.type()));
    variantMap.insert("defaultValue", stateType.defaultValue());

    if (stateType.maxValue().isValid())
        variantMap.insert("maxValue", stateType.maxValue());

    if (stateType.minValue().isValid())
        variantMap.insert("minValue", stateType.minValue());

    if (!stateType.possibleValues().isEmpty())
        variantMap.insert("possibleValues", stateType.possibleValues());

    if(stateType.unit() != Types::UnitNone)
        variantMap.insert("unit", s_unit.at(stateType.unit()));

    return variantMap;
}

/*! Returns a variant map of the given \a stateDescriptor. */
QVariantMap JsonTypes::packStateDescriptor(const StateDescriptor &stateDescriptor)
{
    QVariantMap variantMap;
    if (stateDescriptor.type() == StateDescriptor::TypeDevice) {
        variantMap.insert("stateTypeId", stateDescriptor.stateTypeId().toString());
        variantMap.insert("deviceId", stateDescriptor.deviceId().toString());
    } else {
        variantMap.insert("interface", stateDescriptor.interface());
        variantMap.insert("interfaceState", stateDescriptor.interfaceState());
    }
    variantMap.insert("value", stateDescriptor.stateValue());
    variantMap.insert("operator", s_valueOperator.at(stateDescriptor.operatorType()));
    return variantMap;
}

/*! Returns a variant map of the given \a stateEvaluator. */
QVariantMap JsonTypes::packStateEvaluator(const StateEvaluator &stateEvaluator)
{
    QVariantMap variantMap;
    if (stateEvaluator.stateDescriptor().isValid())
        variantMap.insert("stateDescriptor", packStateDescriptor(stateEvaluator.stateDescriptor()));

    QVariantList childEvaluators;
    foreach (const StateEvaluator &childEvaluator, stateEvaluator.childEvaluators())
        childEvaluators.append(packStateEvaluator(childEvaluator));

    if (!childEvaluators.isEmpty() || stateEvaluator.stateDescriptor().isValid())
        variantMap.insert("operator", s_stateOperator.at(stateEvaluator.operatorType()));

    if (childEvaluators.count() > 0)
        variantMap.insert("childEvaluators", childEvaluators);

    return variantMap;
}

/*! Returns a variant map of the given \a param. */
QVariantMap JsonTypes::packParam(const Param &param)
{
    QVariantMap variantMap;
    variantMap.insert("paramTypeId", param.paramTypeId().toString());
    variantMap.insert("value", param.value());
    return variantMap;
}

/*! Returns a variant map of the given \a paramDescriptor. */
QVariantMap JsonTypes::packParamDescriptor(const ParamDescriptor &paramDescriptor)
{
    QVariantMap variantMap;
    if (!paramDescriptor.paramTypeId().isNull()) {
        variantMap.insert("paramTypeId", paramDescriptor.paramTypeId().toString());
    } else {
        variantMap.insert("paramName", paramDescriptor.paramName());
    }
    variantMap.insert("value", paramDescriptor.value());
    variantMap.insert("operator", s_valueOperator.at(paramDescriptor.operatorType()));
    return variantMap;
}

/*! Returns a variant map of the given \a paramType. */
QVariantMap JsonTypes::packParamType(const ParamType &paramType, const PluginId &pluginId, const QLocale &locale)
{
    QVariantMap variantMap;
    variantMap.insert("id", paramType.id().toString());
    variantMap.insert("name", paramType.name());
    variantMap.insert("displayName", NymeaCore::instance()->deviceManager()->translator()->translate(pluginId, paramType.displayName(), locale));
    variantMap.insert("type", basicTypeToString(paramType.type()));
    variantMap.insert("index", paramType.index());

    // Optional values
    if (paramType.defaultValue().isValid())
        variantMap.insert("defaultValue", paramType.defaultValue());

    if (paramType.minValue().isValid())
        variantMap.insert("minValue", paramType.minValue());

    if (paramType.maxValue().isValid())
        variantMap.insert("maxValue", paramType.maxValue());

    if (!paramType.allowedValues().isEmpty())
        variantMap.insert("allowedValues", paramType.allowedValues());

    if (paramType.inputType() != Types::InputTypeNone)
        variantMap.insert("inputType", s_inputType.at(paramType.inputType()));

    if (paramType.unit() != Types::UnitNone)
        variantMap.insert("unit", s_unit.at(paramType.unit()));

    if (paramType.readOnly())
        variantMap.insert("readOnly", paramType.readOnly());

    return variantMap;
}

/*! Returns a variant map of the given \a vendor. */
QVariantMap JsonTypes::packVendor(const Vendor &vendor, const QLocale &locale)
{
    DevicePlugin *plugin = nullptr;
    foreach (DevicePlugin *p, NymeaCore::instance()->deviceManager()->plugins()) {
        if (p->supportedVendors().contains(vendor)) {
            plugin = p;
        }
    }
    QVariantMap variantMap;
    variantMap.insert("id", vendor.id().toString());
    variantMap.insert("name", vendor.name());
    variantMap.insert("displayName", NymeaCore::instance()->deviceManager()->translator()->translate(plugin->pluginId(), vendor.displayName(), locale));
    return variantMap;
}

/*! Returns a variant map of the given \a deviceClass. */
QVariantMap JsonTypes::packDeviceClass(const DeviceClass &deviceClass, const QLocale &locale)
{
    QVariantMap variant;
    variant.insert("id", deviceClass.id().toString());
    variant.insert("name", deviceClass.name());
    variant.insert("displayName", NymeaCore::instance()->deviceManager()->translator()->translate(deviceClass.pluginId(), deviceClass.displayName(), locale));
    variant.insert("vendorId", deviceClass.vendorId().toString());
    variant.insert("pluginId", deviceClass.pluginId().toString());
    variant.insert("interfaces", deviceClass.interfaces());

    QVariantList stateTypes;
    foreach (const StateType &stateType, deviceClass.stateTypes())
        stateTypes.append(packStateType(stateType, deviceClass.pluginId(), locale));

    QVariantList eventTypes;
    foreach (const EventType &eventType, deviceClass.eventTypes())
        eventTypes.append(packEventType(eventType, deviceClass.pluginId(), locale));

    QVariantList actionTypes;
    foreach (const ActionType &actionType, deviceClass.actionTypes())
        actionTypes.append(packActionType(actionType, deviceClass.pluginId(), locale));

    QVariantList paramTypes;
    foreach (const ParamType &paramType, deviceClass.paramTypes())
        paramTypes.append(packParamType(paramType, deviceClass.pluginId(), locale));

    QVariantList discoveryParamTypes;
    foreach (const ParamType &paramType, deviceClass.discoveryParamTypes())
        discoveryParamTypes.append(packParamType(paramType, deviceClass.pluginId(), locale));

    variant.insert("paramTypes", paramTypes);
    variant.insert("discoveryParamTypes", discoveryParamTypes);
    variant.insert("stateTypes", stateTypes);
    variant.insert("eventTypes", eventTypes);
    variant.insert("actionTypes", actionTypes);
    variant.insert("createMethods", packCreateMethods(deviceClass.createMethods()));
    variant.insert("setupMethod", s_setupMethod.at(deviceClass.setupMethod()));
    return variant;
}

/*! Returns a variant map of the given \a plugin. */
QVariantMap JsonTypes::packPlugin(DevicePlugin *plugin, const QLocale &locale)
{
    QVariantMap pluginMap;
    pluginMap.insert("id", plugin->pluginId().toString());
    pluginMap.insert("name", plugin->pluginName());
    pluginMap.insert("displayName", NymeaCore::instance()->deviceManager()->translator()->translate(plugin->pluginId(), plugin->pluginDisplayName(), locale));

    QVariantList params;
    foreach (const ParamType &param, plugin->configurationDescription())
        params.append(packParamType(param, plugin->pluginId(), locale));

    pluginMap.insert("paramTypes", params);
    return pluginMap;
}

/*! Returns a variant map of the given \a device. */
QVariantMap JsonTypes::packDevice(Device *device)
{
    QVariantMap variant;
    variant.insert("id", device->id().toString());
    variant.insert("deviceClassId", device->deviceClassId().toString());
    variant.insert("name", device->name());
    QVariantList params;
    foreach (const Param &param, device->params())
        params.append(packParam(param));

    if (!device->parentId().isNull())
        variant.insert("parentId", device->parentId().toString());

    variant.insert("params", params);
    variant.insert("states", packDeviceStates(device));
    variant.insert("setupComplete", device->setupComplete());
    return variant;
}

/*! Returns a variant map of the given \a descriptor. */
QVariantMap JsonTypes::packDeviceDescriptor(const DeviceDescriptor &descriptor)
{
    QVariantMap variant;
    variant.insert("id", descriptor.id().toString());
    variant.insert("deviceId", descriptor.deviceId().toString());
    variant.insert("title", descriptor.title());
    variant.insert("description", descriptor.description());
    QVariantList params;
    foreach (const Param &param, descriptor.params()) {
        params.append(packParam(param));
    }
    variant.insert("deviceParams", params);
    return variant;
}

/*! Returns a variant map of the given \a rule. */
QVariantMap JsonTypes::packRule(const Rule &rule)
{
    QVariantMap ruleMap;
    ruleMap.insert("id", rule.id().toString());
    ruleMap.insert("name", rule.name());
    ruleMap.insert("enabled", rule.enabled());
    ruleMap.insert("active", rule.active());
    ruleMap.insert("executable", rule.executable());
    ruleMap.insert("timeDescriptor", JsonTypes::packTimeDescriptor(rule.timeDescriptor()));

    QVariantList eventDescriptorList;
    foreach (const EventDescriptor &eventDescriptor, rule.eventDescriptors())
        eventDescriptorList.append(JsonTypes::packEventDescriptor(eventDescriptor));

    ruleMap.insert("eventDescriptors", eventDescriptorList);
    ruleMap.insert("stateEvaluator", JsonTypes::packStateEvaluator(rule.stateEvaluator()));

    QVariantList actionList;
    foreach (const RuleAction &action, rule.actions())
        actionList.append(JsonTypes::packRuleAction(action));

    ruleMap.insert("actions", actionList);

    QVariantList exitActionList;
    foreach (const RuleAction &action, rule.exitActions())
        exitActionList.append(JsonTypes::packRuleAction(action));

    ruleMap.insert("exitActions", exitActionList);
    return ruleMap;
}

/*! Returns a variant map of the given \a rules. */
QVariantList JsonTypes::packRules(const QList<Rule> rules)
{
    QVariantList rulesList;
    foreach (const Rule &rule, rules)
        rulesList.append(JsonTypes::packRule(rule));

    return rulesList;
}

/*! Returns a variant map of the given \a rule. */
QVariantMap JsonTypes::packRuleDescription(const Rule &rule)
{
    QVariantMap ruleDescriptionMap;
    ruleDescriptionMap.insert("id", rule.id().toString());
    ruleDescriptionMap.insert("name", rule.name());
    ruleDescriptionMap.insert("enabled", rule.enabled());
    ruleDescriptionMap.insert("active", rule.active());
    ruleDescriptionMap.insert("executable", rule.executable());
    return ruleDescriptionMap;
}

/*! Returns a variant map of the given \a logEntry. */
QVariantMap JsonTypes::packLogEntry(const LogEntry &logEntry)
{
    QVariantMap logEntryMap;
    logEntryMap.insert("timestamp", logEntry.timestamp().toMSecsSinceEpoch());
    logEntryMap.insert("loggingLevel", s_loggingLevel.at(logEntry.level()));
    logEntryMap.insert("source", s_loggingSource.at(logEntry.source()));
    logEntryMap.insert("eventType", s_loggingEventType.at(logEntry.eventType()));

    if (logEntry.eventType() == Logging::LoggingEventTypeActiveChange)
        logEntryMap.insert("active", logEntry.active());

    if (logEntry.eventType() == Logging::LoggingEventTypeEnabledChange)
        logEntryMap.insert("active", logEntry.active());

    if (logEntry.level() == Logging::LoggingLevelAlert) {
        switch (logEntry.source()) {
        case Logging::LoggingSourceRules:
            logEntryMap.insert("errorCode", s_ruleError.at(logEntry.errorCode()));
            break;
        case Logging::LoggingSourceActions:
        case Logging::LoggingSourceEvents:
        case Logging::LoggingSourceStates:
            logEntryMap.insert("errorCode", s_deviceError.at(logEntry.errorCode()));
            break;
        case Logging::LoggingSourceSystem:
            // FIXME: Update this once we support error codes for the general system
            //            logEntryMap.insert("errorCode", "");
            break;
        }
    }

    switch (logEntry.source()) {
    case Logging::LoggingSourceActions:
    case Logging::LoggingSourceEvents:
    case Logging::LoggingSourceStates:
        logEntryMap.insert("typeId", logEntry.typeId().toString());
        logEntryMap.insert("deviceId", logEntry.deviceId().toString());
        logEntryMap.insert("value", LogValueTool::convertVariantToString(logEntry.value()));
        break;
    case Logging::LoggingSourceSystem:
        logEntryMap.insert("active", logEntry.active());
        break;
    case Logging::LoggingSourceRules:
        logEntryMap.insert("typeId", logEntry.typeId().toString());
        break;
    }

    return logEntryMap;
}

/*! Returns a variant map of the given \a tag. */
QVariantMap JsonTypes::packTag(const Tag &tag)
{
    QVariantMap ret;
    if (!tag.deviceId().isNull()){
        ret.insert("deviceId", tag.deviceId().toString());
    } else {
        ret.insert("ruleId", tag.ruleId().toString());
    }
    ret.insert("appId", tag.appId());
    ret.insert("tagId", tag.tagId());
    ret.insert("value", tag.value());
    return ret;
}

/*! Returns a variant list of the given \a createMethods. */
QVariantList JsonTypes::packCreateMethods(DeviceClass::CreateMethods createMethods)
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

/*! Returns a variant map of the given \a option. */
QVariantMap JsonTypes::packRepeatingOption(const RepeatingOption &option)
{
    QVariantMap optionVariant;
    optionVariant.insert("mode", s_repeatingMode.at(option.mode()));
    if (!option.weekDays().isEmpty()) {
        QVariantList weekDaysVariantList;
        foreach (const int& weekDay, option.weekDays())
            weekDaysVariantList.append(QVariant(weekDay));

        optionVariant.insert("weekDays", weekDaysVariantList);
    }

    if (!option.monthDays().isEmpty()) {
        QVariantList monthDaysVariantList;
        foreach (const int& monthDay, option.monthDays())
            monthDaysVariantList.append(QVariant(monthDay));

        optionVariant.insert("monthDays", monthDaysVariantList);
    }
    return optionVariant;
}

/*! Returns a variant map of the given \a calendarItem. */
QVariantMap JsonTypes::packCalendarItem(const CalendarItem &calendarItem)
{
    QVariantMap calendarItemVariant;
    calendarItemVariant.insert("duration", calendarItem.duration());

    if (!calendarItem.dateTime().isNull() && calendarItem.dateTime().toTime_t() != 0)
        calendarItemVariant.insert("datetime", calendarItem.dateTime().toTime_t());

    if (!calendarItem.startTime().isNull())
        calendarItemVariant.insert("startTime", calendarItem.startTime().toString("hh:mm"));

    if (!calendarItem.repeatingOption().isEmtpy())
        calendarItemVariant.insert("repeating", packRepeatingOption(calendarItem.repeatingOption()));

    return calendarItemVariant;
}

/*! Returns a variant map of the given \a timeEventItem. */
QVariantMap JsonTypes::packTimeEventItem(const TimeEventItem &timeEventItem)
{
    QVariantMap timeEventItemVariant;

    if (!timeEventItem.dateTime().isNull() && timeEventItem.dateTime().toTime_t() != 0)
        timeEventItemVariant.insert("datetime", timeEventItem.dateTime().toTime_t());

    if (!timeEventItem.time().isNull())
        timeEventItemVariant.insert("time", timeEventItem.time().toString("hh:mm"));

    if (!timeEventItem.repeatingOption().isEmtpy())
        timeEventItemVariant.insert("repeating", packRepeatingOption(timeEventItem.repeatingOption()));

    return timeEventItemVariant;
}

/*! Returns a variant map of the given \a timeDescriptor. */
QVariantMap JsonTypes::packTimeDescriptor(const TimeDescriptor &timeDescriptor)
{
    QVariantMap timeDescriptorVariant;

    if (!timeDescriptor.calendarItems().isEmpty()) {
        QVariantList calendarItems;
        foreach (const CalendarItem &calendarItem, timeDescriptor.calendarItems())
            calendarItems.append(packCalendarItem(calendarItem));

        timeDescriptorVariant.insert("calendarItems", calendarItems);
    }

    if (!timeDescriptor.timeEventItems().isEmpty()) {
        QVariantList timeEventItems;
        foreach (const TimeEventItem &timeEventItem, timeDescriptor.timeEventItems())
            timeEventItems.append(packTimeEventItem(timeEventItem));

        timeDescriptorVariant.insert("timeEventItems", timeEventItems);
    }

    return timeDescriptorVariant;
}

/*! Returns a variant map of the given \a wirelessAccessPoint. */
QVariantMap JsonTypes::packWirelessAccessPoint(WirelessAccessPoint *wirelessAccessPoint)
{
    QVariantMap wirelessAccessPointVariant;
    wirelessAccessPointVariant.insert("ssid", wirelessAccessPoint->ssid());
    wirelessAccessPointVariant.insert("macAddress", wirelessAccessPoint->macAddress());
    wirelessAccessPointVariant.insert("frequency", wirelessAccessPoint->frequency());
    wirelessAccessPointVariant.insert("signalStrength", wirelessAccessPoint->signalStrength());
    wirelessAccessPointVariant.insert("protected", wirelessAccessPoint->isProtected());
    return wirelessAccessPointVariant;
}

/*! Returns a variant map of the given \a networkDevice. */
QVariantMap JsonTypes::packWiredNetworkDevice(WiredNetworkDevice *networkDevice)
{
    QVariantMap networkDeviceVariant;
    networkDeviceVariant.insert("interface", networkDevice->interface());
    networkDeviceVariant.insert("macAddress", networkDevice->macAddress());
    networkDeviceVariant.insert("state", networkDevice->deviceStateString());
    networkDeviceVariant.insert("bitRate", QString("%1 [Mb/s]").arg(QString::number(networkDevice->bitRate())));
    networkDeviceVariant.insert("pluggedIn", networkDevice->pluggedIn());
    return networkDeviceVariant;
}

/*! Returns a variant map of the given \a networkDevice. */
QVariantMap JsonTypes::packWirelessNetworkDevice(WirelessNetworkDevice *networkDevice)
{
    QVariantMap networkDeviceVariant;
    networkDeviceVariant.insert("interface", networkDevice->interface());
    networkDeviceVariant.insert("macAddress", networkDevice->macAddress());
    networkDeviceVariant.insert("state", networkDevice->deviceStateString());
    networkDeviceVariant.insert("bitRate", QString("%1 [Mb/s]").arg(QString::number(networkDevice->bitRate())));
    if (networkDevice->activeAccessPoint())
        networkDeviceVariant.insert("currentAccessPoint", JsonTypes::packWirelessAccessPoint(networkDevice->activeAccessPoint()));

    return networkDeviceVariant;
}

/*! Returns a variant list of the supported vendors. */
QVariantList JsonTypes::packSupportedVendors(const QLocale &locale)
{
    QVariantList supportedVendors;
    foreach (const Vendor &vendor, NymeaCore::instance()->deviceManager()->supportedVendors())
        supportedVendors.append(packVendor(vendor, locale));

    return supportedVendors;
}

/*! Returns a variant list of the supported devices with the given \a vendorId. */
QVariantList JsonTypes::packSupportedDevices(const VendorId &vendorId, const QLocale &locale)
{
    QVariantList supportedDeviceList;
    foreach (const DeviceClass &deviceClass, NymeaCore::instance()->deviceManager()->supportedDevices(vendorId))
        supportedDeviceList.append(packDeviceClass(deviceClass, locale));

    return supportedDeviceList;
}

/*! Returns a variant list of configured devices. */
QVariantList JsonTypes::packConfiguredDevices()
{
    QVariantList configuredDeviceList;
    foreach (Device *device, NymeaCore::instance()->deviceManager()->configuredDevices())
        configuredDeviceList.append(packDevice(device));

    return configuredDeviceList;
}

/*! Returns a variant list of States from the given \a device. */
QVariantList JsonTypes::packDeviceStates(Device *device)
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

/*! Returns a variant list of the given \a deviceDescriptors. */
QVariantList JsonTypes::packDeviceDescriptors(const QList<DeviceDescriptor> deviceDescriptors)
{
    QVariantList deviceDescriptorList;
    foreach (const DeviceDescriptor &deviceDescriptor, deviceDescriptors)
        deviceDescriptorList.append(JsonTypes::packDeviceDescriptor(deviceDescriptor));

    return deviceDescriptorList;
}

/*! Returns a variant map with the current basic configuration of the server. */
QVariantMap JsonTypes::packBasicConfiguration()
{
    QVariantMap basicConfiguration;
    basicConfiguration.insert("serverName", NymeaCore::instance()->configuration()->serverName());
    basicConfiguration.insert("serverUuid", NymeaCore::instance()->configuration()->serverUuid().toString());
    basicConfiguration.insert("serverTime", NymeaCore::instance()->timeManager()->currentDateTime().toTime_t());
    basicConfiguration.insert("timeZone", QString::fromUtf8(NymeaCore::instance()->timeManager()->timeZone()));
    basicConfiguration.insert("language", NymeaCore::instance()->configuration()->locale().name());
    basicConfiguration.insert("debugServerEnabled", NymeaCore::instance()->configuration()->debugServerEnabled());
    return basicConfiguration;
}

QVariantMap JsonTypes::packServerConfiguration(const ServerConfiguration &config)
{
    QVariantMap serverConfiguration;
    serverConfiguration.insert("id", config.id);
    serverConfiguration.insert("address", config.address.toString());
    serverConfiguration.insert("port", config.port);
    serverConfiguration.insert("sslEnabled", config.sslEnabled);
    serverConfiguration.insert("authenticationEnabled", config.authenticationEnabled);
    return serverConfiguration;
}

QVariantMap JsonTypes::packWebServerConfiguration(const WebServerConfiguration &config)
{
    QVariantMap webServerConfiguration = packServerConfiguration(config);
    webServerConfiguration.insert("publicFolder", config.publicFolder);
    return webServerConfiguration;
}

QVariantMap JsonTypes::packMqttPolicy(const MqttPolicy &policy)
{
    QVariantMap policyMap;
    policyMap.insert("clientId", policy.clientId);
    policyMap.insert("username", policy.username);
    policyMap.insert("password", policy.password);
    policyMap.insert("allowedPublishTopicFilters", policy.allowedPublishTopicFilters);
    policyMap.insert("allowedSubscribeTopicFilters", policy.allowedSubscribeTopicFilters);
    return policyMap;
}

/*! Returns a variant list containing all rule descriptions. */
QVariantList JsonTypes::packRuleDescriptions()
{
    QVariantList rulesList;
    foreach (const Rule &rule, NymeaCore::instance()->ruleEngine()->rules())
        rulesList.append(JsonTypes::packRuleDescription(rule));

    return rulesList;
}

/*! Returns a variant list of the given \a rules. */
QVariantList JsonTypes::packRuleDescriptions(const QList<Rule> &rules)
{
    QVariantList rulesList;
    foreach (const Rule &rule, rules)
        rulesList.append(JsonTypes::packRuleDescription(rule));

    return rulesList;
}

/*! Returns a variant list of action types for the given \a deviceClass. */
QVariantList JsonTypes::packActionTypes(const DeviceClass &deviceClass, const QLocale &locale)
{
    QVariantList actionTypes;
    foreach (const ActionType &actionType, deviceClass.actionTypes())
        actionTypes.append(JsonTypes::packActionType(actionType, deviceClass.pluginId(), locale));

    return actionTypes;
}

/*! Returns a variant list of state types for the given \a deviceClass. */
QVariantList JsonTypes::packStateTypes(const DeviceClass &deviceClass, const QLocale &locale)
{
    QVariantList stateTypes;
    foreach (const StateType &stateType, deviceClass.stateTypes())
        stateTypes.append(JsonTypes::packStateType(stateType, deviceClass.pluginId(), locale));

    return stateTypes;
}

/*! Returns a variant list of event types for the given \a deviceClass. */
QVariantList JsonTypes::packEventTypes(const DeviceClass &deviceClass, const QLocale &locale)
{
    QVariantList eventTypes;
    foreach (const EventType &eventType, deviceClass.eventTypes())
        eventTypes.append(JsonTypes::packEventType(eventType, deviceClass.pluginId(), locale));

    return eventTypes;
}

/*! Returns a variant list containing all plugins. */
QVariantList JsonTypes::packPlugins(const QLocale &locale)
{
    QVariantList pluginsList;
    foreach (DevicePlugin *plugin, NymeaCore::instance()->deviceManager()->plugins()) {
        QVariantMap pluginMap = packPlugin(plugin, locale);
        pluginsList.append(pluginMap);
    }
    return pluginsList;
}

QVariantMap JsonTypes::packTokenInfo(const TokenInfo &tokenInfo)
{
    QVariantMap ret;
    ret.insert("id", tokenInfo.id().toString());
    ret.insert("userName", tokenInfo.username());
    ret.insert("deviceName", tokenInfo.deviceName());
    ret.insert("creationTime", tokenInfo.creationTime().toTime_t());
    return ret;
}

QVariantMap JsonTypes::packPackage(const Package &package)
{
    QVariantMap ret;
    ret.insert("id", package.packageId());
    ret.insert("displayName", package.displayName());
    ret.insert("summary", package.summary());
    ret.insert("installedVersion", package.installedVersion());
    ret.insert("candidateVersion", package.candidateVersion());
    ret.insert("changelog", package.changelog());
    ret.insert("updateAvailable", package.updateAvailable());
    ret.insert("rollbackAvailable", package.rollbackAvailable());
    ret.insert("canRemove", package.canRemove());
    return ret;
}

QVariantMap JsonTypes::packRepository(const Repository &repository)
{
    QVariantMap ret;
    ret.insert("id", repository.id());
    ret.insert("displayName", repository.displayName());
    ret.insert("enabled", repository.enabled());
    return ret;
}

/*! Returns the type string for the given \a type. */
QString JsonTypes::basicTypeToString(const QVariant::Type &type)
{
    switch (type) {
    case QVariant::Uuid:
        return "Uuid";
    case QVariant::String:
        return "String";
    case QVariant::StringList:
        return "StringList";
    case QVariant::Int:
        return "Int";
    case QVariant::UInt:
        return "Uint";
    case QVariant::Double:
        return "Double";
    case QVariant::Bool:
        return "Bool";
    case QVariant::Color:
        return "Color";
    case QVariant::Time:
        return "Time";
    default:
        return QVariant::typeToName(static_cast<int>(type));
    }
}

/*! Returns a \l{Param} created from the given \a paramMap. */
Param JsonTypes::unpackParam(const QVariantMap &paramMap)
{
    if (paramMap.keys().count() == 0)
        return Param();

    ParamTypeId paramTypeId = paramMap.value("paramTypeId").toString();
    QVariant value = paramMap.value("value");
    return Param(paramTypeId, value);
}

/*! Returns a \l{ParamList} created from the given \a paramList. */
ParamList JsonTypes::unpackParams(const QVariantList &paramList)
{
    ParamList params;
    foreach (const QVariant &paramVariant, paramList)
        params.append(unpackParam(paramVariant.toMap()));

    return params;
}

/*! Returns a \l{Rule} created from the given \a ruleMap. */
Rule JsonTypes::unpackRule(const QVariantMap &ruleMap)
{
    // The rule id will only be valid if unpacking for edit
    RuleId ruleId = RuleId(ruleMap.value("ruleId").toString());

    QString name = ruleMap.value("name", QString()).toString();

    // By default enabled
    bool enabled = ruleMap.value("enabled", true).toBool();

    // By default executable
    bool executable = ruleMap.value("executable", true).toBool();

    StateEvaluator stateEvaluator = JsonTypes::unpackStateEvaluator(ruleMap.value("stateEvaluator").toMap());
    TimeDescriptor timeDescriptor = JsonTypes::unpackTimeDescriptor(ruleMap.value("timeDescriptor").toMap());

    QList<EventDescriptor> eventDescriptors;
    if (ruleMap.contains("eventDescriptors")) {
        QVariantList eventDescriptorVariantList = ruleMap.value("eventDescriptors").toList();
        foreach (const QVariant &eventDescriptorVariant, eventDescriptorVariantList) {
            eventDescriptors.append(JsonTypes::unpackEventDescriptor(eventDescriptorVariant.toMap()));
        }
    }

    QList<RuleAction> actions;
    if (ruleMap.contains("actions")) {
        QVariantList actionsVariantList = ruleMap.value("actions").toList();
        foreach (const QVariant &actionVariant, actionsVariantList) {
            actions.append(JsonTypes::unpackRuleAction(actionVariant.toMap()));
        }
    }

    QList<RuleAction> exitActions;
    if (ruleMap.contains("exitActions")) {
        QVariantList exitActionsVariantList = ruleMap.value("exitActions").toList();
        foreach (const QVariant &exitActionVariant, exitActionsVariantList) {
            exitActions.append(JsonTypes::unpackRuleAction(exitActionVariant.toMap()));
        }
    }

    Rule rule;
    rule.setId(ruleId);
    rule.setName(name);
    rule.setTimeDescriptor(timeDescriptor);
    rule.setStateEvaluator(stateEvaluator);
    rule.setEventDescriptors(eventDescriptors);
    rule.setActions(actions);
    rule.setExitActions(exitActions);
    rule.setEnabled(enabled);
    rule.setExecutable(executable);
    return rule;
}

/*! Returns a \l{RuleAction} created from the given \a ruleActionMap. */
RuleAction JsonTypes::unpackRuleAction(const QVariantMap &ruleActionMap)
{
    ActionTypeId actionTypeId(ruleActionMap.value("actionTypeId").toString());
    DeviceId actionDeviceId(ruleActionMap.value("deviceId").toString());
    QString interface = ruleActionMap.value("interface").toString();
    QString interfaceAction = ruleActionMap.value("interfaceAction").toString();
    RuleActionParamList actionParamList = JsonTypes::unpackRuleActionParams(ruleActionMap.value("ruleActionParams").toList());

    if (!actionTypeId.isNull() && !actionDeviceId.isNull()) {
        return RuleAction(actionTypeId, actionDeviceId, actionParamList);
    }
    return RuleAction(interface, interfaceAction, actionParamList);
}

/*! Returns a \l{RuleActionParam} created from the given \a ruleActionParamMap. */
RuleActionParam JsonTypes::unpackRuleActionParam(const QVariantMap &ruleActionParamMap)
{
    if (ruleActionParamMap.keys().count() == 0)
        return RuleActionParam();

    ParamTypeId paramTypeId = ParamTypeId(ruleActionParamMap.value("paramTypeId").toString());
    QString paramName = ruleActionParamMap.value("paramName").toString();

    RuleActionParam param;
    if (paramTypeId.isNull()) {
        param = RuleActionParam(paramName);
    } else {
        param = RuleActionParam(paramTypeId);
    }
    param.setValue(ruleActionParamMap.value("value"));
    param.setEventTypeId(EventTypeId(ruleActionParamMap.value("eventTypeId").toString()));
    param.setEventParamTypeId(ParamTypeId(ruleActionParamMap.value("eventParamTypeId").toString()));
    param.setStateDeviceId(DeviceId(ruleActionParamMap.value("stateDeviceId").toString()));
    param.setStateTypeId(StateTypeId(ruleActionParamMap.value("stateTypeId").toString()));
    return param;
}

/*! Returns a \l{RuleActionParamList} created from the given \a ruleActionParamList. */
RuleActionParamList JsonTypes::unpackRuleActionParams(const QVariantList &ruleActionParamList)
{
    RuleActionParamList ruleActionParams;
    foreach (const QVariant &paramVariant, ruleActionParamList)
        ruleActionParams.append(unpackRuleActionParam(paramVariant.toMap()));

    return ruleActionParams;
}

/*! Returns a \l{ParamDescriptor} created from the given \a paramMap. */
ParamDescriptor JsonTypes::unpackParamDescriptor(const QVariantMap &paramMap)
{
    QString operatorString = paramMap.value("operator").toString();
    QMetaObject metaObject = Types::staticMetaObject;
    int enumIndex = metaObject.indexOfEnumerator("ValueOperator");
    QMetaEnum metaEnum = metaObject.enumerator(enumIndex);
    Types::ValueOperator valueOperator = static_cast<Types::ValueOperator>(metaEnum.keyToValue(operatorString.toLatin1().data()));

    if (paramMap.contains("paramTypeId")) {
        ParamDescriptor param = ParamDescriptor(ParamTypeId(paramMap.value("paramTypeId").toString()), paramMap.value("value"));
        param.setOperatorType(valueOperator);
        return param;
    }
    ParamDescriptor param = ParamDescriptor(paramMap.value("paramName").toString(), paramMap.value("value"));
    param.setOperatorType(valueOperator);
    return param;
}

/*! Returns a list of \l{ParamDescriptor} created from the given \a paramList. */
QList<ParamDescriptor> JsonTypes::unpackParamDescriptors(const QVariantList &paramList)
{
    QList<ParamDescriptor> params;
    foreach (const QVariant &paramVariant, paramList)
        params.append(unpackParamDescriptor(paramVariant.toMap()));

    return params;
}

/*! Returns a \l{EventDescriptor} created from the given \a eventDescriptorMap. */
EventDescriptor JsonTypes::unpackEventDescriptor(const QVariantMap &eventDescriptorMap)
{
    EventTypeId eventTypeId(eventDescriptorMap.value("eventTypeId").toString());
    DeviceId eventDeviceId(eventDescriptorMap.value("deviceId").toString());
    QString interface = eventDescriptorMap.value("interface").toString();
    QString interfaceEvent = eventDescriptorMap.value("interfaceEvent").toString();
    QList<ParamDescriptor> eventParams = JsonTypes::unpackParamDescriptors(eventDescriptorMap.value("paramDescriptors").toList());
    if (!eventDeviceId.isNull() && !eventTypeId.isNull()) {
        return EventDescriptor(eventTypeId, eventDeviceId, eventParams);
    }
    return EventDescriptor(interface, interfaceEvent, eventParams);
}

/*! Returns a \l{StateEvaluator} created from the given \a stateEvaluatorMap. */
StateEvaluator JsonTypes::unpackStateEvaluator(const QVariantMap &stateEvaluatorMap)
{
    StateEvaluator ret(unpackStateDescriptor(stateEvaluatorMap.value("stateDescriptor").toMap()));
    if (stateEvaluatorMap.contains("operator")) {
        ret.setOperatorType(static_cast<Types::StateOperator>(s_stateOperator.indexOf(stateEvaluatorMap.value("operator").toString())));
    } else {
        ret.setOperatorType(Types::StateOperatorAnd);
    }

    QList<StateEvaluator> childEvaluators;
    foreach (const QVariant &childEvaluator, stateEvaluatorMap.value("childEvaluators").toList())
        childEvaluators.append(unpackStateEvaluator(childEvaluator.toMap()));

    ret.setChildEvaluators(childEvaluators);
    return ret;
}

/*! Returns a \l{StateDescriptor} created from the given \a stateDescriptorMap. */
StateDescriptor JsonTypes::unpackStateDescriptor(const QVariantMap &stateDescriptorMap)
{
    StateTypeId stateTypeId(stateDescriptorMap.value("stateTypeId").toString());
    DeviceId deviceId(stateDescriptorMap.value("deviceId").toString());
    QString interface(stateDescriptorMap.value("interface").toString());
    QString interfaceState(stateDescriptorMap.value("interfaceState").toString());
    QVariant value = stateDescriptorMap.value("value");
    Types::ValueOperator operatorType = static_cast<Types::ValueOperator>(s_valueOperator.indexOf(stateDescriptorMap.value("operator").toString()));
    if (!deviceId.isNull() && !stateTypeId.isNull()) {
        StateDescriptor stateDescriptor(stateTypeId, deviceId, value, operatorType);
        return stateDescriptor;
    }
    StateDescriptor stateDescriptor(interface, interfaceState, value, operatorType);
    return stateDescriptor;
}

/*! Returns a \l{LogFilter} created from the given \a logFilterMap. */
LogFilter JsonTypes::unpackLogFilter(const QVariantMap &logFilterMap)
{
    LogFilter filter;
    if (logFilterMap.contains("timeFilters")) {
        QVariantList timeFilters = logFilterMap.value("timeFilters").toList();
        foreach (const QVariant &timeFilter, timeFilters) {
            QVariantMap timeFilterMap = timeFilter.toMap();
            QDateTime startDate; QDateTime endDate;
            if (timeFilterMap.contains("startDate"))
                startDate = QDateTime::fromTime_t(timeFilterMap.value("startDate").toUInt());

            if (timeFilterMap.contains("endDate"))
                endDate = QDateTime::fromTime_t(timeFilterMap.value("endDate").toUInt());

            filter.addTimeFilter(startDate, endDate);
        }
    }

    if (logFilterMap.contains("loggingSources")) {
        QVariantList loggingSources = logFilterMap.value("loggingSources").toList();
        foreach (const QVariant &source, loggingSources) {
            filter.addLoggingSource(static_cast<Logging::LoggingSource>(s_loggingSource.indexOf(source.toString())));
        }
    }
    if (logFilterMap.contains("loggingLevels")) {
        QVariantList loggingLevels = logFilterMap.value("loggingLevels").toList();
        foreach (const QVariant &level, loggingLevels) {
            filter.addLoggingLevel(static_cast<Logging::LoggingLevel>(s_loggingLevel.indexOf(level.toString())));
        }
    }
    if (logFilterMap.contains("eventTypes")) {
        QVariantList eventTypes = logFilterMap.value("eventTypes").toList();
        foreach (const QVariant &eventType, eventTypes) {
            filter.addLoggingEventType(static_cast<Logging::LoggingEventType>(s_loggingEventType.indexOf(eventType.toString())));
        }
    }
    if (logFilterMap.contains("typeIds")) {
        QVariantList typeIds = logFilterMap.value("typeIds").toList();
        foreach (const QVariant &typeId, typeIds) {
            filter.addTypeId(typeId.toUuid());
        }
    }
    if (logFilterMap.contains("deviceIds")) {
        QVariantList deviceIds = logFilterMap.value("deviceIds").toList();
        foreach (const QVariant &deviceId, deviceIds) {
            filter.addDeviceId(DeviceId(deviceId.toString()));
        }
    }
    if (logFilterMap.contains("values")) {
        QVariantList values = logFilterMap.value("values").toList();
        foreach (const QVariant &value, values) {
            filter.addValue(value.toString());
        }
    }
    if (logFilterMap.contains("limit")) {
        filter.setLimit(logFilterMap.value("limit", -1).toInt());
    }
    if (logFilterMap.contains("offset")) {
        filter.setOffset(logFilterMap.value("offset").toInt());
    }

    return filter;
}

/*! Returns a \l{RepeatingOption} created from the given \a repeatingOptionMap. */
RepeatingOption JsonTypes::unpackRepeatingOption(const QVariantMap &repeatingOptionMap)
{
    RepeatingOption::RepeatingMode mode = static_cast<RepeatingOption::RepeatingMode>(s_repeatingMode.indexOf(repeatingOptionMap.value("mode").toString()));

    QList<int> weekDays;
    if (repeatingOptionMap.contains("weekDays")) {
        foreach (const QVariant weekDayVariant, repeatingOptionMap.value("weekDays").toList()) {
            weekDays.append(weekDayVariant.toInt());
        }
    }

    QList<int> monthDays;
    if (repeatingOptionMap.contains("monthDays")) {
        foreach (const QVariant monthDayVariant, repeatingOptionMap.value("monthDays").toList()) {
            monthDays.append(monthDayVariant.toInt());
        }
    }

    return RepeatingOption(mode, weekDays, monthDays);
}

/*! Returns a \l{CalendarItem} created from the given \a calendarItemMap. */
CalendarItem JsonTypes::unpackCalendarItem(const QVariantMap &calendarItemMap)
{
    CalendarItem calendarItem;
    calendarItem.setDuration(calendarItemMap.value("duration").toUInt());

    if (calendarItemMap.contains("datetime"))
        calendarItem.setDateTime(QDateTime::fromTime_t(calendarItemMap.value("datetime").toUInt()));

    if (calendarItemMap.contains("startTime"))
        calendarItem.setStartTime(QTime::fromString(calendarItemMap.value("startTime").toString(), "hh:mm"));

    if (calendarItemMap.contains("repeating"))
        calendarItem.setRepeatingOption(unpackRepeatingOption(calendarItemMap.value("repeating").toMap()));

    return calendarItem;
}

/*! Returns a \l{TimeEventItem} created from the given \a timeEventItemMap. */
TimeEventItem JsonTypes::unpackTimeEventItem(const QVariantMap &timeEventItemMap)
{
    TimeEventItem timeEventItem;

    if (timeEventItemMap.contains("datetime"))
        timeEventItem.setDateTime(timeEventItemMap.value("datetime").toUInt());

    if (timeEventItemMap.contains("time"))
        timeEventItem.setTime(timeEventItemMap.value("time").toTime());

    if (timeEventItemMap.contains("repeating"))
        timeEventItem.setRepeatingOption(unpackRepeatingOption(timeEventItemMap.value("repeating").toMap()));

    return timeEventItem;
}

/*! Returns a \l{TimeDescriptor} created from the given \a timeDescriptorMap. */
TimeDescriptor JsonTypes::unpackTimeDescriptor(const QVariantMap &timeDescriptorMap)
{
    TimeDescriptor timeDescriptor;

    if (timeDescriptorMap.contains("calendarItems")) {
        QList<CalendarItem> calendarItems;
        foreach (const QVariant &calendarItemValiant, timeDescriptorMap.value("calendarItems").toList()) {
            calendarItems.append(unpackCalendarItem(calendarItemValiant.toMap()));
        }
        timeDescriptor.setCalendarItems(calendarItems);
    }

    if (timeDescriptorMap.contains("timeEventItems")) {
        QList<TimeEventItem> timeEventItems;
        foreach (const QVariant &timeEventItemValiant, timeDescriptorMap.value("timeEventItems").toList()) {
            timeEventItems.append(unpackTimeEventItem(timeEventItemValiant.toMap()));
        }
        timeDescriptor.setTimeEventItems(timeEventItems);
    }

    return timeDescriptor;
}

/*! Returns a \l{Tag} created from the given \a tagMap. */
Tag JsonTypes::unpackTag(const QVariantMap &tagMap)
{
    DeviceId deviceId = DeviceId(tagMap.value("deviceId").toString());
    RuleId ruleId = RuleId(tagMap.value("ruleId").toString());
    QString appId = tagMap.value("appId").toString();
    QString tagId = tagMap.value("tagId").toString();
    QString value = tagMap.value("value").toString();
    if (!deviceId.isNull()) {
        return Tag(deviceId, appId, tagId, value);
    }
    return Tag(ruleId, appId, tagId, value);
}

ServerConfiguration JsonTypes::unpackServerConfiguration(const QVariantMap &serverConfigurationMap)
{
    ServerConfiguration serverConfiguration;
    serverConfiguration.id = serverConfigurationMap.value("id").toString();
    serverConfiguration.address = QHostAddress(serverConfigurationMap.value("address").toString());
    serverConfiguration.port = serverConfigurationMap.value("port").toUInt();
    serverConfiguration.sslEnabled = serverConfigurationMap.value("sslEnabled", true).toBool();
    serverConfiguration.authenticationEnabled = serverConfigurationMap.value("authenticationEnabled", true).toBool();
    return serverConfiguration;
}

WebServerConfiguration JsonTypes::unpackWebServerConfiguration(const QVariantMap &webServerConfigurationMap)
{
    ServerConfiguration tmp = unpackServerConfiguration(webServerConfigurationMap);
    WebServerConfiguration webServerConfiguration;
    webServerConfiguration.id = tmp.id;
    webServerConfiguration.address = tmp.address;
    webServerConfiguration.port = tmp.port;
    webServerConfiguration.sslEnabled = tmp.sslEnabled;
    webServerConfiguration.authenticationEnabled = tmp.authenticationEnabled;
    webServerConfiguration.publicFolder = webServerConfigurationMap.value("publicFolder").toString();
    return webServerConfiguration;
}

MqttPolicy JsonTypes::unpackMqttPolicy(const QVariantMap &mqttPolicyMap)
{
    MqttPolicy policy;
    policy.clientId = mqttPolicyMap.value("clientId").toString();
    policy.username = mqttPolicyMap.value("username").toString();
    policy.password = mqttPolicyMap.value("password").toString();
    policy.allowedPublishTopicFilters = mqttPolicyMap.value("allowedPublishTopicFilters").toStringList();
    policy.allowedSubscribeTopicFilters = mqttPolicyMap.value("allowedSubscribeTopicFilters").toStringList();
    return policy;
}

/*! Compairs the given \a map with the given \a templateMap. Returns the error string and false if
    the params are not valid. */
QPair<bool, QString> JsonTypes::validateMap(const QVariantMap &templateMap, const QVariantMap &map)
{
    s_lastError.clear();

    // Make sure all values defined in the template are around
    foreach (const QString &key, templateMap.keys()) {
        QString strippedKey = key;
        strippedKey.remove(QRegExp("^o:"));
        if (!key.startsWith("o:") && !map.contains(strippedKey)) {
            qCWarning(dcJsonRpc) << "*** missing key" << key;
            qCWarning(dcJsonRpc) << "Expected:      " << templateMap;
            qCWarning(dcJsonRpc) << "Got:           " << map;
            QJsonDocument jsonDoc = QJsonDocument::fromVariant(map);
            return report(false, QString("Missing key %1 in %2").arg(key).arg(QString(jsonDoc.toJson(QJsonDocument::Compact))));
        }
        if (map.contains(strippedKey)) {
            QPair<bool, QString> result = validateVariant(templateMap.value(key), map.value(strippedKey));
            if (!result.first) {
                QJsonDocument templateDoc = QJsonDocument::fromVariant(templateMap.value(key));
                QJsonDocument mapDoc = QJsonDocument::fromVariant(map.value(strippedKey));
                qCWarning(dcJsonRpc).nospace() << "Object\n" << qUtf8Printable(mapDoc.toJson(QJsonDocument::Indented)) << "not matching template\n" << qUtf8Printable(templateDoc.toJson(QJsonDocument::Indented));
                return result;
            }
        }
    }

    // Make sure there aren't any other parameters than the allowed ones
    foreach (const QString &key, map.keys()) {
        QString optKey = "o:" + key;

        if (!templateMap.contains(key) && !templateMap.contains(optKey)) {
            qCWarning(dcJsonRpc) << "Forbidden param" << key << "in params";
            QJsonDocument jsonDoc = QJsonDocument::fromVariant(map);
            return report(false, QString("Forbidden key \"%1\" in %2").arg(key).arg(QString(jsonDoc.toJson(QJsonDocument::Compact))));
        }
    }

    return report(true, "");
}

/*! Compairs the given \a value with the given \a templateValue. Returns the error string and false if
    the params are not valid. */
QPair<bool, QString> JsonTypes::validateProperty(const QVariant &templateValue, const QVariant &value)
{
    QString strippedTemplateValue = templateValue.toString();

    if (strippedTemplateValue == JsonTypes::basicTypeToString(JsonTypes::Variant)) {
        return report(true, "");
    }
    if (strippedTemplateValue == JsonTypes::basicTypeToString(QVariant::Uuid)) {
        QString errorString = QString("Param %1 is not a uuid.").arg(value.toString());
        return report(value.canConvert(QVariant::Uuid), errorString);
    }
    if (strippedTemplateValue == JsonTypes::basicTypeToString(QVariant::String)) {
        QString errorString = QString("Param %1 is not a string.").arg(value.toString());
        return report(value.canConvert(QVariant::String), errorString);
    }
    if (strippedTemplateValue == JsonTypes::basicTypeToString(QVariant::StringList)) {
        QString errorString = QString("Param %1 is not a string list.").arg(value.toString());
        return report(value.canConvert(QVariant::StringList), errorString);
    }
    if (strippedTemplateValue == JsonTypes::basicTypeToString(QVariant::Bool)) {
        QString errorString = QString("Param %1 is not a bool.").arg(value.toString());
        return report(value.canConvert(QVariant::Bool), errorString);
    }
    if (strippedTemplateValue == JsonTypes::basicTypeToString(QVariant::Int)) {
        QString errorString = QString("Param %1 is not a int.").arg(value.toString());
        return report(value.canConvert(QVariant::Int), errorString);
    }
    if (strippedTemplateValue == JsonTypes::basicTypeToString(QVariant::UInt)) {
        QString errorString = QString("Param %1 is not a uint.").arg(value.toString());
        return report(value.canConvert(QVariant::UInt), errorString);
    }
    if (strippedTemplateValue == JsonTypes::basicTypeToString(QVariant::Double)) {
        QString errorString = QString("Param %1 is not a double.").arg(value.toString());
        return report(value.canConvert(QVariant::Double), errorString);
    }
    if (strippedTemplateValue == JsonTypes::basicTypeToString(QVariant::Time)) {
        QString errorString = QString("Param %1 is not a time (hh:mm).").arg(value.toString());
        return report(value.canConvert(QVariant::Time), errorString);
    }

    qCWarning(dcJsonRpc) << QString("Unhandled property type: %1 (expected: %2)").arg(value.toString()).arg(strippedTemplateValue);
    QString errorString = QString("Unhandled property type: %1 (expected: %2)").arg(value.toString()).arg(strippedTemplateValue);
    return report(false, errorString);
}

/*! Compairs the given \a list with the given \a templateList. Returns the error string and false if
    the params are not valid. */
QPair<bool, QString> JsonTypes::validateList(const QVariantList &templateList, const QVariantList &list)
{
    Q_ASSERT(templateList.count() == 1);
    QVariant entryTemplate = templateList.first();

    for (int i = 0; i < list.count(); ++i) {
        QVariant listEntry = list.at(i);
        QPair<bool, QString> result = validateVariant(entryTemplate, listEntry);
        if (!result.first) {
            qCWarning(dcJsonRpc) << "List entry not matching template";
            return result;
        }
    }
    return report(true, "");
}

/*! Compairs the given \a variant with the given \a templateVariant. Returns the error string and false if
    the params are not valid. */
QPair<bool, QString> JsonTypes::validateVariant(const QVariant &templateVariant, const QVariant &variant)
{
    switch(templateVariant.type()) {
    case QVariant::String:
        if (templateVariant.toString().startsWith("$ref:")) {
            QString refName = templateVariant.toString();
            if (refName == actionRef()) {
                QPair<bool, QString> result = validateMap(actionDescription(), variant.toMap());
                if (!result.first) {
                    qCWarning(dcJsonRpc) << "Error validating action";
                    return result;
                }
            } else if (refName == eventRef()) {
                QPair<bool, QString> result = validateMap(eventDescription(), variant.toMap());
                if (!result.first) {
                    qCWarning(dcJsonRpc) << "Event not valid";
                    return result;
                }
            } else if (refName == paramRef()) {
                if (!variant.canConvert(QVariant::Map)) {
                    report(false, "Param not valid. Should be a map.");
                }
            } else if (refName == paramDescriptorRef()) {
                QPair<bool, QString> result = validateMap(paramDescriptorDescription(), variant.toMap());
                if (!result.first) {
                    qCWarning(dcJsonRpc) << "ParamDescriptor not valid";
                    return result;
                }
            } else if (refName == deviceRef()) {
                QPair<bool, QString> result = validateMap(deviceDescription(), variant.toMap());
                if (!result.first) {
                    qCWarning(dcJsonRpc) << "Device not valid";
                    return result;
                }
            } else if (refName == deviceDescriptorRef()) {
                QPair<bool, QString> result = validateMap(deviceDescriptorDescription(), variant.toMap());
                if (!result.first) {
                    qCWarning(dcJsonRpc) << "Devicedescriptor not valid";
                    return result;
                }
            } else if (refName == vendorRef()) {
                QPair<bool, QString> result = validateMap(vendorDescription(), variant.toMap());
                if (!result.first) {
                    qCWarning(dcJsonRpc) << "Value not allowed in" << vendorRef();
                }
            } else if (refName == deviceClassRef()) {
                QPair<bool, QString> result = validateMap(deviceClassDescription(), variant.toMap());
                if (!result.first) {
                    qCWarning(dcJsonRpc) << "Device class not valid";
                    return result;
                }
            } else if (refName == paramTypeRef()) {
                QPair<bool, QString> result = validateMap(paramTypeDescription(), variant.toMap());
                if (!result.first) {
                    qCWarning(dcJsonRpc) << "Param types not matching";
                    return result;
                }
            } else if (refName == ruleActionRef()) {
                QPair<bool, QString> result = validateMap(ruleActionDescription(), variant.toMap());
                if (!result.first) {
                    qCWarning(dcJsonRpc) << "RuleAction type not matching";
                    return result;
                }
            } else if (refName == ruleActionParamRef()) {
                QPair<bool, QString> result = validateMap(ruleActionParamDescription(), variant.toMap());
                if (!result.first) {
                    qCWarning(dcJsonRpc) << "RuleActionParam type not matching";
                    return result;
                }
            } else if (refName == actionTypeRef()) {
                QPair<bool, QString> result = validateMap(actionTypeDescription(), variant.toMap());
                if (!result.first) {
                    qCWarning(dcJsonRpc) << "Action type not matching";
                    return result;
                }
            } else if (refName == eventTypeRef()) {
                QPair<bool, QString> result = validateMap(eventTypeDescription(), variant.toMap());
                if (!result.first) {
                    qCWarning(dcJsonRpc) << "Event type not matching";
                    return result;
                }
            } else if (refName == stateTypeRef()) {
                QPair<bool, QString> result = validateMap(stateTypeDescription(), variant.toMap());
                if (!result.first) {
                    qCWarning(dcJsonRpc) << "State type not matching";
                    return result;
                }
            } else if (refName == stateEvaluatorRef()) {
                QPair<bool, QString> result = validateMap(stateEvaluatorDescription(), variant.toMap());
                if (!result.first) {
                    qCWarning(dcJsonRpc) << "StateEvaluator type not matching";
                    return result;
                }
            } else if (refName == stateDescriptorRef()) {
                QPair<bool, QString> result = validateMap(stateDescriptorDescription(), variant.toMap());
                if (!result.first) {
                    qCWarning(dcJsonRpc) << "StateDescriptor type not matching";
                    return result;
                }
            } else if (refName == pluginRef()) {
                QPair<bool, QString> result = validateMap(pluginDescription(), variant.toMap());
                if (!result.first) {
                    qCWarning(dcJsonRpc) << "Plugin not matching";
                    return result;
                }
            } else if (refName == ruleRef()) {
                QPair<bool, QString> result = validateMap(ruleDescription(), variant.toMap());
                if (!result.first) {
                    qCWarning(dcJsonRpc) << "Rule type not matching";
                    return result;
                }
            } else if (refName == ruleDescriptionRef()) {
                QPair<bool, QString> result = validateMap(s_ruleDescription, variant.toMap());
                if (!result.first) {
                    qCWarning(dcJsonRpc) << "RuleDescription type not matching";
                    return result;
                }
            } else if (refName == stateRef()) {
                QPair<bool, QString> result = validateMap(s_state, variant.toMap());
                if (!result.first) {
                    qCWarning(dcJsonRpc) << "State not matching";
                    return result;
                }
            } else if (refName == eventDescriptorRef()) {
                QPair<bool, QString> result = validateMap(eventDescriptorDescription(), variant.toMap());
                if (!result.first) {
                    qCWarning(dcJsonRpc) << "Eventdescriptor not matching";
                    return result;
                }
            } else if (refName == logEntryRef()) {
                QPair<bool, QString> result = validateMap(logEntryDescription(), variant.toMap());
                if (!result.first) {
                    qCWarning(dcJsonRpc) << "LogEntry not matching";
                    return result;
                }
            } else if (refName == timeDescriptorRef()) {
                QPair<bool, QString> result = validateMap(timeDescriptorDescription(), variant.toMap());
                if (!result.first) {
                    qCWarning(dcJsonRpc) << "TimeDescriptor not matching";
                    return result;
                }
            } else if (refName == calendarItemRef()) {
                QPair<bool, QString> result = validateMap(calendarItemDescription(), variant.toMap());
                if (!result.first) {
                    qCWarning(dcJsonRpc) << "CalendarItem not matching";
                    return result;
                }
            } else if (refName == timeDescriptorRef()) {
                QPair<bool, QString> result = validateMap(timeEventItemDescription(), variant.toMap());
                if (!result.first) {
                    qCWarning(dcJsonRpc) << "TimeEventItem not matching";
                    return result;
                }
            } else if (refName == repeatingOptionRef()) {
                QPair<bool, QString> result = validateMap(repeatingOptionDescription(), variant.toMap());
                if (!result.first) {
                    qCWarning(dcJsonRpc) << "RepeatingOption not matching";
                    return result;
                }
            } else if (refName == timeEventItemRef()) {
                QPair<bool, QString> result = validateMap(timeEventItemDescription(), variant.toMap());
                if (!result.first) {
                    qCWarning(dcJsonRpc) << "TimeEventItem not matching";
                    return result;
                }
            } else if (refName == wirelessAccessPointRef()) {
                QPair<bool, QString> result = validateMap(wirelessAccessPointDescription(), variant.toMap());
                if (!result.first) {
                    qCWarning(dcJsonRpc) << "WirelessAccessPoint not matching";
                    return result;
                }
            } else if (refName == wiredNetworkDeviceRef()) {
                QPair<bool, QString> result = validateMap(wiredNetworkDeviceDescription(), variant.toMap());
                if (!result.first) {
                    qCWarning(dcJsonRpc) << "WiredNetworkDevice not matching";
                    return result;
                }
            } else if (refName == wirelessNetworkDeviceRef()) {
                QPair<bool, QString> result = validateMap(wirelessNetworkDeviceDescription(), variant.toMap());
                if (!result.first) {
                    qCWarning(dcJsonRpc) << "WirelessNetworkDevice not matching";
                    return result;
                }
            } else if (refName == tokenInfoRef()) {
                QPair<bool, QString> result = validateMap(tokenInfoDescription(), variant.toMap());
                if (!result.first) {
                    qCWarning(dcJsonRpc) << "TokenInfo not matching";
                    return result;
                }
            } else if (refName == serverConfigurationRef()) {
                QPair<bool, QString> result = validateMap(serverConfigurationDescription(), variant.toMap());
                if (!result.first) {
                    qCWarning(dcJsonRpc) << "ServerConfiguration not matching";
                    return result;
                }
            } else if (refName == webServerConfigurationRef()) {
                QPair<bool, QString> result = validateMap(webServerConfigurationDescription(), variant.toMap());
                if (!result.first) {
                    qCWarning(dcJsonRpc) << "WebServerConfiguration not matching";
                    return result;
                }
            } else if (refName == mqttPolicyRef()) {
                QPair<bool, QString> result = validateMap(s_mqttPolicy, variant.toMap());
                if (!result.first) {
                    qCWarning(dcJsonRpc) << "MqttPolicy not matching";
                    return result;
                }
            } else if (refName == tagRef()) {
                QPair<bool, QString> result = validateMap(tagDescription(), variant.toMap());
                if (!result.first) {
                    qCWarning(dcJsonRpc) << "Tag not matching";
                    return result;
                }
            } else if (refName == packageRef()) {
                QPair<bool, QString> result = validateMap(packageDescription(), variant.toMap());
                if (!result.first) {
                    qCWarning(dcJsonRpc) << "Package not matching";
                    return result;
                }
            } else if (refName == repositoryRef()) {
                QPair<bool, QString> result = validateMap(repositoryDescription(), variant.toMap());
                if (!result.first) {
                    qCWarning(dcJsonRpc) << "Repository not matching";
                    return result;
                }
            } else if (refName == basicTypeRef()) {
                QPair<bool, QString> result = validateBasicType(variant);
                if (!result.first) {
                    qCWarning(dcJsonRpc) << QString("Value %1 not allowed in %2").arg(variant.toString()).arg(basicTypeRef());
                    return result;
                }
            } else if (refName == stateOperatorRef()) {
                QPair<bool, QString> result = validateEnum(s_stateOperator, variant);
                if (!result.first) {
                    qCWarning(dcJsonRpc) << QString("Value %1 not allowed in %2").arg(variant.toString()).arg(stateOperatorRef());
                    return result;
                }
            } else if (refName == createMethodRef()) {
                QPair<bool, QString> result = validateEnum(s_createMethod, variant);
                if (!result.first) {
                    qCWarning(dcJsonRpc) << QString("Value %1 not allowed in %2").arg(variant.toString()).arg(createMethodRef());
                    return result;
                }
            } else if (refName == setupMethodRef()) {
                QPair<bool, QString> result = validateEnum(s_setupMethod, variant);
                if (!result.first) {
                    qCWarning(dcJsonRpc) << QString("Value %1 not allowed in %2").arg(variant.toString()).arg(setupMethodRef());
                    return result;
                }
            } else if (refName == valueOperatorRef()) {
                QPair<bool, QString> result = validateEnum(s_valueOperator, variant);
                if (!result.first) {
                    qCWarning(dcJsonRpc) << QString("Value %1 not allowed in %2").arg(variant.toString()).arg(valueOperatorRef());
                    return result;
                }
            } else if (refName == deviceErrorRef()) {
                QPair<bool, QString> result = validateEnum(s_deviceError, variant);
                if (!result.first) {
                    qCWarning(dcJsonRpc) << QString("Value %1 not allowed in %2").arg(variant.toString()).arg(deviceErrorRef());
                    return result;
                }
            } else if (refName == ruleErrorRef()) {
                QPair<bool, QString> result = validateEnum(s_ruleError, variant);
                if (!result.first) {
                    qCWarning(dcJsonRpc) << QString("Value %1 not allowed in %2").arg(variant.toString()).arg(ruleErrorRef());
                    return result;
                }
            } else if (refName == loggingErrorRef()) {
                QPair<bool, QString> result = validateEnum(s_loggingError, variant);
                if (!result.first) {
                    qCWarning(dcJsonRpc) << QString("Value %1 not allowed in %2").arg(variant.toString()).arg(loggingErrorRef());
                    return result;
                }
            } else if (refName == loggingSourceRef()) {
                QPair<bool, QString> result = validateEnum(s_loggingSource, variant);
                if (!result.first) {
                    qCWarning(dcJsonRpc) << QString("Value %1 not allowed in %2").arg(variant.toString()).arg(loggingSourceRef());
                    return result;
                }
            } else if (refName == loggingLevelRef()) {
                QPair<bool, QString> result = validateEnum(s_loggingLevel, variant);
                if (!result.first) {
                    qCWarning(dcJsonRpc) << QString("Value %1 not allowed in %2").arg(variant.toString()).arg(loggingLevelRef());
                    return result;
                }
            } else if (refName == loggingEventTypeRef()) {
                QPair<bool, QString> result = validateEnum(s_loggingEventType, variant);
                if (!result.first) {
                    qCWarning(dcJsonRpc) << QString("Value %1 not allowed in %2").arg(variant.toString()).arg(loggingEventTypeRef());
                    return result;
                }
            } else if (refName == inputTypeRef()) {
                QPair<bool, QString> result = validateEnum(s_inputType, variant);
                if (!result.first) {
                    qCWarning(dcJsonRpc) << QString("Value %1 not allowed in %2").arg(variant.toString()).arg(inputTypeRef());
                    return result;
                }
            } else if (refName == unitRef()) {
                QPair<bool, QString> result = validateEnum(s_unit, variant);
                if (!result.first) {
                    qCWarning(dcJsonRpc) << QString("Value %1 not allowed in %2").arg(variant.toString()).arg(unitRef());
                    return result;
                }
            } else if (refName == repeatingModeRef()) {
                QPair<bool, QString> result = validateEnum(s_repeatingMode, variant);
                if (!result.first) {
                    qCWarning(dcJsonRpc) << QString("Value %1 not allowed in %2").arg(variant.toString()).arg(repeatingModeRef());
                    return result;
                }
            } else if (refName == removePolicyRef()) {
                QPair<bool, QString> result = validateEnum(s_removePolicy, variant);
                if (!result.first) {
                    qCWarning(dcJsonRpc) << QString("Value %1 not allowed in %2").arg(variant.toString()).arg(removePolicyRef());
                    return result;
                }
            } else if (refName == configurationErrorRef()) {
                QPair<bool, QString> result = validateEnum(s_configurationError, variant);
                if (!result.first) {
                    qCWarning(dcJsonRpc) << QString("Value %1 not allowed in %2").arg(variant.toString()).arg(configurationErrorRef());
                    return result;
                }
            } else if (refName == networkManagerStateRef()) {
                QPair<bool, QString> result = validateEnum(s_networkManagerState, variant);
                if (!result.first) {
                    qCWarning(dcJsonRpc) << QString("Value %1 not allowed in %2").arg(variant.toString()).arg(networkManagerStateRef());
                    return result;
                }
            } else if (refName == networkManagerErrorRef()) {
                QPair<bool, QString> result = validateEnum(s_networkManagerError, variant);
                if (!result.first) {
                    qCWarning(dcJsonRpc) << QString("Value %1 not allowed in %2").arg(variant.toString()).arg(networkManagerErrorRef());
                    return result;
                }
            } else if (refName == networkDeviceStateRef()) {
                QPair<bool, QString> result = validateEnum(s_networkDeviceState, variant);
                if (!result.first) {
                    qCWarning(dcJsonRpc) << QString("Value %1 not allowed in %2").arg(variant.toString()).arg(networkDeviceStateRef());
                    return result;
                }
            } else if (refName == userErrorRef()) {
                QPair<bool, QString> result = validateEnum(s_userError, variant);
                if (!result.first) {
                    qCWarning(dcJsonRpc) << QString("Value %1 not allowed in %2").arg(variant.toString()).arg(userErrorRef());
                    return result;
                }
            } else if (refName == tagErrorRef()) {
                QPair<bool, QString> result = validateEnum(s_tagError, variant);
                if (!result.first) {
                    qCWarning(dcJsonRpc()) << QString("Value %1 not allowed in %2").arg(variant.toString()).arg(tagErrorRef());
                    return result;
                }
            } else if (refName == cloudConnectionStateRef()) {
                QPair<bool, QString> result = validateEnum(s_cloudConnectionState, variant);
                if (!result.first) {
                    qCWarning(dcJsonRpc()) << QString("Value %1 not allowed in %2").arg(variant.toString()).arg(cloudConnectionStateRef());
                    return result;
                }
            } else {
                Q_ASSERT_X(false, "JsonTypes", QString("Unhandled ref: %1").arg(refName).toLatin1().data());
                return report(false, QString("Unhandled ref %1. Server implementation incomplete.").arg(refName));
            }
        } else {
            QPair<bool, QString> result = JsonTypes::validateProperty(templateVariant, variant);
            if (!result.first) {
                qCWarning(dcJsonRpc) << "property not matching:" << templateVariant << "!=" << variant;
                return result;
            }
        }
        break;
    case QVariant::Map: {
        QPair<bool, QString> result = validateMap(templateVariant.toMap(), variant.toMap());
        if (!result.first) {
            return result;
        }
        break;
    }
    case QVariant::List: {
        QPair<bool, QString> result = validateList(templateVariant.toList(), variant.toList());
        if (!result.first) {
            return result;
        }
        break;
    }
    default:
        qCWarning(dcJsonRpc) << "Unhandled value" << templateVariant;
        return report(false, QString("Unhandled value %1.").arg(templateVariant.toString()));
    }
    return report(true, "");
}

/*! Verify the given \a variant with the possible \l{BasicType}. Returns the error string and false if
    the params are not valid. */
QPair<bool, QString> JsonTypes::validateBasicType(const QVariant &variant)
{
    if (variant.canConvert(QVariant::Uuid) && QVariant(variant).convert(QVariant::Uuid)) {
        return report(true, "");
    }
    if (variant.canConvert(QVariant::String) && QVariant(variant).convert(QVariant::String)) {
        return report(true, "");
    }
    if (variant.canConvert(QVariant::StringList) && QVariant(variant).convert(QVariant::StringList)) {
        return report(true, "");
    }
    if (variant.canConvert(QVariant::Int) && QVariant(variant).convert(QVariant::Int)) {
        return report(true, "");
    }
    if (variant.canConvert(QVariant::UInt) && QVariant(variant).convert(QVariant::UInt)){
        return report(true, "");
    }
    if (variant.canConvert(QVariant::Double) && QVariant(variant).convert(QVariant::Double)) {
        return report(true, "");
    }
    if (variant.canConvert(QVariant::Bool && QVariant(variant).convert(QVariant::Bool))) {
        return report(true, "");
    }
    if (variant.canConvert(QVariant::Color) && QVariant(variant).convert(QVariant::Color)) {
        return report(true, "");
    }
    if (variant.canConvert(QVariant::Time) && QVariant(variant).convert(QVariant::Time)) {
        return report(true, "");
    }

    return report(false, QString("Error validating basic type %1.").arg(variant.toString()));
}

/*! Compairs the given \a value with the given \a enumDescription. Returns the error string and false if
    the enum does not contain the given \a value. */
QPair<bool, QString> JsonTypes::validateEnum(const QVariantList &enumDescription, const QVariant &value)
{
    QStringList enumStrings;
    foreach (const QVariant &variant, enumDescription)
        enumStrings.append(variant.toString());

    return report(enumDescription.contains(value.toString()), QString("Value %1 not allowed in %2").arg(value.toString()).arg(enumStrings.join(", ")));
}

}
