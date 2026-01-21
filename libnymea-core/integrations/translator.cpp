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

#include "translator.h"
#include "thingmanagerimplementation.h"

#include <nymeasettings.h>
#include <loggingcategories.h>
#include <integrations/integrationplugin.h>

#include <QDir>
#include <QCoreApplication>
#include <QRegularExpression>

Translator::Translator(ThingManagerImplementation *thingManager):
    m_thingManager(thingManager)
{

}

Translator::~Translator()
{
    foreach (const TranslatorContext &ctx, m_translatorContexts) {
        foreach (QTranslator *t, ctx.translators) {
            delete t;
        }
    }
    m_translatorContexts.clear();
}

QString Translator::translate(const PluginId &pluginId, const QString &string, const QLocale &locale)
{
    IntegrationPlugin *plugin = m_thingManager->plugins().findById(pluginId);
    if (!plugin) {
        qCDebug(dcThingManager()) << "Unable to translate" << string << "Plugin not found";
        return string;
    }

    if (!m_translatorContexts.contains(plugin->pluginId()) || !m_translatorContexts.value(plugin->pluginId()).translators.contains(locale.name())) {
        loadTranslator(plugin, locale);
    }

    QTranslator* translator = m_translatorContexts.value(plugin->pluginId()).translators.value(locale.name());
    QString translatedString = translator->translate(plugin->pluginName().toUtf8(), string.toUtf8());
    if (translatedString.isEmpty()) {
        translatedString = translator->translate(plugin->metaObject()->className(), string.toUtf8());
    }
    return translatedString.isEmpty() ? string : translatedString;
}

void Translator::loadTranslator(IntegrationPlugin *plugin, const QLocale &locale)
{
    if (!m_translatorContexts.contains(plugin->pluginId())) {
        // Create default translator for this plugin
        TranslatorContext defaultCtx;
        defaultCtx.pluginId = plugin->pluginId();
        defaultCtx.translators.insert("en_US", new QTranslator());
        m_translatorContexts.insert(plugin->pluginId(), defaultCtx);
        if (locale == QLocale("en_US")) {
            return;
        }
    }

    bool loaded = false;
    // check if there are local translations
    QTranslator* translator = new QTranslator();

    if (plugin->isBuiltIn()) {
        if (translator->load(locale, QCoreApplication::instance()->applicationName(), "-", QDir(QCoreApplication::applicationDirPath() + "../../translations/").absolutePath(), ".qm")) {
            qCDebug(dcTranslations()) << "* Loaded translation" << locale.name() << "for plugin" << plugin->pluginName() << "from" << QDir(QCoreApplication::applicationDirPath() + "../../translations/").absolutePath() + "/" + QCoreApplication::applicationName() + "-[" + locale.name() + "].qm";
            loaded = true;
        } else if (translator->load(locale, QCoreApplication::instance()->applicationName(), "-", NymeaSettings::translationsPath(), ".qm")) {
            qCDebug(dcTranslations()) << "* Loaded translation" << locale.name() << "for plugin" << plugin->pluginName() << "from" << NymeaSettings::translationsPath()+ "/" + QCoreApplication::applicationName() + "-[" + locale.name() + "].qm";
            loaded = true;
        }
    } else {
        QString pluginId = plugin->pluginId().toString().remove(QRegularExpression("[{}]"));

        foreach (const QString &pluginPath, m_thingManager->pluginSearchDirs()) {
            if (translator->load(locale, pluginId, "-", QDir(pluginPath + "/translations/").absolutePath(), ".qm")) {
                qCDebug(dcTranslations()) << "* Loaded translation" << locale.name() << "for plugin" << plugin->pluginName() << "from" << QDir(pluginPath + "/translations/").absolutePath();
                loaded = true;
                break;
            }
            foreach (const QString &subdir, QDir(pluginPath).entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
                qCDebug(dcTranslations()) << "|- Searching for translations for" << plugin->pluginName() << "in subdir" << QDir(pluginPath + "/" + subdir + "/translations/").absolutePath() << locale << pluginId;
                if (translator->load(locale, pluginId, "-", QDir(pluginPath + "/" + subdir + "/translations/").absolutePath(), ".qm")) {
                    qCDebug(dcTranslations()) << "* Loaded translation" << locale.name() << "for plugin" << plugin->pluginName() << "from" << QDir(pluginPath + "/" + subdir + "/translations/").absolutePath() + "/" + pluginId + "-[" + locale.name() + "].qm";
                    loaded = true;
                    break;
                }
            }
            if (loaded) {
                break;
            }
        }

        // otherwise use the system translations
        if (!loaded && translator->load(locale, pluginId, "-", NymeaSettings::translationsPath(), ".qm")) {
            qCDebug(dcTranslations()) << "* Loaded translation" << locale.name() << "for" << plugin->pluginName() << "from" <<  NymeaSettings::translationsPath() + "/" + pluginId + "-[" + locale.name() + "].qm";
            loaded = true;
        }

        if (!loaded && locale.name() != "en_US") {
            qCDebug(dcTranslations()) << "* Could not load translation" << locale.name() << "for plugin" << plugin->pluginName() << "(" << pluginId << ")";
        }
    }


    if (!loaded) {
        translator = m_translatorContexts.value(plugin->pluginId()).translators.value("en_US");
    }

    if (!m_translatorContexts.contains(plugin->pluginId())) {
        TranslatorContext ctx;
        ctx.pluginId = plugin->pluginId();
        m_translatorContexts.insert(plugin->pluginId(), ctx);
    }
    m_translatorContexts[plugin->pluginId()].translators.insert(locale.name(), translator);

}
