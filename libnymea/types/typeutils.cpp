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

#include "typeutils.h"

#include <QMetaEnum>

QStringList Types::scopesToStringList(Types::PermissionScopes scopes)
{
    QStringList ret;
    QMetaEnum metaEnum = QMetaEnum::fromType<PermissionScope>();
    for (int i = 0; i < metaEnum.keyCount(); i++) {
        if (scopes.testFlag(static_cast<PermissionScope>(metaEnum.value(i)))) {
            ret << metaEnum.key(i);
        }
    }
    return ret;
}

QString Types::scopeToString(Types::PermissionScope scope)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<PermissionScope>();
    return metaEnum.valueToKey(scope);
}

QStringList Types::thingIdsToStringList(const QList<ThingId> &thingIds)
{
    QStringList stringList;
    foreach (const ThingId &thingId, thingIds)
        stringList.append(thingId.toString());

    return stringList;
}

QList<ThingId> Types::thingIdsFromStringList(const QStringList &stringList)
{
    QList<ThingId> thingIds;
    foreach (const QString &idString, stringList)
        thingIds.append(ThingId(idString));

    return thingIds;
}

Types::PermissionScope Types::scopeFromString(const QString &scopeString)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<PermissionScope>();
    return static_cast<PermissionScope>(metaEnum.keyToValue(scopeString.toUtf8()));
}

Types::PermissionScopes Types::scopesFromStringList(const QStringList &scopeList)
{
    PermissionScopes ret;
    QMetaEnum metaEnum = QMetaEnum::fromType<PermissionScopes>();
    for (int i = 0; i < metaEnum.keyCount(); i++) {
        if (scopeList.contains(metaEnum.key(i))) {
            ret |= static_cast<PermissionScope>(metaEnum.value(i));
        }
    }
    return ret;
}

