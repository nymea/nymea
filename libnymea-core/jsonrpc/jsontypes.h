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

#ifndef JSONTYPES_H
#define JSONTYPES_H

#include "plugin/devicedescriptor.h"
#include "rule.h"
#include "devicemanager.h"
#include "ruleengine.h"
#include "nymeaconfiguration.h"
#include "usermanager.h"

#include "types/deviceclass.h"
#include "types/event.h"
#include "types/action.h"
#include "types/actiontype.h"
#include "types/paramtype.h"
#include "types/paramdescriptor.h"
#include "types/ruleactionparam.h"

#include "logging/logging.h"
#include "logging/logentry.h"
#include "logging/logfilter.h"

#include "tagging/tagsstorage.h"
#include "tagging/tag.h"

#include "time/calendaritem.h"
#include "time/repeatingoption.h"
#include "time/timedescriptor.h"
#include "time/timeeventitem.h"

#include "networkmanager/networkmanager.h"
#include "networkmanager/networkdevice.h"
#include "networkmanager/wirednetworkdevice.h"
#include "networkmanager/wirelessnetworkdevice.h"
#include "networkmanager/wirelessaccesspoint.h"

#include "cloud/cloudmanager.h"
#include "platform/package.h"
#include "platform/repository.h"

#include <QObject>

#include <QVariantMap>
#include <QString>
#include <QMetaEnum>

class DevicePlugin;
class Device;

namespace nymeaserver {

#define DECLARE_OBJECT(typeName, jsonName) \
    public: \
    static QString typeName##Ref() { return QStringLiteral("$ref:") + QStringLiteral(jsonName); } \
    static QVariantMap typeName##Description() { \
        if (!s_initialized) { init(); } \
        return s_##typeName; \
    } \
    private: \
    static QVariantMap s_##typeName; \
    public:

#define DECLARE_TYPE(typeName, enumString, className, enumName) \
    public: \
    static QString typeName##Ref() { return QStringLiteral("$ref:") + QStringLiteral(enumString); } \
    static QVariantList typeName() { \
        if (!s_initialized) { init(); } \
        return s_##typeName; \
    } \
    static QString typeName##ToString(className::enumName value) { \
        const QMetaObject &metaObject = className::staticMetaObject; \
        int enumIndex = metaObject.indexOfEnumerator(enumString); \
        QMetaEnum metaEnum = metaObject.enumerator(enumIndex); \
        return metaEnum.valueToKey(metaEnum.value(value)); \
    } \
    private: \
    static QVariantList s_##typeName; \
    public:

class JsonTypes
{
    Q_GADGET
    Q_ENUMS(BasicType)

public:
    enum BasicType {
        Uuid,
        String,
        StringList,
        Int,
        Uint,
        Double,
        Bool,
        Variant,
        Color,
        Time,
        Object
    };

    static QVariantMap allTypes();

    DECLARE_TYPE(basicType, "BasicType", JsonTypes, BasicType)
    DECLARE_TYPE(stateOperator, "StateOperator", Types, StateOperator)
    DECLARE_TYPE(valueOperator, "ValueOperator", Types, ValueOperator)
    DECLARE_TYPE(inputType, "InputType", Types, InputType)
    DECLARE_TYPE(unit, "Unit", Types, Unit)
    DECLARE_TYPE(createMethod, "CreateMethod", DeviceClass, CreateMethod)
    DECLARE_TYPE(setupMethod, "SetupMethod", DeviceClass, SetupMethod)
    DECLARE_TYPE(deviceError, "DeviceError", DeviceManager, DeviceError)
    DECLARE_TYPE(removePolicy, "RemovePolicy", RuleEngine, RemovePolicy)
    DECLARE_TYPE(ruleError, "RuleError", RuleEngine, RuleError)
    DECLARE_TYPE(loggingError, "LoggingError", Logging, LoggingError)
    DECLARE_TYPE(loggingSource, "LoggingSource", Logging, LoggingSource)
    DECLARE_TYPE(loggingLevel, "LoggingLevel", Logging, LoggingLevel)
    DECLARE_TYPE(loggingEventType, "LoggingEventType", Logging, LoggingEventType)
    DECLARE_TYPE(repeatingMode, "RepeatingMode", RepeatingOption, RepeatingMode)
    DECLARE_TYPE(configurationError, "ConfigurationError", NymeaConfiguration, ConfigurationError)
    DECLARE_TYPE(networkManagerError, "NetworkManagerError", NetworkManager, NetworkManagerError)
    DECLARE_TYPE(networkManagerState, "NetworkManagerState", NetworkManager, NetworkManagerState)
    DECLARE_TYPE(networkDeviceState, "NetworkDeviceState", NetworkDevice, NetworkDeviceState)
    DECLARE_TYPE(userError, "UserError", UserManager, UserError)
    DECLARE_TYPE(tagError, "TagError", TagsStorage, TagError)
    DECLARE_TYPE(cloudConnectionState, "CloudConnectionState", CloudManager, CloudConnectionState)

    DECLARE_OBJECT(paramType, "ParamType")
    DECLARE_OBJECT(param, "Param")
    DECLARE_OBJECT(paramDescriptor, "ParamDescriptor")
    DECLARE_OBJECT(ruleAction, "RuleAction")
    DECLARE_OBJECT(ruleActionParam, "RuleActionParam")
    DECLARE_OBJECT(stateType, "StateType")
    DECLARE_OBJECT(stateDescriptor, "StateDescriptor")
    DECLARE_OBJECT(state, "State")
    DECLARE_OBJECT(stateEvaluator, "StateEvaluator")
    DECLARE_OBJECT(eventType, "EventType")
    DECLARE_OBJECT(event, "Event")
    DECLARE_OBJECT(eventDescriptor, "EventDescriptor")
    DECLARE_OBJECT(actionType, "ActionType")
    DECLARE_OBJECT(action, "Action")
    DECLARE_OBJECT(plugin, "Plugin")
    DECLARE_OBJECT(vendor, "Vendor")
    DECLARE_OBJECT(deviceClass, "DeviceClass")
    DECLARE_OBJECT(device, "Device")
    DECLARE_OBJECT(deviceDescriptor, "DeviceDescriptor")
    DECLARE_OBJECT(rule, "Rule")
    DECLARE_OBJECT(ruleDescription, "RuleDescription")
    DECLARE_OBJECT(logEntry, "LogEntry")
    DECLARE_OBJECT(timeDescriptor, "TimeDescriptor")
    DECLARE_OBJECT(calendarItem, "CalendarItem")
    DECLARE_OBJECT(timeEventItem, "TimeEventItem")
    DECLARE_OBJECT(repeatingOption, "RepeatingOption")
    DECLARE_OBJECT(wirelessAccessPoint, "WirelessAccessPoint")
    DECLARE_OBJECT(wiredNetworkDevice, "WiredNetworkDevice")
    DECLARE_OBJECT(wirelessNetworkDevice, "WirelessNetworkDevice")
    DECLARE_OBJECT(tokenInfo, "TokenInfo")
    DECLARE_OBJECT(serverConfiguration, "ServerConfiguration")
    DECLARE_OBJECT(webServerConfiguration, "WebServerConfiguration")
    DECLARE_OBJECT(tag, "Tag")
    DECLARE_OBJECT(mqttPolicy, "MqttPolicy")
    DECLARE_OBJECT(package, "Package")
    DECLARE_OBJECT(repository, "Repository")

    // pack types
    static QVariantMap packEventType(const EventType &eventType, const PluginId &pluginId, const QLocale &locale);
    static QVariantMap packEvent(const Event &event);
    static QVariantMap packEventDescriptor(const EventDescriptor &event);
    static QVariantMap packActionType(const ActionType &actionType, const PluginId &pluginId, const QLocale &locale);
    static QVariantMap packAction(const Action &action);
    static QVariantMap packRuleAction(const RuleAction &ruleAction);
    static QVariantMap packRuleActionParam(const RuleActionParam &ruleActionParam);
    static QVariantMap packState(const State &state);
    static QVariantMap packStateType(const StateType &stateType, const PluginId &pluginId, const QLocale &locale);
    static QVariantMap packStateDescriptor(const StateDescriptor &stateDescriptor);
    static QVariantMap packStateEvaluator(const StateEvaluator &stateEvaluator);
    static QVariantMap packParam(const Param &param);
    static QVariantMap packParamType(const ParamType &paramType, const PluginId &pluginId, const QLocale &locale);
    static QVariantMap packParamDescriptor(const ParamDescriptor &paramDescriptor);
    static QVariantMap packVendor(const Vendor &vendor, const QLocale &locale);
    static QVariantMap packDeviceClass(const DeviceClass &deviceClass, const QLocale &locale);
    static QVariantMap packPlugin(DevicePlugin *pluginid, const QLocale &locale);
    static QVariantMap packDevice(Device *device);
    static QVariantMap packDeviceDescriptor(const DeviceDescriptor &descriptor);
    static QVariantMap packRule(const Rule &rule);
    static QVariantMap packRuleDescription(const Rule &rule);
    static QVariantMap packLogEntry(const LogEntry &logEntry);
    static QVariantMap packTag(const Tag &tag);
    static QVariantMap packRepeatingOption(const RepeatingOption &option);
    static QVariantMap packCalendarItem(const CalendarItem &calendarItem);
    static QVariantMap packTimeEventItem(const TimeEventItem &timeEventItem);
    static QVariantMap packTimeDescriptor(const TimeDescriptor &timeDescriptor);
    static QVariantMap packWirelessAccessPoint(WirelessAccessPoint *wirelessAccessPoint);
    static QVariantMap packWiredNetworkDevice(WiredNetworkDevice *networkDevice);
    static QVariantMap packWirelessNetworkDevice(WirelessNetworkDevice *networkDevice);

    static QVariantList packRules(const QList<Rule> rules);
    static QVariantList packCreateMethods(DeviceClass::CreateMethods createMethods);
    static QVariantList packSupportedVendors(const QLocale &locale);
    static QVariantList packSupportedDevices(const VendorId &vendorId, const QLocale &locale);
    static QVariantList packConfiguredDevices();
    static QVariantList packDeviceStates(Device *device);
    static QVariantList packDeviceDescriptors(const QList<DeviceDescriptor> deviceDescriptors);

    static QVariantMap packBasicConfiguration();
    static QVariantMap packServerConfiguration(const ServerConfiguration &config);
    static QVariantMap packWebServerConfiguration(const WebServerConfiguration &config);
    static QVariantMap packMqttPolicy(const MqttPolicy &policy);

    static QVariantList packRuleDescriptions();
    static QVariantList packRuleDescriptions(const QList<Rule> &rules);

    static QVariantList packActionTypes(const DeviceClass &deviceClass, const QLocale &locale);
    static QVariantList packStateTypes(const DeviceClass &deviceClass, const QLocale &locale);
    static QVariantList packEventTypes(const DeviceClass &deviceClass, const QLocale &locale);
    static QVariantList packPlugins(const QLocale &locale);

    static QVariantMap packTokenInfo(const TokenInfo &tokenInfo);

    static QVariantMap packPackage(const Package &package);
    static QVariantMap packRepository(const Repository &repository);

    static QString basicTypeToString(const QVariant::Type &type);

    // unpack Types
    static Param unpackParam(const QVariantMap &paramMap);
    static ParamList unpackParams(const QVariantList &paramList);
    static Rule unpackRule(const QVariantMap &ruleMap);
    static RuleAction unpackRuleAction(const QVariantMap &ruleActionMap);
    static RuleActionParam unpackRuleActionParam(const QVariantMap &ruleActionParamMap);
    static RuleActionParamList unpackRuleActionParams(const QVariantList &ruleActionParamList);
    static ParamDescriptor unpackParamDescriptor(const QVariantMap &paramDescriptorMap);
    static QList<ParamDescriptor> unpackParamDescriptors(const QVariantList &paramDescriptorList);
    static EventDescriptor unpackEventDescriptor(const QVariantMap &eventDescriptorMap);
    static StateEvaluator unpackStateEvaluator(const QVariantMap &stateEvaluatorMap);
    static StateDescriptor unpackStateDescriptor(const QVariantMap &stateDescriptorMap);
    static LogFilter unpackLogFilter(const QVariantMap &logFilterMap);
    static RepeatingOption unpackRepeatingOption(const QVariantMap &repeatingOptionMap);
    static CalendarItem unpackCalendarItem(const QVariantMap &calendarItemMap);
    static TimeEventItem unpackTimeEventItem(const QVariantMap &timeEventItemMap);
    static TimeDescriptor unpackTimeDescriptor(const QVariantMap &timeDescriptorMap);
    static Tag unpackTag(const QVariantMap &tagMap);

    static ServerConfiguration unpackServerConfiguration(const QVariantMap &serverConfigurationMap);
    static WebServerConfiguration unpackWebServerConfiguration(const QVariantMap &webServerConfigurationMap);
    static MqttPolicy unpackMqttPolicy(const QVariantMap &mqttPolicyMap);

    // validate
    static QPair<bool, QString> validateMap(const QVariantMap &templateMap, const QVariantMap &map);
    static QPair<bool, QString> validateProperty(const QVariant &templateValue, const QVariant &value);
    static QPair<bool, QString> validateList(const QVariantList &templateList, const QVariantList &list);
    static QPair<bool, QString> validateVariant(const QVariant &templateVariant, const QVariant &variant);
    static QPair<bool, QString> validateEnum(const QVariantList &enumList, const QVariant &value);
    static QPair<bool, QString> validateBasicType(const QVariant &variant);

private:
    static bool s_initialized;
    static void init();

    static QPair<bool, QString> report(bool status, const QString &message);
    static QVariantList enumToStrings(const QMetaObject &metaObject, const QString &enumName);

    static QString s_lastError;
};

}

#endif // JSONTYPES_H
