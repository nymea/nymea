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

#ifndef USERSHANDLER_H
#define USERSHANDLER_H

#include <QObject>

#include "jsonrpc/jsonhandler.h"

namespace nymeaserver {

class UserManager;

class UsersHandler : public JsonHandler
{
    Q_OBJECT
public:
    explicit UsersHandler(UserManager *userManager, QObject *parent = nullptr);

    QString name() const override;

    Q_INVOKABLE JsonReply *CreateUser(const QVariantMap &params);
    Q_INVOKABLE JsonReply *ChangePassword(const QVariantMap &params, const JsonContext &context);
    Q_INVOKABLE JsonReply *ChangeUserPassword(const QVariantMap &params, const JsonContext &context);
    Q_INVOKABLE JsonReply *GetUserInfo(const QVariantMap &params, const JsonContext &context);
    Q_INVOKABLE JsonReply *GetTokens(const QVariantMap &params, const JsonContext &context);
    Q_INVOKABLE JsonReply *RemoveToken(const QVariantMap &params, const JsonContext &context);
    Q_INVOKABLE JsonReply *GetUsers(const QVariantMap &params);
    Q_INVOKABLE JsonReply *RemoveUser(const QVariantMap &params, const JsonContext &context);
    Q_INVOKABLE JsonReply *SetUserScopes(const QVariantMap &params, const JsonContext &context);
    Q_INVOKABLE JsonReply *SetUserInfo(const QVariantMap &params, const JsonContext &context);

signals:
    void UserAdded(const QVariantMap &params);
    void UserRemoved(const QVariantMap &params);
    void UserChanged(const QVariantMap &params);

private:
    UserManager *m_userManager = nullptr;
    QHash<int, QUuid> m_pushButtonTransactions;

};

}

#endif // USERSHANDLER_H
