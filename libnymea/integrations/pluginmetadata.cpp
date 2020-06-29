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

#include "pluginmetadata.h"
#include "thingutils.h"

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

PluginMetadata::PluginMetadata(const QJsonObject &jsonObject, bool isBuiltIn, bool strict):
    m_isBuiltIn(isBuiltIn),
    m_strictRun(strict)
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

ThingClasses PluginMetadata::thingClasses() const
{
    return m_thingClasses;
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

    m_pluginId = PluginId(jsonObject.value("id").toString());
    m_pluginName = jsonObject.value("name").toString();
    m_pluginDisplayName = jsonObject.value("displayName").toString();

    if (!verificationResult.second.isEmpty()) {
        m_validationErrors.append("Plugin \"" + m_pluginName + "\" has unknown fields: \"" + verificationResult.second.join("\", \"") + "\"");
        hasError = true;
    }

    if (m_pluginId.isNull()) {
        m_validationErrors.append("Plugin \"" + m_pluginName + "\" has invalid UUID: " + jsonObject.value("id").toString());
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

        QStringList vendorMandatoryJsonProperties = QStringList() << "id" << "name" << "displayName" << "thingClasses";
        QStringList vendorJsonProperties = QStringList() << "id" << "name" << "displayName" << "thingClasses";

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

        if (vendorId.isNull()) {
            m_validationErrors.append("Vendor \"" + vendorName + "\" has invalid UUID: " + vendorObject.value("id").toString());
            hasError = true;
        }
        if (!verifyDuplicateUuid(vendorId)) {
            m_validationErrors.append("Vendor \"" + vendorName + "\" has duplicate UUID: " + vendorId.toString());
            hasError = true;
        }
        Vendor vendor(vendorId, vendorName);
        vendor.setDisplayName(vendorObject.value("displayName").toString());
        m_vendors.append(vendor);

        // Load thing classes of this vendor
        foreach (const QJsonValue &thingClassJson, vendorJson.toObject().value("thingClasses").toArray()) {

            // FIXME: Drop this when possible, see .h for context
            m_currentScopUuids.clear();

            QJsonObject thingClassObject = thingClassJson.toObject();
            /*! Returns a list of all valid JSON properties a ThingClass JSON definition can have. */
            QStringList thingClassProperties = QStringList() << "id" << "name" << "displayName" << "createMethods" << "setupMethod"
                                     << "interfaces" << "browsable" << "discoveryParamTypes"
                                     << "paramTypes" << "settingsTypes" << "stateTypes" << "actionTypes" << "eventTypes" << "browserItemActionTypes";
            QStringList mandatoryThingClassProperties = QStringList() << "id" << "name" << "displayName";

            QPair<QStringList, QStringList> verificationResult = verifyFields(thingClassProperties, mandatoryThingClassProperties, thingClassObject);

            // Check mandatory fields
            if (!verificationResult.first.isEmpty()) {
                m_validationErrors.append("Thing class has missing fields: \"" + verificationResult.first.join("\", \"") + "\"\n" + qUtf8Printable(QJsonDocument::fromVariant(thingClassObject.toVariantMap()).toJson(QJsonDocument::Indented)));
                hasError = true;
                // Stop parsing this thingClass as we rely on mandatory fields being around.
                continue;
            }

            ThingClassId thingClassId = ThingClassId(thingClassObject.value("id").toString());
            QString thingClassName = thingClassObject.value("name").toString();

            // Check if there are any unknown fields
            if (!verificationResult.second.isEmpty()) {
                m_validationErrors.append("Thing class \"" + thingClassName + "\" has unknown fields: \"" + verificationResult.second.join("\", \"") + "\"\n" + qUtf8Printable(QJsonDocument::fromVariant(thingClassObject.toVariantMap()).toJson(QJsonDocument::Indented)));
                hasError = true;
            }

            if (thingClassId.isNull()) {
                m_validationErrors.append("Thing class \"" + thingClassName + "\" has invalid UUID: " + thingClassObject.value("id").toString());
                hasError = true;
            }
            if (!verifyDuplicateUuid(thingClassId)) {
                m_validationErrors.append("Thing class \"" + thingClassName + "\" has duplicate UUID: " + thingClassName);
                hasError = true;
            }

            ThingClass thingClass(pluginId(), vendorId, thingClassId);
            thingClass.setName(thingClassName);
            thingClass.setDisplayName(thingClassObject.value("displayName").toString());
            thingClass.setBrowsable(thingClassObject.value("browsable").toBool());

            // Read create methods
            ThingClass::CreateMethods createMethods;
            if (!thingClassObject.contains("createMethods")) {
                // Default if not specified
                createMethods |= ThingClass::CreateMethodUser;
            } else {
                foreach (const QJsonValue &createMethodValue, thingClassObject.value("createMethods").toArray()) {
                    if (createMethodValue.toString().toLower() == "discovery") {
                        createMethods |= ThingClass::CreateMethodDiscovery;
                    } else if (createMethodValue.toString().toLower() == "auto") {
                        createMethods |= ThingClass::CreateMethodAuto;
                    } else if (createMethodValue.toString().toLower() == "user") {
                        createMethods |= ThingClass::CreateMethodUser;
                    } else {
                        m_validationErrors.append("Unknown createMehtod \"" + createMethodValue.toString() + "\" in thingClass \"" + thingClass.name() +  "\".");
                        hasError = true;
                    }
                }
            }
            thingClass.setCreateMethods(createMethods);

            // Read params
            QPair<bool, QList<ParamType> > paramTypesVerification = parseParamTypes(thingClassObject.value("paramTypes").toArray());
            if (!paramTypesVerification.first) {
                hasError = true;
            } else {
                thingClass.setParamTypes(paramTypesVerification.second);
            }

            // Read settings
            QPair<bool, QList<ParamType> > settingsTypesVerification = parseParamTypes(thingClassObject.value("settingsTypes").toArray());
            if (!settingsTypesVerification.first) {
                hasError = true;
            } else {
                thingClass.setSettingsTypes(settingsTypesVerification.second);
            }

            // Read discover params
            QPair<bool, QList<ParamType> > discoveryParamVerification = parseParamTypes(thingClassObject.value("discoveryParamTypes").toArray());
            if (!discoveryParamVerification.first) {
                hasError = true;
            } else {
                thingClass.setDiscoveryParamTypes(discoveryParamVerification.second);
            }

            // Read setup method
            ThingClass::SetupMethod setupMethod = ThingClass::SetupMethodJustAdd;
            if (thingClassObject.contains("setupMethod")) {
                QString setupMethodString = thingClassObject.value("setupMethod").toString();
                if (setupMethodString.toLower() == "pushbutton") {
                    setupMethod = ThingClass::SetupMethodPushButton;
                } else if (setupMethodString.toLower() == "displaypin") {
                    setupMethod = ThingClass::SetupMethodDisplayPin;
                } else if (setupMethodString.toLower() == "enterpin") {
                    setupMethod = ThingClass::SetupMethodEnterPin;
                } else if (setupMethodString.toLower() == "justadd") {
                    setupMethod = ThingClass::SetupMethodJustAdd;
                } else if (setupMethodString.toLower() == "userandpassword") {
                    setupMethod = ThingClass::SetupMethodUserAndPassword;
                } else if (setupMethodString.toLower() == "oauth") {
                    setupMethod = ThingClass::SetupMethodOAuth;
                } else {
                    m_validationErrors.append("Unknown setupMethod \"" + setupMethodString + "\" in thingClass \"" + thingClass.name() + "\".");
                    hasError = true;
                }
            }
            thingClass.setSetupMethod(setupMethod);

            ActionTypes actionTypes;
            StateTypes stateTypes;
            EventTypes eventTypes;
            ActionTypes browserItemActionTypes;

            // Read StateTypes
            int index = 0;
            foreach (const QJsonValue &stateTypesJson, thingClassObject.value("stateTypes").toArray()) {
                QJsonObject st = stateTypesJson.toObject();
                bool writableState = false;

                QPair<QStringList, QStringList> verificationResult = verifyFields(StateType::typeProperties(), StateType::mandatoryTypeProperties(), st);

                // Check mandatory fields
                if (!verificationResult.first.isEmpty()) {
                    m_validationErrors.append("Thing class \"" + thingClass.name() + "\" has missing properties \"" + verificationResult.first.join("\", \"") + "\" in stateType definition\n" + qUtf8Printable(QJsonDocument::fromVariant(st.toVariantMap()).toJson(QJsonDocument::Indented)));
                    hasError = true;
                    // Not processing further as mandatory fields are expected to be here
                    continue;
                }

                StateTypeId stateTypeId = StateTypeId(st.value("id").toString());
                QString stateTypeName = st.value("name").toString();

                // Check if there are any unknown fields
                if (!verificationResult.second.isEmpty()) {
                    m_validationErrors.append("Thing class \"" + thingClass.name() + "\" state type \"" + stateTypeName + "\" has unknown properties \"" + verificationResult.second.join("\", \"") + "\"");
                    hasError = true;
                }

                // If this is a writable stateType, there must be also the displayNameAction property
                if (st.contains("writable") && st.value("writable").toBool()) {
                    writableState = true;
                    if (!st.contains("displayNameAction")) {
                        m_validationErrors.append("Thing class \"" + thingClass.name() + "\" state type \"" + stateTypeName + "\" has writable state but does not define the displayNameAction property");
                        hasError = true;
                    }
                }

                QVariant::Type t = QVariant::nameToType(st.value("type").toString().toLatin1().data());
                if (t == QVariant::Invalid) {
                    m_validationErrors.append("Thing class \"" + thingClass.name() + "\" state type \"" + stateTypeName + "\" has invalid type: \"" + st.value("type").toString() + "\"");
                    hasError = true;
                }

                if (stateTypeId.isNull()) {
                    m_validationErrors.append("Thing class \"" + thingClass.name() + "\" state type \"" + stateTypeName + "\" has invalid UUID: " + st.value("id").toString());
                    hasError = true;
                }
                if (!verifyDuplicateUuid(stateTypeId)) {
                    m_validationErrors.append("Thing class \"" + thingClass.name() + "\" state type \"" + stateTypeName + "\" has duplicate UUID: " + stateTypeId.toString());
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

                QVariant defaultValue = st.value("defaultValue").toVariant();
                defaultValue.convert(stateType.type());
                stateType.setDefaultValue(defaultValue);

                if (st.contains("minValue")) {
                    QVariant minValue = st.value("minValue").toVariant();
                    minValue.convert(stateType.type());
                    stateType.setMinValue(minValue);
                }

                if (st.contains("maxValue")) {
                    QVariant maxValue = st.value("maxValue").toVariant();
                    maxValue.convert(stateType.type());
                    stateType.setMaxValue(maxValue);
                }

                if (st.contains("possibleValues")) {
                    QVariantList possibleValues;
                    foreach (const QJsonValue &possibleValueJson, st.value("possibleValues").toArray()) {
                        QVariant possibleValue = possibleValueJson.toVariant();
                        possibleValue.convert(stateType.type());
                        possibleValues.append(possibleValue);
                    }
                    stateType.setPossibleValues(possibleValues);

                    if (!stateType.possibleValues().contains(stateType.defaultValue())) {
                        m_validationErrors.append("Thing class \"" + thingClass.name() + "\" state type \"" + stateTypeName + "\" has invalid default value \"" + stateType.defaultValue().toString() + "\" which is not in the list of possible values.");
                        hasError = true;
                        break;
                    }
                }

                if (st.contains("cached")) {
                    stateType.setCached(st.value("cached").toBool());
                }
                if (st.contains("writable")) {
                    stateType.setWritable(st.value("writable").toBool());
                }

                if (st.contains("ioType")) {
                    QString ioTypeString = st.value("ioType").toString();
                    Types::IOType ioType = Types::IOTypeNone;
                    if (ioTypeString == "digitalInput") {
                        if (stateType.type() != QVariant::Bool) {
                            m_validationErrors.append("Thing class \"" + thingClass.name() + "\" state type \"" + stateTypeName + "\" is marked as digital input but type is not \"bool\"");
                            hasError = true;
                            break;
                        }
                        ioType = Types::IOTypeDigitalInput;
                    } else if (ioTypeString == "digitalOutput") {
                        if (stateType.type() != QVariant::Bool) {
                            m_validationErrors.append("Thing class \"" + thingClass.name() + "\" state type \"" + stateTypeName + "\" is marked as digital output but type is not \"bool\"");
                            hasError = true;
                            break;
                        }
                        if (!stateType.writable()) {
                            m_validationErrors.append("Thing class \"" + thingClass.name() + "\" state type \"" + stateTypeName + "\" is marked as digital output but is not writable");
                            hasError = true;
                            break;
                        }
                        ioType = Types::IOTypeDigitalOutput;
                    } else if (ioTypeString == "analogInput") {
                        if (stateType.type() != QVariant::Double) {
                            m_validationErrors.append("Thing class \"" + thingClass.name() + "\" state type \"" + stateTypeName + "\" is marked as analog input but type is not \"double\"");
                            hasError = true;
                            break;
                        }
                        if (stateType.minValue().isNull() || stateType.maxValue().isNull()) {
                            m_validationErrors.append("Thing class \"" + thingClass.name() + "\" state type \"" + stateTypeName + "\" is marked as analog input but it does not define \"minValue\" and \"maxValue\"");
                            hasError = true;
                            break;
                        }
                        ioType = Types::IOTypeAnalogInput;
                    } else if (ioTypeString == "analogOutput") {
                        if (stateType.type() != QVariant::Double) {
                            m_validationErrors.append("Thing class \"" + thingClass.name() + "\" state type \"" + stateTypeName + "\" is marked as analog output but type is not \"double\"");
                            hasError = true;
                            break;
                        }
                        if (!stateType.writable()) {
                            m_validationErrors.append("Thing class \"" + thingClass.name() + "\" state type \"" + stateTypeName + "\" is marked as analog output but is not writable");
                            hasError = true;
                            break;
                        }
                        if (stateType.minValue().isNull() || stateType.maxValue().isNull()) {
                            m_validationErrors.append("Thing class \"" + thingClass.name() + "\" state type \"" + stateTypeName + "\" is marked as analog output but it does not define \"minValue\" and \"maxValue\"");
                            hasError = true;
                            break;
                        }
                        ioType = Types::IOTypeAnalogOutput;
                    } else {
                        m_validationErrors.append("Thing class \"" + thingClass.name() + "\" state type \"" + stateTypeName + "\" has invalid ioType value \"" + ioType + "\" which is not any of \"digitalInput\", \"digitalOutput\", \"analogInput\" or \"analogOutput\"");
                        hasError = true;
                        break;
                    }
                    stateType.setIOType(ioType);
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
            thingClass.setStateTypes(stateTypes);

            // ActionTypes
            index = 0;
            foreach (const QJsonValue &actionTypesJson, thingClassObject.value("actionTypes").toArray()) {
                QJsonObject at = actionTypesJson.toObject();
                QPair<QStringList, QStringList> verificationResult = verifyFields(ActionType::typeProperties(), ActionType::mandatoryTypeProperties(), at);

                // Check mandatory fields
                if (!verificationResult.first.isEmpty()) {
                    m_validationErrors.append("Thing class \"" + thingClass.name() + "\" has missing fields \"" + verificationResult.first.join("\", \"") + "\" in action type definition.");
                    hasError = true;
                    continue;
                }

                ActionTypeId actionTypeId = ActionTypeId(at.value("id").toString());
                QString actionTypeName = at.value("name").toString();

                // Check if there are any unknown fields
                if (!verificationResult.second.isEmpty()) {
                    m_validationErrors.append("Thing class \"" + thingClass.name() + "\" action type \"" + actionTypeName + "\" has unknown fields \"" + verificationResult.second.join("\", \"") + "\"");
                    hasError = true;
                }

                if (actionTypeId.isNull()) {
                    m_validationErrors.append("Thing class \"" + thingClass.name() + "\" action type \"" + actionTypeName + "\" has invalid UUID: " + at.value("id").toString());
                    hasError = true;
                }
                if (!verifyDuplicateUuid(actionTypeId)) {
                    m_validationErrors.append("Thing class \"" + thingClass.name() + "\" action type \"" + actionTypeName + "\" has duplicate UUID: " + actionTypeId.toString());
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
            thingClass.setActionTypes(actionTypes);

            // EventTypes
            index = 0;
            foreach (const QJsonValue &eventTypesJson, thingClassObject.value("eventTypes").toArray()) {
                QJsonObject et = eventTypesJson.toObject();

                QPair<QStringList, QStringList> verificationResult = verifyFields(EventType::typeProperties(), EventType::mandatoryTypeProperties(), et);

                // Check mandatory fields
                if (!verificationResult.first.isEmpty()) {
                    m_validationErrors.append("Thing class \"" + thingClass.name() + "\" has missing fields \"" + verificationResult.first.join("\", \"") + "\" in event type defintion");
                    hasError = true;
                    continue;
                }

                EventTypeId eventTypeId = EventTypeId(et.value("id").toString());
                QString eventTypeName = et.value("name").toString();

                // Check if there are any unknown fields
                if (!verificationResult.second.isEmpty()) {
                    m_validationErrors.append("Thing class \"" + thingClass.name() + "\" event type \"" + eventTypeName + "\" has unknown fields \"" + verificationResult.second.join("\", \"") + "\"");
                    hasError = true;
                }

                if (eventTypeId.isNull()) {
                    m_validationErrors.append("Thing class \"" + thingClass.name() + "\" event type \"" + eventTypeName + "\" has invalid UUID: " + et.value("id").toString());
                    hasError = true;
                }
                if (!verifyDuplicateUuid(eventTypeId)) {
                    m_validationErrors.append("Thing class \"" + thingClass.name() + "\" event type \"" + eventTypeName + "\" has duplicate UUID: " + eventTypeId.toString());
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
            thingClass.setEventTypes(eventTypes);

            // BrowserItemActionTypes
            index = 0;
            foreach (const QJsonValue &browserItemActionTypesJson, thingClassObject.value("browserItemActionTypes").toArray()) {
                QJsonObject at = browserItemActionTypesJson.toObject();
                QPair<QStringList, QStringList> verificationResult = verifyFields(ActionType::typeProperties(), ActionType::mandatoryTypeProperties(), at);

                // Check mandatory fields
                if (!verificationResult.first.isEmpty()) {
                    m_validationErrors.append("Thing class \"" + thingClass.name() + "\" has missing fields \"" + verificationResult.first.join("\", \"") + "\" in browser item action type definition");
                    hasError = true;
                    continue;
                }

                ActionTypeId actionTypeId = ActionTypeId(at.value("id").toString());
                QString actionTypeName = at.value("name").toString();

                // Check if there are any unknown fields
                if (!verificationResult.second.isEmpty()) {
                    m_validationErrors.append("Thing class \"" + thingClass.name() + "\" browser action type \"" + actionTypeName + "\" has unknown fields \"" + verificationResult.first.join("\", \"") + "\"");
                    hasError = true;
                }

                if (actionTypeId.isNull()) {
                    m_validationErrors.append("Thing class \"" + thingClass.name() + "\" browser action type \"" + actionTypeName + "\" has invalid UUID: " + at.value("id").toString());
                    hasError = true;
                }
                if (!verifyDuplicateUuid(actionTypeId)) {
                    m_validationErrors.append("Thing class \"" + thingClass.name() + "\" browser action type \"" + actionTypeName + "\" has duplicate UUID: " + actionTypeId.toString());
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
            thingClass.setBrowserItemActionTypes(browserItemActionTypes);

            // Read interfaces
            QStringList interfaces;
            foreach (const QJsonValue &value, thingClassObject.value("interfaces").toArray()) {
                Interface iface = ThingUtils::loadInterface(value.toString());

                StateTypes stateTypes(thingClass.stateTypes());
                ActionTypes actionTypes(thingClass.actionTypes());
                EventTypes eventTypes(thingClass.eventTypes());

                foreach (const StateType &ifaceStateType, iface.stateTypes()) {
                    StateType stateType = stateTypes.findByName(ifaceStateType.name());
                    if (stateType.id().isNull()) {
                        m_validationErrors.append("Thing class \"" + thingClass.name() + "\" claims to implement interface \"" + value.toString() + "\" but doesn't implement state \"" + ifaceStateType.name() + "\"");
                        hasError = true;
                        continue;
                    }
                    if (ifaceStateType.type() != stateType.type()) {
                        m_validationErrors.append("Thing class \"" + thingClass.name() + "\" claims to implement interface \"" + value.toString() + "\" but state \"" + stateType.name() + "\" has not matching type: \"" + QVariant::typeToName(stateType.type()) + "\" != \"" + QVariant::typeToName(ifaceStateType.type()) + "\"");
                        hasError = true;
                        continue;
                    }
                    if (ifaceStateType.minValue().isValid() && !ifaceStateType.minValue().isNull()) {
                        if (ifaceStateType.minValue().toString() == "any") {
                            if (stateType.minValue().isNull()) {
                                m_validationErrors.append("Thing class \"" + thingClass.name() + "\" claims to implement interface \"" + value.toString() + "\" but state \"" + stateType.name() + "\" has no minimum value defined.");
                                hasError = true;
                                continue;
                            }
                        } else if (ifaceStateType.minValue() != stateType.minValue()) {
                            m_validationErrors.append("Thing class \"" + thingClass.name() + "\" claims to implement interface \"" + value.toString() + "\" but state \"" + stateType.name() + "\" has not matching minimum value: \"" + ifaceStateType.minValue().toString() + "\" != \"" + stateType.minValue().toString() + "\"");
                            hasError = true;
                            continue;
                        }
                    }
                    if (ifaceStateType.maxValue().isValid() && !ifaceStateType.maxValue().isNull()) {
                        if (ifaceStateType.maxValue().toString() == "any") {
                            if (stateType.maxValue().isNull()) {
                                m_validationErrors.append("Thing class \"" + thingClass.name() + "\" claims to implement interface \"" + value.toString() + "\" but state \"" + stateType.name() + "\" has no maximum value defined.");
                                hasError = true;
                                continue;
                            }
                        } else if (ifaceStateType.maxValue() != stateType.maxValue()) {
                            m_validationErrors.append("Thing class \"" + thingClass.name() + "\" claims to implement interface \"" + value.toString() + "\" but state \"" + stateType.name() + "\" has not matching maximum value: \"" + ifaceStateType.maxValue().toString() + "\" != \"" + stateType.minValue().toString() + "\"");
                            hasError = true;
                            continue;
                        }
                    }
                    if (!ifaceStateType.possibleValues().isEmpty() && ifaceStateType.possibleValues() != stateType.possibleValues()) {
                        m_validationErrors.append("Thing class \"" + thingClass.name() + "\" claims to implement interface \"" + value.toString() + "\" but state \"" + stateType.name() + "\" has not matching allowed values.");
                        hasError = true;
                        continue;
                    }
                    if (ifaceStateType.unit() != Types::UnitNone && ifaceStateType.unit() != stateType.unit()) {
                        QMetaEnum unitEnum = QMetaEnum::fromType<Types::Unit>();
                        m_validationErrors.append("Thing class \"" + thingClass.name() + "\" claims to implement interface \"" + value.toString() + "\" but state \"" + stateType.name() + "\" has not matching unit: \"" + unitEnum.valueToKey(ifaceStateType.unit()) + "\" != \"" + unitEnum.valueToKey(stateType.unit()));
                        hasError = true;
                        continue;
                    }
                }

                foreach (const ActionType &ifaceActionType, iface.actionTypes()) {
                    ActionType actionType = actionTypes.findByName(ifaceActionType.name());
                    if (actionType.id().isNull()) {
                        m_validationErrors.append("Thing class \"" + thingClass.name() + "\" claims to implement interface \"" + value.toString() + "\" but doesn't implement action \"" + ifaceActionType.name() + "\"");
                        hasError = true;
                    }
                    foreach (const ParamType &ifaceActionParamType, ifaceActionType.paramTypes()) {
                        ParamType paramType = actionType.paramTypes().findByName(ifaceActionParamType.name());
                        if (!paramType.isValid()) {
                            m_validationErrors.append("Thing class \"" + thingClass.name() + "\" claims to implement interface \"" + value.toString() + "\" but action \"" + actionType.name() + "\" doesn't implement action param \"" + ifaceActionParamType.name() + "\"");
                            hasError = true;
                        } else {
                            if (paramType.type() != ifaceActionParamType.type()) {
                                m_validationErrors.append("Thing class \"" + thingClass.name() + "\" claims to implement interface \"" + value.toString() + "\" but action \"" + actionType.name() + "\" param \"" + paramType.name() + "\" is of wrong type: \"" + QVariant::typeToName(paramType.type()) + "\" expected: \"" + QVariant::typeToName(ifaceActionParamType.type()) + "\"");
                                hasError = true;
                            }
                            foreach (const QVariant &allowedValue, ifaceActionParamType.allowedValues()) {
                                if (!paramType.allowedValues().contains(allowedValue)) {
                                    m_validationErrors.append("Thing class \"" + thingClass.name() + "\" claims to implement interface \"" + value.toString() + "\" but action \"" + actionType.name() + "\" param \"" + paramType.name() + "\" is missing allowed value \"" + allowedValue.toString() + "\"");
                                    hasError = true;
                                }
                            }
                        }
                    }
                }

                foreach (const EventType &ifaceEventType, iface.eventTypes()) {
                    EventType eventType = eventTypes.findByName(ifaceEventType.name());
                    if (!eventType.isValid()) {
                        m_validationErrors.append("Thing class \"" + thingClass.name() + "\" claims to implement interface \"" + value.toString() + "\" but doesn't implement event \"" + ifaceEventType.name() + "\"");
                        hasError = true;
                    }
                    foreach (const ParamType &ifaceEventParamType, ifaceEventType.paramTypes()) {
                        ParamType paramType = eventType.paramTypes().findByName(ifaceEventParamType.name());
                        if (!paramType.isValid()) {
                            m_validationErrors.append("Thing class \"" + thingClass.name() + "\" claims to implement interface \"" + value.toString() + "\" but event \"" + eventType.name() + "\" doesn't implement event param \"" + ifaceEventParamType.name() + "\"");
                            hasError = true;
                        } else {
                            if (paramType.type() != ifaceEventParamType.type()) {
                                m_validationErrors.append("Thing class \"" + thingClass.name() + "\" claims to implement interface \"" + value.toString() + "\" but event \"" + eventType.name() + "\" param \"" + paramType.name() + "\" is of wrong type: \"" + QVariant::typeToName(paramType.type()) + "\" expected: \"" + QVariant::typeToName(ifaceEventParamType.type()) + "\"");
                                hasError = true;
                            }
                        }
                    }
                }

                interfaces.append(ThingUtils::generateInterfaceParentList(value.toString()));
            }
            interfaces.removeDuplicates();
            thingClass.setInterfaces(interfaces);

            m_thingClasses.append(thingClass);
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

        if (paramTypeId.isNull()) {
            m_validationErrors.append("Param type \"" + paramName + "\" has invalid UUID: " + pt.value("id").toString());
            hasErrors = true;
        }
        if (!verifyDuplicateUuid(paramTypeId)) {
            m_validationErrors.append("Param type \"" + paramName + "\" has duplicate UUID: " + paramTypeId.toString());
            hasErrors = true;
        }
        QVariant defaultValue = pt.value("defaultValue").toVariant();
        if (!defaultValue.isNull()) {
            // Only convert if there actually is a value as we want it to be null if it isn't specced
            // explicitly and convert() would initialize it to the variant's default value
            defaultValue.convert(t);
        }
        ParamType paramType(paramTypeId, paramName, t, defaultValue);
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
        QVariant minValue = pt.value("minValue").toVariant();
        if (!minValue.isNull()) {
            // Only convert if there actually is a value as we want it to be null if it isn't specced
            // explicitly and convert() would initialize it to the variant's default value
            minValue.convert(t);
        }
        QVariant maxValue = pt.value("maxValue").toVariant();
        if (!maxValue.isNull()) {
            // Only convert if there actually is a value as we want it to be null if it isn't specced
            // explicitly and convert() would initialize it to the variant's default value
            maxValue.convert(t);
        }
        paramType.setLimits(minValue, maxValue);
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
        // FIXME: Drop non-strict run! (see .h for more context)
        if (m_strictRun) {
            return false;
        } else {
            qCWarning(dcPluginMetadata()) << "THIS PLUGIN USES DUPLICATE UUID" << uuid.toString() << "! THIS IS NOT SUPPORTED AND MAY CAUSE RUNTIME ISSUES.";
        }
    }
    if (m_currentScopUuids.contains(uuid)) {
        return false;
    }
    m_allUuids.append(uuid);
    return true;
}
