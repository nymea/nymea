/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2019 Michael Zanetti <michael.zanetti@nymea.io>          *
 *                                                                         *
 *  This file is part of nymea.                                            *
 *                                                                         *
 *  This library is free software; you can redistribute it and/or          *
 *  modify it under the terms of the GNU Lesser General Public             *
 *  License as published by the Free Software Foundation; either           *
 *  version 2.1 of the License, or (at your option) any later version.     *
 *                                                                         *
 *  This library is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
 *  Lesser General Public License for more details.                        *
 *                                                                         *
 *  You should have received a copy of the GNU Lesser General Public       *
 *  License along with this library; If not, see                           *
 *  <http://www.gnu.org/licenses/>.                                        *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "deviceutils.h"
#include "loggingcategories.h"

#include <QDir>
#include <QFileInfo>
#include <QJsonParseError>

DeviceUtils::DeviceUtils()
{

}

/*! Verify if the given \a params matches the given \a paramTypes. Ith \a requireAll
 *  is true, all \l{ParamList}{Params} has to be valid. Returns \l{Device::DeviceError} to inform about the result.*/
Device::DeviceError DeviceUtils::verifyParams(const QList<ParamType> paramTypes, const ParamList &params)
{
    foreach (const Param &param, params) {
        Device::DeviceError result = verifyParam(paramTypes, param);
        if (result != Device::DeviceErrorNoError) {
            return result;
        }
    }
    foreach (const ParamType &paramType, paramTypes) {
        bool found = false;
        foreach (const Param &param, params) {
            if (paramType.id() == param.paramTypeId()) {
                found = true;
                break;
            }
        }

        if (!found) {
            qCWarning(dcDevice) << "Missing parameter:" << paramType.name() << params;
            return Device::DeviceErrorMissingParameter;
        }
    }
    return Device::DeviceErrorNoError;
}

/*! Verify if the given \a param matches one of the given \a paramTypes. Returns \l{Device::DeviceError} to inform about the result.*/
Device::DeviceError DeviceUtils::verifyParam(const QList<ParamType> paramTypes, const Param &param)
{
    foreach (const ParamType &paramType, paramTypes) {
        if (paramType.id() == param.paramTypeId()) {
            return verifyParam(paramType, param);
        }
    }

    qCWarning(dcDevice) << "Invalid parameter" << param.paramTypeId().toString() << "in parameter list";
    return Device::DeviceErrorInvalidParameter;
}

/*! Verify if the given \a param matches the given \a paramType. Returns \l{Device::DeviceError} to inform about the result.*/
Device::DeviceError DeviceUtils::verifyParam(const ParamType &paramType, const Param &param)
{
    if (paramType.id() != param.paramTypeId()) {
        qCWarning(dcDevice) << "Parameter id" << param.paramTypeId().toString() << "does not match with ParamType id" << paramType.id().toString();
        return Device::DeviceErrorInvalidParameter;
    }

    if (!param.value().canConvert(static_cast<int>(paramType.type()))) {
        qCWarning(dcDevice) << "Wrong parameter type for param" << param.paramTypeId().toString() << " Got:" << param.value() << " Expected:" << QVariant::typeToName(static_cast<int>(paramType.type()));
        return Device::DeviceErrorInvalidParameter;
    }

    if (!param.value().convert(static_cast<int>(paramType.type()))) {
        qCWarning(dcDevice) << "Could not convert value of param" << param.paramTypeId().toString() << " to:" << QVariant::typeToName(static_cast<int>(paramType.type())) << " Got:" << param.value();
        return Device::DeviceErrorInvalidParameter;
    }

    if (paramType.type() == QVariant::Int) {
        if (paramType.maxValue().isValid() && param.value().toInt() > paramType.maxValue().toInt()) {
            qCWarning(dcDevice) << "Value out of range for param" << param.paramTypeId().toString() << " Got:" << param.value() << " Max:" << paramType.maxValue();
            return Device::DeviceErrorInvalidParameter;
        }

        if (paramType.minValue().isValid() && param.value().toInt() < paramType.minValue().toInt()) {
            qCWarning(dcDevice) << "Value out of range for param" << param.paramTypeId().toString() << " Got:" << param.value() << " Min:" << paramType.minValue();
            return Device::DeviceErrorInvalidParameter;
        }
    } else if (paramType.type() == QVariant::UInt) {
        if (paramType.maxValue().isValid() && param.value().toUInt() > paramType.maxValue().toUInt()) {
            qCWarning(dcDevice) << "Value out of range for param" << param.paramTypeId().toString() << " Got:" << param.value() << " Max:" << paramType.maxValue();
            return Device::DeviceErrorInvalidParameter;
        }

        if (paramType.minValue().isValid() && param.value().toUInt() < paramType.minValue().toUInt()) {
            qCWarning(dcDevice) << "Value out of range for param" << param.paramTypeId().toString() << " Got:" << param.value() << " Min:" << paramType.minValue();
            return Device::DeviceErrorInvalidParameter;
        }
    } else if (paramType.type() == QVariant::Double) {
        if (paramType.maxValue().isValid() && param.value().toDouble() > paramType.maxValue().toDouble()) {
            qCWarning(dcDevice) << "Value out of range for param" << param.paramTypeId().toString() << " Got:" << param.value() << " Max:" << paramType.maxValue();
            return Device::DeviceErrorInvalidParameter;
        }

        if (paramType.minValue().isValid() && param.value().toDouble() < paramType.minValue().toDouble()) {
            qCWarning(dcDevice) << "Value out of range for param" << param.paramTypeId().toString() << " Got:" << param.value() << " Min:" << paramType.minValue();
            return Device::DeviceErrorInvalidParameter;
        }
    } else {
        if (paramType.maxValue().isValid() && param.value() > paramType.maxValue()) {
            qCWarning(dcDevice) << "Value out of range for param" << param.paramTypeId().toString() << " Got:" << param.value() << " Max:" << paramType.maxValue();
            return Device::DeviceErrorInvalidParameter;
        }

        if (paramType.minValue().isValid() && param.value() < paramType.minValue()) {
            qCWarning(dcDevice) << "Value out of range for param" << param.paramTypeId().toString() << " Got:" << param.value() << " Min:" << paramType.minValue();
            return Device::DeviceErrorInvalidParameter;
        }
    }

    if (!paramType.allowedValues().isEmpty() && !paramType.allowedValues().contains(param.value())) {
        QStringList allowedValues;
        foreach (const QVariant &value, paramType.allowedValues()) {
            allowedValues.append(value.toString());
        }

        qCWarning(dcDevice) << "Value not in allowed values for param" << param.paramTypeId().toString() << " Got:" << param.value() << " Allowed:" << allowedValues.join(",");
        return Device::DeviceErrorInvalidParameter;
    }

    return Device::DeviceErrorNoError;
}

Interfaces DeviceUtils::allInterfaces()
{
    Interfaces ret;
    QDir dir(":/interfaces/");
    foreach (const QFileInfo &ifaceFile, dir.entryInfoList()) {
        ret.append(loadInterface(ifaceFile.baseName()));
    }
    return ret;
}

Interface DeviceUtils::loadInterface(const QString &name)
{
    Interface iface;
    QFile f(QString(":/interfaces/%1.json").arg(name));
    if (!f.open(QFile::ReadOnly)) {
        qCWarning(dcDeviceManager()) << "Failed to load interface" << name;
        return iface;
    }
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(f.readAll(), &error);
    if (error.error != QJsonParseError::NoError) {
        qCWarning(dcDeviceManager) << "Cannot load interface definition for interface" << name << ":" << error.errorString();
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

    StateTypes stateTypes;
    ActionTypes actionTypes;
    EventTypes eventTypes;
    foreach (const QVariant &stateVariant, content.value("states").toList()) {
        StateType stateType;
        stateType.setName(stateVariant.toMap().value("name").toString());
        stateType.setType(QVariant::nameToType(stateVariant.toMap().value("type").toByteArray()));
        stateType.setPossibleValues(stateVariant.toMap().value("allowedValues").toList());
        stateType.setMinValue(stateVariant.toMap().value("minValue"));
        stateType.setMaxValue(stateVariant.toMap().value("maxValue"));
        stateTypes.append(stateType);

        EventType stateChangeEventType;
        stateChangeEventType.setName(stateType.name());
        ParamType stateChangeEventParamType;
        stateChangeEventParamType.setName(stateType.name());
        stateChangeEventParamType.setType(stateType.type());
        stateChangeEventParamType.setAllowedValues(stateType.possibleValues());
        stateChangeEventParamType.setMinValue(stateType.minValue());
        stateChangeEventParamType.setMaxValue(stateType.maxValue());
        stateChangeEventType.setParamTypes(ParamTypes() << stateChangeEventParamType);
        eventTypes.append(stateChangeEventType);

        if (stateVariant.toMap().value("writable", false).toBool()) {
            ActionType stateChangeActionType;
            stateChangeActionType.setName(stateType.name());
            stateChangeActionType.setParamTypes(ParamTypes() << stateChangeEventParamType);
            actionTypes.append(stateChangeActionType);
        }
    }

    foreach (const QVariant &actionVariant, content.value("actions").toList()) {
        ActionType actionType;
        actionType.setName(actionVariant.toMap().value("name").toString());
        ParamTypes paramTypes;
        foreach (const QVariant &actionParamVariant, actionVariant.toMap().value("params").toList()) {
            ParamType paramType;
            paramType.setName(actionParamVariant.toMap().value("name").toString());
            paramType.setType(QVariant::nameToType(actionParamVariant.toMap().value("type").toByteArray()));
            paramType.setAllowedValues(actionParamVariant.toMap().value("allowedValues").toList());
            paramType.setMinValue(actionParamVariant.toMap().value("min"));
            paramTypes.append(paramType);
        }
        actionType.setParamTypes(paramTypes);
        actionTypes.append(actionType);
    }

    foreach (const QVariant &eventVariant, content.value("events").toList()) {
        EventType eventType;
        eventType.setName(eventVariant.toMap().value("name").toString());
        ParamTypes paramTypes;
        foreach (const QVariant &eventParamVariant, eventVariant.toMap().value("params").toList()) {
            ParamType paramType;
            paramType.setName(eventParamVariant.toMap().value("name").toString());
            paramType.setType(QVariant::nameToType(eventParamVariant.toMap().value("type").toByteArray()));
            paramType.setAllowedValues(eventParamVariant.toMap().value("allowedValues").toList());
            paramType.setMinValue(eventParamVariant.toMap().value("minValue"));
            paramType.setMaxValue(eventParamVariant.toMap().value("maxValue"));
            paramTypes.append(paramType);
        }
        eventType.setParamTypes(paramTypes);
        eventTypes.append(eventType);
    }

    return Interface(name, iface.actionTypes() << actionTypes, iface.eventTypes() << eventTypes, iface.stateTypes() << stateTypes);
}

Interface DeviceUtils::mergeInterfaces(const Interface &iface1, const Interface &iface2)
{
    EventTypes eventTypes = iface1.eventTypes();
    foreach (const EventType &et, iface2.eventTypes()) {
        if (eventTypes.findByName(et.name()).name().isEmpty()) {
            eventTypes.append(et);
        }
    }
    StateTypes stateTypes = iface1.stateTypes();
    foreach (const StateType &st, iface2.stateTypes()) {
        if (stateTypes.findByName(st.name()).name().isEmpty()) {
            stateTypes.append(st);
        }
    }
    ActionTypes actionTypes = iface1.actionTypes();
    foreach (const ActionType &at, iface2.actionTypes()) {
        if (actionTypes.findByName(at.name()).name().isEmpty()) {
            actionTypes.append(at);
        }
    }
    return Interface(QString(), actionTypes, eventTypes, stateTypes);
}

QStringList DeviceUtils::generateInterfaceParentList(const QString &interface)
{
    QFile f(QString(":/interfaces/%1.json").arg(interface));
    if (!f.open(QFile::ReadOnly)) {
        qCWarning(dcDeviceManager()) << "Failed to load interface" << interface;
        return QStringList();
    }
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(f.readAll(), &error);
    if (error.error != QJsonParseError::NoError) {
        qCWarning(dcDeviceManager) << "Cannot load interface definition for interface" << interface << ":" << error.errorString();
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
