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

#ifndef PLUGINMETADATA_H
#define PLUGINMETADATA_H

#include "types/paramtype.h"
#include "types/thingclass.h"

#include <QJsonObject>

class PluginMetadata
{
public:
    PluginMetadata();
    PluginMetadata(const QJsonObject &jsonObject, bool isBuiltIn = false, bool strict = true);

    bool isValid() const;
    QStringList validationErrors() const;

    PluginId pluginId() const;
    QString pluginName() const;
    QString pluginDisplayName() const;
    bool isBuiltIn() const;
    QStringList apiKeys() const;

    ParamTypes pluginSettings() const;

    Vendors vendors() const;
    ThingClasses thingClasses() const;

    QJsonObject jsonObject() const;

private:
    void parse(const QJsonObject &jsonObject);
    QPair<bool, ParamTypes> parseParamTypes(const QJsonArray &array);
    QPair<QStringList, QStringList> verifyFields(const QStringList &possibleFields, const QStringList &mandatoryFields, const QJsonObject &value);
    QPair<bool, Types::Unit> loadAndVerifyUnit(const QString &unitString);
    QPair<bool, Types::InputType> loadAndVerifyInputType(const QString &inputType);

    bool verifyDuplicateUuid(const QUuid &uuid);

private:
    QJsonObject m_jsonObject;
    bool m_isValid = false;
    bool m_isBuiltIn = false;
    PluginId m_pluginId;
    QString m_pluginName;
    QString m_pluginDisplayName;
    ParamTypes m_pluginSettings;
    Vendors m_vendors;
    ThingClasses m_thingClasses;
    QStringList m_apiKeys;

    QList<QUuid> m_allUuids;

    // FIXME: Due to the fact that we have duplicate UUIDs in use in plugins out there in
    // products, we can't just break those plugins now. For now, let's make the check
    // As strict as possible without breaking them, but this should be removed ASAP
    // and only m_allUuids should be used to check for dupes
    QList<QUuid> m_currentScopUuids;
    bool m_strictRun = true;

    QStringList m_validationErrors;
};

#endif // PLUGINMETADATA_H
