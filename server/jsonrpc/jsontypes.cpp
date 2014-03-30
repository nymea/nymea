#include "jsontypes.h"

#include "device.h"

#include <QStringList>
#include <QDebug>

bool JsonTypes::s_initialized = false;

QVariantList JsonTypes::s_basicTypes;
QVariantList JsonTypes::s_ruleTypes;

QVariantMap JsonTypes::s_paramType;
QVariantMap JsonTypes::s_param;
QVariantMap JsonTypes::s_stateType;
QVariantMap JsonTypes::s_state;
QVariantMap JsonTypes::s_eventType;
QVariantMap JsonTypes::s_event;
QVariantMap JsonTypes::s_actionType;
QVariantMap JsonTypes::s_action;
QVariantMap JsonTypes::s_plugin;
QVariantMap JsonTypes::s_deviceClass;
QVariantMap JsonTypes::s_device;
QVariantMap JsonTypes::s_rule;

void JsonTypes::init()
{
    // BasicTypes
    s_basicTypes << "uuid" << "string" << "integer" << "double" << "bool";
    s_ruleTypes << "RuleTypeMatchAll" << "RuleTypeMatchAny";

    // ParamType
    s_paramType.insert("name", "string");
    s_paramType.insert("type", basicTypesRef());
//    s_paramType.insert("default", "value");
//    s_paramType.insert("value", "value");

    // Param
    s_param.insert("name", "string");
    s_param.insert("value", basicTypesRef());

    // StateType
    s_stateType.insert("id", "uuid");
    s_stateType.insert("name", "string");
    s_stateType.insert("type", basicTypesRef());
//    s_stateType.insert("default", "value");

    // State
    s_state.insert("stateTypeId", "uuid");
    s_state.insert("deviceId", "uuid");
    s_state.insert("value", "variant");

    // EventType
    s_eventType.insert("id", "uuid");
    s_eventType.insert("name", "string");
    s_eventType.insert("params", QVariantList() << paramTypeRef());

    // Event
    s_event.insert("eventTypeId", "uuid");
    s_event.insert("deviceId", "uuid");
    s_event.insert("params", QVariantList() << paramRef());

    // ActionType
    s_actionType.insert("id", "uuid");
    s_actionType.insert("name", "string");
    s_actionType.insert("params", QVariantList() << paramTypeRef());

    // Action
    s_action.insert("actionTypeId", "uuid");
    s_action.insert("deviceId", "uuid");
    s_action.insert("params", QVariantList() << paramRef());

    // Pugin
    s_plugin.insert("id", "uuid");
    s_plugin.insert("name", "string");
    s_plugin.insert("params", QVariantList() << paramTypeRef());

    // DeviceClass
    s_deviceClass.insert("id", "uuid");
    s_deviceClass.insert("name", "string");
    s_deviceClass.insert("states", QVariantList() << stateTypeRef());
    s_deviceClass.insert("events", QVariantList() << eventTypeRef());
    s_deviceClass.insert("actions", QVariantList() << actionTypeRef());
    s_deviceClass.insert("params", QVariantList() << paramTypeRef());

    // Device
    s_device.insert("id", "uuid");
    s_device.insert("deviceClassId", "uuid");
    s_device.insert("name", "string");
    s_device.insert("params", QVariantList() << paramRef());

    s_rule.insert("id", "uuid");
    s_rule.insert("ruleType", ruleTypesRef());
    s_rule.insert("event", eventRef());
    s_rule.insert("actions", QVariantList() << actionRef());
    s_rule.insert("states", QVariantList() << stateRef());

    s_initialized = true;
}

QVariantMap JsonTypes::allTypes()
{
    QVariantMap allTypes;
    allTypes.insert("BasicType", basicTypes());
    allTypes.insert("ParamType", paramTypeDescription());
    allTypes.insert("StateType", stateTypeDescription());
    allTypes.insert("EventType", eventTypeDescription());
    allTypes.insert("ActionType", actionTypeDescription());
    allTypes.insert("DeviceClass", deviceClassDescription());
    allTypes.insert("Plugin", pluginDescription());
    allTypes.insert("Param", paramDescription());
    allTypes.insert("State", stateDescription());
    allTypes.insert("Event", eventDescription());
    allTypes.insert("Device", deviceDescription());
    allTypes.insert("Action", actionDescription());
    allTypes.insert("RuleType", ruleTypes());
    allTypes.insert("Rule", ruleDescription());
    return allTypes;
}

QVariantMap JsonTypes::packEventType(const EventType &eventType)
{
    QVariantMap variant;
    variant.insert("id", eventType.id());
    variant.insert("name", eventType.name());
    variant.insert("params", eventType.parameters());
    return variant;
}

QVariantMap JsonTypes::packEvent(const Event &event)
{
    QVariantMap variant;
    variant.insert("eventTypeId", event.eventTypeId());
    variant.insert("deviceId", event.deviceId());
    variant.insert("params", event.params());
    return variant;
}

QVariantMap JsonTypes::packActionType(const ActionType &actionType)
{
    QVariantMap variantMap;
    variantMap.insert("id", actionType.id());
    variantMap.insert("name", actionType.name());
    variantMap.insert("params", actionType.parameters());
    return variantMap;
}

QVariantMap JsonTypes::packAction(const Action &action)
{
    QVariantMap variant;
    variant.insert("actionTypeId", action.actionTypeId());
    variant.insert("deviceId", action.deviceId());
    variant.insert("params", action.params());
    return variant;
}

QVariantMap JsonTypes::packDeviceClass(const DeviceClass &deviceClass)
{
    QVariantMap variant;
    variant.insert("name", deviceClass.name());
    variant.insert("id", deviceClass.id());
    QVariantList stateTypes;
    foreach (const StateType &stateType, deviceClass.states()) {
        QVariantMap stateMap;
        stateMap.insert("id", stateType.id().toString());
        stateMap.insert("name", stateType.name());
        stateMap.insert("type", QVariant::typeToName(stateType.type()));

        stateTypes.append(stateMap);
    }
    QVariantList eventTypes;
    foreach (const EventType &eventType, deviceClass.events()) {
        QVariantMap eventMap;
        eventMap.insert("id", eventType.id().toString());
        eventMap.insert("name", eventType.name());
        eventMap.insert("params", eventType.parameters());

        eventTypes.append(eventMap);
    }
    QVariantList actionTypes;
    foreach (const ActionType &actionType, deviceClass.actions()) {
        QVariantMap actionMap;
        actionMap.insert("id", actionType.id().toString());
        actionMap.insert("name", actionType.name());
        actionMap.insert("params", actionType.parameters());

        actionTypes.append(actionMap);
    }
    variant.insert("params", deviceClass.params());
    variant.insert("states", stateTypes);
    variant.insert("events", eventTypes);
    variant.insert("actions", actionTypes);
    return variant;
}

QVariantMap JsonTypes::packPlugin(DevicePlugin *plugin)
{

}

QVariantMap JsonTypes::packDevice(Device *device)
{
    QVariantMap variant;
    variant.insert("id", device->id());
    variant.insert("deviceClassId", device->deviceClassId());
    variant.insert("name", device->name());
    variant.insert("params", device->params());
    return variant;
}

QVariantMap JsonTypes::packRule(const Rule &rule)
{
    QVariantMap ruleMap;
    ruleMap.insert("id", rule.id());
    ruleMap.insert("event", JsonTypes::packEvent(rule.event()));
    ruleMap.insert("ruleType", s_ruleTypes.at(rule.ruleType()));
    QVariantList actionList;
    foreach (const Action &action, rule.actions()) {
        actionList.append(JsonTypes::packAction(action));
    }
    ruleMap.insert("actions", actionList);

    QVariantList states;
    ruleMap.insert("states", states);
    return ruleMap;
}

bool JsonTypes::validateMap(const QVariantMap &templateMap, const QVariantMap &map)
{
    foreach (const QString &key, templateMap.keys()) {
        if (!map.contains(key)) {
            qDebug() << "missing key" << key << templateMap << map;
            return false;
        }
        if (!validateVariant(templateMap.value(key), map.value(key))) {
            qDebug() << "Object not matching template";
            return false;
        }
    }
    return true;
}

bool JsonTypes::validateProperty(const QVariant &templateValue, const QVariant &value)
{
    if (templateValue == "uuid") {
        return value.canConvert(QVariant::Uuid);
    }
    if (templateValue == "string") {
        return value.canConvert(QVariant::String);
    }
    if (templateValue == "bool") {
        return value.canConvert(QVariant::Bool);
    }
    qWarning() << "Unhandled property type!" << templateValue;
    return false;
}

bool JsonTypes::validateList(const QVariantList &templateList, const QVariantList &list)
{
    Q_ASSERT(templateList.count() == 1);
    QVariant entryTemplate = templateList.first();

    qDebug() << "validating list" << templateList;

    for (int i = 0; i < list.count(); ++i) {
        QVariant listEntry = list.at(i);
        if (!validateVariant(entryTemplate, listEntry)) {
            qDebug() << "List entry not matching template";
            return false;
        }
    }
    return true;
}

bool JsonTypes::validateVariant(const QVariant &templateVariant, const QVariant &variant)
{
    switch(templateVariant.type()) {
    case QVariant::String:
        if (templateVariant.toString().startsWith("$ref:")) {
            QString refName = templateVariant.toString();
            if (refName == JsonTypes::actionRef()) {
                qDebug() << "validating action";
                if (!validateMap(actionDescription(), variant.toMap())) {
                    qDebug() << "Error validating action";
                    return false;
                }
            } else if (refName == JsonTypes::eventRef()) {
                if (!validateMap(eventDescription(), variant.toMap())) {
                    qDebug() << "event not valid";
                    return false;
                }
            } else if (refName == deviceRef()) {
                if (!validateMap(deviceDescription(), variant.toMap())) {
                    qDebug() << "device not valid";
                    return false;
                }
            } else if (refName == deviceClassRef()) {
                if (!validateMap(deviceClassDescription(), variant.toMap())) {
                    qDebug() << "device class not valid";
                    return false;
                }
            } else if (refName == paramTypeRef()) {
                if (!validateMap(paramTypeDescription(), variant.toMap())) {
                    qDebug() << "param types not matching";
                    return false;
                }
            } else if (refName == actionTypeRef()) {
                if (!validateMap(actionTypeDescription(), variant.toMap())) {
                    qDebug() << "action type not matching";
                    return false;
                }
            } else if (refName == eventTypeRef()) {
                if (!validateMap(eventTypeDescription(), variant.toMap())) {
                    qDebug() << "event type not matching";
                    return false;
                }
            } else if (refName == stateTypeRef()) {
                if (!validateMap(stateTypeDescription(), variant.toMap())) {
                    qDebug() << "state type not matching";
                    return false;
                }
            } else if (refName == pluginRef()) {
                if (!validateMap(pluginDescription(), variant.toMap())) {
                    qDebug() << "plugin not matching";
                    return false;
                }
            } else if (refName == ruleRef()) {
                if (!validateMap(ruleDescription(), variant.toMap())) {
                    qDebug() << "rule type not matching";
                    return false;
                }
            } else if (refName == basicTypesRef()) {
                if (!validateBasicType(variant)) {
                    qDebug() << "value not allowed in" << basicTypesRef();
                    return false;
                }
            } else if (refName == ruleTypesRef()) {
                if (!validateRuleType(variant)) {
                    qDebug() << "value not allowed in" << ruleTypesRef();
                    return false;
                }
            } else {
                qDebug() << "unhandled ref:" << refName;
                return false;
            }

        } else {
            if (!JsonTypes::validateProperty(templateVariant, variant)) {
                qDebug() << "property not matching:" << templateVariant << "!=" << variant;
                return false;
            }
        }
        break;
    case QVariant::Map:
        if (!validateMap(templateVariant.toMap(), variant.toMap())) {
            return false;
        }
        break;
    case QVariant::List:
        if (!validateList(templateVariant.toList(), variant.toList())) {
            return false;
        }
        break;
    default:
        qDebug() << "unhandled value" << templateVariant;
        return false;
    }
    return true;
}

bool JsonTypes::validateBasicType(const QVariant &variant)
{
    if (variant.canConvert(QVariant::Uuid)) {
        return true;
    }
    if (variant.canConvert(QVariant::String)) {
        return true;
    }
    if (variant.canConvert(QVariant::Int)) {
        return true;
    }
    if (variant.canConvert(QVariant::Double)) {
        return true;
    }
    if (variant.canConvert(QVariant::Bool)) {
        return true;
    }
    return false;
}

bool JsonTypes::validateRuleType(const QVariant &variant)
{
    return s_ruleTypes.contains(variant.toString());
}
