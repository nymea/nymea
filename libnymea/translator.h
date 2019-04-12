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

#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include "typeutils.h"
#include "types/deviceclass.h"

#include <QTranslator>

class DevicePlugin;
class DeviceManager;

class Translator
{
public:
    Translator(DeviceManager *deviceManager);
    ~Translator();

    QString translate(const PluginId &pluginId, const QString &string, const QLocale &locale);

private:
    void loadTranslator(DevicePlugin *plugin, const QLocale &locale);

private:
    DeviceManager *m_deviceManager = nullptr;

    struct TranslatorContext {
        PluginId pluginId;
        QHash<QString, QTranslator*> translators;
    };
    QHash<PluginId, TranslatorContext> m_translatorContexts;
};

#endif // TRANSLATOR_H
