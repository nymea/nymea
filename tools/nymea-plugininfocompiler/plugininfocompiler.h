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

    int compile(const QString &inputFile, const QString &outputFile, const QString outputFileExtern, const QString &translationsPath);


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
