/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2024, nymea GmbH
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
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
#include <QColor>
#endif

PluginMetadata::PluginMetadata()
{

}

PluginMetadata::PluginMetadata(const QJsonObject &jsonObject, bool isBuiltIn, bool strict):
    m_jsonObject(jsonObject),
    m_isBuiltIn(isBuiltIn),
    m_strictRun(strict)
{
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
    qRegisterMetaType<QColor>("QColor");
#endif

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

QStringList PluginMetadata::apiKeys() const
{
    return m_apiKeys;
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

QJsonObject PluginMetadata::jsonObject() const
{
    return m_jsonObject;
}

void PluginMetadata::parse(const QJsonObject &jsonObject)
{
    bool hasError = false;

    // General plugin info
    QStringList pluginMandatoryJsonProperties = QStringList() << "id" << "name" << "displayName" << "vendors";
    QStringList pluginJsonProperties = QStringList() << "id" << "name" << "displayName" << "vendors" << "paramTypes" << "builtIn" << "apiKeys";
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
    foreach (const QVariant &apiKeyVariant, jsonObject.value("apiKeys").toArray().toVariantList()) {
        m_apiKeys.append(apiKeyVariant.toString());
    }

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
                                                             << "interfaces" << "providedInterfaces" << "browsable" << "discoveryParamTypes"
                                                             << "paramTypes" << "settingsTypes" << "stateTypes" << "actionTypes" << "eventTypes" << "browserItemActionTypes"
                                                             << "discoveryType";
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

            if (thingClassObject.contains("discoveryType")) {
                QString discoveryTypeString = thingClassObject.value("discoveryType").toString();
                if (discoveryTypeString == "precise") {
                    thingClass.setDiscoveryType(ThingClass::DiscoveryTypePrecise);
                } else if (discoveryTypeString == "weak") {
                    thingClass.setDiscoveryType(ThingClass::DiscoveryTypeWeak);
                } else {
                    m_validationErrors.append("Unknown discoveryType \"" + discoveryTypeString + "\" in thingClass \"" + thingClass.name() +  "\".");
                    hasError = true;
                }
            } else {
                thingClass.setDiscoveryType(ThingClass::DiscoveryTypePrecise);
            }

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

                // TODO: DEPRECATED 1.2: Remove displayNameEvent eventually (requires updating all plugins)
                QStringList stateTypeProperties = {"id", "name", "displayName", "displayNameEvent", "type", "defaultValue", "cached",
                                                   "unit", "minValue", "maxValue", "possibleValues", "writable", "displayNameAction",
                                                   "ioType", "suggestLogging", "filter"};
                QStringList mandatoryStateTypeProperties = {"id", "name", "displayName", "type", "defaultValue"};
                QPair<QStringList, QStringList> verificationResult = verifyFields(stateTypeProperties, mandatoryStateTypeProperties, st);

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
                // Print warning on deprecated fields
                if (st.contains("displayNameEvent")) {
                    m_validationErrors.contains("Thing class \"" + thingClass.name() + "\" state type \"" + stateTypeName + "\" contains deprecated displayNameEvent property");
                    // Not a real error, not setting hasError.
                }

                // If this is a writable stateType, there must be also the displayNameAction property
                if (st.contains("writable") && st.value("writable").toBool()) {
                    writableState = true;
                    if (!st.contains("displayNameAction")) {
                        m_validationErrors.append("Thing class \"" + thingClass.name() + "\" state type \"" + stateTypeName + "\" has writable state but does not define the displayNameAction property");
                        hasError = true;
                    }
                }
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
                QMetaType::Type t = static_cast<QMetaType::Type>(QMetaType::fromName(QByteArray(st.value("type").toString().toUtf8())).id());
#else
                QMetaType::Type t = static_cast<QMetaType::Type>(QVariant::nameToType(st.value("type").toString().toLatin1().data()));
#endif
                if (t == QMetaType::UnknownType) {
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
                    m_validationErrors.append("Thing class \"" + thingClass.name() + "\" state type \"" + stateTypeName + "\" has invalid unit: " + st.value("unit").toString());
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
                    QStringList possibleValuesDisplayNames;
                    foreach (const QJsonValue &possibleValueJson, st.value("possibleValues").toArray()) {
                        QVariant possibleValue = possibleValueJson.toVariant();
                        QVariant value;
                        QString name;

                        if (possibleValueJson.isObject()) {
                            if (!possibleValue.toMap().contains("value") || !possibleValue.toMap().contains("displayName")) {
                                m_validationErrors.append("Thing class \"" + thingClass.name() + "\" state type \"" + stateTypeName + "\" has invalid possible value \"" + possibleValueJson.toString() + "\" which is of object type but does not have \"value\" and \"displayName\" properties.");
                                hasError = true;
                                break;
                            }
                            value = possibleValue.toMap().value("value");
                            name = possibleValue.toMap().value("displayName").toString();
                        } else {
                            value = possibleValue;
                            name = possibleValue.toString();
                        }
                        value.convert(stateType.type());
                        possibleValues.append(value);
                        possibleValuesDisplayNames.append(name);
                    }
                    stateType.setPossibleValues(possibleValues);
                    stateType.setPossibleValuesDisplayNames(possibleValuesDisplayNames);

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
                        if (stateType.type() != QMetaType::Bool) {
                            m_validationErrors.append("Thing class \"" + thingClass.name() + "\" state type \"" + stateTypeName + "\" is marked as digital input but type is not \"bool\"");
                            hasError = true;
                            break;
                        }
                        ioType = Types::IOTypeDigitalInput;
                    } else if (ioTypeString == "digitalOutput") {
                        if (stateType.type() != QMetaType::Bool) {
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
                        if (stateType.type() != QMetaType::Double && stateType.type() != QMetaType::Int && stateType.type() != QMetaType::UInt) {
                            m_validationErrors.append("Thing class \"" + thingClass.name() + "\" state type \"" + stateTypeName + "\" is marked as analog input but type is not \"double\", \"int\" or \"uint\"");
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
                        if (stateType.type() != QMetaType::Double && stateType.type() != QMetaType::Int && stateType.type() != QMetaType::UInt) {
                            m_validationErrors.append("Thing class \"" + thingClass.name() + "\" state type \"" + stateTypeName + "\" is marked as analog output but type is not \"double\", \"int\" or \"uint\"");
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
                        m_validationErrors.append("Thing class \"" + thingClass.name() + "\" state type \"" + stateTypeName + "\" has invalid ioType value \"IOTypeNone\" which is not any of \"digitalInput\", \"digitalOutput\", \"analogInput\" or \"analogOutput\"");
                        hasError = true;
                        break;
                    }
                    stateType.setIOType(ioType);
                }

                stateType.setSuggestLogging(st.value("suggestLogging").toBool());

                if (st.contains("filter")) {
                    QString filter = st.value("filter").toString();
                    if (filter == "adaptive") {
                        stateType.setFilter(Types::StateValueFilterAdaptive);
                    } else if (!filter.isEmpty()) {
                        m_validationErrors.append("Thing class \"" + thingClass.name() + "\" state type \"" + stateTypeName + "\" has invalid filter value \"" + filter + "\". Supported filters are: \"adaptive\"");
                        hasError = true;
                    }
                }
                stateTypes.append(stateType);

                // ActionTypes for writeable StateTypes
                if (writableState) {
                    ParamType paramType(ParamTypeId(stateType.id().toString()), st.value("name").toString(), stateType.type());
                    paramType.setDisplayName(st.value("displayName").toString());
                    paramType.setAllowedValues(stateType.possibleValues());
                    paramType.setDefaultValue(stateType.defaultValue());
                    paramType.setMinValue(stateType.minValue());
                    paramType.setMaxValue(stateType.maxValue());
                    paramType.setUnit(stateType.unit());

                    ActionType actionType(ActionTypeId(stateType.id().toString()));
                    actionType.setName(stateType.name());
                    actionType.setDisplayName(st.value("displayNameAction").toString());
                    actionType.setIndex(stateType.index());
                    actionType.setParamTypes(QList<ParamType>() << paramType);
                    actionTypes.append(actionType);
                }
            }

            // ActionTypes
            index = 0;
            foreach (const QJsonValue &actionTypesJson, thingClassObject.value("actionTypes").toArray()) {
                QJsonObject at = actionTypesJson.toObject();

                QStringList actionTypeProperties = {"id", "name", "displayName", "paramTypes"};
                QStringList mandatoryActionTypeProperties = {"id", "name", "displayName"};
                QPair<QStringList, QStringList> verificationResult = verifyFields(actionTypeProperties, mandatoryActionTypeProperties, at);

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

            // EventTypes
            index = 0;
            foreach (const QJsonValue &eventTypesJson, thingClassObject.value("eventTypes").toArray()) {
                QJsonObject et = eventTypesJson.toObject();

                QStringList eventTypeProperties = {"id", "name", "displayName", "paramTypes", "suggestLogging"};
                QStringList mandatoryEventTypeProperties = {"id", "name", "displayName"};

                QPair<QStringList, QStringList> verificationResult = verifyFields(eventTypeProperties, mandatoryEventTypeProperties, et);

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
                eventType.setSuggestLogging(et.value("suggestLogging").toBool());
                eventType.setIndex(index++);

                QPair<bool, QList<ParamType> > paramVerification = parseParamTypes(et.value("paramTypes").toArray());
                if (!paramVerification.first) {
                    hasError = true;
                } else {
                    eventType.setParamTypes(paramVerification.second);
                }
                eventTypes.append(eventType);
            }

            // BrowserItemActionTypes
            index = 0;
            foreach (const QJsonValue &browserItemActionTypesJson, thingClassObject.value("browserItemActionTypes").toArray()) {
                QJsonObject at = browserItemActionTypesJson.toObject();
                QStringList actionTypeProperties = {"id", "name", "displayName", "paramTypes"};
                QStringList mandatoryActionTypeProperties = {"id", "name", "displayName"};
                QPair<QStringList, QStringList> verificationResult = verifyFields(actionTypeProperties, mandatoryActionTypeProperties, at);

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

            // Read interfaces
            QStringList interfaces;
            foreach (const QJsonValue &value, thingClassObject.value("interfaces").toArray()) {
                Interface iface = ThingUtils::loadInterface(value.toString());
                if (!iface.isValid()) {
                    m_validationErrors.append("Thing class \"" + thingClass.name() + "\" uses non-existing interface \"" + value.toString() + "\"");
                    hasError = true;
                    continue;
                }

                foreach (const InterfaceParamType &ifaceParamType, iface.paramTypes()) {
                    if (!thingClass.paramTypes().contains(ifaceParamType.name())) {
                        if (!ifaceParamType.optional()) {
                            m_validationErrors.append("Thing class \"" + thingClass.name() + "\" claims to implement interface \"" + value.toString() +
                                                      "\" but doesn't implement param \"" + ifaceParamType.name() + "\"");
                            hasError = true;
                        }
                        continue;
                    }
                    ParamType &paramType = thingClass.paramTypes()[ifaceParamType.name()];
                    if (ifaceParamType.type() != paramType.type()) {
                        m_validationErrors.append("Thing class \"" + thingClass.name() + "\" claims to implement interface \"" + value.toString() +
                                                  "\" but param \"" + paramType.name() + "\" has not matching type: \"" +
                                                  QVariant::typeToName(paramType.type()) + "\" != \"" + QVariant::typeToName(ifaceParamType.type()) + "\"");
                        hasError = true;
                    }
                    if (ifaceParamType.minValue().isValid() && !ifaceParamType.minValue().isNull()) {
                        if (ifaceParamType.minValue().toString() == "any") {
                            if (paramType.minValue().isNull()) {
                                m_validationErrors.append("Thing class \"" + thingClass.name() + "\" claims to implement interface \"" + value.toString() +
                                                          "\" but param \"" + paramType.name() + "\" has no minimum value defined.");
                                hasError = true;
                            }
                        } else if (ifaceParamType.minValue() != paramType.minValue()) {
                            m_validationErrors.append("Thing class \"" + thingClass.name() + "\" claims to implement interface \"" + value.toString() +
                                                      "\" but param \"" + paramType.name() + "\" has not matching minimum value: \"" +
                                                      ifaceParamType.minValue().toString() + "\" != \"" + paramType.minValue().toString() + "\"");
                            hasError = true;
                        }
                    }
                    if (ifaceParamType.maxValue().isValid() && !ifaceParamType.maxValue().isNull()) {
                        if (ifaceParamType.maxValue().toString() == "any") {
                            if (paramType.maxValue().isNull()) {
                                m_validationErrors.append("Thing class \"" + thingClass.name() + "\" claims to implement interface \"" + value.toString() +
                                                          "\" but param \"" + paramType.name() + "\" has no maximum value defined.");
                                hasError = true;
                            }
                        } else if (ifaceParamType.maxValue() != paramType.maxValue()) {
                            m_validationErrors.append("Thing class \"" + thingClass.name() + "\" claims to implement interface \"" + value.toString() +
                                                      "\" but param \"" + paramType.name() + "\" has not matching maximum value: \"" +
                                                      ifaceParamType.maxValue().toString() + "\" != \"" + paramType.minValue().toString() + "\"");
                            hasError = true;
                        }
                    }
                    if (!ifaceParamType.allowedValues().isEmpty() && ifaceParamType.allowedValues() != paramType.allowedValues()) {
                        m_validationErrors.append("Thing class \"" + thingClass.name() + "\" claims to implement interface \"" + value.toString() + "\" but param \"" +
                                                  paramType.name() + "\" has not matching allowed values.");
                        hasError = true;
                    }
                    if (ifaceParamType.unit() != Types::UnitNone && ifaceParamType.unit() != paramType.unit()) {
                        QMetaEnum unitEnum = QMetaEnum::fromType<Types::Unit>();
                        m_validationErrors.append("Thing class \"" + thingClass.name() + "\" claims to implement interface \"" + value.toString() + "\" but param \"" +
                                                  paramType.name() + "\" has not matching unit: \"" + unitEnum.valueToKey(ifaceParamType.unit()) + "\" != \"" + unitEnum.valueToKey(paramType.unit()));
                        hasError = true;
                    }
                }


                foreach (const InterfaceStateType &ifaceStateType, iface.stateTypes()) {
                    if (!stateTypes.contains(ifaceStateType.name())) {
                        if (!ifaceStateType.optional()) {
                            m_validationErrors.append("Thing class \"" + thingClass.name() + "\" claims to implement interface \"" + value.toString() + "\" but doesn't implement state \"" + ifaceStateType.name() + "\"");
                            hasError = true;
                        }
                        continue;
                    }
                    StateType &stateType = stateTypes[ifaceStateType.name()];
                    if (ifaceStateType.type() != stateType.type()) {
                        m_validationErrors.append("Thing class \"" + thingClass.name() + "\" claims to implement interface \"" + value.toString() + "\" but state \"" + stateType.name() + "\" has not matching type: \"" + QVariant::typeToName(stateType.type()) + "\" != \"" + QVariant::typeToName(ifaceStateType.type()) + "\"");
                        hasError = true;
                    }
                    if (ifaceStateType.minValue().isValid() && !ifaceStateType.minValue().isNull()) {
                        if (ifaceStateType.minValue().toString() == "any") {
                            if (stateType.minValue().isNull()) {
                                m_validationErrors.append("Thing class \"" + thingClass.name() + "\" claims to implement interface \"" + value.toString() + "\" but state \"" + stateType.name() + "\" has no minimum value defined.");
                                hasError = true;
                            }
                        } else if (ifaceStateType.minValue() != stateType.minValue()) {
                            m_validationErrors.append("Thing class \"" + thingClass.name() + "\" claims to implement interface \"" + value.toString() + "\" but state \"" + stateType.name() + "\" has not matching minimum value: \"" + ifaceStateType.minValue().toString() + "\" != \"" + stateType.minValue().toString() + "\"");
                            hasError = true;
                        }
                    }
                    if (ifaceStateType.maxValue().isValid() && !ifaceStateType.maxValue().isNull()) {
                        if (ifaceStateType.maxValue().toString() == "any") {
                            if (stateType.maxValue().isNull()) {
                                m_validationErrors.append("Thing class \"" + thingClass.name() + "\" claims to implement interface \"" + value.toString() + "\" but state \"" + stateType.name() + "\" has no maximum value defined.");
                                hasError = true;
                            }
                        } else if (ifaceStateType.maxValue() != stateType.maxValue()) {
                            m_validationErrors.append("Thing class \"" + thingClass.name() + "\" claims to implement interface \"" + value.toString() + "\" but state \"" + stateType.name() + "\" has not matching maximum value: \"" + ifaceStateType.maxValue().toString() + "\" != \"" + stateType.minValue().toString() + "\"");
                            hasError = true;
                        }
                    }
                    if (!ifaceStateType.possibleValues().isEmpty() && ifaceStateType.possibleValues() != stateType.possibleValues()) {
                        m_validationErrors.append("Thing class \"" + thingClass.name() + "\" claims to implement interface \"" + value.toString() + "\" but state \"" + stateType.name() + "\" has not matching allowed values.");
                        hasError = true;
                    }
                    if (ifaceStateType.unit() != Types::UnitNone && ifaceStateType.unit() != stateType.unit()) {
                        QMetaEnum unitEnum = QMetaEnum::fromType<Types::Unit>();
                        m_validationErrors.append("Thing class \"" + thingClass.name() + "\" claims to implement interface \"" + value.toString() + "\" but state \"" + stateType.name() + "\" has not matching unit: \"" + unitEnum.valueToKey(ifaceStateType.unit()) + "\" != \"" + unitEnum.valueToKey(stateType.unit()));
                        hasError = true;
                    }

                    // Override logged property as the interface has higher priority than the plugin dev
                    if (ifaceStateType.loggingOverride()) {
                        stateType.setSuggestLogging(ifaceStateType.suggestLogging());
                    }
                }

                foreach (const InterfaceActionType &ifaceActionType, iface.actionTypes()) {
                    if (!actionTypes.contains(ifaceActionType.name())) {
                        if (!ifaceActionType.optional()) {
                            m_validationErrors.append("Thing class \"" + thingClass.name() + "\" claims to implement interface \"" + value.toString() + "\" but doesn't implement action \"" + ifaceActionType.name() + "\"");
                            hasError = true;
                        }
                        continue;
                    }
                    ActionType &actionType = actionTypes[ifaceActionType.name()];
                    // Verify the params as required by the interface are available
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
                            if (ifaceActionParamType.minValue() == "any") {
                                if (paramType.minValue().isNull()) {
                                    m_validationErrors.append("Thing class \"" + thingClass.name() + "\" claims to implement interface \"" + value.toString() + "\" but action \"" + actionType.name() + "\" param \"" + paramType.name() + "\" is missing a minimum value");
                                    hasError = true;
                                }
                            } else if (!ifaceActionParamType.minValue().isNull()) {
                                if (paramType.minValue() != ifaceActionParamType.minValue()) {
                                    m_validationErrors.append("Thing class \"" + thingClass.name() + "\" claims to implement interface \"" + value.toString() + "\" but action \"" + actionType.name() + "\" param \"" + paramType.name() + "\" has not matching minimum value: \"" + paramType.minValue().toString() + "\" != \"" + ifaceActionParamType.minValue().toString() + "\"");
                                    hasError = true;
                                }
                            }
                            if (ifaceActionParamType.maxValue() == "any") {
                                if (paramType.maxValue().isNull()) {
                                    m_validationErrors.append("Thing class \"" + thingClass.name() + "\" claims to implement interface \"" + value.toString() + "\" but action \"" + actionType.name() + "\" param \"" + paramType.name() + "\" is missing a maximum value");
                                    hasError = true;
                                }
                            } else if (!ifaceActionParamType.maxValue().isNull()) {
                                if (paramType.maxValue() != ifaceActionParamType.maxValue()) {
                                    m_validationErrors.append("Thing class \"" + thingClass.name() + "\" claims to implement interface \"" + value.toString() + "\" but action \"" + actionType.name() + "\" param \"" + paramType.name() + "\" has not matching maximum value: \"" + paramType.maxValue().toString() + "\" != \"" + ifaceActionParamType.maxValue().toString() + "\"");
                                    hasError = true;
                                }
                            }
                            if (ifaceActionParamType.defaultValue() == "any") {
                                if (paramType.defaultValue().isNull()) {
                                    m_validationErrors.append("Thing class \"" + thingClass.name() + "\" claims to implement interface \"" + value.toString() + "\" but action \"" + actionType.name() + "\" param \"" + paramType.name() + "\" is missing a default value");
                                    hasError = true;
                                }
                            } else if (!ifaceActionParamType.defaultValue().isNull()) {
                                if (paramType.defaultValue() != ifaceActionParamType.defaultValue()) {
                                    m_validationErrors.append("Thing class \"" + thingClass.name() + "\" claims to implement interface \"" + value.toString() + "\" but action \"" + actionType.name() + "\" param \"" + paramType.name() + "\" is has incompatible default value: \"" + paramType.defaultValue().toString() + "\" != \"" + ifaceActionParamType.defaultValue().toString() + "\"");
                                    hasError = true;
                                }
                            }
                        }
                    }

                    // Verify that additional params don't "break" the interface
                    // If there's an action without params in the interface, the actual action still can have params
                    // but those params must have a default value so they still can be invoked without params
                    foreach (const ParamType &paramType, actionType.paramTypes()) {
                        // Note: We can't use ParamType::isValid() on ParamTypes from interfaces because the don't
                        // have an ID set and aren't valid in any case. Let's instead check if the returned ParamType's
                        // name is set or not.
                        if (ifaceActionType.paramTypes().findByName(paramType.name()).name().isEmpty()) {
                            if (paramType.defaultValue().isNull()) {
                                m_validationErrors.append("Thing class \"" + thingClass.name() + "\" claims to implement interface \"" + value.toString() + "\" but action \"" + actionType.name() + "\" param \"" + paramType.name() + "\" is missing a default value as the interface requires this action to be executable without params.");
                                hasError = true;
                            }
                        }
                    }
                }

                foreach (const InterfaceEventType &ifaceEventType, iface.eventTypes()) {
                    if (!eventTypes.contains(ifaceEventType.name())) {
                        if (!ifaceEventType.optional()) {
                            m_validationErrors.append("Thing class \"" + thingClass.name() + "\" claims to implement interface \"" + value.toString() + "\" but doesn't implement event \"" + ifaceEventType.name() + "\"");
                            hasError = true;
                        }
                        continue;
                    }
                    EventType &eventType = eventTypes[ifaceEventType.name()];

                    // Verify all the params as required by the interface are available
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
                            foreach (const QVariant &allowedValue, ifaceEventParamType.allowedValues()) {
                                if (!paramType.allowedValues().contains(allowedValue)) {
                                    m_validationErrors.append("Thing class \"" + thingClass.name() + "\" claims to implement interface \"" + value.toString() + "\" but event \"" + eventType.name() + "\" param \"" + paramType.name() + "\" is missing allowed value \"" + allowedValue.toString() + "\"");
                                    hasError = true;
                                }
                            }
                            if (ifaceEventParamType.minValue() == "any") {
                                if (paramType.minValue().isNull()) {
                                    m_validationErrors.append("Thing class \"" + thingClass.name() + "\" claims to implement interface \"" + value.toString() + "\" but event \"" + eventType.name() + "\" param \"" + paramType.name() + "\" is missing a minimum value");
                                    hasError = true;
                                }
                            } else if (!ifaceEventParamType.minValue().isNull()) {
                                if (paramType.minValue() != ifaceEventParamType.minValue()) {
                                    m_validationErrors.append("Thing class \"" + thingClass.name() + "\" claims to implement interface \"" + value.toString() + "\" but event \"" + eventType.name() + "\" param \"" + paramType.name() + "\" has not matching minimum value: \"" + paramType.minValue().toString() + "\" != \"" + ifaceEventParamType.minValue().toString() + "\"");
                                    hasError = true;
                                }
                            }
                            if (ifaceEventParamType.maxValue() == "any") {
                                if (paramType.maxValue().isNull()) {
                                    m_validationErrors.append("Thing class \"" + thingClass.name() + "\" claims to implement interface \"" + value.toString() + "\" but event \"" + eventType.name() + "\" param \"" + paramType.name() + "\" is missing a maximum value");
                                    hasError = true;
                                }
                            } else if (!ifaceEventParamType.maxValue().isNull()) {
                                if (paramType.maxValue() != ifaceEventParamType.maxValue()) {
                                    m_validationErrors.append("Thing class \"" + thingClass.name() + "\" claims to implement interface \"" + value.toString() + "\" but event \"" + eventType.name() + "\" param \"" + paramType.name() + "\" has not matching maximum value: \"" + paramType.maxValue().toString() + "\" != \"" + ifaceEventParamType.maxValue().toString() + "\"");
                                    hasError = true;
                                }
                            }
                            if (ifaceEventParamType.defaultValue().toString() == "any") {
                                if (paramType.defaultValue().isNull()) {
                                    m_validationErrors.append("Thing class \"" + thingClass.name() + "\" claims to implement interface \"" + value.toString() + "\" but event \"" + eventType.name() + "\" param \"" + paramType.name() + "\" is missing a default value");
                                    hasError = true;
                                }
                            } else if (!ifaceEventParamType.defaultValue().isNull()) {
                                if (paramType.defaultValue() != ifaceEventParamType.defaultValue()) {
                                    m_validationErrors.append("Thing class \"" + thingClass.name() + "\" claims to implement interface \"" + value.toString() + "\" but event \"" + eventType.name() + "\" param \"" + paramType.name() + "\" is has incompatible default value: \"" + paramType.defaultValue().toString() + "\" != \"" + ifaceEventParamType.defaultValue().toString() + "\"");
                                    hasError = true;
                                }
                            }
                        }
                    }

                    // Override logging
                    if (ifaceEventType.loggingOverride()) {
                        eventType.setSuggestLogging(ifaceEventType.suggestLogging());
                    }

                    // Note: No need to check for default values (as with actions) for additional params as
                    // an emitted event always needs to have params filled with values. The client might use them or not...
                }

                interfaces.append(ThingUtils::generateInterfaceParentList(value.toString()));
            }
            interfaces.removeDuplicates();
            thingClass.setInterfaces(interfaces);

            QStringList providedInterfaces;
            foreach (const QJsonValue &value, thingClassObject.value("providedInterfaces").toArray()) {
                Interface iface = ThingUtils::loadInterface(value.toString());
                if (!iface.isValid()) {
                    m_validationErrors.append("Thing class \"" + thingClass.name() + "\" uses non-existing interface \"" + value.toString() + "\" in providedInterfaces.");
                    hasError = true;
                    continue;
                }
                providedInterfaces.append(iface.name());
            }

            thingClass.setProvidedInterfaces(providedInterfaces);

            thingClass.setStateTypes(stateTypes);
            thingClass.setActionTypes(actionTypes);
            thingClass.setEventTypes(eventTypes);
            thingClass.setBrowserItemActionTypes(browserItemActionTypes);

            m_thingClasses.append(thingClass);
        }
    }
    if (!hasError) {
        m_isValid = true;
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
        QMetaType::Type t = static_cast<QMetaType::Type>(QVariant::nameToType(pt.value("type").toString().toLatin1().data()));
        if (t == QMetaType::UnknownType) {
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
                m_validationErrors.append("Param type \"" + paramName + "\" has invalid inputType \"" + pt.value("type").toString() + "\"");
                hasErrors = true;
            } else {
                paramType.setInputType(inputTypeVerification.second);
            }
        }

        // set the unit if there is any
        if (pt.contains("unit")) {
            QPair<bool, Types::Unit> unitVerification = loadAndVerifyUnit(pt.value("unit").toString());
            if (!unitVerification.first) {
                m_validationErrors.append("Param type \"" + paramName + "\" has invalid unit \"" + pt.value("unit").toString() + "\"");
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
            // Using regular qWarning here as I'm struggling with making the nymea debug categories work in a pic only build
            // This is only used in special cirumstances and it's probably ok that one cannot filter away this warning
            qWarning() << "THIS PLUGIN USES DUPLICATE UUID" << uuid.toString() << "! THIS IS NOT SUPPORTED AND MAY CAUSE RUNTIME ISSUES.";
        }
    }
    if (m_currentScopUuids.contains(uuid)) {
        return false;
    }
    m_allUuids.append(uuid);
    return true;
}
