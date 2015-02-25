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

#include "jsontypes.h"

#include "plugin/device.h"
#include "devicemanager.h"
#include "ruleengine.h"

#include <QStringList>
#include <QJsonDocument>
#include <QDebug>
#include <QMetaEnum>

bool JsonTypes::s_initialized = false;
QString JsonTypes::s_lastError;

QVariantList JsonTypes::s_basicType;
QVariantList JsonTypes::s_stateOperator;
QVariantList JsonTypes::s_valueOperator;
QVariantList JsonTypes::s_createMethod;
QVariantList JsonTypes::s_setupMethod;
QVariantList JsonTypes::s_removePolicy;
QVariantList JsonTypes::s_deviceError;
QVariantList JsonTypes::s_ruleError;
QVariantList JsonTypes::s_loggingError;
QVariantList JsonTypes::s_loggingSource;
QVariantList JsonTypes::s_loggingLevel;
QVariantList JsonTypes::s_loggingEventType;

QVariantMap JsonTypes::s_paramType;
QVariantMap JsonTypes::s_param;
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
QVariantMap JsonTypes::s_logEntry;

void JsonTypes::init()
{
    // BasicTypes
    s_basicType = enumToStrings(JsonTypes::staticMetaObject, "BasicType");
    s_stateOperator = enumToStrings(Types::staticMetaObject, "StateOperator");
    s_valueOperator = enumToStrings(Types::staticMetaObject, "ValueOperator");
    s_createMethod = enumToStrings(DeviceClass::staticMetaObject, "CreateMethod");
    s_setupMethod = enumToStrings(DeviceClass::staticMetaObject, "SetupMethod");
    s_removePolicy = enumToStrings(RuleEngine::staticMetaObject, "RemovePolicy");
    s_deviceError = enumToStrings(DeviceManager::staticMetaObject, "DeviceError");
    s_ruleError = enumToStrings(RuleEngine::staticMetaObject, "RuleError");
    s_loggingError = enumToStrings(Logging::staticMetaObject, "LoggingError");
    s_loggingSource = enumToStrings(Logging::staticMetaObject, "LoggingSource");
    s_loggingLevel = enumToStrings(Logging::staticMetaObject, "LoggingLevel");
    s_loggingEventType = enumToStrings(Logging::staticMetaObject, "LoggingEventType");

    // ParamType
    s_paramType.insert("name", basicTypeToString(String));
    s_paramType.insert("type", basicTypeRef());
    s_paramType.insert("o:defaultValue", basicTypeToString(Variant));
    s_paramType.insert("o:minValue", basicTypeToString(Variant));
    s_paramType.insert("o:maxValue", basicTypeToString(Variant));
    s_paramType.insert("o:allowedValues", QVariantList() << basicTypeToString(Variant));

    // Param
    s_param.insert("name", basicTypeToString(String));
    s_param.insert("value", basicTypeRef());

    // ParamDescriptor
    s_paramDescriptor.insert("name", basicTypeToString(String));
    s_paramDescriptor.insert("value", basicTypeRef());
    s_paramDescriptor.insert("operator", valueOperatorRef());

    // StateType
    s_stateType.insert("id", basicTypeToString(Uuid));
    s_stateType.insert("name", basicTypeToString(String));
    s_stateType.insert("type", basicTypeRef());
    s_stateType.insert("defaultValue", basicTypeToString(Variant));

    // State
    s_state.insert("stateTypeId", basicTypeToString(Uuid));
    s_state.insert("deviceId", basicTypeToString(Uuid));
    s_state.insert("value", basicTypeToString(Variant));

    // StateDescriptor
    s_stateDescriptor.insert("stateTypeId", basicTypeToString(Uuid));
    s_stateDescriptor.insert("deviceId", basicTypeToString(Uuid));
    s_stateDescriptor.insert("value", basicTypeToString(Variant));
    s_stateDescriptor.insert("operator", valueOperatorRef());

    // StateEvaluator
    s_stateEvaluator.insert("o:stateDescriptor", stateDescriptorRef());
    s_stateEvaluator.insert("o:childEvaluators", QVariantList() << stateEvaluatorRef());
    s_stateEvaluator.insert("o:operator", stateOperatorRef());

    // EventType
    s_eventType.insert("id", basicTypeToString(Uuid));
    s_eventType.insert("name", basicTypeToString(String));
    s_eventType.insert("paramTypes", QVariantList() << paramTypeRef());

    // Event
    s_event.insert("eventTypeId", basicTypeToString(Uuid));
    s_event.insert("deviceId", basicTypeToString(Uuid));
    s_event.insert("o:params", QVariantList() << paramRef());

    // EventDescriptor
    s_eventDescriptor.insert("eventTypeId", basicTypeToString(Uuid));
    s_eventDescriptor.insert("deviceId", basicTypeToString(Uuid));
    s_eventDescriptor.insert("o:paramDescriptors", QVariantList() << paramDescriptorRef());

    // ActionType
    s_actionType.insert("id", basicTypeToString(Uuid));
    s_actionType.insert("name", basicTypeToString(Uuid));
    s_actionType.insert("paramTypes", QVariantList() << paramTypeRef());

    // Action
    s_action.insert("actionTypeId", basicTypeToString(Uuid));
    s_action.insert("deviceId", basicTypeToString(Uuid));
    s_action.insert("o:params", QVariantList() << paramRef());

    // Pugin
    s_plugin.insert("id", basicTypeToString(Uuid));
    s_plugin.insert("name", basicTypeToString(String));
    s_plugin.insert("params", QVariantList() << paramRef());

    // Vendor
    s_vendor.insert("id", basicTypeToString(Uuid));
    s_vendor.insert("name", basicTypeToString(String));

    // DeviceClass
    s_deviceClass.insert("id", basicTypeToString(Uuid));
    s_deviceClass.insert("vendorId", basicTypeToString(Uuid));
    s_deviceClass.insert("name", basicTypeToString(String));
    s_deviceClass.insert("stateTypes", QVariantList() << stateTypeRef());
    s_deviceClass.insert("eventTypes", QVariantList() << eventTypeRef());
    s_deviceClass.insert("actionTypes", QVariantList() << actionTypeRef());
    s_deviceClass.insert("paramTypes", QVariantList() << paramTypeRef());
    s_deviceClass.insert("discoveryParamTypes", QVariantList() << paramTypeRef());
    s_deviceClass.insert("setupMethod", setupMethodRef());
    s_deviceClass.insert("createMethods", QVariantList() << createMethodRef());

    // Device
    s_device.insert("id", basicTypeToString(Uuid));
    s_device.insert("deviceClassId", basicTypeToString(Uuid));
    s_device.insert("name", basicTypeToString(String));
    s_device.insert("params", QVariantList() << paramRef());
    s_device.insert("setupComplete", basicTypeToString(Bool));

    // DeviceDescription
    s_deviceDescriptor.insert("id", basicTypeToString(Uuid));
    s_deviceDescriptor.insert("title", basicTypeToString(String));
    s_deviceDescriptor.insert("description", basicTypeToString(String));

    // Rule
    s_rule.insert("id", basicTypeToString(Uuid));
    s_rule.insert("enabled", basicTypeToString(Bool));
    s_rule.insert("eventDescriptors", QVariantList() << eventDescriptorRef());
    s_rule.insert("actions", QVariantList() << actionRef());
    s_rule.insert("stateEvaluator", stateEvaluatorRef());

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
    for (int i = 0; i < metaEnum.keyCount(); i++) {
        enumStrings << metaEnum.valueToKey(metaEnum.value(i));
    }
    return enumStrings;
}

QVariantMap JsonTypes::allTypes()
{
    QVariantMap allTypes;
    allTypes.insert("BasicType", basicType());
    allTypes.insert("ParamType", paramTypeDescription());
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
    allTypes.insert("ParamDescriptor", paramDescriptorDescription());
    allTypes.insert("State", stateDescription());
    allTypes.insert("Device", deviceDescription());
    allTypes.insert("DeviceDescriptor", deviceDescriptorDescription());
    allTypes.insert("Action", actionDescription());
    allTypes.insert("Rule", ruleDescription());
    allTypes.insert("LogEntry", logEntryDescription());
    return allTypes;
}

QVariantMap JsonTypes::packEventType(const EventType &eventType)
{
    QVariantMap variant;
    variant.insert("id", eventType.id());
    variant.insert("name", eventType.name());
    QVariantList paramTypes;
    foreach (const ParamType &paramType, eventType.paramTypes()) {
        paramTypes.append(packParamType(paramType));
    }
    variant.insert("paramTypes", paramTypes);
    return variant;
}

QVariantMap JsonTypes::packEvent(const Event &event)
{
    QVariantMap variant;
    variant.insert("eventTypeId", event.eventTypeId());
    variant.insert("deviceId", event.deviceId());
    QVariantList params;
    foreach (const Param &param, event.params()) {
        params.append(packParam(param));
    }
    variant.insert("params", params);
    return variant;
}

QVariantMap JsonTypes::packEventDescriptor(const EventDescriptor &eventDescriptor)
{
    QVariantMap variant;
    variant.insert("eventTypeId", eventDescriptor.eventTypeId());
    variant.insert("deviceId", eventDescriptor.deviceId());
    QVariantList params;
    foreach (const ParamDescriptor &paramDescriptor, eventDescriptor.paramDescriptors()) {
        params.append(packParamDescriptor(paramDescriptor));
    }
    variant.insert("paramDescriptors", params);
    return variant;
}

QVariantMap JsonTypes::packActionType(const ActionType &actionType)
{
    QVariantMap variantMap;
    variantMap.insert("id", actionType.id());
    variantMap.insert("name", actionType.name());
    QVariantList paramTypes;
    foreach (const ParamType &paramType, actionType.paramTypes()) {
        paramTypes.append(packParamType(paramType));
    }
    variantMap.insert("paramTypes", paramTypes);
    return variantMap;
}

QVariantMap JsonTypes::packAction(const Action &action)
{
    QVariantMap variant;
    variant.insert("actionTypeId", action.actionTypeId());
    variant.insert("deviceId", action.deviceId());
    QVariantList params;
    foreach (const Param &param, action.params()) {
        params.append(packParam(param));
    }
    variant.insert("params", params);
    return variant;
}

QVariantMap JsonTypes::packStateType(const StateType &stateType)
{
    QVariantMap variantMap;
    variantMap.insert("id", stateType.id());
    variantMap.insert("name", stateType.name());
    variantMap.insert("type", QVariant::typeToName(stateType.type()));
    variantMap.insert("defaultValue", stateType.defaultValue());
    return variantMap;
}

QVariantMap JsonTypes::packStateDescriptor(const StateDescriptor &stateDescriptor)
{
    QVariantMap variantMap;
    variantMap.insert("stateTypeId", stateDescriptor.stateTypeId().toString());
    variantMap.insert("deviceId", stateDescriptor.deviceId().toString());
    variantMap.insert("value", stateDescriptor.stateValue());
    variantMap.insert("operator", valueOperator().at(stateDescriptor.operatorType()));
    return variantMap;
}

QVariantMap JsonTypes::packStateEvaluator(const StateEvaluator &stateEvaluator)
{
    QVariantMap variantMap;
    if (stateEvaluator.stateDescriptor().isValid()) {
        variantMap.insert("stateDescriptor", packStateDescriptor(stateEvaluator.stateDescriptor()));
    }
    QVariantList childEvaluators;
    foreach (const StateEvaluator &childEvaluator, stateEvaluator.childEvaluators()) {
        childEvaluators.append(packStateEvaluator(childEvaluator));
    }
    qDebug() << "state operator:" << stateOperator() << stateEvaluator.operatorType();
    variantMap.insert("operator", stateOperator().at(stateEvaluator.operatorType()));
    if (childEvaluators.count() > 0) {
        variantMap.insert("childEvaluators", childEvaluators);
    }

    return variantMap;
}

QVariantMap JsonTypes::packParam(const Param &param)
{
    QVariantMap variantMap;
    variantMap.insert("name", param.name());
    variantMap.insert("value", param.value());
    return variantMap;
}

QVariantMap JsonTypes::packParamDescriptor(const ParamDescriptor &paramDescriptor)
{
    QVariantMap variantMap;
    variantMap.insert("name", paramDescriptor.name());
    variantMap.insert("value", paramDescriptor.value());
    variantMap.insert("operator", s_valueOperator.at(paramDescriptor.operatorType()));
    return variantMap;
}

QVariantMap JsonTypes::packParamType(const ParamType &paramType)
{
    QVariantMap variantMap;
    variantMap.insert("name", paramType.name());
    variantMap.insert("type", QVariant::typeToName(paramType.type()));
    if (paramType.defaultValue().isValid()) {
        variantMap.insert("defaultValue", paramType.defaultValue());
    }
    if (paramType.minValue().isValid()) {
        variantMap.insert("minValue", paramType.minValue());
    }
    if (paramType.maxValue().isValid()) {
        variantMap.insert("maxValue", paramType.maxValue());
    }
    if (!paramType.allowedValues().isEmpty()) {
        variantMap.insert("allowedValues", paramType.allowedValues());
    }
    return variantMap;
}

QVariantMap JsonTypes::packVendor(const Vendor &vendor)
{
    QVariantMap variantMap;
    variantMap.insert("id", vendor.id());
    variantMap.insert("name", vendor.name());
    return variantMap;
}

QVariantMap JsonTypes::packDeviceClass(const DeviceClass &deviceClass)
{
    QVariantMap variant;
    variant.insert("name", deviceClass.name());
    variant.insert("id", deviceClass.id());
    variant.insert("vendorId", deviceClass.vendorId());
    QVariantList stateTypes;
    foreach (const StateType &stateType, deviceClass.stateTypes()) {
        stateTypes.append(packStateType(stateType));
    }
    QVariantList eventTypes;
    foreach (const EventType &eventType, deviceClass.eventTypes()) {
        eventTypes.append(packEventType(eventType));
    }
    QVariantList actionTypes;
    foreach (const ActionType &actionType, deviceClass.actionTypes()) {
        actionTypes.append(packActionType(actionType));
    }
    QVariantList paramTypes;
    foreach (const ParamType &paramType, deviceClass.paramTypes()) {
        paramTypes.append(packParamType(paramType));
    }
    QVariantList discoveryParamTypes;
    foreach (const ParamType &paramType, deviceClass.discoveryParamTypes()) {
        qDebug() << "packing discoverparam" << packParamType(paramType);
        discoveryParamTypes.append(packParamType(paramType));
    }

    variant.insert("paramTypes", paramTypes);
    variant.insert("discoveryParamTypes", discoveryParamTypes);
    variant.insert("stateTypes", stateTypes);
    variant.insert("eventTypes", eventTypes);
    variant.insert("actionTypes", actionTypes);
    variant.insert("createMethods", packCreateMethods(deviceClass.createMethods()));
    variant.insert("setupMethod", s_setupMethod.at(deviceClass.setupMethod()));
    return variant;
}

QVariantMap JsonTypes::packPlugin(DevicePlugin *plugin)
{
    Q_UNUSED(plugin)
    qWarning() << "packPlugin not implemented yet!";
    return QVariantMap();
}

QVariantMap JsonTypes::packDevice(Device *device)
{
    QVariantMap variant;
    variant.insert("id", device->id());
    variant.insert("deviceClassId", device->deviceClassId());
    variant.insert("name", device->name());
    QVariantList params;
    foreach (const Param &param, device->params()) {
        params.append(packParam(param));
    }
    variant.insert("params", params);
    variant.insert("setupComplete", device->setupComplete());
    return variant;
}

QVariantMap JsonTypes::packDeviceDescriptor(const DeviceDescriptor &descriptor)
{
    QVariantMap variant;
    variant.insert("id", descriptor.id());
    variant.insert("title", descriptor.title());
    variant.insert("description", descriptor.description());
    return variant;
}

QVariantMap JsonTypes::packRule(const Rule &rule)
{
    QVariantMap ruleMap;
    ruleMap.insert("id", rule.id());
    ruleMap.insert("enabled", rule.enabled());
    QVariantList eventDescriptorList;
    foreach (const EventDescriptor &eventDescriptor, rule.eventDescriptors()) {
        eventDescriptorList.append(JsonTypes::packEventDescriptor(eventDescriptor));
    }
    ruleMap.insert("eventDescriptors", eventDescriptorList);

    QVariantList actionList;
    foreach (const Action &action, rule.actions()) {
        actionList.append(JsonTypes::packAction(action));
    }
    ruleMap.insert("actions", actionList);
    ruleMap.insert("stateEvaluator", JsonTypes::packStateEvaluator(rule.stateEvaluator()));
    return ruleMap;
}

QVariantMap JsonTypes::packLogEntry(const LogEntry &logEntry)
{
    QVariantMap logEntryMap;
    logEntryMap.insert("timestamp", logEntry.timestamp().toMSecsSinceEpoch());
    logEntryMap.insert("loggingLevel", s_loggingLevel.at(logEntry.level()));
    logEntryMap.insert("source", s_loggingSource.at(logEntry.source()));
    logEntryMap.insert("eventType", s_loggingEventType.at(logEntry.eventType()));

    if (logEntry.eventType() == Logging::LoggingEventTypeActiveChange) {
        logEntryMap.insert("active", logEntry.active());
    }

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
        logEntryMap.insert("value", logEntry.value());
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

QVariantList JsonTypes::packCreateMethods(DeviceClass::CreateMethods createMethods)
{
    QVariantList ret;
    if (createMethods.testFlag(DeviceClass::CreateMethodUser)) {
        ret << "CreateMethodUser";
    }
    if (createMethods.testFlag(DeviceClass::CreateMethodAuto)) {
        ret << "CreateMethodAuto";
    }
    if (createMethods.testFlag(DeviceClass::CreateMethodDiscovery)) {
        ret << "CreateMethodDiscovery";
    }
    return ret;
}

Param JsonTypes::unpackParam(const QVariantMap &paramMap)
{
    if (paramMap.keys().count() == 0) {
        return Param();
    }
    QString name = paramMap.value("name").toString();
    QVariant value = paramMap.value("value");
    return Param(name, value);
}

ParamList JsonTypes::unpackParams(const QVariantList &paramList)
{
    ParamList params;
    foreach (const QVariant &paramVariant, paramList) {
//        qDebug() << "unpacking param" << paramVariant;
        params.append(unpackParam(paramVariant.toMap()));
    }
    return params;
}

ParamDescriptor JsonTypes::unpackParamDescriptor(const QVariantMap &paramMap)
{
    ParamDescriptor param(paramMap.value("name").toString(), paramMap.value("value"));
    QString operatorString = paramMap.value("operator").toString();

    QMetaObject metaObject = Types::staticMetaObject;
    int enumIndex = metaObject.indexOfEnumerator("ValueOperator");
    QMetaEnum metaEnum = metaObject.enumerator(enumIndex);
    param.setOperatorType((Types::ValueOperator)metaEnum.keyToValue(operatorString.toLatin1().data()));
    return param;
}

QList<ParamDescriptor> JsonTypes::unpackParamDescriptors(const QVariantList &paramList)
{
    QList<ParamDescriptor> params;
    foreach (const QVariant &paramVariant, paramList) {
        params.append(unpackParamDescriptor(paramVariant.toMap()));
    }
    return params;
}

EventDescriptor JsonTypes::unpackEventDescriptor(const QVariantMap &eventDescriptorMap)
{
    EventTypeId eventTypeId(eventDescriptorMap.value("eventTypeId").toString());
    DeviceId eventDeviceId(eventDescriptorMap.value("deviceId").toString());
    QList<ParamDescriptor> eventParams = JsonTypes::unpackParamDescriptors(eventDescriptorMap.value("paramDescriptors").toList());
    EventDescriptor eventDescriptor(eventTypeId, eventDeviceId, eventParams);
    return eventDescriptor;
}

StateEvaluator JsonTypes::unpackStateEvaluator(const QVariantMap &stateEvaluatorMap)
{
    StateEvaluator ret(unpackStateDescriptor(stateEvaluatorMap.value("stateDescriptor").toMap()));
    if (stateEvaluatorMap.contains("operator")) {
        ret.setOperatorType((Types::StateOperator)s_stateOperator.indexOf(stateEvaluatorMap.value("operator").toString()));
    }
    QList<StateEvaluator> childEvaluators;
    foreach (const QVariant &childEvaluator, stateEvaluatorMap.value("childEvaluators").toList()) {
        childEvaluators.append(unpackStateEvaluator(childEvaluator.toMap()));
    }
    ret.setChildEvaluators(childEvaluators);
    return ret;
}

StateDescriptor JsonTypes::unpackStateDescriptor(const QVariantMap &stateDescriptorMap)
{
    StateTypeId stateTypeId(stateDescriptorMap.value("stateTypeId").toString());
    DeviceId deviceId(stateDescriptorMap.value("deviceId").toString());
    QVariant value = stateDescriptorMap.value("value");
    Types::ValueOperator operatorType = (Types::ValueOperator)s_valueOperator.indexOf(stateDescriptorMap.value("operator").toString());
    StateDescriptor stateDescriptor(stateTypeId, deviceId, value, operatorType);
    return stateDescriptor;
}

QPair<bool, QString> JsonTypes::validateMap(const QVariantMap &templateMap, const QVariantMap &map)
{
    s_lastError.clear();
    // qDebug() << "validating Map" << templateMap << map;

    // Make sure all values defined in the template are around
    foreach (const QString &key, templateMap.keys()) {
        QString strippedKey = key;
        strippedKey.remove(QRegExp("^o:"));
        if (!key.startsWith("o:") && !map.contains(strippedKey)) {
            qDebug() << "*** missing key" << key;
            qDebug() << "Expected:      " << templateMap;
            qDebug() << "Got:           " << map;
            QJsonDocument jsonDoc = QJsonDocument::fromVariant(map);
            return report(false, QString("Missing key %1 in %2").arg(key).arg(QString(jsonDoc.toJson())));
        }
        if (map.contains(strippedKey)) {
            QPair<bool, QString> result = validateVariant(templateMap.value(key), map.value(strippedKey));
            if (!result.first) {
                qDebug() << "Object not matching template" << templateMap.value(key) << map.value(strippedKey);
                return result;
            }
        }
    }

    // Make sure there aren't any other parameters than the allowed ones
    foreach (const QString &key, map.keys()) {
        QString optKey = "o:" + key;

        if (!templateMap.contains(key) && !templateMap.contains(optKey)) {
            qDebug() << "Forbidden param" << key << "in params";
            QJsonDocument jsonDoc = QJsonDocument::fromVariant(map);
            return report(false, QString("Forbidden key \"%1\" in %2").arg(key).arg(QString(jsonDoc.toJson())));
        }
    }

    return report(true, "");
}

QPair<bool, QString> JsonTypes::validateProperty(const QVariant &templateValue, const QVariant &value)
{
    //qDebug() << "validating property. template:" << templateValue << "got:" << value;
    QString strippedTemplateValue = templateValue.toString();

    if (strippedTemplateValue == JsonTypes::basicTypeToString(JsonTypes::Variant)) {
        return report(true, "");
    }
    if (strippedTemplateValue == JsonTypes::basicTypeToString(JsonTypes::Uuid)) {
        QString errorString = QString("Param %1 is not a uuid.").arg(value.toString());
        return report(value.canConvert(QVariant::Uuid), errorString);
    }
    if (strippedTemplateValue == JsonTypes::basicTypeToString(JsonTypes::String)) {
        QString errorString = QString("Param %1 is not a string.").arg(value.toString());
        return report(value.canConvert(QVariant::String), errorString);
    }
    if (strippedTemplateValue == JsonTypes::basicTypeToString(JsonTypes::Bool)) {
        QString errorString = QString("Param %1 is not a bool.").arg(value.toString());
        return report(value.canConvert(QVariant::Bool), errorString);
    }
    if (strippedTemplateValue == JsonTypes::basicTypeToString(JsonTypes::Int)) {
        QString errorString = QString("Param %1 is not a int.").arg(value.toString());
        return report(value.canConvert(QVariant::Int), errorString);
    }
    qWarning() << QString("Unhandled property type: %1 (expected: %2)").arg(value.toString()).arg(strippedTemplateValue);
    QString errorString = QString("Unhandled property type: %1 (expected: %2)").arg(value.toString()).arg(strippedTemplateValue);
    return report(false, errorString);
}

QPair<bool, QString> JsonTypes::validateList(const QVariantList &templateList, const QVariantList &list)
{
    Q_ASSERT(templateList.count() == 1);
    QVariant entryTemplate = templateList.first();

    for (int i = 0; i < list.count(); ++i) {
        QVariant listEntry = list.at(i);
//        qDebug() << "validating" << list << templateList;
        QPair<bool, QString> result = validateVariant(entryTemplate, listEntry);
        if (!result.first) {
            qDebug() << "List entry not matching template";
            return result;
        }
    }
    return report(true, "");
}

QPair<bool, QString> JsonTypes::validateVariant(const QVariant &templateVariant, const QVariant &variant)
{
    switch(templateVariant.type()) {
    case QVariant::String:
        if (templateVariant.toString().startsWith("$ref:")) {
            QString refName = templateVariant.toString();
            if (refName == actionRef()) {
//                qDebug() << "validating action";
                QPair<bool, QString> result = validateMap(actionDescription(), variant.toMap());
                if (!result.first) {
                    qDebug() << "Error validating action";
                    return result;
                }
            } else if (refName == eventRef()) {
//                qDebug() << "validating event";
                QPair<bool, QString> result = validateMap(eventDescription(), variant.toMap());
                if (!result.first) {
                    qDebug() << "event not valid";
                    return result;
                }
            } else if (refName == paramRef()) {
                if (!variant.canConvert(QVariant::Map)) {
                    report(false, "Param not valid. Should be a map.");
                }
            } else if (refName == paramDescriptorRef()) {
                QPair<bool, QString> result = validateMap(paramDescriptorDescription(), variant.toMap());
                if (!result.first) {
                    qDebug() << "ParamDescriptor not valid";
                    return result;
                }
            } else if (refName == deviceRef()) {
                QPair<bool, QString> result = validateMap(deviceDescription(), variant.toMap());
                if (!result.first) {
                    qDebug() << "device not valid";
                    return result;
                }
            } else if (refName == deviceDescriptorRef()) {
                QPair<bool, QString> result = validateMap(deviceDescriptorDescription(), variant.toMap());
                if (!result.first) {
                    qDebug() << "devicedescription not valid";
                    return result;
                }
            } else if (refName == vendorRef()) {
                QPair<bool, QString> result = validateMap(vendorDescription(), variant.toMap());
                if (!result.first) {
                    qDebug() << "value not allowed in" << vendorRef();
                }
            } else if (refName == deviceClassRef()) {
                QPair<bool, QString> result = validateMap(deviceClassDescription(), variant.toMap());
                if (!result.first) {
                    qDebug() << "device class not valid";
                    return result;
                }
            } else if (refName == paramTypeRef()) {
                QPair<bool, QString> result = validateMap(paramTypeDescription(), variant.toMap());
                if (!result.first) {
                    qDebug() << "param types not matching";
                    return result;
                }
            } else if (refName == actionTypeRef()) {
                QPair<bool, QString> result = validateMap(actionTypeDescription(), variant.toMap());
                if (!result.first) {
                    qDebug() << "action type not matching";
                    return result;
                }
            } else if (refName == eventTypeRef()) {
                QPair<bool, QString> result = validateMap(eventTypeDescription(), variant.toMap());
                if (!result.first) {
                    qDebug() << "event type not matching";
                    return result;
                }
            } else if (refName == stateTypeRef()) {
                QPair<bool, QString> result = validateMap(stateTypeDescription(), variant.toMap());
                if (!result.first) {
                    qDebug() << "state type not matching";
                    return result;
                }
            } else if (refName == stateEvaluatorRef()) {
                QPair<bool, QString> result = validateMap(stateEvaluatorDescription(), variant.toMap());
                if (!result.first) {
                    qDebug() << "stateEvaluator type not matching";
                    return result;
                }
            } else if (refName == stateDescriptorRef()) {
                QPair<bool, QString> result = validateMap(stateDescriptorDescription(), variant.toMap());
                if (!result.first) {
                    qDebug() << "stateDescriptor type not matching";
                    return result;
                }
            } else if (refName == pluginRef()) {
                QPair<bool, QString> result = validateMap(pluginDescription(), variant.toMap());
                if (!result.first) {
                    qDebug() << "plugin not matching";
                    return result;
                }
            } else if (refName == ruleRef()) {
                QPair<bool, QString> result = validateMap(ruleDescription(), variant.toMap());
                if (!result.first) {
                    qDebug() << "rule type not matching";
                    return result;
                }
            } else if (refName == eventDescriptorRef()) {
                QPair<bool, QString> result = validateMap(eventDescriptorDescription(), variant.toMap());
                if (!result.first) {
                    qDebug() << "evendescriptor not matching";
                    return result;
                }
            } else if (refName == logEntryRef()) {
                QPair<bool, QString> result = validateMap(logEntryDescription(), variant.toMap());
                if (!result.first) {
                    qDebug() << "logEntry not matching";
                    return result;
                }
            } else if (refName == basicTypeRef()) {
                QPair<bool, QString> result = validateBasicType(variant);
                if (!result.first) {
                    qDebug() << "value not allowed in" << basicTypeRef();
                    return result;
                }
            } else if (refName == stateOperatorRef()) {
                QPair<bool, QString> result = validateEnum(s_stateOperator, variant);
                if (!result.first) {
                    qDebug() << "value not allowed in" << stateOperatorRef();
                    return result;
                }
            } else if (refName == createMethodRef()) {
                QPair<bool, QString> result = validateEnum(s_createMethod, variant);
                if (!result.first) {
                    qDebug() << "value not allowed in" << createMethodRef() << variant;
                    return result;
                }
            } else if (refName == setupMethodRef()) {
                QPair<bool, QString> result = validateEnum(s_setupMethod, variant);
                if (!result.first) {
                    qDebug() << "value not allowed in" << createMethodRef();
                    return result;
                }
            } else if (refName == valueOperatorRef()) {
                QPair<bool, QString> result = validateEnum(s_valueOperator, variant);
                if (!result.first) {
                    qDebug() << QString("value %1 not allowed in %2").arg(variant.toString()).arg(valueOperatorRef());
                    return result;
                }
            } else if (refName == deviceErrorRef()) {
                QPair<bool, QString> result = validateEnum(s_deviceError, variant);
                if (!result.first) {
                    qDebug() << QString("value %1 not allowed in %2").arg(variant.toString()).arg(deviceErrorRef());
                    return result;
                }
            } else if (refName == ruleErrorRef()) {
                QPair<bool, QString> result = validateEnum(s_ruleError, variant);
                if (!result.first) {
                    qDebug() << QString("value %1 not allowed in %2").arg(variant.toString()).arg(ruleErrorRef());
                    return result;
                }
            } else if (refName == loggingErrorRef()) {
                QPair<bool, QString> result = validateEnum(s_loggingError, variant);
                if (!result.first) {
                    qDebug() << QString("value %1 not allowed in %2").arg(variant.toString()).arg(loggingErrorRef());
                    return result;
                }
            } else if (refName == loggingSourceRef()) {
                QPair<bool, QString> result = validateEnum(s_loggingSource, variant);
                if (!result.first) {
                    qDebug() << QString("value %1 not allowed in %2").arg(variant.toString()).arg(loggingSourceRef());
                    return result;
                }
            } else if (refName == loggingLevelRef()) {
                QPair<bool, QString> result = validateEnum(s_loggingLevel, variant);
                if (!result.first) {
                    qDebug() << QString("value %1 not allowed in %2").arg(variant.toString()).arg(loggingLevelRef());
                    return result;
                }
            } else if (refName == loggingEventTypeRef()) {
                QPair<bool, QString> result = validateEnum(s_loggingEventType, variant);
                if (!result.first) {
                    qDebug() << QString("value %1 not allowed in %2").arg(variant.toString()).arg(loggingEventTypeRef());
                    return result;
                }
            } else {
                Q_ASSERT_X(false, "JsonTypes", QString("Unhandled ref: %1").arg(refName).toLatin1().data());
                return report(false, QString("Unhandled ref %1. Server implementation incomplete.").arg(refName));
            }

        } else {
            QPair<bool, QString> result = JsonTypes::validateProperty(templateVariant, variant);
            if (!result.first) {
                qDebug() << "property not matching:" << templateVariant << "!=" << variant;
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
        qDebug() << "unhandled value" << templateVariant;
        return report(false, QString("Unhandled value %1.").arg(templateVariant.toString()));
    }
    return report(true, "");
}

QPair<bool, QString> JsonTypes::validateBasicType(const QVariant &variant)
{
    if (variant.canConvert(QVariant::Uuid)) {
        return report(true, "");
    }
    if (variant.canConvert(QVariant::String)) {
        return report(true, "");
    }
    if (variant.canConvert(QVariant::Int)) {
        return report(true, "");
    }
    if (variant.canConvert(QVariant::UInt)){
        return report(true, "");
    }
    if (variant.canConvert(QVariant::Double)) {
        return report(true, "");
    }
    if (variant.canConvert(QVariant::Bool)) {
        return report(true, "");
    }
    if (variant.canConvert(QVariant::Color)) {
        return report(true, "");
    }
    return report(false, QString("Error validating basic type %1.").arg(variant.toString()));
}

QPair<bool, QString> JsonTypes::validateEnum(const QVariantList &enumDescription, const QVariant &value)
{
    QStringList enumStrings;
    foreach (const QVariant &variant, enumDescription) {
        enumStrings.append(variant.toString());
    }

    return report(enumDescription.contains(value.toString()), QString("Value %1 not allowed in %2").arg(value.toString()).arg(enumStrings.join(", ")));
}
