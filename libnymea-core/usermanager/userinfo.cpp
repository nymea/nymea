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

#include "userinfo.h"

#include <QMetaEnum>

namespace nymeaserver {

UserInfo::UserInfo()
{

}

UserInfo::UserInfo(const QString &username):
    m_username(username)
{

}

QString UserInfo::username() const
{
    return m_username;
}

void UserInfo::setUsername(const QString &username)
{
    m_username = username;
}

QString UserInfo::email()
{
    return m_email;
}

void UserInfo::setEmail(const QString &email)
{
    m_email = email;
}

QString UserInfo::displayName() const
{
    return m_displayName;
}

void UserInfo::setDisplayName(const QString &displayName)
{
    m_displayName = displayName;
}

Types::PermissionScopes UserInfo::scopes() const
{
    return m_scopes;
}

void UserInfo::setScopes(Types::PermissionScopes scopes)
{
    m_scopes = scopes;
}

void UserInfo::setAllowedThingIds(const QList<ThingId> &allowedThingIds)
{
    m_allowedThingIds = allowedThingIds;
}

QList<ThingId> UserInfo::allowedThingIds() const
{
    return m_allowedThingIds;
}

QVariant UserInfoList::get(int index) const
{
    return QVariant::fromValue(at(index));
}

void UserInfoList::put(const QVariant &variant)
{
    append(variant.value<UserInfo>());
}

}
