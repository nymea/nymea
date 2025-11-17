// SPDX-License-Identifier: LGPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2025, chargebyte austria GmbH
*
* This file is part of nymea.
*
* nymea is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* as published by the Free Software Foundation, either version 3
* of the License, or (at your option) any later version.
*
* nymea is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with nymea. If not, see <https://www.gnu.org/licenses/>.
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "debughandler.h"

#include "nymeacore.h"
#include "nymeasettings.h"
#include "loggingcategories.h"

namespace nymeaserver {

DebugHandler::DebugHandler(QObject *parent)
    : JsonHandler{parent}
{
    // Types
    registerEnum<DebugHandler::DebugError>();
    registerEnum<DebugHandler::LoggingLevel>();
    registerEnum<DebugHandler::LoggingCategoryType>();

    QVariantMap loggingCategory;
    loggingCategory.insert("name", enumValueName(String));
    loggingCategory.insert("level", enumRef<DebugHandler::LoggingLevel>());
    loggingCategory.insert("type", enumRef<DebugHandler::LoggingCategoryType>());
    registerObject("LoggingCategory", loggingCategory);

    QVariantMap params, returns;
    QString description;

    // Methods
    params.clear(); returns.clear();
    description = "Get all available logging categories.";
    returns.insert("loggingCategories", QVariantList() << objectRef("LoggingCategory"));
    registerMethod("GetLoggingCategories", description, params, returns);

    params.clear(); returns.clear();
    description = "Set the logging category with the given name to the given logging level.";
    params.insert("name", enumValueName(String));
    params.insert("level", enumRef<DebugHandler::LoggingLevel>());
    returns.insert("debugError", enumRef<DebugHandler::DebugError>());
    registerMethod("SetLoggingCategoryLevel", description, params, returns);

    // Notifications
    params.clear(); returns.clear();
    description = "Emitted whenever a logging category has changed the logging level.";
    params.insert("name", enumValueName(String));
    params.insert("level", enumRef<DebugHandler::LoggingLevel>());
    registerNotification("LoggingCategoryLevelChanged", description, params);
}

QString DebugHandler::name() const
{
    return "Debug";
}

JsonReply *DebugHandler::GetLoggingCategories(const QVariantMap &params)
{
    Q_UNUSED(params)

    QVariantList categories;
    QStringList allCategories;

    NymeaSettings settings(NymeaSettings::SettingsRoleGlobal);
    settings.beginGroup("LoggingRules");

    // System
    foreach (const QString &loggingCategory, NymeaCore::loggingFilters()) {
        allCategories.append(loggingCategory);

        LoggingLevel level = DebugHandler::LoggingLevelCritical;
        if (settings.value(QString("%1.warning").arg(loggingCategory), true).toBool()) {
            level = DebugHandler::LoggingLevelWarning;
        }
        if (settings.value(QString("%1.info").arg(loggingCategory), false).toBool()) {
            level = DebugHandler::LoggingLevelInfo;
        }
        if (settings.value(QString("%1.debug").arg(loggingCategory), false).toBool()) {
            level = DebugHandler::LoggingLevelDebug;
        }

        QVariantMap category;
        category.insert("name", loggingCategory);
        category.insert("level", enumValueName(level));
        category.insert("type", enumValueName(LoggingCategoryTypeSystem));
        categories.append(category);
    }

    // Plugins
    foreach (const QString &loggingCategory, NymeaCore::loggingFiltersPlugins()) {
        allCategories.append(loggingCategory);

        LoggingLevel level = DebugHandler::LoggingLevelCritical;
        if (settings.value(QString("%1.warning").arg(loggingCategory), true).toBool()) {
            level = DebugHandler::LoggingLevelWarning;
        }
        if (settings.value(QString("%1.info").arg(loggingCategory), false).toBool()) {
            level = DebugHandler::LoggingLevelInfo;
        }
        if (settings.value(QString("%1.debug").arg(loggingCategory), false).toBool()) {
            level = DebugHandler::LoggingLevelDebug;
        }

        QVariantMap category;
        category.insert("name", loggingCategory);
        category.insert("level", enumValueName(level));
        category.insert("type", enumValueName(LoggingCategoryTypePlugin));
        categories.append(category);
    }

    // Now create all categories, which are not nymea system related
    foreach (const QString &categoryFilter, settings.childGroups()) {
        QStringList categoryParts =  categoryFilter.split(".");
        if (categoryParts.isEmpty())
            continue;

        QString loggingCategory = categoryParts.first();
        if (allCategories.contains(loggingCategory) || loggingCategory.isEmpty())
            continue;

        LoggingLevel level = DebugHandler::LoggingLevelCritical;
        if (settings.value(QString("%1.warning").arg(loggingCategory), true).toBool()) {
            level = DebugHandler::LoggingLevelWarning;
        }
        if (settings.value(QString("%1.info").arg(loggingCategory), false).toBool()) {
            level = DebugHandler::LoggingLevelInfo;
        }
        if (settings.value(QString("%1.debug").arg(loggingCategory), false).toBool()) {
            level = DebugHandler::LoggingLevelDebug;
        }

        QVariantMap category;
        category.insert("name", loggingCategory);
        category.insert("level", enumValueName(level));
        category.insert("type", enumValueName(LoggingCategoryTypeCustom));
        categories.append(category);
    }

    settings.endGroup(); // LoggingRules

    QVariantMap returns;
    returns.insert("loggingCategories", categories);
    return createReply(returns);
}

JsonReply *DebugHandler::SetLoggingCategoryLevel(const QVariantMap &params)
{
    QString category = params.value("name").toString();
    LoggingLevel level = enumNameToValue<DebugHandler::LoggingLevel>(params.value("level").toString());

    NymeaSettings settings(NymeaSettings::SettingsRoleGlobal);
    settings.beginGroup("LoggingRules");

    qCDebug(dcDebugServer()) << "Logging category" << category << level;

    switch (level) {
    case LoggingLevelCritical:
        settings.setValue(QString("%1.warning").arg(category), false);
        settings.setValue(QString("%1.info").arg(category), false);
        settings.setValue(QString("%1.debug").arg(category), false);
        break;
    case LoggingLevelWarning:
        settings.setValue(QString("%1.warning").arg(category), true);
        settings.setValue(QString("%1.info").arg(category), false);
        settings.setValue(QString("%1.debug").arg(category), false);
        break;
    case LoggingLevelInfo:
        settings.setValue(QString("%1.warning").arg(category), true);
        settings.setValue(QString("%1.info").arg(category), true);
        settings.setValue(QString("%1.debug").arg(category), false);
        break;
    case LoggingLevelDebug:
        settings.setValue(QString("%1.warning").arg(category), true);
        settings.setValue(QString("%1.info").arg(category), true);
        settings.setValue(QString("%1.debug").arg(category), true);
        break;
    }

    // Update logging filter rules according to the nw settings
    QStringList loggingRules;
    loggingRules << "*.debug=false";
    // Load the rules from nymead.conf file and append them to the rules
    foreach (const QString &category, settings.childKeys()) {
        loggingRules << QString("%1=%2").arg(category).arg(settings.value(category, "false").toString());
    }
    settings.endGroup();
    QLoggingCategory::setFilterRules(loggingRules.join('\n'));

    emit LoggingCategoryLevelChanged(params);

    QVariantMap returns;
    returns.insert("debugError", enumValueName(DebugErrorNoError));
    return createReply(returns);
 }

}
