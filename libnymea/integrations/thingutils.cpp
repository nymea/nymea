/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2020, nymea GmbH
* Contact: contact@nymea.io
*
* This file is part of nymea.
* This project including source code and documentation is protected by
* copyright law, and remains the property of nymea GmbH. All rights, including
* reproduction, publication, editing and translation, are reserved. The use of
* this project is subject to the terms of a license agreement to be concluded
* with nymea GmbH in accordance with the terms of use of nymea GmbH, available
* under https://nymea.io/license
*
* GNU Lesser General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU Lesser General Public License as published by the Free
* Software Foundation; version 3. This project is distributed in the hope that
* it will be useful, but WITHOUT ANY WARRANTY; without even the implied
* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "thingutils.h"
#include "loggingcategories.h"

#include <QDir>
#include <QFileInfo>
#include <QJsonParseError>
#include <QMetaEnum>

ThingUtils::ThingUtils()
{

}

/*! Verify if the given \a params matches the given \a paramTypes.*/
Thing::ThingError ThingUtils::verifyParams(const QList<ParamType> paramTypes, const ParamList &params)
{
    foreach (const Param &param, params) {
        Thing::ThingError result = verifyParam(paramTypes, param);
        if (result != Thing::ThingErrorNoError) {
            return result;
        }
    }
    foreach (const ParamType &paramType, paramTypes) {
        bool found = !paramType.defaultValue().isNull();
        foreach (const Param &param, params) {
            if (paramType.id() == param.paramTypeId()) {
                found = true;
                break;
            }
        }

        if (!found) {
            qCWarning(dcThing) << "Missing parameter:" << paramType.name() << params;
            return Thing::ThingErrorMissingParameter;
        }
    }
    return Thing::ThingErrorNoError;
}

/*! Verify if the given \a param matches one of the given \a paramTypes. Returns \l{Device::DeviceError} to inform about the result.*/
Thing::ThingError ThingUtils::verifyParam(const QList<ParamType> paramTypes, const Param &param)
{
    foreach (const ParamType &paramType, paramTypes) {
        if (paramType.id() == param.paramTypeId()) {
            return verifyParam(paramType, param);
        }
    }

    qCWarning(dcThing) << "Invalid parameter" << param.paramTypeId().toString() << "in parameter list";
    return Thing::ThingErrorInvalidParameter;
}

/*! Verify if the given \a param matches the given \a paramType. Returns \l{Device::DeviceError} to inform about the result.*/
Thing::ThingError ThingUtils::verifyParam(const ParamType &paramType, const Param &param)
{
    if (paramType.id() != param.paramTypeId()) {
        qCWarning(dcThing) << "Parameter id" << param.paramTypeId().toString() << "does not match with ParamType id" << paramType.id().toString();
        return Thing::ThingErrorInvalidParameter;
    }

    if (!param.value().canConvert(static_cast<int>(paramType.type()))) {
        qCWarning(dcThing) << "Wrong parameter type for param" << param.paramTypeId().toString() << " Got:" << param.value() << " Expected:" << QVariant::typeToName(static_cast<int>(paramType.type()));
        return Thing::ThingErrorInvalidParameter;
    }

    if (!param.value().convert(static_cast<int>(paramType.type()))) {
        qCWarning(dcThing) << "Could not convert value of param" << param.paramTypeId().toString() << " to:" << QVariant::typeToName(static_cast<int>(paramType.type())) << " Got:" << param.value();
        return Thing::ThingErrorInvalidParameter;
    }

    if (paramType.type() == QVariant::Int) {
        if (paramType.maxValue().isValid() && param.value().toInt() > paramType.maxValue().toInt()) {
            qCWarning(dcThing) << "Value out of range for param" << param.paramTypeId().toString() << " Got:" << param.value() << " Max:" << paramType.maxValue();
            return Thing::ThingErrorInvalidParameter;
        }

        if (paramType.minValue().isValid() && param.value().toInt() < paramType.minValue().toInt()) {
            qCWarning(dcThing) << "Value out of range for param" << param.paramTypeId().toString() << " Got:" << param.value() << " Min:" << paramType.minValue();
            return Thing::ThingErrorInvalidParameter;
        }
    } else if (paramType.type() == QVariant::UInt) {
        if (paramType.maxValue().isValid() && param.value().toUInt() > paramType.maxValue().toUInt()) {
            qCWarning(dcThing) << "Value out of range for param" << param.paramTypeId().toString() << " Got:" << param.value() << " Max:" << paramType.maxValue();
            return Thing::ThingErrorInvalidParameter;
        }

        if (paramType.minValue().isValid() && param.value().toUInt() < paramType.minValue().toUInt()) {
            qCWarning(dcThing) << "Value out of range for param" << param.paramTypeId().toString() << " Got:" << param.value() << " Min:" << paramType.minValue();
            return Thing::ThingErrorInvalidParameter;
        }
    } else if (paramType.type() == QVariant::Double) {
        if (paramType.maxValue().isValid() && param.value().toDouble() > paramType.maxValue().toDouble()) {
            qCWarning(dcThing) << "Value out of range for param" << param.paramTypeId().toString() << " Got:" << param.value() << " Max:" << paramType.maxValue();
            return Thing::ThingErrorInvalidParameter;
        }

        if (paramType.minValue().isValid() && param.value().toDouble() < paramType.minValue().toDouble()) {
            qCWarning(dcThing) << "Value out of range for param" << param.paramTypeId().toString() << " Got:" << param.value() << " Min:" << paramType.minValue();
            return Thing::ThingErrorInvalidParameter;
        }
    } else {
        if (paramType.maxValue().isValid() && param.value() > paramType.maxValue()) {
            qCWarning(dcThing) << "Value out of range for param" << param.paramTypeId().toString() << " Got:" << param.value() << " Max:" << paramType.maxValue();
            return Thing::ThingErrorInvalidParameter;
        }

        if (paramType.minValue().isValid() && param.value() < paramType.minValue()) {
            qCWarning(dcThing) << "Value out of range for param" << param.paramTypeId().toString() << " Got:" << param.value() << " Min:" << paramType.minValue();
            return Thing::ThingErrorInvalidParameter;
        }
    }

    if (!paramType.allowedValues().isEmpty() && !paramType.allowedValues().contains(param.value())) {
        QStringList allowedValues;
        foreach (const QVariant &value, paramType.allowedValues()) {
            allowedValues.append(value.toString());
        }

        qCWarning(dcThing) << "Value not in allowed values for param" << param.paramTypeId().toString() << " Got:" << param.value() << " Allowed:" << allowedValues.join(",");
        return Thing::ThingErrorInvalidParameter;
    }

    return Thing::ThingErrorNoError;
}

Interfaces ThingUtils::allInterfaces()
{
    Interfaces ret;
    QDir dir(":/interfaces/");
    foreach (const QFileInfo &ifaceFile, dir.entryInfoList()) {
        ret.append(loadInterface(ifaceFile.baseName()));
    }
    return ret;
}

Interface ThingUtils::loadInterface(const QString &name)
{
    Interface iface;
    QFile f(QString(":/interfaces/%1.json").arg(name));
    if (!f.open(QFile::ReadOnly)) {
        qCWarning(dcThingManager()) << "Failed to load interface" << name;
        return iface;
    }
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(f.readAll(), &error);
    if (error.error != QJsonParseError::NoError) {
        qCWarning(dcThingManager) << "Cannot load interface definition for interface" << name << ":" << error.errorString();
        return iface;
    }
    QVariantMap content = jsonDoc.toVariant().toMap();
    if (content.contains("extends")) {
        if (!content.value("extends").toString().isEmpty()) {
            iface = loadInterface(content.value("extends").toString());
        } else if (content.value("extends").toList().count() > 0) {
            foreach (const QVariant &extendedIface, content.value("extends").toList()) {
                Interface tmp = loadInterface(extendedIface.toString());
                iface = mergeInterfaces(iface, tmp);
            }
        }
    }

    InterfaceParamTypes paramTypes;
    InterfaceStateTypes stateTypes;
    InterfaceActionTypes actionTypes;
    InterfaceEventTypes eventTypes;

    foreach (const QVariant &paramVariant, content.value("params").toList()) {
        QVariantMap paramMap = paramVariant.toMap();

        InterfaceParamType paramType;
        paramType.setName(paramMap.value("name").toString());
        paramType.setType(QVariant::nameToType(paramMap.value("type").toByteArray()));
        paramType.setMinValue(paramMap.value("minValue"));
        paramType.setMaxValue(paramMap.value("maxValue"));
        paramType.setDefaultValue(paramMap.value("defaultValue"));
        paramType.setAllowedValues(paramMap.value("allowedValues").toList());
        paramType.setReadOnly(paramMap.value("readOnly").toBool());
        //paramType.setOptional(paramMap.value("optional", false).toBool());

        if (paramMap.contains("unit")) {
            QMetaEnum unitEnum = QMetaEnum::fromType<Types::Unit>();
            int enumValue = unitEnum.keyToValue("Unit" + paramMap.value("unit").toByteArray());
            if (enumValue == -1) {
                qCWarning(dcThingManager) << "Invalid unit" << paramMap.value("unit").toString() << "in interface" << name;
            } else {
                paramType.setUnit(static_cast<Types::Unit>(enumValue));
            }
        } else {
            paramType.setUnit(Types::UnitNone);
        }

        paramTypes.append(paramType);
    }

    foreach (const QVariant &stateVariant, content.value("states").toList()) {
        QVariantMap stateMap = stateVariant.toMap();
        InterfaceStateType stateType;
        stateType.setName(stateMap.value("name").toString());
        stateType.setType(QVariant::nameToType(stateMap.value("type").toByteArray()));
        stateType.setPossibleValues(stateMap.value("allowedValues").toList());
        stateType.setMinValue(stateMap.value("minValue"));
        stateType.setMaxValue(stateMap.value("maxValue"));
        stateType.setOptional(stateMap.value("optional", false).toBool());
        if (stateMap.contains("unit")) {
            QMetaEnum unitEnum = QMetaEnum::fromType<Types::Unit>();
            int enumValue = unitEnum.keyToValue("Unit" + stateMap.value("unit").toByteArray());
            if (enumValue == -1) {
                qCWarning(dcThingManager) << "Invalid unit" << stateMap.value("unit").toString() << "in interface" << name;
            } else {
                stateType.setUnit(static_cast<Types::Unit>(enumValue));
            }
        }

        ParamType stateChangeActionParamType;
        stateChangeActionParamType.setName(stateType.name());
        stateChangeActionParamType.setType(stateType.type());
        stateChangeActionParamType.setAllowedValues(stateType.possibleValues());
        stateChangeActionParamType.setMinValue(stateType.minValue());
        stateChangeActionParamType.setMaxValue(stateType.maxValue());

        if (stateMap.value("writable", false).toBool()) {
            InterfaceActionType stateChangeActionType;
            stateChangeActionType.setName(stateType.name());
            stateChangeActionType.setOptional(stateType.optional());
            stateChangeActionType.setParamTypes(ParamTypes() << stateChangeActionParamType);
            actionTypes.append(stateChangeActionType);
        }

        if (stateMap.contains("logged")) {
            stateType.setLoggingOverride(true);
            stateType.setSuggestLogging(stateMap.value("logged", false).toBool());
        }

        stateTypes.append(stateType);
    }

    foreach (const QVariant &actionVariant, content.value("actions").toList()) {
        InterfaceActionType actionType;
        actionType.setName(actionVariant.toMap().value("name").toString());
        actionType.setOptional(actionVariant.toMap().value("optional").toBool());
//        if (actionVariant.toMap().contains("logged")) {
//            actionType.setLoggingOverride(true);
//            actionType.setSuggestLogging(actionVariant.toMap().value("logged").toBool());
//        }
        ParamTypes paramTypes;
        foreach (const QVariant &actionParamVariant, actionVariant.toMap().value("params").toList()) {
            ParamType paramType;
            paramType.setName(actionParamVariant.toMap().value("name").toString());
            paramType.setType(QVariant::nameToType(actionParamVariant.toMap().value("type").toByteArray()));
            paramType.setAllowedValues(actionParamVariant.toMap().value("allowedValues").toList());
            paramType.setMinValue(actionParamVariant.toMap().value("min"));
            paramType.setDefaultValue(actionParamVariant.toMap().value("defaultValue"));
            paramTypes.append(paramType);
        }
        actionType.setParamTypes(paramTypes);
        actionTypes.append(actionType);
    }

    foreach (const QVariant &eventVariant, content.value("events").toList()) {
        InterfaceEventType eventType;
        eventType.setName(eventVariant.toMap().value("name").toString());
        eventType.setOptional(eventVariant.toMap().value("optional").toBool());
        if (eventVariant.toMap().contains("logged")) {
            eventType.setLoggingOverride(true);
            eventType.setSuggestLogging(eventVariant.toMap().value("logged").toBool());
        }
        ParamTypes paramTypes;
        foreach (const QVariant &eventParamVariant, eventVariant.toMap().value("params").toList()) {
            ParamType paramType;
            paramType.setName(eventParamVariant.toMap().value("name").toString());
            paramType.setType(QVariant::nameToType(eventParamVariant.toMap().value("type").toByteArray()));
            paramType.setAllowedValues(eventParamVariant.toMap().value("allowedValues").toList());
            paramType.setMinValue(eventParamVariant.toMap().value("minValue"));
            paramType.setMaxValue(eventParamVariant.toMap().value("maxValue"));
            paramType.setDefaultValue(eventParamVariant.toMap().value("defaultValue"));
            paramTypes.append(paramType);
        }
        eventType.setParamTypes(paramTypes);
        eventTypes.append(eventType);
    }

    return Interface(name, iface.paramTypes() << paramTypes, iface.actionTypes() << actionTypes, iface.eventTypes() << eventTypes, iface.stateTypes() << stateTypes);
}

Interface ThingUtils::mergeInterfaces(const Interface &iface1, const Interface &iface2)
{
    InterfaceParamTypes paramTypes = iface1.paramTypes();
    foreach (const InterfaceParamType &pt, iface2.paramTypes()) {
        if (paramTypes.findByName(pt.name()).name().isEmpty()) {
            paramTypes.append(pt);
        }
    }
    InterfaceEventTypes eventTypes = iface1.eventTypes();
    foreach (const InterfaceEventType &et, iface2.eventTypes()) {
        if (eventTypes.findByName(et.name()).name().isEmpty()) {
            eventTypes.append(et);
        }
    }
    InterfaceStateTypes stateTypes = iface1.stateTypes();
    foreach (const InterfaceStateType &st, iface2.stateTypes()) {
        if (stateTypes.findByName(st.name()).name().isEmpty()) {
            stateTypes.append(st);
        }
    }
    InterfaceActionTypes actionTypes = iface1.actionTypes();
    foreach (const InterfaceActionType &at, iface2.actionTypes()) {
        if (actionTypes.findByName(at.name()).name().isEmpty()) {
            actionTypes.append(at);
        }
    }
    return Interface(QString(), paramTypes, actionTypes, eventTypes, stateTypes);
}

QStringList ThingUtils::generateInterfaceParentList(const QString &interface)
{
    QFile f(QString(":/interfaces/%1.json").arg(interface));
    if (!f.open(QFile::ReadOnly)) {
        qCWarning(dcThingManager()) << "Failed to load interface" << interface;
        return QStringList();
    }
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(f.readAll(), &error);
    if (error.error != QJsonParseError::NoError) {
        qCWarning(dcThingManager) << "Cannot load interface definition for interface" << interface << ":" << error.errorString();
        return QStringList();
    }
    QStringList ret = {interface};
    QVariantMap content = jsonDoc.toVariant().toMap();
    if (content.contains("extends")) {
        if (!content.value("extends").toString().isEmpty()) {
            ret << generateInterfaceParentList(content.value("extends").toString());
        } else if (content.value("extends").toList().count() > 0) {
            foreach (const QVariant &extendedIface, content.value("extends").toList()) {
                ret << generateInterfaceParentList(extendedIface.toString());
            }
        }
    }
    return ret;
}
