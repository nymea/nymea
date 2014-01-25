#include "jsontypes.h"

#include "device.h"

#include <QDebug>

namespace JsonTypes {

QVariantMap allTypes()
{
    QVariantMap allTypes;
    allTypes.insert("BasicType", basicTypes());
    allTypes.insert("ParamType", paramTypeDescription());
    allTypes.insert("StateType", stateTypeDescription());
    allTypes.insert("TriggerType", triggerTypeDescription());
    allTypes.insert("ActionType", actionTypeDescription());
    allTypes.insert("DeviceClass", deviceClassDescription());
    allTypes.insert("PluginType", pluginTypeDescription());
    allTypes.insert("Param", paramDescription());
    allTypes.insert("State", stateDescription());
    allTypes.insert("Trigger", triggerDescription());
    allTypes.insert("Device", deviceDescription());
    allTypes.insert("Action", actionDescription());
    allTypes.insert("Rule", ruleDescription());
    return allTypes;
}

QString basicTypesRef()
{
    return "$ref:BasicType";
}

QVariantList basicTypes()
{
    QVariantList basicTypes;
    basicTypes.append("uuid");
    basicTypes.append("string");
    basicTypes.append("integer");
    basicTypes.append("double");
    basicTypes.append("bool");
    return basicTypes;
}

QString paramTypeRef()
{
    return "$ref:ParamType";
}

QVariantMap paramTypeDescription()
{
    QVariantMap paramType;
    paramType.insert("name", "string");
    paramType.insert("type", basicTypesRef());
//    paramType.insert("default", "value");
//    paramType.insert("value", "value");
    return paramType;
}

QString paramRef()
{
    return "$ref:Param";
}

QVariantMap paramDescription()
{
    QVariantMap param;
    param.insert("name", "string");
    param.insert("value", basicTypesRef());
    return param;
}

QString stateTypeRef()
{
    return "$ref:StateType";
}

QVariantMap stateTypeDescription()
{
    QVariantMap stateTypeDescription;
    stateTypeDescription.insert("id", "uuid");
    stateTypeDescription.insert("name", "string");
    stateTypeDescription.insert("type", basicTypesRef());
//    stateTypeDescription.insert("default", "value");
    return stateTypeDescription;
}

QString stateRef()
{
    return "$ref:Sate";
}

QVariantMap stateDescription()
{
    QVariantMap stateDescription;
    stateDescription.insert("stateTypeId", "uuid");
    stateDescription.insert("deviceId", "uuid");
    stateDescription.insert("value", "variant");
    return stateDescription;
}


QString triggerTypeRef()
{
    return "$ref:TriggerType";
}

QVariantMap triggerTypeDescription()
{
    QVariantMap triggerTypeDescription;
    triggerTypeDescription.insert("id", "uuid");
    triggerTypeDescription.insert("name", "string");
    QVariantList params;
    params.append(paramTypeRef());
    triggerTypeDescription.insert("params", params);
    return triggerTypeDescription;
}

QVariantMap packTriggerType(const TriggerType &triggerType)
{
    QVariantMap variant;
    variant.insert("id", triggerType.id());
    variant.insert("name", triggerType.name());
    variant.insert("params", triggerType.parameters());
    return variant;
}

QString triggerRef()
{
    return "$ref:Trigger";
}

QVariantMap triggerDescription()
{
    QVariantMap triggerDescription;
    triggerDescription.insert("triggerTypeId", "uuid");
    triggerDescription.insert("deviceId", "uuid");
    QVariantList params;
    params.append(paramRef());
    triggerDescription.insert("params", params);
    return triggerDescription;
}

QVariantMap packTrigger(const Trigger &trigger)
{
    QVariantMap variant;
    variant.insert("triggerTypeId", trigger.triggerTypeId());
    variant.insert("deviceId", trigger.deviceId());
    variant.insert("params", trigger.params());
    return variant;
}

QString actionTypeRef()
{
    return "$ref:ActionType";
}

QVariantMap actionTypeDescription()
{
    QVariantMap actionTypeDescription;
    actionTypeDescription.insert("id", "uuid");
    actionTypeDescription.insert("name", "string");
    QVariantList params;
    params.append(paramTypeRef());
    actionTypeDescription.insert("params", params);
    return actionTypeDescription;
}

QVariantMap packActionType(const ActionType &actionType)
{
    QVariantMap variantMap;
    variantMap.insert("id", actionType.id());
    variantMap.insert("name", actionType.name());
    variantMap.insert("params", actionType.parameters());
    return variantMap;
}

QString actionRef()
{
    return "$ref:Action";
}

QVariantMap actionDescription()
{
    QVariantMap actionDescription;
    actionDescription.insert("actionTypeId", "uuid");
    actionDescription.insert("deviceId", "uuid");
    QVariantList params;
    params.append(paramRef());
    actionDescription.insert("params", params);
    return actionDescription;
}

QVariantMap packAction(const Action &action)
{
    QVariantMap variant;
    variant.insert("actionTypeId", action.actionTypeId());
    variant.insert("deviceId", action.deviceId());
    variant.insert("params", action.params());
    return variant;
}

QString deviceClassRef()
{
    return "$ref:DeviceClass";
}

QVariantMap deviceClassDescription()
{
    QVariantMap deviceClass;
    deviceClass.insert("id", "uuid");
    deviceClass.insert("name", "string");
    QVariantList states;
    states.append(stateTypeRef());
    deviceClass.insert("states", states);
    QVariantList triggers;
    triggers.append(triggerTypeRef());
    deviceClass.insert("triggers", triggers);
    QVariantList actions;
    actions.append(actionTypeRef());
    deviceClass.insert("actions", actions);
    QVariantList params;
    params.append(paramTypeRef());
    deviceClass.insert("params", params);
    return deviceClass;
}

QVariantMap packDeviceClass(const DeviceClass &deviceClass)
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
    QVariantList triggerTypes;
    foreach (const TriggerType &triggerType, deviceClass.triggers()) {
        QVariantMap triggerMap;
        triggerMap.insert("id", triggerType.id().toString());
        triggerMap.insert("name", triggerType.name());
        triggerMap.insert("params", triggerType.parameters());

        triggerTypes.append(triggerMap);
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
    variant.insert("triggers", triggerTypes);
    variant.insert("actions", actionTypes);
    return variant;
}

QString pluginTypeRef()
{
    return "$ref:PluginType";
}

QVariantMap pluginTypeDescription()
{
    QVariantMap pluginDescription;
    pluginDescription.insert("id", "uuid");
    pluginDescription.insert("name", "string");
    QVariantList params;
    params.append(paramTypeRef());
    pluginDescription.insert("params", params);
    return pluginDescription;
}

QVariantMap packPlugin(DevicePlugin *plugin)
{

}

QString deviceRef()
{
    return "$ref:Device";
}

QVariantMap deviceDescription()
{
    QVariantMap deviceDescription;
    deviceDescription.insert("id", "uuid");
    deviceDescription.insert("deviceClassId", "uuid");
    deviceDescription.insert("name", "string");
    QVariantList params;
    params.append(paramRef());
    deviceDescription.insert("params", params);
    return deviceDescription;
}

QVariantMap packDevice(Device *device)
{
    QVariantMap variant;
    variant.insert("id", device->id());
    variant.insert("deviceClassId", device->deviceClassId());
    variant.insert("name", device->name());
    variant.insert("params", device->params());
    return variant;
}

QString ruleRef()
{
    return "$ref:Rule";
}

QVariantMap ruleDescription()
{
    QVariantMap ruleDescription;
    ruleDescription.insert("id", "uuid");
    ruleDescription.insert("trigger", triggerRef());
    QVariantList actions;
    actions.append(actionRef());
    ruleDescription.insert("actions", actions);
    QVariantList states;
    states.append(stateRef());
    ruleDescription.insert("states", states);
    return ruleDescription;
}

//QVariantMap packRule(const Rule &rule);

bool validateMap(const QVariantMap &templateMap, const QVariantMap &map)
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

bool validateProperty(const QVariant &templateValue, const QVariant &value)
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

bool validateList(const QVariantList &templateList, const QVariantList &list)
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

bool validateVariant(const QVariant &templateVariant, const QVariant &variant)
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
            } else if (refName == JsonTypes::triggerRef()) {
                if (!validateMap(triggerDescription(), variant.toMap())) {
                    qDebug() << "trigger not valid";
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
            } else if (refName == triggerTypeRef()) {
                if (!validateMap(triggerTypeDescription(), variant.toMap())) {
                    qDebug() << "trigger type not matching";
                    return false;
                }
            } else if (refName == stateTypeRef()) {
                if (!validateMap(stateTypeDescription(), variant.toMap())) {
                    qDebug() << "state type not matching";
                    return false;
                }
            } else if (refName == pluginTypeRef()) {
                if (!validateMap(pluginTypeDescription(), variant.toMap())) {
                    qDebug() << "plugin type not matching";
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

bool validateBasicType(const QVariant &variant)
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
}
