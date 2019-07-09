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

#ifndef PLUGINMETADATA_H
#define PLUGINMETADATA_H

#include "types/paramtype.h"
#include "types/deviceclass.h"

class PluginMetadata
{
public:
    PluginMetadata();
    PluginMetadata(const QJsonObject &jsonObject, bool isBuiltIn = false);

    bool isValid() const;

    PluginId pluginId() const;
    QString pluginName() const;
    QString pluginDisplayName() const;
    bool isBuiltIn() const;

    ParamTypes pluginSettings() const;

    Vendors vendors() const;
    DeviceClasses deviceClasses() const;


private:
    void parse(const QJsonObject &jsonObject);
    QPair<bool, ParamTypes> parseParamTypes(const QJsonArray &array);
    QPair<QStringList, QStringList> verifyFields(const QStringList &possibleFields, const QStringList &mandatoryFields, const QJsonObject &value);
    QPair<bool, Types::Unit> loadAndVerifyUnit(const QString &unitString);
    QPair<bool, Types::InputType> loadAndVerifyInputType(const QString &inputType);

    bool verifyDuplicateUuid(const QUuid &uuid);

private:
    bool m_isValid = false;
    bool m_isBuiltIn = false;
    PluginId m_pluginId;
    QString m_pluginName;
    QString m_pluginDisplayName;
    ParamTypes m_pluginSettings;
    Vendors m_vendors;
    DeviceClasses m_deviceClasses;

    QList<QUuid> m_allUuids;
};

#endif // PLUGINMETADATA_H
