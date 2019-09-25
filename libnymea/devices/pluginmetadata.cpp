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
#include <QJsonDocument>
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

QStringList PluginMetadata::validationErrors() const
{
    return m_validationErrors;
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
    bool hasError = false;

    // General plugin info
    QStringList pluginMandatoryJsonProperties = QStringList() << "id" << "name" << "displayName" << "vendors";
    QStringList pluginJsonProperties = QStringList() << "id" << "name" << "displayName" << "vendors" << "paramTypes" << "builtIn";
    QPair<QStringList, QStringList> verificationResult = verifyFields(pluginJsonProperties, pluginMandatoryJsonProperties, jsonObject);
    if (!verificationResult.first.isEmpty()) {
        m_validationErrors.append("Plugin metadata has missing fields: " + verificationResult.first.join(", "));
        hasError = true;
        // Not gonna continue parsing as we rely on mandatory fields being available
        return;
    }

    m_pluginId = jsonObject.value("id").toString();
    m_pluginName = jsonObject.value("name").toString();
    m_pluginDisplayName = jsonObject.value("displayName").toString();

    if (!verificationResult.second.isEmpty()) {
        m_validationErrors.append("Plugin \"" + m_pluginName + "\" has unknown fields: \"" + verificationResult.second.join("\", \"") + "\"");
        hasError = true;
    }

    if (!verifyDuplicateUuid(m_pluginId)) {
        m_validationErrors.append("Plugin \"" + m_pluginName + "\" has duplicate UUID: " + m_pluginId.toString());
        hasError = true;
    }

    // parse plugin configuration params
    if (jsonObject.contains("paramTypes")) {
        QPair<bool, QList<ParamType> > paramVerification = parseParamTypes(jsonObject.value("paramTypes").toArray());
        if (paramVerification.first) {
            m_pluginSettings = paramVerification.second;
        } else {
            hasError = true;
        }
    }

    // Load vendors
    foreach (const QJsonValue &vendorJson, jsonObject.value("vendors").toArray()) {
        QJsonObject vendorObject = vendorJson.toObject();

        QStringList vendorMandatoryJsonProperties = QStringList() << "id" << "name" << "displayName" << "deviceClasses";
        QStringList vendorJsonProperties = QStringList() << "id" << "name" << "displayName" << "deviceClasses";

        QPair<QStringList, QStringList> verificationResult = verifyFields(vendorJsonProperties, vendorMandatoryJsonProperties, vendorObject);

        // Check mandatory fields
        if (!verificationResult.first.isEmpty()) {
            m_validationErrors.append("Vendor metadata has missing fields: " + verificationResult.first.join(", ") + "\n" + qUtf8Printable(QJsonDocument::fromVariant(vendorObject.toVariantMap()).toJson(QJsonDocument::Indented)));
            hasError = true;
            // Not continuing parsing vendor as we rely on mandatory fields being around.
            break;
        }

        VendorId vendorId = VendorId(vendorObject.value("id").toString());
        QString vendorName = vendorObject.value("name").toString();

        // Check if there are any unknown fields
        if (!verificationResult.second.isEmpty()) {
            m_validationErrors.append("Vendor \"" + vendorName + "\" has unknown fields: \"" + verificationResult.second.join("\", \"") + "\"\n" + qUtf8Printable(QJsonDocument::fromVariant(vendorObject.toVariantMap()).toJson(QJsonDocument::Indented)));
            hasError = true;
        }

        if (!verifyDuplicateUuid(vendorId)) {
            m_validationErrors.append("Vendor \"" + vendorName + "\" has duplicate UUID: " + vendorId.toString());
            hasError = true;
        }
        Vendor vendor(vendorId, vendorName);
        vendor.setDisplayName(vendorObject.value("displayName").toString());
        m_vendors.append(vendor);

        // Load deviceclasses of this vendor
        foreach (const QJsonValue &deviceClassJson, vendorJson.toObject().value("deviceClasses").toArray()) {

            // FIXME: Drop this when possible, see .h for context
            m_currentScopUuids.clear();

            QJsonObject deviceClassObject = deviceClassJson.toObject();
            /*! Returns a list of all valid JSON properties a DeviceClass JSON definition can have. */
            QStringList deviceClassProperties = QStringList() << "id" << "name" << "displayName" << "createMethods" << "setupMethod"
                                     << "interfaces" << "browsable" << "pairingInfo" << "discoveryParamTypes" << "discoveryParamTypes"
                                     << "paramTypes" << "settingsTypes" << "stateTypes" << "actionTypes" << "eventTypes" << "browserItemActionTypes";
            QStringList mandatoryDeviceClassProperties = QStringList() << "id" << "name" << "displayName";

            QPair<QStringList, QStringList> verificationResult = verifyFields(deviceClassProperties, mandatoryDeviceClassProperties, deviceClassObject);

            // Check mandatory fields
            if (!verificationResult.first.isEmpty()) {
                m_validationErrors.append("Device class has missing fields: \"" + verificationResult.first.join("\", \"") + "\"\n" + qUtf8Printable(QJsonDocument::fromVariant(deviceClassObject.toVariantMap()).toJson(QJsonDocument::Indented)));
                hasError = true;
                // Stop parsing this deviceClass as we rely on mandatory fields being around.
                continue;
            }

            DeviceClassId deviceClassId = deviceClassObject.value("id").toString();
            QString deviceClassName = deviceClassObject.value("name").toString();

            // Check if there are any unknown fields
            if (!verificationResult.second.isEmpty()) {
                m_validationErrors.append("Device class \"" + deviceClassName + "\" has unknown fields: \"" + verificationResult.second.join("\", \"") + "\"\n" + qUtf8Printable(QJsonDocument::fromVariant(deviceClassObject.toVariantMap()).toJson(QJsonDocument::Indented)));
                hasError = true;
            }

            if (!verifyDuplicateUuid(deviceClassId)) {
                m_validationErrors.append("Device class \"" + deviceClassName + "\" has duplicate UUID: " + deviceClassName);
                hasError = true;
            }

            DeviceClass deviceClass(pluginId(), vendorId, deviceClassId);
            deviceClass.setName(deviceClassName);
            deviceClass.setDisplayName(deviceClassObject.value("displayName").toString());
            deviceClass.setBrowsable(deviceClassObject.value("browsable").toBool());

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
                        m_validationErrors.append("Unknown createMehtod \"" + createMethodValue.toString() + "\" in deviceClass \"" + deviceClass.name() +  "\".");
                        hasError = true;
                    }
                }
            }
            deviceClass.setCreateMethods(createMethods);

            // Read params
            QPair<bool, QList<ParamType> > paramTypesVerification = parseParamTypes(deviceClassObject.value("paramTypes").toArray());
            if (!paramTypesVerification.first) {
                hasError = true;
            } else {
                deviceClass.setParamTypes(paramTypesVerification.second);
            }

            // Read settings
            QPair<bool, QList<ParamType> > settingsTypesVerification = parseParamTypes(deviceClassObject.value("settingsTypes").toArray());
            if (!settingsTypesVerification.first) {
                hasError = true;
            } else {
                deviceClass.setSettingsTypes(settingsTypesVerification.second);
            }

            // Read discover params
            QPair<bool, QList<ParamType> > discoveryParamVerification = parseParamTypes(deviceClassObject.value("discoveryParamTypes").toArray());
            if (!discoveryParamVerification.first) {
                hasError = true;
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
                    m_validationErrors.append("Unknown setupMethod \"" + setupMethodString + "\" in deviceClass \"" + deviceClass.name() + "\".");
                    hasError = true;
                }
            }
            deviceClass.setSetupMethod(setupMethod);

            // Read pairing info
            deviceClass.setPairingInfo(deviceClassObject.value("pairingInfo").toString());

            ActionTypes actionTypes;
            StateTypes stateTypes;
            EventTypes eventTypes;
            ActionTypes browserItemActionTypes;

            // Read StateTypes
            int index = 0;
            foreach (const QJsonValue &stateTypesJson, deviceClassObject.value("stateTypes").toArray()) {
                QJsonObject st = stateTypesJson.toObject();
                bool writableState = false;

                QPair<QStringList, QStringList> verificationResult = verifyFields(StateType::typeProperties(), StateType::mandatoryTypeProperties(), st);

                // Check mandatory fields
                if (!verificationResult.first.isEmpty()) {
                    m_validationErrors.append("Device class \"" + deviceClass.name() + "\" has missing properties \"" + verificationResult.first.join("\", \"") + "\" in stateType definition\n" + qUtf8Printable(QJsonDocument::fromVariant(st.toVariantMap()).toJson(QJsonDocument::Indented)));
                    hasError = true;
                    // Not processing further as mandatory fields are expected to be here
                    continue;
                }

                StateTypeId stateTypeId = st.value("id").toString();
                QString stateTypeName = st.value("name").toString();

                // Check if there are any unknown fields
                if (!verificationResult.second.isEmpty()) {
                    m_validationErrors.append("Device class \"" + deviceClass.name() + "\" state type \"" + stateTypeName + "\" has unknown properties \"" + verificationResult.second.join("\", \"") + "\"");
                    hasError = true;
                }

                // If this is a writable stateType, there must be also the displayNameAction property
                if (st.contains("writable") && st.value("writable").toBool()) {
                    writableState = true;
                    if (!st.contains("displayNameAction")) {
                        m_validationErrors.append("Device class \"" + deviceClass.name() + "\" state type \"" + stateTypeName + "\" has writable state but does not define the displayNameAction property");
                        hasError = true;
                    }
                }

                QVariant::Type t = QVariant::nameToType(st.value("type").toString().toLatin1().data());
                if (t == QVariant::Invalid) {
                    m_validationErrors.append("Device class \"" + deviceClass.name() + "\" state type \"" + stateTypeName + "\" has invalid type: \"" + st.value("type").toString() + "\"");
                    hasError = true;
                }

                if (!verifyDuplicateUuid(stateTypeId)) {
                    m_validationErrors.append("Device class \"" + deviceClass.name() + "\" state type \"" + stateTypeName + "\" has duplicate UUID: " + stateTypeId.toString());
                    hasError = true;
                }

                StateType stateType(stateTypeId);
                stateType.setName(stateTypeName);
                stateType.setDisplayName(st.value("displayName").toString());
                stateType.setIndex(index++);
                stateType.setType(t);
                QPair<bool, Types::Unit> unitVerification = loadAndVerifyUnit(st.value("unit").toString());
                if (!unitVerification.first) {
                    hasError = true;
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
                        m_validationErrors.append("Device class \"" + deviceClass.name() + "\" state type \"" + stateTypeName + "\" has invalid default value \"" + stateType.defaultValue().toString() + "\" which is not in the list of possible values.");
                        hasError = true;
                        break;
                    }
                }

                if (st.contains("cached")) {
                    stateType.setCached(st.value("cached").toBool());
                }
                stateTypes.append(stateType);

                // Events for state changed (Not checking for duplicate UUID, this is expected to be the same as the state!)
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
                    m_validationErrors.append("Device class \"" + deviceClass.name() + "\" has missing fields \"" + verificationResult.first.join("\", \"") + "\" in action type definition.");
                    hasError = true;
                    continue;
                }

                ActionTypeId actionTypeId = ActionTypeId(at.value("id").toString());
                QString actionTypeName = at.value("name").toString();

                // Check if there are any unknown fields
                if (!verificationResult.second.isEmpty()) {
                    m_validationErrors.append("Device class \"" + deviceClass.name() + "\" action type \"" + actionTypeName + "\" has unknown fields \"" + verificationResult.second.join("\", \"") + "\"");
                    hasError = true;
                }

                if (!verifyDuplicateUuid(actionTypeId)) {
                    m_validationErrors.append("Device class \"" + deviceClass.name() + "\" action type \"" + actionTypeName + "\" has duplicate UUID: " + actionTypeId.toString());
                    hasError = true;
                }
                ActionType actionType(actionTypeId);
                actionType.setName(actionTypeName);
                actionType.setDisplayName(at.value("displayName").toString());
                actionType.setIndex(index++);

                QPair<bool, QList<ParamType> > paramVerification = parseParamTypes(at.value("paramTypes").toArray());
                if (!paramVerification.first) {
                    hasError = true;
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
                    m_validationErrors.append("Device class \"" + deviceClass.name() + "\" has missing fields \"" + verificationResult.first.join("\", \"") + "\" in event type defintion");
                    hasError = true;
                    continue;
                }

                EventTypeId eventTypeId = EventTypeId(et.value("id").toString());
                QString eventTypeName = et.value("name").toString();

                // Check if there are any unknown fields
                if (!verificationResult.second.isEmpty()) {
                    m_validationErrors.append("Device class \"" + deviceClass.name() + "\" event type \"" + eventTypeName + "\" has unknown fields \"" + verificationResult.second.join("\", \"") + "\"");
                    hasError = true;
                }

                if (!verifyDuplicateUuid(eventTypeId)) {
                    m_validationErrors.append("Device class \"" + deviceClass.name() + "\" event type \"" + eventTypeName + "\" has duplicate UUID: " + eventTypeId.toString());
                    hasError = true;
                }
                EventType eventType(eventTypeId);
                eventType.setName(eventTypeName);
                eventType.setDisplayName(et.value("displayName").toString());
                eventType.setIndex(index++);

                QPair<bool, QList<ParamType> > paramVerification = parseParamTypes(et.value("paramTypes").toArray());
                if (!paramVerification.first) {
                    hasError = true;
                } else {
                    eventType.setParamTypes(paramVerification.second);
                }
                eventTypes.append(eventType);
            }
            deviceClass.setEventTypes(eventTypes);

            // BrowserItemActionTypes
            index = 0;
            foreach (const QJsonValue &browserItemActionTypesJson, deviceClassObject.value("browserItemActionTypes").toArray()) {
                QJsonObject at = browserItemActionTypesJson.toObject();
                QPair<QStringList, QStringList> verificationResult = verifyFields(ActionType::typeProperties(), ActionType::mandatoryTypeProperties(), at);

                // Check mandatory fields
                if (!verificationResult.first.isEmpty()) {
                    m_validationErrors.append("Device class \"" + deviceClass.name() + "\" has missing fields \"" + verificationResult.first.join("\", \"") + "\" in browser item action type definition");
                    hasError = true;
                    continue;
                }

                ActionTypeId actionTypeId = ActionTypeId(at.value("id").toString());
                QString actionTypeName = at.value("name").toString();

                // Check if there are any unknown fields
                if (!verificationResult.second.isEmpty()) {
                    m_validationErrors.append("Device class \"" + deviceClass.name() + "\" browser action type \"" + actionTypeName + "\" has unknown fields \"" + verificationResult.first.join("\", \"") + "\"");
                    hasError = true;
                }

                if (!verifyDuplicateUuid(actionTypeId)) {
                    m_validationErrors.append("Device class \"" + deviceClass.name() + "\" browser action type \"" + actionTypeName + "\" has duplicate UUID: " + actionTypeId.toString());
                    hasError = true;
                }
                ActionType actionType(actionTypeId);
                actionType.setName(actionTypeName);
                actionType.setDisplayName(at.value("displayName").toString());
                actionType.setIndex(index++);

                QPair<bool, QList<ParamType> > paramVerification = parseParamTypes(at.value("paramTypes").toArray());
                if (!paramVerification.first) {
                    hasError = true;
                    break;
                } else {
                    actionType.setParamTypes(paramVerification.second);
                }

                browserItemActionTypes.append(actionType);
            }
            deviceClass.setBrowserItemActionTypes(browserItemActionTypes);

            // Read interfaces
            QStringList interfaces;
            foreach (const QJsonValue &value, deviceClassObject.value("interfaces").toArray()) {
                Interface iface = DeviceUtils::loadInterface(value.toString());

                StateTypes stateTypes(deviceClass.stateTypes());
                ActionTypes actionTypes(deviceClass.actionTypes());
                EventTypes eventTypes(deviceClass.eventTypes());

                foreach (const StateType &ifaceStateType, iface.stateTypes()) {
                    StateType stateType = stateTypes.findByName(ifaceStateType.name());
                    if (stateType.id().isNull()) {
                        m_validationErrors.append("Device class \"" + deviceClass.name() + "\" claims to implement interface \"" + value.toString() + "\" but doesn't implement state \"" + ifaceStateType.name() + "\"");
                        hasError = true;
                        continue;
                    }
                    if (ifaceStateType.type() != stateType.type()) {
                        m_validationErrors.append("Device class \"" + deviceClass.name() + "\" claims to implement interface \"" + value.toString() + "\" but state \"" + stateType.name() + "\" has not matching type: \"" + QVariant::typeToName(stateType.type()) + "\" != \"" + QVariant::typeToName(ifaceStateType.type()) + "\"");
                        hasError = true;
                        continue;
                    }
                    if (ifaceStateType.minValue().isValid() && !ifaceStateType.minValue().isNull()) {
                        if (ifaceStateType.minValue().toString() == "any") {
                            if (stateType.minValue().isNull()) {
                                m_validationErrors.append("Device class \"" + deviceClass.name() + "\" claims to implement interface \"" + value.toString() + "\" but state \"" + stateType.name() + "\" has no minimum value defined.");
                                hasError = true;
                                continue;
                            }
                        } else if (ifaceStateType.minValue() != stateType.minValue()) {
                            m_validationErrors.append("Device class \"" + deviceClass.name() + "\" claims to implement interface \"" + value.toString() + "\" but state \"" + stateType.name() + "\" has not matching minimum value: \"" + ifaceStateType.minValue().toString() + "\" != \"" + stateType.minValue().toString() + "\"");
                            hasError = true;
                            continue;
                        }
                    }
                    if (ifaceStateType.maxValue().isValid() && !ifaceStateType.maxValue().isNull()) {
                        if (ifaceStateType.maxValue().toString() == "any") {
                            if (stateType.maxValue().isNull()) {
                                m_validationErrors.append("Device class \"" + deviceClass.name() + "\" claims to implement interface \"" + value.toString() + "\" but state \"" + stateType.name() + "\" has no maximum value defined.");
                                hasError = true;
                                continue;
                            }
                        } else if (ifaceStateType.maxValue() != stateType.maxValue()) {
                            m_validationErrors.append("Device class \"" + deviceClass.name() + "\" claims to implement interface \"" + value.toString() + "\" but state \"" + stateType.name() + "\" has not matching maximum value: \"" + ifaceStateType.maxValue().toString() + "\" != \"" + stateType.minValue().toString() + "\"");
                            hasError = true;
                            continue;
                        }
                    }
                    if (!ifaceStateType.possibleValues().isEmpty() && ifaceStateType.possibleValues() != stateType.possibleValues()) {
                        m_validationErrors.append("Device class \"" + deviceClass.name() + "\" claims to implement interface \"" + value.toString() + "\" but state \"" + stateType.name() + "\" has not matching allowed values.");
                        hasError = true;
                        continue;
                    }
                }

                foreach (const ActionType &ifaceActionType, iface.actionTypes()) {
                    ActionType actionType = actionTypes.findByName(ifaceActionType.name());
                    if (actionType.id().isNull()) {
                        m_validationErrors.append("Device class \"" + deviceClass.name() + "\" claims to implement interface \"" + value.toString() + "\" but doesn't implement action \"" + ifaceActionType.name() + "\"");
                        hasError = true;
                    }
                    foreach (const ParamType &ifaceActionParamType, ifaceActionType.paramTypes()) {
                        ParamType paramType = actionType.paramTypes().findByName(ifaceActionParamType.name());
                        if (!paramType.isValid()) {
                            m_validationErrors.append("Device class \"" + deviceClass.name() + "\" claims to implement interface \"" + value.toString() + "\" but action \"" + actionType.name() + "\" doesn't implement action param \"" + ifaceActionParamType.name() + "\"");
                            hasError = true;
                        } else {
                            if (paramType.type() != ifaceActionParamType.type()) {
                                m_validationErrors.append("Device class \"" + deviceClass.name() + "\" claims to implement interface \"" + value.toString() + "\" but action \"" + actionType.name() + "\" param \"" + paramType.name() + "\" is of wrong type: \"" + QVariant::typeToName(paramType.type()) + "\" expected: \"" + QVariant::typeToName(ifaceActionParamType.type()) + "\"");
                                hasError = true;
                            }
                            foreach (const QVariant &allowedValue, ifaceActionParamType.allowedValues()) {
                                if (!paramType.allowedValues().contains(allowedValue)) {
                                    m_validationErrors.append("Device class \"" + deviceClass.name() + "\" claims to implement interface \"" + value.toString() + "\" but action \"" + actionType.name() + "\" param \"" + paramType.name() + "\" is missing allowed value \"" + allowedValue.toString() + "\"");
                                    hasError = true;
                                }
                            }
                        }
                    }
                }

                foreach (const EventType &ifaceEventType, iface.eventTypes()) {
                    EventType eventType = eventTypes.findByName(ifaceEventType.name());
                    if (!eventType.isValid()) {
                        m_validationErrors.append("Device class \"" + deviceClass.name() + "\" claims to implement interface \"" + value.toString() + "\" but doesn't implement event \"" + ifaceEventType.name() + "\"");
                        hasError = true;
                    }
                    foreach (const ParamType &ifaceEventParamType, ifaceEventType.paramTypes()) {
                        ParamType paramType = eventType.paramTypes().findByName(ifaceEventParamType.name());
                        if (!paramType.isValid()) {
                            m_validationErrors.append("Device class \"" + deviceClass.name() + "\" claims to implement interface \"" + value.toString() + "\" but event \"" + eventType.name() + "\" doesn't implement event param \"" + ifaceEventParamType.name() + "\"");
                            hasError = true;
                        } else {
                            if (paramType.type() != ifaceEventParamType.type()) {
                                m_validationErrors.append("Device class \"" + deviceClass.name() + "\" claims to implement interface \"" + value.toString() + "\" but event \"" + eventType.name() + "\" param \"" + paramType.name() + "\" is of wrong type: \"" + QVariant::typeToName(paramType.type()) + "\" expected: \"" + QVariant::typeToName(ifaceEventParamType.type()) + "\"");
                                hasError = true;
                            }
                        }
                    }
                }

                interfaces.append(DeviceUtils::generateInterfaceParentList(value.toString()));
            }
            interfaces.removeDuplicates();
            deviceClass.setInterfaces(interfaces);

            m_deviceClasses.append(deviceClass);
        }
    }
    if (!hasError) {
        m_isValid = true;
    } else {
        qCWarning(dcPluginMetadata()) << "Device metadata has errors.";
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
        qCWarning(dcPluginMetadata()) << QString("\"%1\" plugin:").arg(pluginName()).toLatin1().data() << QString("Invalid unit type \"%1\" in json file.").arg(unitString).toLatin1().data();
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
    bool hasErrors = false;
    int index = 0;
    QList<ParamType> paramTypes;
    foreach (const QJsonValue &paramTypesJson, array) {
        QJsonObject pt = paramTypesJson.toObject();

        QPair<QStringList, QStringList> verificationResult = verifyFields(ParamType::typeProperties(), ParamType::mandatoryTypeProperties(), pt);

        // Check mandatory fields
        if (!verificationResult.first.isEmpty()) {
            m_validationErrors.append("Error parsing ParamType. Missing fields: \"" + verificationResult.first.join("\", \"") + "\"");
            hasErrors = true;
            continue;
        }

        ParamTypeId paramTypeId = ParamTypeId(pt.value("id").toString());
        QString paramName = pt.value("name").toString();

        // Check if there are any unknown fields
        if (!verificationResult.second.isEmpty()) {
            m_validationErrors.append("Param type \"" + paramName + "\" has unknown fields: \"" + verificationResult.second.join("\", \"") + "\"");
            hasErrors = true;
        }

        // Check type
        QVariant::Type t = QVariant::nameToType(pt.value("type").toString().toLatin1().data());
        if (t == QVariant::Invalid) {
            m_validationErrors.append("Param type \"" + paramName + "\" has unknown invalid type \"" + pt.value("type").toString() + "\"");
            hasErrors = true;
        }

        if (!verifyDuplicateUuid(paramTypeId)) {
            m_validationErrors.append("Param type \"" + paramName + "\" has duplicate UUID: " + paramTypeId.toString());
            hasErrors = true;
        }
        ParamType paramType(paramTypeId, paramName, t, pt.value("defaultValue").toVariant());
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
                qCWarning(dcPluginMetadata()) << pluginName() << QString("Invalid inputType for paramType") << pt;
                hasErrors = true;
            } else {
                paramType.setInputType(inputTypeVerification.second);
            }
        }

        // set the unit if there is any
        if (pt.contains("unit")) {
            QPair<bool, Types::Unit> unitVerification = loadAndVerifyUnit(pt.value("unit").toString());
            if (!unitVerification.first) {
                qCWarning(dcPluginMetadata()) << pluginName() << QString("Invalid unit type for paramType") << pt;
                hasErrors = true;
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

    return QPair<bool, QList<ParamType> >(!hasErrors, paramTypes);
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
        qCWarning(dcPluginMetadata()) << QString("\"%1\" plugin:").arg(pluginName()).toLatin1().data() << QString("Invalid inputType \"%1\" in json file.").arg(inputType).toLatin1().data();
        return QPair<bool, Types::InputType>(false, Types::InputTypeNone);
    }

    return QPair<bool, Types::InputType>(true, (Types::InputType)enumValue);
}

bool PluginMetadata::verifyDuplicateUuid(const QUuid &uuid)
{
    if (m_allUuids.contains(uuid)) {
        // FIXME: Drop debug, activate return! (see .h for more context)
        qCWarning(dcPluginMetadata()) << "THIS PLUGIN USES DUPLICATE UUID" << uuid.toString() << "! THIS WILL STOP WORKING SOON.";
//        return false;
    }
    if (m_currentScopUuids.contains(uuid)) {
        return false;
    }
    m_allUuids.append(uuid);
    return true;
}
