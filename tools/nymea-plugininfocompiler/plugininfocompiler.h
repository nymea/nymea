// SPDX-License-Identifier: GPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2025, chargebyte austria GmbH
*
* This file is part of nymea.
*
* nymea is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* nymea is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with nymea. If not, see <https://www.gnu.org/licenses/>.
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef PLUGININFOCOMPILER_H
#define PLUGININFOCOMPILER_H

#include <QFile>
#include <QString>

#include "integrations/pluginmetadata.h"
#include "types/paramtype.h"

class PluginInfoCompiler
{
public:
    PluginInfoCompiler();

    int compile(const QString &inputFile, const QString &outputFile, const QString outputFileExtern, const QString &translationsPath, bool strictMode);

private:
    void writePlugin(const PluginMetadata &metadata);
    void writeParams(const ParamTypes &paramTypes, const QString &thingClassName, const QString &typeClass, const QString &typeName);
    void writeVendor(const Vendor &vendor);
    void writeThingClass(const ThingClass &thingClass);
    void writeStateTypes(const StateTypes &stateTypes, const QString &thingClassName);
    void writeEventTypes(const EventTypes &eventTypes, const QString &thingClassName);
    void writeActionTypes(const ActionTypes &actionTypes, const QString &thingClassName);
    void writeBrowserItemActionTypes(const ActionTypes &actionTypes, const QString &thingClassName);

    void write(const QString &line = QString());
    void writeExtern(const QString &line = QString());

    QMultiMap<QString, QString> m_translationStrings;

    QStringList m_variableNames;

    QFile m_outputFile;
    QFile m_outputFileExtern;
};

#endif // PLUGININFOCOMPILER_H
