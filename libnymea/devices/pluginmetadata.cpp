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

#include "pluginmetadata.h"
#include "deviceutils.h"

#include "loggingcategories.h"

#include "types/interface.h"

#include <QJsonObject>
#include <QJsonArray>
#include <QMetaObject>
#include <QMetaEnum>

PluginMetadata::PluginMetadata()
{

}

PluginMetadata::PluginMetadata(const QJsonObject &jsonObject, bool isBuiltIn): m_isBuiltIn(isBuiltIn)
{
    parse(jsonObject);
}

bool PluginMetadata::isValid() const
{
    return m_isValid;
}

PluginId PluginMetadata::pluginId() const
{
    return m_pluginId;
}

QString PluginMetadata::pluginName() const
{
    return m_pluginName;
}

QString PluginMetadata::pluginDisplayName() const
{
    return m_pluginDisplayName;
}

bool PluginMetadata::isBuiltIn() const
{
    return m_isBuiltIn;
}

ParamTypes PluginMetadata::pluginSettings() const
{
    return m_pluginSettings;
}

Vendors PluginMetadata::vendors() const
{
    return m_vendors;
}

DeviceClasses PluginMetadata::deviceClasses() const
{
    return m_deviceClasses;
}

void PluginMetadata::parse(const QJsonObject &jsonObject)
{
    // General plugin info
    QStringList pluginMandatoryJsonProperties = QStringList() << "id" << "name" << "displayName" << "vendors";
    QStringList pluginJsonProperties = QStringList() << "id" << "name" << "displayName" << "vendors" << "paramTypes" << "builtIn";
    QPair<QStringList, QStringList> verificationResult = verifyFields(pluginJsonProperties, pluginMandatoryJsonProperties, jsonObject);
    if (!verificationResult.first.isEmpty()) {
        qCWarning(dcDevice()) << pluginName() << "Skipping plugin because of missing fields:" << verificationResult.first.join(", ") << endl << jsonObject;
        return;
    }
    if (!verificationResult.second.isEmpty()) {
        qCWarning(dcDevice()) << pluginName() << "Skipping plugin because of unknown fields:" << verificationResult.second.join(", ") << endl << jsonObject;
        return;
    }

    m_pluginId = jsonObject.value("id").toString();
    m_pluginName = jsonObject.value("name").toString();
    m_pluginDisplayName = jsonObject.value("displayName").toString();

    // Mandatory fields available... All the rest will be skipped if not valid, but it won't invalidate the entire meta data
    m_isValid = true;

    // parse plugin configuration params
    if (jsonObject.contains("paramTypes")) {
        QPair<bool, QList<ParamType> > paramVerification = parseParamTypes(jsonObject.value("paramTypes").toArray());
        if (paramVerification.first) {
            m_pluginSettings = paramVerification.second;
        }
    }

    // Load vendors
    foreach (const QJsonValue &vendorJson, jsonObject.value("vendors").toArray()) {
        bool broken = false;
        QJsonObject vendorObject = vendorJson.toObject();

        QStringList vendorMandatoryJsonProperties = QStringList() << "id" << "name" << "displayName" << "deviceClasses";
        QStringList vendorJsonProperties = QStringList() << "id" << "name" << "displayName" << "deviceClasses";

        QPair<QStringList, QStringList> verificationResult = verifyFields(vendorJsonProperties, vendorMandatoryJsonProperties, vendorObject);

        // Check mandatory fields
        if (!verificationResult.first.isEmpty()) {
            qCWarning(dcDevice()) << pluginName() << "Skipping vendor because of missing fields:" << verificationResult.first.join(", ") << endl << vendorObject;
            broken = true;
            break;
        }

        // Check if there are any unknown fields
        if (!verificationResult.second.isEmpty()) {
            qCWarning(dcDevice()) << pluginName() << "Skipping vendor because of unknown fields:" << verificationResult.second.join(", ") << endl << vendorObject;
            broken = true;
            break;
        }

        VendorId vendorId = VendorId(vendorObject.value("id").toString());
        Vendor vendor(vendorId, vendorObject.value("name").toString());
        vendor.setDisplayName(vendorObject.value("displayName").toString());
        m_vendors.append(vendor);

        // Load deviceclasses of this vendor
        foreach (const QJsonValue &deviceClassJson, vendorJson.toObject().value("deviceClasses").toArray()) {
            QJsonObject deviceClassObject = deviceClassJson.toObject();
            /*! Returns a list of all valid JSON properties a DeviceClass JSON definition can have. */
            QStringList deviceClassProperties = QStringList() << "id" << "name" << "displayName" << "createMethods" << "setupMethod"
                                     << "interfaces" << "pairingInfo" << "discoveryParamTypes" << "discoveryParamTypes"
                                     << "paramTypes" << "settingsTypes" << "stateTypes" << "actionTypes" << "eventTypes";
            QStringList mandatoryDeviceClassProperties = QStringList() << "id" << "name" << "displayName";

            QPair<QStringList, QStringList> verificationResult = verifyFields(deviceClassProperties, mandatoryDeviceClassProperties, deviceClassObject);

            // Check mandatory fields
            if (!verificationResult.first.isEmpty()) {
                qCWarning(dcDevice()) << pluginName() << "Skipping device class because of missing fields:" << verificationResult.first.join(", ") << endl << deviceClassObject;
                broken = true;
                break;
            }

            // Check if there are any unknown fields
            if (!verificationResult.second.isEmpty()) {
                qCWarning(dcDevice()) << pluginName() << "Skipping device class because of unknown fields:" << verificationResult.second.join(", ") << endl << deviceClassObject;
                broken = true;
                break;
            }

            DeviceClass deviceClass(pluginId(), vendorId, deviceClassObject.value("id").toString());
            deviceClass.setName(deviceClassObject.value("name").toString());
            deviceClass.setDisplayName(deviceClassObject.value("displayName").toString());

            // Read create methods
            DeviceClass::CreateMethods createMethods;
            if (!deviceClassObject.contains("createMethods")) {
                // Default if not specified
                createMethods |= DeviceClass::CreateMethodUser;
            } else {
                foreach (const QJsonValue &createMethodValue, deviceClassObject.value("createMethods").toArray()) {
                    if (createMethodValue.toString().toLower() == "discovery") {
                        createMethods |= DeviceClass::CreateMethodDiscovery;
                    } else if (createMethodValue.toString().toLower() == "auto") {
                        createMethods |= DeviceClass::CreateMethodAuto;
                    } else if (createMethodValue.toString().toLower() == "user") {
                        createMethods |= DeviceClass::CreateMethodUser;
                    } else {
                        qCWarning(dcDevice()) << "Unknown createMehtod" << createMethodValue.toString() << "in deviceClass "
                                                     << deviceClass.name() << ". Falling back to CreateMethodUser.";
                        createMethods |= DeviceClass::CreateMethodUser;
                    }
                }
            }
            deviceClass.setCreateMethods(createMethods);

            // Read params
            QPair<bool, QList<ParamType> > paramTypesVerification = parseParamTypes(deviceClassObject.value("paramTypes").toArray());
            if (!paramTypesVerification.first) {
                broken = true;
                break;
            } else {
                deviceClass.setParamTypes(paramTypesVerification.second);
            }

            // Read settings
            QPair<bool, QList<ParamType> > settingsTypesVerification = parseParamTypes(deviceClassObject.value("settingsTypes").toArray());
            if (!settingsTypesVerification.first) {
                broken = true;
                break;
            } else {
                deviceClass.setSettingsTypes(settingsTypesVerification.second);
            }

            // Read discover params
            QPair<bool, QList<ParamType> > discoveryParamVerification = parseParamTypes(deviceClassObject.value("discoveryParamTypes").toArray());
            if (!discoveryParamVerification.first) {
                broken = true;
                break;
            } else {
                deviceClass.setDiscoveryParamTypes(discoveryParamVerification.second);
            }

            // Read setup method
            DeviceClass::SetupMethod setupMethod = DeviceClass::SetupMethodJustAdd;
            if (deviceClassObject.contains("setupMethod")) {
                QString setupMethodString = deviceClassObject.value("setupMethod").toString();
                if (setupMethodString.toLower() == "pushbutton") {
                    setupMethod = DeviceClass::SetupMethodPushButton;
                } else if (setupMethodString.toLower() == "displaypin") {
                    setupMethod = DeviceClass::SetupMethodDisplayPin;
                } else if (setupMethodString.toLower() == "enterpin") {
                    setupMethod = DeviceClass::SetupMethodEnterPin;
                } else if (setupMethodString.toLower() == "justadd") {
                    setupMethod = DeviceClass::SetupMethodJustAdd;
                } else {
                    qCWarning(dcDevice()) << "Unknown setupMehtod" << setupMethod << "in deviceClass"
                                               << deviceClass.name() << ". Falling back to SetupMethodJustAdd.";
                    setupMethod = DeviceClass::SetupMethodJustAdd;
                }
            }
            deviceClass.setSetupMethod(setupMethod);

            // Read pairing info
            deviceClass.setPairingInfo(deviceClassObject.value("pairingInfo").toString());

            QList<ActionType> actionTypes;
            QList<StateType> stateTypes;
            QList<EventType> eventTypes;

            // Read StateTypes
            int index = 0;
            foreach (const QJsonValue &stateTypesJson, deviceClassObject.value("stateTypes").toArray()) {
                QJsonObject st = stateTypesJson.toObject();
                bool writableState = false;

                QPair<QStringList, QStringList> verificationResult = verifyFields(StateType::typeProperties(), StateType::mandatoryTypeProperties(), st);

                // Check mandatory fields
                if (!verificationResult.first.isEmpty()) {
                    qCWarning(dcDevice()) << "Skipping device class" << deviceClass.name() << "because of missing" << verificationResult.first.join(", ") << "in stateType" << st;
                    broken = true;
                    break;
                }

                // Check if there are any unknown fields
                if (!verificationResult.second.isEmpty()) {
                    qCWarning(dcDevice()) << "Skipping device class" << deviceClass.name() << "because of unknown properties" << verificationResult.second.join(", ") << "in stateType" << st;
                    broken = true;
                    break;
                }

                // If this is a writable stateType, there must be also the displayNameAction property
                if (st.contains("writable") && st.value("writable").toBool()) {
                    writableState = true;
                    if (!st.contains("displayNameAction")) {
                        qCWarning(dcDevice()) << "Skipping device class" << deviceClass.name() << ". The state is writable, but does not define the displayNameAction property" << st;
                        broken = true;
                        break;
                    }
                }

                QVariant::Type t = QVariant::nameToType(st.value("type").toString().toLatin1().data());
                if (t == QVariant::Invalid) {
                    qCWarning(dcDevice()) << "Invalid StateType type:" << st.value("type").toString();
                    broken = true;
                    break;
                }

                StateType stateType(st.value("id").toString());
                stateType.setName(st.value("name").toString());
                stateType.setDisplayName(st.value("displayName").toString());
                stateType.setIndex(index++);
                stateType.setType(t);
                QPair<bool, Types::Unit> unitVerification = loadAndVerifyUnit(st.value("unit").toString());
                if (!unitVerification.first) {
                    broken = true;
                    break;
                } else {
                    stateType.setUnit(unitVerification.second);
                }

                stateType.setDefaultValue(st.value("defaultValue").toVariant());
                if (st.contains("minValue"))
                    stateType.setMinValue(st.value("minValue").toVariant());

                if (st.contains("maxValue"))
                    stateType.setMaxValue(st.value("maxValue").toVariant());

                if (st.contains("possibleValues")) {
                    QVariantList possibleValues;
                    foreach (const QJsonValue &possibleValueJson, st.value("possibleValues").toArray()) {
                        possibleValues.append(possibleValueJson.toVariant());
                    }
                    stateType.setPossibleValues(possibleValues);

                    if (!stateType.possibleValues().contains(stateType.defaultValue())) {
                        qCWarning(dcDevice()) << QString("\"%1\" plugin:").arg(pluginName()).toLatin1().data() << QString("The given default value \"%1\" is not in the possible values of the stateType \"%2\".")
                                                        .arg(stateType.defaultValue().toString()).arg(stateType.name()).toLatin1().data();
                        broken = true;
                        break;
                    }
                }

                if (st.contains("cached")) {
                    stateType.setCached(st.value("cached").toBool());
                }
                stateTypes.append(stateType);

                // Events for state changed
                EventType eventType(EventTypeId(stateType.id().toString()));
                eventType.setName(st.value("name").toString());
                eventType.setDisplayName(st.value("displayNameEvent").toString());
                ParamType paramType(ParamTypeId(stateType.id().toString()), st.value("name").toString(), stateType.type());
                paramType.setDisplayName(st.value("displayName").toString());
                paramType.setAllowedValues(stateType.possibleValues());
                paramType.setDefaultValue(stateType.defaultValue());
                paramType.setMinValue(stateType.minValue());
                paramType.setMaxValue(stateType.maxValue());
                paramType.setUnit(stateType.unit());
                eventType.setParamTypes(QList<ParamType>() << paramType);
                eventType.setIndex(stateType.index());
                eventTypes.append(eventType);

                // ActionTypes for writeable StateTypes
                if (writableState) {
                    ActionType actionType(ActionTypeId(stateType.id().toString()));
                    actionType.setName(stateType.name());
                    actionType.setDisplayName(st.value("displayNameAction").toString());
                    actionType.setIndex(stateType.index());
                    actionType.setParamTypes(QList<ParamType>() << paramType);
                    actionTypes.append(actionType);
                }
            }
            deviceClass.setStateTypes(stateTypes);

            // ActionTypes
            index = 0;
            foreach (const QJsonValue &actionTypesJson, deviceClassObject.value("actionTypes").toArray()) {
                QJsonObject at = actionTypesJson.toObject();
                QPair<QStringList, QStringList> verificationResult = verifyFields(ActionType::typeProperties(), ActionType::mandatoryTypeProperties(), at);

                // Check mandatory fields
                if (!verificationResult.first.isEmpty()) {
                    qCWarning(dcDevice()) << pluginName() << "Skipping device class" << deviceClass.name() << "because of missing" << verificationResult.first.join(", ") << "in action type:" << endl << at;
                    broken = true;
                    break;
                }

                // Check if there are any unknown fields
                if (!verificationResult.second.isEmpty()) {
                    qCWarning(dcDevice()) << pluginName() << "Skipping device class" << deviceClass.name() << "because of unknown fields:" << verificationResult.second.join(", ") << "in action type:" << endl << at;
                    broken = true;
                    break;
                }

                ActionType actionType(at.value("id").toString());
                actionType.setName(at.value("name").toString());
                actionType.setDisplayName(at.value("displayName").toString());
                actionType.setIndex(index++);

                QPair<bool, QList<ParamType> > paramVerification = parseParamTypes(at.value("paramTypes").toArray());
                if (!paramVerification.first) {
                    broken = true;
                    break;
                } else {
                    actionType.setParamTypes(paramVerification.second);
                }

                actionTypes.append(actionType);
            }
            deviceClass.setActionTypes(actionTypes);

            // EventTypes
            index = 0;
            foreach (const QJsonValue &eventTypesJson, deviceClassObject.value("eventTypes").toArray()) {
                QJsonObject et = eventTypesJson.toObject();

                QPair<QStringList, QStringList> verificationResult = verifyFields(EventType::typeProperties(), EventType::mandatoryTypeProperties(), et);

                // Check mandatory fields
                if (!verificationResult.first.isEmpty()) {
                    qCWarning(dcDevice()) << pluginName() << "Skipping device class" << deviceClass.name() << "because of missing" << verificationResult.first.join(", ") << "in event type:" << endl << et;
                    broken = true;
                    break;
                }

                // Check if there are any unknown fields
                if (!verificationResult.second.isEmpty()) {
                    qCWarning(dcDevice()) << pluginName() << "Skipping device class" << deviceClass.name() << "because of unknown fields:" << verificationResult.second.join(", ") << "in event type:" << endl << et;
                    broken = true;
                    break;
                }

                EventType eventType(et.value("id").toString());
                eventType.setName(et.value("name").toString());
                eventType.setDisplayName(et.value("displayName").toString());
                eventType.setIndex(index++);

                QPair<bool, QList<ParamType> > paramVerification = parseParamTypes(et.value("paramTypes").toArray());
                if (!paramVerification.first) {
                    broken = true;
                    break;
                } else {
                    eventType.setParamTypes(paramVerification.second);
                }
                eventTypes.append(eventType);
            }
            deviceClass.setEventTypes(eventTypes);

            // Read interfaces
            QStringList interfaces;
            foreach (const QJsonValue &value, deviceClassObject.value("interfaces").toArray()) {
                Interface iface = DeviceUtils::loadInterface(value.toString());

                StateTypes stateTypes(deviceClass.stateTypes());
                ActionTypes actionTypes(deviceClass.actionTypes());
                EventTypes eventTypes(deviceClass.eventTypes());

                bool valid = true;
                foreach (const StateType &ifaceStateType, iface.stateTypes()) {
                    StateType stateType = stateTypes.findByName(ifaceStateType.name());
                    if (stateType.id().isNull()) {
                        qCWarning(dcDevice()) << "DeviceClass" << deviceClass.name() << "claims to implement interface" << value.toString() << "but doesn't implement state" << ifaceStateType.name();
                        valid = false;
                        continue;
                    }
                    if (ifaceStateType.type() != stateType.type()) {
                        qCWarning(dcDevice()) << "DeviceClass" << deviceClass.name() << "claims to implement interface" << value.toString() << "but state" << stateType.name() << "has not matching type" << stateType.type() << "!=" << ifaceStateType.type();
                        valid = false;
                        continue;
                    }
                    if (ifaceStateType.minValue().isValid() && !ifaceStateType.minValue().isNull()) {
                        if (ifaceStateType.minValue().toString() == "any") {
                            if (stateType.minValue().isNull()) {
                                qCWarning(dcDevice()) << "DeviceClass" << deviceClass.name() << "claims to implement interface" << value.toString() << "but state" << stateType.name() << "has no minimum value defined.";
                                valid = false;
                                continue;
                            }
                        } else if (ifaceStateType.minValue() != stateType.minValue()) {
                            qCWarning(dcDevice()) << "DeviceClass" << deviceClass.name() << "claims to implement interface" << value.toString() << "but state" << stateType.name() << "has not matching minimum value:" << ifaceStateType.minValue() << "!=" << stateType.minValue();
                            valid = false;
                            continue;
                        }
                    }
                    if (ifaceStateType.maxValue().isValid() && !ifaceStateType.maxValue().isNull()) {
                        if (ifaceStateType.maxValue().toString() == "any") {
                            if (stateType.maxValue().isNull()) {
                                qCWarning(dcDevice()) << "DeviceClass" << deviceClass.name() << "claims to implement interface" << value.toString() << "but state" << stateType.name() << "has no maximum value defined.";
                                valid = false;
                                continue;
                            }
                        } else if (ifaceStateType.maxValue() != stateType.maxValue()) {
                            qCWarning(dcDevice()) << "DeviceClass" << deviceClass.name() << "claims to implement interface" << value.toString() << "but state" << stateType.name() << "has not matching maximum value:" << ifaceStateType.maxValue() << "!=" << stateType.minValue();
                            valid = false;
                            continue;
                        }
                    }
                    if (!ifaceStateType.possibleValues().isEmpty() && ifaceStateType.possibleValues() != stateType.possibleValues()) {
                        qCWarning(dcDevice()) << "DeviceClass" << deviceClass.name() << "claims to implement interface" << value.toString() << "but state" << stateType.name() << "has not matching allowed values" << ifaceStateType.possibleValues() << "!=" << stateType.possibleValues();
                        valid = false;
                        continue;
                    }
                }

                foreach (const ActionType &ifaceActionType, iface.actionTypes()) {
                    ActionType actionType = actionTypes.findByName(ifaceActionType.name());
                    if (actionType.id().isNull()) {
                        qCWarning(dcDevice) << "DeviceClass" << deviceClass.name() << "claims to implement interface" << value.toString() << "but doesn't implement action" << ifaceActionType.name();
                        valid = false;
                    }
                    foreach (const ParamType &ifaceActionParamType, ifaceActionType.paramTypes()) {
                        ParamType paramType = actionType.paramTypes().findByName(ifaceActionParamType.name());
                        if (!paramType.isValid()) {
                            qCWarning(dcDevice) << "DeviceClass" << deviceClass.name() << "claims to implement interface" << value.toString() << "but doesn't implement action param" << ifaceActionType.name() << ":" << ifaceActionParamType.name();
                            valid = false;
                        } else {
                            if (paramType.type() != ifaceActionParamType.type()) {
                                qCWarning(dcDevice()) << "DeviceClass" << deviceClass.name() << "claims to implement interface" << value.toString() << "but param" << paramType.name() << "is of wrong type:" << QVariant::typeToName(paramType.type()) << "expected:" << QVariant::typeToName(ifaceActionParamType.type());
                                valid = false;
                            }
                        }
                    }
                }

                foreach (const EventType &ifaceEventType, iface.eventTypes()) {
                    EventType eventType = eventTypes.findByName(ifaceEventType.name());
                    if (!eventType.isValid()) {
                        qCWarning(dcDevice) << "DeviceClass" << deviceClass.name() << "claims to implement interface" << value.toString() << "but doesn't implement event" << ifaceEventType.name();
                        valid = false;
                    }
                    foreach (const ParamType &ifaceEventParamType, ifaceEventType.paramTypes()) {
                        ParamType paramType = eventType.paramTypes().findByName(ifaceEventParamType.name());
                        if (!paramType.isValid()) {
                            qCWarning(dcDevice) << "DeviceClass" << deviceClass.name() << "claims to implement interface" << value.toString() << "but doesn't implement event param" << ifaceEventType.name() << ":" << ifaceEventParamType.name();
                            valid = false;
                        } else {
                            if (paramType.type() != ifaceEventParamType.type()) {
                                qCWarning(dcDevice()) << "DeviceClass" << deviceClass.name() << "claims to implement interface" << value.toString() << "but param" << paramType.name() << "is of wrong type:" << QVariant::typeToName(paramType.type()) << "expected:" << QVariant::typeToName(ifaceEventParamType.type());
                                valid = false;
                            }
                        }
                    }
                }

                if (valid) {
                    interfaces.append(DeviceUtils::generateInterfaceParentList(value.toString()));
                }
            }
            interfaces.removeDuplicates();
            deviceClass.setInterfaces(interfaces);

            if (!broken) {
                m_deviceClasses.append(deviceClass);
            } else {
                qCWarning(dcDevice()) << "Skipping device class" << deviceClass.name();
            }
        }
    }
}

QPair<bool, Types::Unit> PluginMetadata::loadAndVerifyUnit(const QString &unitString)
{
    if (unitString.isEmpty())
        return QPair<bool, Types::Unit>(true, Types::UnitNone);

    QMetaObject metaObject = Types::staticMetaObject;
    int enumIndex = metaObject.indexOfEnumerator(QString("Unit").toLatin1().data());
    QMetaEnum metaEnum = metaObject.enumerator(enumIndex);

    int enumValue = -1;
    for (int i = 0; i < metaEnum.keyCount(); i++) {
        if (QString(metaEnum.valueToKey(metaEnum.value(i))) == QString("Unit" + unitString)) {
            enumValue = metaEnum.value(i);
            break;
        }
    }

    // inform the plugin developer about the error in the plugin json file
    if (enumValue == -1) {
        qCWarning(dcDeviceManager()) << QString("\"%1\" plugin:").arg(pluginName()).toLatin1().data() << QString("Invalid unit type \"%1\" in json file.").arg(unitString).toLatin1().data();
        return QPair<bool, Types::Unit>(false, Types::UnitNone);
    }

    return QPair<bool, Types::Unit>(true, (Types::Unit)enumValue);
}

QPair<QStringList, QStringList> PluginMetadata::verifyFields(const QStringList &possibleFields, const QStringList &mandatoryFields, const QJsonObject &value)
{
    QStringList missingFields;
    QStringList unknownFields;

    // Check if we have an unknown field
    foreach (const QString &property, value.keys()) {
        if (!possibleFields.contains(property)) {
            unknownFields << property;
        }
    }

    // Check if a mandatory field is missing
    foreach (const QString &field, mandatoryFields) {
        if (!value.contains(field)) {
            missingFields << field;
        }
    }

    return QPair<QStringList, QStringList>(missingFields, unknownFields);
}

QPair<bool, ParamTypes> PluginMetadata::parseParamTypes(const QJsonArray &array)
{
    int index = 0;
    QList<ParamType> paramTypes;
    foreach (const QJsonValue &paramTypesJson, array) {
        QJsonObject pt = paramTypesJson.toObject();

        QPair<QStringList, QStringList> verificationResult = verifyFields(ParamType::typeProperties(), ParamType::mandatoryTypeProperties(), pt);

        // Check mandatory fields
        if (!verificationResult.first.isEmpty()) {
            qCWarning(dcDevice()) << pluginName() << "Error parsing ParamType: missing fields:" << verificationResult.first.join(", ") << endl << pt;
            return QPair<bool, QList<ParamType> >(false, QList<ParamType>());
        }

        // Check if there are any unknown fields
        if (!verificationResult.second.isEmpty()) {
            qCWarning(dcDevice()) << pluginName() << "Error parsing ParamType: unknown fields:" << verificationResult.second.join(", ") << endl << pt;
            return QPair<bool, QList<ParamType> >(false, QList<ParamType>());
        }

        // Check type
        QVariant::Type t = QVariant::nameToType(pt.value("type").toString().toLatin1().data());
        if (t == QVariant::Invalid) {
            qCWarning(dcDevice()) << pluginName() << QString("Invalid type %1 for param %2 in json file.")
                                            .arg(pt.value("type").toString())
                                            .arg(pt.value("name").toString()).toLatin1().data();
            return QPair<bool, QList<ParamType> >(false, QList<ParamType>());
        }

        ParamType paramType(ParamTypeId(pt.value("id").toString()), pt.value("name").toString(), t, pt.value("defaultValue").toVariant());
        paramType.setDisplayName(pt.value("displayName").toString());


        // Set allowed values
        QVariantList allowedValues;
        foreach (const QJsonValue &allowedTypesJson, pt.value("allowedValues").toArray()) {
            allowedValues.append(allowedTypesJson.toVariant());
        }

        // Set the input type if there is any
        if (pt.contains("inputType")) {
            QPair<bool, Types::InputType> inputTypeVerification = loadAndVerifyInputType(pt.value("inputType").toString());
            if (!inputTypeVerification.first) {
                qCWarning(dcDevice()) << pluginName() << QString("Invalid inputType for paramType") << pt;
                return QPair<bool, QList<ParamType> >(false, QList<ParamType>());
            } else {
                paramType.setInputType(inputTypeVerification.second);
            }
        }

        // set the unit if there is any
        if (pt.contains("unit")) {
            QPair<bool, Types::Unit> unitVerification = loadAndVerifyUnit(pt.value("unit").toString());
            if (!unitVerification.first) {
                qCWarning(dcDevice()) << pluginName() << QString("Invalid unit type for paramType") << pt;
                return QPair<bool, QList<ParamType> >(false, QList<ParamType>());
            } else {
                paramType.setUnit(unitVerification.second);
            }
        }

        // set readOnly if given (default false)
        if (pt.contains("readOnly"))
            paramType.setReadOnly(pt.value("readOnly").toBool());

        paramType.setAllowedValues(allowedValues);
        paramType.setLimits(pt.value("minValue").toVariant(), pt.value("maxValue").toVariant());
        paramType.setIndex(index++);
        paramTypes.append(paramType);
    }

    return QPair<bool, QList<ParamType> >(true, paramTypes);
}

QPair<bool, Types::InputType> PluginMetadata::loadAndVerifyInputType(const QString &inputType)
{
    if (inputType.isEmpty())
        return QPair<bool, Types::InputType>(true, Types::InputTypeNone);

    QMetaObject metaObject = Types::staticMetaObject;
    int enumIndex = metaObject.indexOfEnumerator(QString("InputType").toLatin1().data());
    QMetaEnum metaEnum = metaObject.enumerator(enumIndex);

    int enumValue = -1;
    for (int i = 0; i < metaEnum.keyCount(); i++) {
        if (QString(metaEnum.valueToKey(metaEnum.value(i))) == QString("InputType" + inputType)) {
            enumValue = metaEnum.value(i);
            break;
        }
    }

    // inform the plugin developer about the error in the plugin json file
    if (enumValue == -1) {
        qCWarning(dcDeviceManager()) << QString("\"%1\" plugin:").arg(pluginName()).toLatin1().data() << QString("Invalid inputType \"%1\" in json file.").arg(inputType).toLatin1().data();
        return QPair<bool, Types::InputType>(false, Types::InputTypeNone);
    }

    return QPair<bool, Types::InputType>(true, (Types::InputType)enumValue);
}
