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

#include <QStringList>
#include <QJsonDocument>
#include <QDebug>

bool JsonTypes::s_initialized = false;
QString JsonTypes::s_lastError;

QVariantList JsonTypes::s_basicTypes;
QVariantList JsonTypes::s_stateOperatorTypes;
QVariantList JsonTypes::s_valueOperatorTypes;
QVariantList JsonTypes::s_createMethodTypes;
QVariantList JsonTypes::s_setupMethodTypes;
QVariantList JsonTypes::s_removePolicyTypes;

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

void JsonTypes::init()
{
    // BasicTypes
    s_basicTypes << "uuid" << "string" << "integer" << "double" << "bool";
    s_stateOperatorTypes << "StateOperatorAnd" << "StateOperatorOr";
    s_valueOperatorTypes << "OperatorTypeEquals" << "OperatorTypeNotEquals" << "OperatorTypeLess" << "OperatorTypeGreater" << "OperatorTypeLessThan" << "OperatorTypeGreaterThan";
    s_createMethodTypes << "CreateMethodUser" << "CreateMethodAuto" << "CreateMethodDiscovery";
    s_setupMethodTypes << "SetupMethodJustAdd" << "SetupMethodDisplayPin" << "SetupMethodEnterPin" << "SetupMethodPushButton";
    s_removePolicyTypes << "RemovePolicyCascade" << "RemovePolicyUpdate";

    // ParamType
    s_paramType.insert("name", "string");
    s_paramType.insert("type", basicTypesRef());
    s_paramType.insert("o:defaultValue", "variant");
    s_paramType.insert("o:minValue", "variant");
    s_paramType.insert("o:maxValue", "variant");
    s_paramType.insert("o:allowedValues", QVariantList() << "variant");

    // Param
    s_param.insert("name", "string");
    s_param.insert("value", basicTypesRef());

    // ParamDescriptor
    s_paramDescriptor.insert("name", "string");
    s_paramDescriptor.insert("value", basicTypesRef());
    s_paramDescriptor.insert("operator", valueOperatorTypesRef());

    // StateType
    s_stateType.insert("id", "uuid");
    s_stateType.insert("name", "string");
    s_stateType.insert("type", basicTypesRef());
    s_stateType.insert("defaultValue", "variant");

    // State
    s_state.insert("stateTypeId", "uuid");
    s_state.insert("deviceId", "uuid");
    s_state.insert("value", "variant");

    // StateDescriptor
    s_stateDescriptor.insert("stateTypeId", "uuid");
    s_stateDescriptor.insert("deviceId", "uuid");
    s_stateDescriptor.insert("value", "variant");
    s_stateDescriptor.insert("operator", valueOperatorTypesRef());

    // StateEvaluator
    s_stateEvaluator.insert("o:stateDescriptor", stateDescriptorRef());
    s_stateEvaluator.insert("o:childEvaluators", QVariantList() << stateEvaluatorRef());
    s_stateEvaluator.insert("o:operator", stateOperatorTypesRef());

    // EventType
    s_eventType.insert("id", "uuid");
    s_eventType.insert("name", "string");
    s_eventType.insert("paramTypes", QVariantList() << paramTypeRef());

    // Event
    s_event.insert("eventTypeId", "uuid");
    s_event.insert("deviceId", "uuid");
    s_event.insert("o:params", QVariantList() << paramRef());

    // EventDescriptor
    s_eventDescriptor.insert("eventTypeId", "uuid");
    s_eventDescriptor.insert("deviceId", "uuid");
    s_eventDescriptor.insert("o:paramDescriptors", QVariantList() << paramDescriptorRef());

    // ActionType
    s_actionType.insert("id", "uuid");
    s_actionType.insert("name", "string");
    s_actionType.insert("paramTypes", QVariantList() << paramTypeRef());

    // Action
    s_action.insert("actionTypeId", "uuid");
    s_action.insert("deviceId", "uuid");
    s_action.insert("o:params", QVariantList() << paramRef());

    // Pugin
    s_plugin.insert("id", "uuid");
    s_plugin.insert("name", "string");
    s_plugin.insert("params", QVariantList() << paramRef());

    // Vendor
    s_vendor.insert("id", "uuid");
    s_vendor.insert("name", "string");

    // DeviceClass
    s_deviceClass.insert("id", "uuid");
    s_deviceClass.insert("vendorId", "uuid");
    s_deviceClass.insert("name", "string");
    s_deviceClass.insert("stateTypes", QVariantList() << stateTypeRef());
    s_deviceClass.insert("eventTypes", QVariantList() << eventTypeRef());
    s_deviceClass.insert("actionTypes", QVariantList() << actionTypeRef());
    s_deviceClass.insert("paramTypes", QVariantList() << paramTypeRef());
    s_deviceClass.insert("discoveryParamTypes", QVariantList() << paramTypeRef());
    s_deviceClass.insert("setupMethod", setupMethodTypesRef());
    s_deviceClass.insert("createMethod", createMethodTypesRef());

    // Device
    s_device.insert("id", "uuid");
    s_device.insert("deviceClassId", "uuid");
    s_device.insert("name", "string");
    s_device.insert("params", QVariantList() << paramRef());
    s_device.insert("setupComplete", "bool");

    // DeviceDescription
    s_deviceDescriptor.insert("id", "uuid");
    s_deviceDescriptor.insert("title", "string");
    s_deviceDescriptor.insert("description", "string");

    // Rule
    s_rule.insert("id", "uuid");
    s_rule.insert("eventDescriptors", QVariantList() << eventDescriptorRef());
    s_rule.insert("actions", QVariantList() << actionRef());
    s_rule.insert("stateEvaluator", stateEvaluatorRef());

    s_initialized = true;
}

QPair<bool, QString> JsonTypes::report(bool status, const QString &message)
{
    return qMakePair<bool, QString>(status, message);
}

QVariantMap JsonTypes::allTypes()
{
    QVariantMap allTypes;
    allTypes.insert("BasicType", basicTypes());
    allTypes.insert("ParamType", paramTypeDescription());
    allTypes.insert("CreateMethodType", createMethodTypes());
    allTypes.insert("SetupMethodType", setupMethodTypes());
    allTypes.insert("ValueOperatorType", valueOperatorTypes());
    allTypes.insert("StateOperatorType", stateOperatorTypes());
    allTypes.insert("RemovePolicyType", removePolicyTypes());
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
    return allTypes;
}

QVariantMap JsonTypes::packEventType(const EventType &eventType)
{
    QVariantMap variant;
    variant.insert("id", eventType.id());
    variant.insert("name", eventType.name());
    QVariantList paramTypes;
    foreach (const ParamType &paramType, eventType.parameters()) {
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
    foreach (const ParamType &paramType, actionType.parameters()) {
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
    variantMap.insert("operator", valueOperatorTypes().at(stateDescriptor.operatorType()));
    return variantMap;
}

QVariantMap JsonTypes::packStateEvaluator(const StateEvaluator &stateEvaluator)
{
    QVariantMap variantMap;
    variantMap.insert("stateDescriptor", packStateDescriptor(stateEvaluator.stateDescriptor()));
    QVariantList childEvaluators;
    foreach (const StateEvaluator &childEvaluator, stateEvaluator.childEvaluators()) {
        childEvaluators.append(packStateEvaluator(childEvaluator));
    }
    variantMap.insert("childEvaluators", childEvaluators);

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
    variantMap.insert("operator", s_valueOperatorTypes.at(paramDescriptor.operatorType()));
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
        discoveryParamTypes.append(packParamType(paramType));
    }

    variant.insert("paramTypes", paramTypes);
    variant.insert("discoveryParamTypes", discoveryParamTypes);
    variant.insert("stateTypes", stateTypes);
    variant.insert("eventTypes", eventTypes);
    variant.insert("actionTypes", actionTypes);
    variant.insert("createMethod", s_createMethodTypes.at(deviceClass.createMethod()));
    variant.insert("setupMethod", s_setupMethodTypes.at(deviceClass.setupMethod()));
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
        qDebug() << "unpacking param" << paramVariant;
        params.append(unpackParam(paramVariant.toMap()));
    }
    return params;
}

ParamDescriptor JsonTypes::unpackParamDescriptor(const QVariantMap &paramMap)
{
    ParamDescriptor param(paramMap.value("name").toString(), paramMap.value("value"));
    QString operatorString = paramMap.value("operator").toString();
    if (operatorString == "ValueOperatorEquals") {
        param.setOperatorType(ValueOperatorEquals);
    } else if (operatorString == "ValueOperatorNotEquals") {
        param.setOperatorType(ValueOperatorNotEquals);
    } else if (operatorString == "ValueOperatorLess") {
        param.setOperatorType(ValueOperatorLess);
    } else if (operatorString == "ValueOperatorGreater") {
        param.setOperatorType(ValueOperatorGreater);
    } else if (operatorString == "ValueOperatorLessOrEqual") {
        param.setOperatorType(ValueOperatorLessOrEqual);
    } else if (operatorString == "ValueOperatorGreaterOrEqual") {
        param.setOperatorType(ValueOperatorGreaterOrEqual);
    }
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

QPair<bool, QString> JsonTypes::validateMap(const QVariantMap &templateMap, const QVariantMap &map)
{
    s_lastError.clear();

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
                qDebug() << "Object not matching template or object not matching" << templateMap.value(key) << map.value(strippedKey);
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
//    qDebug() << "validating property. template:" << templateValue << "got:" << value;
    QString strippedTemplateValue = templateValue.toString();

    if (strippedTemplateValue == "variant") {
        return report(true, "");
    }
    if (strippedTemplateValue == "uuid") {
        QString errorString = QString("Param %1 is not a uuid.").arg(value.toString());
        return report(value.canConvert(QVariant::Uuid), errorString);
    }
    if (strippedTemplateValue == "string") {
        QString errorString = QString("Param %1 is not a string.").arg(value.toString());
        return report(value.canConvert(QVariant::String), errorString);
    }
    if (strippedTemplateValue == "bool") {
        QString errorString = QString("Param %1 is not a bool.").arg(value.toString());
        return report(value.canConvert(QVariant::Bool), errorString);
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
            } else if (refName == basicTypesRef()) {
                QPair<bool, QString> result = validateBasicType(variant);
                if (!result.first) {
                    qDebug() << "value not allowed in" << basicTypesRef();
                    return result;
                }
            } else if (refName == stateOperatorTypesRef()) {
                QPair<bool, QString> result = validateStateOperatorType(variant);
                if (!result.first) {
                    qDebug() << "value not allowed in" << stateOperatorTypesRef();
                    return result;
                }
            } else if (refName == createMethodTypesRef()) {
                QPair<bool, QString> result = validateCreateMethodType(variant);
                if (!result.first) {
                    qDebug() << "value not allowed in" << createMethodTypesRef();
                    return result;
                }
            } else if (refName == setupMethodTypesRef()) {
                QPair<bool, QString> result = validateSetupMethodType(variant);
                if (!result.first) {
                    qDebug() << "value not allowed in" << createMethodTypesRef();
                    return result;
                }
            } else if (refName == valueOperatorTypesRef()) {
                QPair<bool, QString> result = validateValueOperatorType(variant);
                if (!result.first) {
                    qDebug() << QString("value %1 not allowed in %2").arg(variant.toString()).arg(valueOperatorTypesRef());
                    return result;
                }
            } else {
                qDebug() << "unhandled ref:" << refName;
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
    if (variant.canConvert(QVariant::Double)) {
        return report(true, "");
    }
    if (variant.canConvert(QVariant::Bool)) {
        return report(true, "");
    }
    return report(false, QString("Error validating basic type %1.").arg(variant.toString()));
}

QPair<bool, QString> JsonTypes::validateStateOperatorType(const QVariant &variant)
{
    return report(s_stateOperatorTypes.contains(variant.toString()), QString("Unknown state operator %1").arg(variant.toString()));
}

QPair<bool, QString> JsonTypes::validateCreateMethodType(const QVariant &variant)
{
    return report(s_createMethodTypes.contains(variant.toString()), QString("Unknwon createMethod type %1").arg(variant.toString()));
}

QPair<bool, QString> JsonTypes::validateSetupMethodType(const QVariant &variant)
{
    return report(s_setupMethodTypes.contains(variant.toString()), QString("Unknwon setupMethod type %1").arg(variant.toString()));
}

QPair<bool, QString> JsonTypes::validateValueOperatorType(const QVariant &variant)
{
    return report(s_valueOperatorTypes.contains(variant.toString()), QString("Unknown value operator type %1").arg(variant.toString()));
}
