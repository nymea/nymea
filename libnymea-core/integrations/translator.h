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

#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include "typeutils.h"
#include "types/thingclass.h"

#include <QTranslator>

class IntegrationPlugin;
class ThingManagerImplementation;

class Translator
{
public:
    Translator(ThingManagerImplementation *thingManager);
    ~Translator();

    QString translate(const PluginId &pluginId, const QString &string, const QLocale &locale);

private:
    void loadTranslator(IntegrationPlugin *plugin, const QLocale &locale);

private:
    ThingManagerImplementation *m_thingManager = nullptr;

    struct TranslatorContext {
        PluginId pluginId;
        QHash<QString, QTranslator*> translators;
    };
    QHash<PluginId, TranslatorContext> m_translatorContexts;
};

#endif // TRANSLATOR_H
