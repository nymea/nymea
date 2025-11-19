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

#ifndef USERMANAGER_H
#define USERMANAGER_H

#include "tokeninfo.h"
#include "userinfo.h"

#include <QObject>
#include <QSqlDatabase>

namespace nymeaserver {

class PushButtonDBusService;

class UserManager : public QObject
{
    Q_OBJECT
public:
    enum UserError {
        UserErrorNoError,
        UserErrorBackendError,
        UserErrorInvalidUserId,
        UserErrorDuplicateUserId,
        UserErrorBadPassword,
        UserErrorTokenNotFound,
        UserErrorPermissionDenied,
        UserErrorInconsistantScopes
    };
    Q_ENUM(UserError)

    explicit UserManager(const QString &dbName, QObject *parent = nullptr);

    bool initRequired() const;
    UserInfoList users() const;

    UserError createUser(const QString &username, const QString &password, const QString &email, const QString &displayName, Types::PermissionScopes scopes, const QList<ThingId> &allowedThingIds = QList<ThingId>());
    UserError changePassword(const QString &username, const QString &newPassword);
    UserError removeUser(const QString &username);
    UserError setUserScopes(const QString &username, Types::PermissionScopes scopes, const QList<ThingId> &allowedThingIds = QList<ThingId>());
    UserError setUserInfo(const QString &username, const QString &email, const QString &displayName);

    bool pushButtonAuthAvailable() const;

    QByteArray authenticate(const QString &username, const QString &password, const QString &deviceName);
    int requestPushButtonAuth(const QString &deviceName);
    void cancelPushButtonAuth(int transactionId);

    UserInfo userInfo(const QString &username = QString()) const;
    TokenInfo tokenInfo(const QByteArray &token) const;
    TokenInfo tokenInfo(const QUuid &tokenId) const;
    QList<TokenInfo> tokens(const QString &username) const;

    UserError removeToken(const QUuid &tokenId);


    bool verifyToken(const QByteArray &token);

    bool hasRestrictedThingAccess(const QByteArray &token) const;
    bool accessToThingGranted(const ThingId &thingId, const QByteArray &token);
    QList<ThingId> getAllowedThingIdsForToken(const QByteArray &token) const;

public slots:
    void onThingRemoved(const ThingId &thingId);

signals:
    void userAdded(const QString &username);
    void userRemoved(const QString &username);
    void userChanged(const QString &username);
    void pushButtonAuthFinished(int transactionId, bool success, const QByteArray &token);

    void userThingRestrictionsChanged(const nymeaserver::UserInfo &userInfo, const ThingId &thingId, bool accessGranted);

private:
    bool initDB();
    void rotate(const QString &dbName);
    bool validateUsername(const QString &username) const;
    bool validatePassword(const QString &password) const;
    bool validateToken(const QByteArray &token) const;
    bool validateScopes(Types::PermissionScopes scopes) const;

    void dumpDBError(const QString &message);

    void evaluateAllowedThingsForUser();

private slots:
    void onPushButtonPressed();

private:
    QSqlDatabase m_db;
    PushButtonDBusService *m_pushButtonDBusService = nullptr;
    int m_pushButtonTransactionIdCounter = 0;
    QPair<int, QString> m_pushButtonTransaction;

};

}

Q_DECLARE_METATYPE(nymeaserver::UserManager::UserError)

#endif // USERMANAGER_H
