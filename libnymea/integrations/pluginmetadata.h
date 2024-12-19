/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2024, nymea GmbH
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
* GNU Lesser General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU Lesser General Public License as published by the Free
* Software Foundation; version 3. This project is distributed in the hope that
* it will be useful, but WITHOUT ANY WARRANTY; without even the implied
* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
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
