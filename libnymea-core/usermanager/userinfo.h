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

#ifndef USERINFO_H
#define USERINFO_H

#include <QUuid>
#include <QObject>
#include <QVariant>
#include "typeutils.h"

namespace nymeaserver {

class UserInfo
{
    Q_GADGET
    Q_PROPERTY(QString username READ username)
    Q_PROPERTY(QString email READ email)
    Q_PROPERTY(QString displayName READ displayName)
    Q_PROPERTY(Types::PermissionScopes scopes READ scopes)
    Q_PROPERTY(QList<ThingId> allowedThingIds READ allowedThingIds)

public:
    UserInfo();
    UserInfo(const QString &username);

    QString username() const;
    void setUsername(const QString &username);

    QString email();
    void setEmail(const QString &email);

    QString displayName() const;
    void setDisplayName(const QString &displayName);

    Types::PermissionScopes scopes() const;
    void setScopes(Types::PermissionScopes scopes);

    void setAllowedThingIds(const QList<ThingId> &allowedThingIds);
    QList<ThingId> allowedThingIds() const;

private:
    QString m_username;
    QString m_email;
    QString m_displayName;
    Types::PermissionScopes m_scopes = Types::PermissionScopeNone;
    QList<ThingId> m_allowedThingIds;
};

class UserInfoList: public QList<UserInfo>
{
    Q_GADGET
    Q_PROPERTY(int count READ count)
public:
    Q_INVOKABLE QVariant get(int index) const;
    Q_INVOKABLE void put(const QVariant &variant);
};
}

Q_DECLARE_METATYPE(nymeaserver::UserInfo);

#endif // USERINFO_H
