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
* GNU General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU General Public License as published by the Free Software
* Foundation, GNU version 3. This project is distributed in the hope that it
* will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
* of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
* Public License for more details.
*
* You should have received a copy of the GNU General Public License along with
* this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "plugininfocompiler.h"
#include "version.h"

#include <QJsonObject>
#include <QFile>
#include <QJsonParseError>
#include <QDataStream>
#include <QDebug>
#include <QDir>

PluginInfoCompiler::PluginInfoCompiler()
{

}

int PluginInfoCompiler::compile(const QString &inputFile, const QString &outputFile, const QString outputFileExtern, const QString &translationsPath, bool strictMode)
{
    // First, process the input json...
    QFile jsonFile(inputFile);
    if (!jsonFile.open(QFile::ReadOnly)) {
        qWarning() << "Error opening input JSON file for reading. Aborting.";
        return 1;
    }
    QJsonParseError error;
    QByteArray data = jsonFile.readAll();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);
    jsonFile.close();

    if (error.error != QJsonParseError::NoError) {
        int errorOffset = error.offset;
        int newLineIndex = data.indexOf("\n");
        int lineIndex = 1;
        while (newLineIndex > 0 && errorOffset > newLineIndex) {
            data.remove(0, newLineIndex + 2);
            errorOffset -= (newLineIndex + 2);
            newLineIndex = data.indexOf("\n");
            lineIndex++;
        }
        if (newLineIndex >= 0) {
            data = data.left(newLineIndex);
        }
        QString spacer;
        for (int i = 0; i < errorOffset; i++) {
            spacer += ' ';
        }
        QDebug dbg = qWarning().nospace().noquote();
        dbg << inputFile << ":" << lineIndex << ":" << errorOffset + 2 << ": error: JSON parsing failed: " << error.errorString() << ": " << data.trimmed() << '\n';
        dbg << data << '\n';
        dbg << spacer << "^";
        return 1;
    }
    QJsonObject jsonObject = QJsonObject::fromVariantMap(jsonDoc.toVariant().toMap());

    PluginMetadata metadata(jsonObject, false, strictMode);
    if (!metadata.isValid()) {
        foreach (const QString &error, metadata.validationErrors()) {
            QDebug dbg = qWarning().noquote().nospace();
            dbg << inputFile << ": error: Plugin JSON failed validation: " << error;
        }
        return 2;
    }

    // OK. Json parsed fine. Let's open files for writing

    if (!outputFile.isEmpty()) {
        if (outputFile == "-") {
            if (!m_outputFile.open(stdout, QFile::WriteOnly | QFile::Text)) {
                qWarning() << "Error opening stdout for writing. Aborting.";
                return 1;
            }
        } else {
            m_outputFile.setFileName(outputFile);
            if (!m_outputFile.open(QFile::WriteOnly | QFile::Text)) {
                qWarning() << "Error opening output file for writing. Aborting.";
                return 1;
            }
        }
    }

    if (!outputFileExtern.isEmpty()) {
        if (outputFileExtern == "-") {
            if (!m_outputFileExtern.open(stdout, QFile::WriteOnly | QFile::Text)) {
                qWarning() << "Error opening stdout for writing. Aborting.";
                return 1;
            }
        } else {
            m_outputFileExtern.setFileName(outputFileExtern);
            if (!m_outputFileExtern.open(QFile::WriteOnly | QFile::Text)) {
                qWarning() << "Error opening output file for writing. Aborting.";
                return 1;
            }
        }
    }

    if (!translationsPath.isEmpty()) {
        QDir dir;
        if (!dir.exists(translationsPath)) {
            if(!dir.mkpath(translationsPath)) {
                qWarning() << "Error creating translation file directory" << translationsPath;
                return 1;
            }
            qDebug() << "Created translations dir";
        }

        QFile f(translationsPath + '/' + metadata.pluginId().toString().remove(QRegularExpression("[{}]")) + "-en_US.ts");
        QByteArray translationsStub = "<?xml version=\"1.0\" encoding=\"utf-8\"?><!DOCTYPE TS><TS version=\"2.1\"></TS>";
        if (!f.exists()) {
            if (!f.open(QFile::WriteOnly | QFile::Text)) {
                qWarning() << "Error creating translation file";
                return 1;
            }
            if (f.write(translationsStub) == -1) {
                qWarning() << "Error writing translation file";
                return 1;
            }
            f.close();
            qDebug() << "Created translations stub";
        }
    }


    // Files are open. Ready to write content.

    QString header;
    header.append("/* This file is generated by the nymea build system. Any changes to this file will *\n");
    header.append(" * be lost. If you want to change this file, edit the plugin's json file.          *\n");
    header.append(" *                                                                                 *\n");
    header.append(" * NOTE: This file can be included only once per plugin. If you need to access     *\n");
    header.append(" * definitions from this file in multiple source files, use                        *\n");
    header.append(" * #include extern-plugininfo.h                                                    *\n");
    header.append(" * instead and re-run qmake.                                                       */\n");
    write(header);
    writeExtern(header);

    write("#ifndef PLUGININFO_H");
    write("#define PLUGININFO_H");
    write();
    writeExtern("#ifndef EXTERNPLUGININFO_H");
    writeExtern("#define EXTERNPLUGININFO_H");
    writeExtern();

    write("#include \"typeutils.h\"");
    write();
    writeExtern("#include \"typeutils.h\"");
    writeExtern();

    write("#include <QLoggingCategory>");
    writeExtern("#include <QLoggingCategory>");
    write("#include <QObject>");
    write();
    writeExtern();

    // Include our API version in plugininfo.h so we can know against which library this plugin was built.
    write(QString("extern \"C\" const QString libnymea_api_version() { return QString(\"%1\");}").arg(LIBNYMEA_API_VERSION));
    write();

    // Declare a logging category for this plugin
    QString debugCategoryName = metadata.pluginName()[0].toUpper() + metadata.pluginName().right(metadata.pluginName().length() - 1);
    QString debugCategoryDeclaration = QString("Q_DECLARE_LOGGING_CATEGORY(dc%1)").arg(debugCategoryName);
    write(debugCategoryDeclaration);
    write(QString("Q_LOGGING_CATEGORY(dc%1, \"%1\")").arg(debugCategoryName));
    write();
    writeExtern(debugCategoryDeclaration);
    writeExtern();

    // Write down all the IDs
    writePlugin(metadata);
    write();
    writeExtern();

    // And the translations
    write(QString("const QString translations[] {"));
    for (auto i = m_translationStrings.begin(); i != m_translationStrings.end();) {
        write(QString("    //: %1").arg(i.value()));
        QString line = QString("    QT_TRANSLATE_NOOP(\"%1\", \"%2\")").arg(metadata.pluginName(), i.key());
        i++;
        if (i != m_translationStrings.end()) {
            line.append(",\n");
        }
        write(line);
    }
    write("};");
    write();

    write("#endif // PLUGININFO_H");
    writeExtern("#endif // EXTERNPLUGININFO_H");

    m_outputFile.close();
    m_outputFileExtern.close();
    return 0;
}

void PluginInfoCompiler::writePlugin(const PluginMetadata &metadata)
{
    write(QString("PluginId pluginId = PluginId(\"%1\");").arg(metadata.pluginId().toString()));
    m_translationStrings.insert(metadata.pluginDisplayName(), QString("The name of the plugin %1 (%2)").arg(metadata.pluginName()).arg(metadata.pluginId().toString()));
    writeExtern(QString("extern %1 %2;").arg("PluginId").arg("pluginId"));

    writeParams(metadata.pluginSettings(), metadata.pluginName()[0].toLower() + metadata.pluginName().right(metadata.pluginName().length() - 1), "", "plugin");

    foreach (const Vendor &vendor, metadata.vendors()) {
        writeVendor(vendor);
    }

    foreach (const ThingClass &thingClass, metadata.thingClasses()) {
        writeThingClass(thingClass);
    }

}

void PluginInfoCompiler::writeParams(const ParamTypes &paramTypes, const QString &thingClassName, const QString &typeClass, const QString &typeName)
{
    foreach (const ParamType &paramType, paramTypes) {
        QString variableName = QString("%1ParamTypeId").arg(thingClassName + typeName[0].toUpper() + typeName.right(typeName.length()-1) + typeClass + paramType.name()[0].toUpper() + paramType.name().right(paramType.name().length() -1 ));
        if (m_variableNames.contains(variableName)) {
            qWarning().nospace() << "Error: Duplicate name " << variableName << " for ParamTypeId " << paramType.id() << ". Skipping entry.";
            continue;
        }
        m_variableNames.append(variableName);

        write(QString("ParamTypeId %1 = ParamTypeId(\"%2\");").arg(variableName).arg(paramType.id().toString()));
        m_translationStrings.insert(paramType.displayName(), QString("The name of the ParamType (ThingClass: %1, %2Type: %3, ID: %4)").arg(thingClassName).arg(typeClass).arg(typeName).arg(paramType.id().toString()));
        writeExtern(QString("extern ParamTypeId %1;").arg(variableName));
    }
}

void PluginInfoCompiler::writeVendor(const Vendor &vendor)
{
    QString variableName = QString("%1VendorId").arg(vendor.name());
    if (m_variableNames.contains(variableName)) {
        qWarning().nospace() << "Error: Duplicate name " << variableName << " for Vendor " << vendor.id() << ". Skipping entry.";
        return;
    }
    m_variableNames.append(variableName);

    write(QString("VendorId %1 = VendorId(\"%2\");").arg(variableName).arg(vendor.id().toString()));
    m_translationStrings.insert(vendor.displayName(), QString("The name of the vendor (%1)").arg(vendor.id().toString()));
    writeExtern(QString("extern VendorId %1;").arg(variableName));
}

void PluginInfoCompiler::writeThingClass(const ThingClass &thingClass)
{
    QString variableName = QString("%1ThingClassId").arg(thingClass.name());
    if (m_variableNames.contains(variableName)) {
        qWarning().nospace() << "Error: Duplicate name " << variableName << " for ThingClass " << thingClass.id() << ". Skipping entry.";
        return;
    }
    m_variableNames.append(variableName);

    write(QString("ThingClassId %1 = ThingClassId(\"%2\");").arg(variableName).arg(thingClass.id().toString()));
    m_translationStrings.insert(thingClass.displayName(), QString("The name of the ThingClass (%1)").arg(thingClass.id().toString()));
    writeExtern(QString("extern ThingClassId %1;").arg(variableName));

    writeParams(thingClass.paramTypes(), thingClass.name(), "", "thing");
    writeParams(thingClass.settingsTypes(), thingClass.name(), "", "settings");
    writeParams(thingClass.discoveryParamTypes(), thingClass.name(), "", "discovery");

    writeStateTypes(thingClass.stateTypes(), thingClass.name());
    writeEventTypes(thingClass.eventTypes(), thingClass.name());
    writeActionTypes(thingClass.actionTypes(), thingClass.name());
    writeBrowserItemActionTypes(thingClass.browserItemActionTypes(), thingClass.name());
}

void PluginInfoCompiler::writeStateTypes(const StateTypes &stateTypes, const QString &thingClassName)
{
    foreach (const StateType &stateType, stateTypes) {
        QString variableName = QString("%1%2StateTypeId").arg(thingClassName, stateType.name()[0].toUpper() + stateType.name().right(stateType.name().length() - 1));
        if (m_variableNames.contains(variableName)) {
            qWarning().nospace() << "Error: Duplicate name " << variableName << " for StateType " << stateType.name() << " in ThingClass " << thingClassName << ". Skipping entry.";
            return;
        }
        m_variableNames.append(variableName);
        write(QString("StateTypeId %1 = StateTypeId(\"%2\");").arg(variableName).arg(stateType.id().toString()));
        writeExtern(QString("extern StateTypeId %1;").arg(variableName));
        m_translationStrings.insert(stateType.displayName(), QString("The name of the StateType (%1) of ThingClass %2").arg(stateType.id().toString()).arg(thingClassName));
        foreach (const QString &possibleValueDisplayName, stateType.possibleValuesDisplayNames()) {
            m_translationStrings.insert(possibleValueDisplayName, QString("The name of a possible value of StateType %1 of ThingClass %2").arg(stateType.id().toString()).arg(thingClassName));
        }
    }
}

void PluginInfoCompiler::writeEventTypes(const EventTypes &eventTypes, const QString &thingClassName)
{
    foreach (const EventType &eventType, eventTypes) {
        QString variableName = QString("%1%2EventTypeId").arg(thingClassName, eventType.name()[0].toUpper() + eventType.name().right(eventType.name().length() - 1));
        if (m_variableNames.contains(variableName)) {
            qWarning().nospace() << "Error: Duplicate name " << variableName << " for EventType " << eventType.name() << " in ThingClass " << thingClassName << ". Skipping entry.";
            return;
        }
        m_variableNames.append(variableName);
        write(QString("EventTypeId %1 = EventTypeId(\"%2\");").arg(variableName).arg(eventType.id().toString()));
        m_translationStrings.insert(eventType.displayName(), QString("The name of the EventType (%1) of ThingClass %2").arg(eventType.id().toString()).arg(thingClassName));
        writeExtern(QString("extern EventTypeId %1;").arg(variableName));

        writeParams(eventType.paramTypes(), thingClassName, "Event", eventType.name());
    }
}

void PluginInfoCompiler::writeActionTypes(const ActionTypes &actionTypes, const QString &thingClassName)
{
    foreach (const ActionType &actionType, actionTypes) {
        QString variableName = QString("%1%2ActionTypeId").arg(thingClassName, actionType.name()[0].toUpper() + actionType.name().right(actionType.name().length() - 1));
        if (m_variableNames.contains(variableName)) {
            qWarning().nospace() << "Error: Duplicate name " << variableName << " for ActionType " << actionType.name() << " in ThingClass " << thingClassName << ". Skipping entry.";
            return;
        }
        m_variableNames.append(variableName);
        write(QString("ActionTypeId %1 = ActionTypeId(\"%2\");").arg(variableName).arg(actionType.id().toString()));
        m_translationStrings.insert(actionType.displayName(), QString("The name of the ActionType (%1) of ThingClass %2").arg(actionType.id().toString()).arg(thingClassName));
        writeExtern(QString("extern ActionTypeId %1;").arg(variableName));

        writeParams(actionType.paramTypes(), thingClassName, "Action", actionType.name());
    }    
}

void PluginInfoCompiler::writeBrowserItemActionTypes(const ActionTypes &actionTypes, const QString &thingClassName)
{
    foreach (const ActionType &actionType, actionTypes) {
        QString variableName = QString("%1%2BrowserItemActionTypeId").arg(thingClassName, actionType.name()[0].toUpper() + actionType.name().right(actionType.name().length() - 1));
        if (m_variableNames.contains(variableName)) {
            qWarning().nospace() << "Error: Duplicate name " << variableName << " for Browser Item ActionType " << actionType.name() << " in ThingClass " << thingClassName << ". Skipping entry.";
            return;
        }
        m_variableNames.append(variableName);
        write(QString("ActionTypeId %1 = ActionTypeId(\"%2\");").arg(variableName).arg(actionType.id().toString()));
        m_translationStrings.insert(actionType.displayName(), QString("The name of the Browser Item ActionType (%1) of ThingClass %2").arg(actionType.id().toString()).arg(thingClassName));
        writeExtern(QString("extern ActionTypeId %1;").arg(variableName));

        writeParams(actionType.paramTypes(), thingClassName, "BrowserItemAction", actionType.name());
    }
}

void PluginInfoCompiler::write(const QString &line)
{
    if (m_outputFile.isOpen()) {
        m_outputFile.write(QString("%1\n").arg(line).toUtf8());
    }
}

void PluginInfoCompiler::writeExtern(const QString &line)
{
    if (m_outputFileExtern.isOpen()) {
        m_outputFileExtern.write(QString("%1\n").arg(line).toUtf8());
    }
}
