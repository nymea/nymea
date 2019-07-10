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

#ifndef PLUGININFOCOMPILER_H
#define PLUGININFOCOMPILER_H

#include <QString>
#include <QFile>

#include "types/paramtype.h"
#include "devices/pluginmetadata.h"

class PluginInfoCompiler
{
public:
    PluginInfoCompiler();

    int compile(const QString &inputFile, const QString &outputFile, const QString outputFileExtern);


private:
    void writePlugin(const PluginMetadata &metadata);
    void writeParams(const ParamTypes &paramTypes, const QString &deviceClassName, const QString &typeClass, const QString &typeName);
    void writeVendor(const Vendor &vendor);
    void writeDeviceClass(const DeviceClass &deviceClass);
    void writeStateTypes(const StateTypes &stateTypes, const QString &deviceClassName);
    void writeEventTypes(const EventTypes &eventTypes, const QString &deviceClassName);
    void writeActionTypes(const ActionTypes &actionTypes, const QString &deviceClassName);
    void writeBrowserItemActionTypes(const ActionTypes &actionTypes, const QString &deviceClassName);

    void write(const QString &line = QString());
    void writeExtern(const QString &line = QString());

    QMultiMap<QString, QString> m_translationStrings;

    QStringList m_variableNames;

    QFile m_outputFile;
    QFile m_outputFileExtern;
};

#endif // PLUGININFOCOMPILER_H
