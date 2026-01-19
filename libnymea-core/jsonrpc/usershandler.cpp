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

#include "usershandler.h"
#include "usermanager/usermanager.h"
#include "usermanager/userinfo.h"

#include "loggingcategories.h"

namespace nymeaserver {

UsersHandler::UsersHandler(UserManager *userManager, QObject *parent):
    JsonHandler(parent),
    m_userManager(userManager)
{
    registerFlag<Types::PermissionScope, Types::PermissionScopes>();
    registerObject<UserInfo, UserInfoList>();
    registerObject<TokenInfo, TokenInfoList>();

    QVariantMap params, returns;
    QString description;

    params.clear(); returns.clear();
    description = "Create a new user in the API with the given username and password. Use scopes to define the permissions for the new user. If the user has not the permission \"PermissionScopeAccessAllThings\", the list of things this user has access to can be defined in the \"allowedThingIds\" property. If no scopes are given, this user will be an admin user. Call Authenticate after this to obtain a device token for this user.";
    params.insert("username", enumValueName(String));
    params.insert("password", enumValueName(String));
    params.insert("o:email", enumValueName(String));
    params.insert("o:displayName", enumValueName(String));
    params.insert("o:scopes", flagRef<Types::PermissionScopes>());
    params.insert("o:allowedThingIds", QVariantList() << enumValueName(Uuid));
    returns.insert("error", enumRef<UserManager::UserError>());
    registerMethod("CreateUser", description, params, returns);

    params.clear(); returns.clear();
    description = "Change the password for the currently logged in user.";
    params.insert("newPassword", enumValueName(String));
    returns.insert("error", enumRef<UserManager::UserError>());
    registerMethod("ChangePassword", description, params, returns); // TODO: PermissionScopeChangeUserInfos

    params.clear(); returns.clear();
    description = "Change the password for the given user. All tokens for this user will be removed in order to force all clients to log in again.";
    params.insert("username", enumValueName(String));
    params.insert("newPassword", enumValueName(String));
    returns.insert("error", enumRef<UserManager::UserError>());
    registerMethod("ChangeUserPassword", description, params, returns);

    params.clear(); returns.clear();
    description = "Get info about the current token (the currently logged in user).";
    returns.insert("o:userInfo", objectRef<UserInfo>());
    returns.insert("error", enumRef<UserManager::UserError>());
    registerMethod("GetUserInfo", description, params, returns, Types::PermissionScopeNone);

    params.clear(); returns.clear();
    description = "Get all the tokens for the current user.";
    returns.insert("o:tokenInfoList", objectRef<TokenInfoList>());
    returns.insert("error", enumRef<UserManager::UserError>());
    registerMethod("GetTokens", description, params, returns);  // TODO: PermissionScopeChangeUserInfos

    params.clear(); returns.clear();
    description = "Get all the tokens for the given username.";
    params.insert("username", enumValueName(String));
    returns.insert("o:tokenInfoList", objectRef<TokenInfoList>());
    returns.insert("error", enumRef<UserManager::UserError>());
    registerMethod("GetUserTokens", description, params, returns);


    params.clear(); returns.clear();
    description = "Revoke access for a given token. Depending on the logged in user only the own tokens can be removed. If you are logged in as admin, any token can be removed.";
    params.insert("tokenId", enumValueName(Uuid));
    returns.insert("error", enumRef<UserManager::UserError>());
    registerMethod("RemoveToken", description, params, returns);  // TODO: PermissionScopeChangeUserInfos

    params.clear(); returns.clear();
    description = "Return a list of all users in the system.";
    returns.insert("users", objectRef<UserInfoList>());
    registerMethod("GetUsers", description, params, returns);

    params.clear(); returns.clear();
    description = "Remove a user from the system.";
    params.insert("username", enumValueName(String));
    returns.insert("error", enumRef<UserManager::UserError>());
    registerMethod("RemoveUser", description, params, returns);

    params.clear(); returns.clear();
    description = "Set the permissions (scopes) for a given user. If the user has not the permission \"PermissionScopeAccessAllThings\" the list of thing IDs this user has access to can be defined in the \"allowedThingIds\" property.";
    params.insert("username", enumValueName(String));
    params.insert("scopes", flagRef<Types::PermissionScopes>());
    params.insert("o:allowedThingIds", QVariantList() << enumValueName(Uuid));
    returns.insert("error", enumRef<UserManager::UserError>());
    registerMethod("SetUserScopes", description, params, returns);

    params.clear(); returns.clear();
    description = "Change user info. If username is given, info for the respective user is changed, otherwise the current user info is edited. Requires admin permissions to edit user info other than the own.";
    params.insert("o:username", enumValueName(String));
    params.insert("o:displayName", enumValueName(String));
    params.insert("o:email", enumValueName(String));
    returns.insert("error", enumRef<UserManager::UserError>());
    registerMethod("SetUserInfo", description, params, returns);

    // Notifications
    params.clear();
    description = "Emitted when a user is added to the system.";
    params.insert("userInfo", objectRef<UserInfo>());
    registerNotification("UserAdded", description, params);

    params.clear();
    description = "Emitted when a user is removed from the system.";
    params.insert("username", enumValueName(String));
    registerNotification("UserRemoved", description, params);

    params.clear();
    description = "Emitted whenever a user is changed.";
    params.insert("userInfo", objectRef<UserInfo>());
    registerNotification("UserChanged", description, params);

    params.clear();
    description = "Emitted when a push button authentication reaches final state. NOTE: This notification is "
                  "special. It will only be emitted to connections that did actively request a push button "
                  "authentication, but also it will be emitted regardless of the notification settings.";
    params.insert("success", enumValueName(Bool));
    params.insert("transactionId", enumValueName(Int));
    params.insert("o:token", enumValueName(String));
    registerNotification("PushButtonAuthFinished", description, params);

    connect(m_userManager, &UserManager::userAdded, this, [this](const QString &username){
        QVariantMap params;
        params.insert("userInfo", pack(m_userManager->userInfo(username)));
        emit UserAdded(params);
    });
    connect(m_userManager, &UserManager::userChanged, this, [this](const QString &username){
        QVariantMap params;
        params.insert("userInfo", pack(m_userManager->userInfo(username)));
        emit UserChanged(params);
    });
    connect(m_userManager, &UserManager::userRemoved, this, [this](const QString &username){
        QVariantMap params;
        params.insert("username", username);
        emit UserRemoved(params);
    });
}

QString UsersHandler::name() const
{
    return "Users";
}

JsonReply *UsersHandler::CreateUser(const QVariantMap &params)
{
    QString username = params.value("username").toString();
    QString password = params.value("password").toString();
    QString email = params.value("email").toString();
    QString displayName = params.value("displayName").toString();
    QStringList scopesList = params.value("scopes", Types::scopesToStringList(Types::PermissionScopeAdmin)).toStringList();
    Types::PermissionScopes scopes = Types::scopesFromStringList(scopesList);
    QList<ThingId> allowedThingIds;
    foreach (const QString &thingIdString, params.value("allowedThingIds").toStringList())
        allowedThingIds.append(ThingId(thingIdString));

    UserManager::UserError status = m_userManager->createUser(username, password, email, displayName, scopes, allowedThingIds);

    QVariantMap returns;
    returns.insert("error", enumValueName<UserManager::UserError>(status));
    return createReply(returns);
}

JsonReply *UsersHandler::ChangePassword(const QVariantMap &params, const JsonContext &context)
{
    QVariantMap returns;

    QByteArray currentToken = context.token();
    if (currentToken.isEmpty()) {
        qCWarning(dcJsonRpc()) << "Cannot change password from an unauthenticated connection";
        returns.insert("error", enumValueName<UserManager::UserError>(UserManager::UserErrorPermissionDenied));
        return createReply(returns);
    }

    if (!m_userManager->verifyToken(currentToken)) {
        // Might happen if the client is connecting via an unauthenticated connection but tries to sneak in an invalid token
        qCWarning(dcJsonRpc()) << "Invalid token. Is this an unauthenticated connection?";
        returns.insert("error", enumValueName<UserManager::UserError>(UserManager::UserErrorPermissionDenied));
        return createReply(returns);
    }

    QString newPassword = params.value("newPassword").toString();

    TokenInfo tokenInfo = m_userManager->tokenInfo(currentToken);

    UserManager::UserError status = m_userManager->changePassword(tokenInfo.username(), newPassword);
    returns.insert("error", enumValueName<UserManager::UserError>(status));
    return createReply(returns);
}

JsonReply *UsersHandler::ChangeUserPassword(const QVariantMap &params, const JsonContext &context)
{
    QVariantMap returns;

    QByteArray currentToken = context.token();
    if (currentToken.isEmpty()) {
        qCWarning(dcJsonRpc()) << "Cannot change a user password from an unauthenticated connection";
        returns.insert("error", enumValueName<UserManager::UserError>(UserManager::UserErrorPermissionDenied));
        return createReply(returns);
    }

    if (!m_userManager->verifyToken(currentToken)) {
        // Might happen if the client is connecting via an unauthenticated connection but tries to sneak in an invalid token
        qCWarning(dcJsonRpc()) << "Invalid token. Cannot change a user password from an unauthenticated connection";
        returns.insert("error", enumValueName<UserManager::UserError>(UserManager::UserErrorPermissionDenied));
        return createReply(returns);
    }

    QString username = params.value("username").toString();;
    QString newPassword = params.value("newPassword").toString();

    UserManager::UserError status = m_userManager->changePassword(username, newPassword);
    returns.insert("error", enumValueName<UserManager::UserError>(status));
    return createReply(returns);
}

JsonReply *UsersHandler::GetUserInfo(const QVariantMap &params, const JsonContext &context)
{
    Q_UNUSED(params)

    QVariantMap returns;

    QByteArray currentToken = context.token();
    if (currentToken.isEmpty()) {
        qCWarning(dcJsonRpc()) << "Cannot get user info from an unauthenticated connection";
        returns.insert("error", enumValueName<UserManager::UserError>(UserManager::UserErrorPermissionDenied));
        return createReply(returns);
    }

    if (!m_userManager->verifyToken(currentToken)) {
        // Might happen if the client is connecting via an unauthenticated connection but tries to sneak in an invalid token
        qCWarning(dcJsonRpc()) << "Invalid token. Is this an unauthenticated connection?";
        returns.insert("error", enumValueName<UserManager::UserError>(UserManager::UserErrorPermissionDenied));
        return createReply(returns);
    }

    TokenInfo tokenInfo = m_userManager->tokenInfo(currentToken);

    UserInfo userInfo = m_userManager->userInfo(tokenInfo.username());
    returns.insert("userInfo", pack(userInfo));
    returns.insert("error", enumValueName<UserManager::UserError>(UserManager::UserErrorNoError));
    return createReply(returns);
}

JsonReply *UsersHandler::GetTokens(const QVariantMap &params, const JsonContext &context)
{
    Q_UNUSED(params)

    QVariantMap returns;

    QByteArray currentToken = context.token();
    if (currentToken.isEmpty()) {
        qCWarning(dcJsonRpc()) << "Cannot fetch tokens for an unauthenticated connection";
        returns.insert("error", enumValueName<UserManager::UserError>(UserManager::UserErrorPermissionDenied));
        return createReply(returns);
    }

    if (!m_userManager->verifyToken(currentToken)) {
        // Might happen if the client is connecting via an unauthenticated connection but tries to sneak in an invalid token
        qCWarning(dcJsonRpc()) << "Invalid token. Is this an unauthenticated connection?";
        returns.insert("error", enumValueName<UserManager::UserError>(UserManager::UserErrorPermissionDenied));
        return createReply(returns);
    }

    TokenInfo tokenInfo = m_userManager->tokenInfo(currentToken);
    qCDebug(dcJsonRpc()) << "Fetching tokens for user" << currentToken << tokenInfo.username();
    QList<TokenInfo> tokens = m_userManager->tokens(tokenInfo.username());
    QVariantList retList;
    foreach (const TokenInfo &tokenInfo, tokens) {
        retList << pack(tokenInfo);
    }
    returns.insert("tokenInfoList", retList);
    returns.insert("error", enumValueName<UserManager::UserError>(UserManager::UserErrorNoError));
    return createReply(returns);
}

JsonReply *UsersHandler::GetUserTokens(const QVariantMap &params, const JsonContext &context)
{
    QVariantMap returns;

    QByteArray currentToken = context.token();
    if (currentToken.isEmpty()) {
        qCWarning(dcJsonRpc()) << "Cannot fetch tokens for an unauthenticated connection";
        returns.insert("error", enumValueName<UserManager::UserError>(UserManager::UserErrorPermissionDenied));
        return createReply(returns);
    }

    if (!m_userManager->verifyToken(currentToken)) {
        // Might happen if the client is connecting via an unauthenticated connection but tries to sneak in an invalid token
        qCWarning(dcJsonRpc()) << "Invalid token. Is this an unauthenticated connection?";
        returns.insert("error", enumValueName<UserManager::UserError>(UserManager::UserErrorPermissionDenied));
        return createReply(returns);
    }

    QString username = params.value("username").toString();;

    qCDebug(dcJsonRpc()) << "Fetching tokens for user" << username;
    QList<TokenInfo> tokens = m_userManager->tokens(username);
    QVariantList retList;
    foreach (const TokenInfo &tokenInfo, tokens) {
        retList << pack(tokenInfo);
    }
    returns.insert("tokenInfoList", retList);
    returns.insert("error", enumValueName<UserManager::UserError>(UserManager::UserErrorNoError));
    return createReply(returns);
}

JsonReply *UsersHandler::RemoveToken(const QVariantMap &params, const JsonContext &context)
{
    QVariantMap returns;

    QByteArray currentToken = context.token();
    if (currentToken.isEmpty()) {
        qCWarning(dcJsonRpc()) << "Cannot remove a token from an unauthenticated connection.";
        returns.insert("error", enumValueName<UserManager::UserError>(UserManager::UserErrorPermissionDenied));
        return createReply(returns);
    }

    if (!m_userManager->verifyToken(currentToken)) {
        // Might happen if the client is connecting via an unauthenticated connection but tries to sneak in an invalid token
        qCWarning(dcJsonRpc()) << "Invalid token. Is this an unauthenticated connection?";
        returns.insert("error", enumValueName<UserManager::UserError>(UserManager::UserErrorPermissionDenied));
        return createReply(returns);
    }

    QUuid tokenId = params.value("tokenId").toUuid();

    TokenInfo tokenToRemove = m_userManager->tokenInfo(tokenId);
    if (tokenToRemove.id().isNull()) {
        qCWarning(dcJsonRpc()) << "Token with ID" << tokenId << "not found";
        returns.insert("error", enumValueName<UserManager::UserError>(UserManager::UserErrorTokenNotFound));
        return createReply(returns);
    }

    TokenInfo currentTokenInfo = m_userManager->tokenInfo(currentToken);
    if (currentTokenInfo.username() != tokenToRemove.username()) {
        qCWarning(dcJsonRpc()) << "Cannot remove a token from another user!";
        returns.insert("error", enumValueName<UserManager::UserError>(UserManager::UserErrorPermissionDenied));
        return createReply(returns);
    }

    qCDebug(dcJsonRpc()) << "Removing token" << tokenId << "for user" << currentTokenInfo.username();

    UserManager::UserError error = m_userManager->removeToken(tokenId);
    returns.insert("error", enumValueName<UserManager::UserError>(error));
    return createReply(returns);
}

JsonReply *UsersHandler::GetUsers(const QVariantMap &params)
{
    Q_UNUSED(params)

    QVariantMap reply;
    reply.insert("users", pack(m_userManager->users()));
    return createReply(reply);
}

JsonReply *UsersHandler::RemoveUser(const QVariantMap &params, const JsonContext &context)
{
    Q_UNUSED(context)
    QString username = params.value("username").toString();
    QVariantMap returns;
    UserManager::UserError error = m_userManager->removeUser(username);
    returns.insert("error", enumValueName<UserManager::UserError>(error));
    return createReply(returns);
}

JsonReply *UsersHandler::SetUserScopes(const QVariantMap &params, const JsonContext &context)
{
    Q_UNUSED(context)

    qCWarning(dcJsonRpc()) << params;

    QString username = params.value("username").toString();
    Types::PermissionScopes scopes = Types::scopesFromStringList(params.value("scopes").toStringList());
    QList<ThingId> allowedThingIds;
    if (params.contains("allowedThingIds")) {
        allowedThingIds = Types::thingIdsFromStringList(params.value("allowedThingIds").toStringList());
    } else {
        allowedThingIds = m_userManager->userInfo(username).allowedThingIds();
    }

    UserManager::UserError error = m_userManager->setUserScopes(username, scopes, allowedThingIds);

    QVariantMap returns;
    returns.insert("error", enumValueName<UserManager::UserError>(error));
    return createReply(returns);
}

JsonReply *UsersHandler::SetUserInfo(const QVariantMap &params, const JsonContext &context)
{
    QVariantMap returns;

    TokenInfo callingTokenInfo = m_userManager->tokenInfo(context.token());
    QString username;

    if (params.contains("username")) {
        username = params.value("username").toString();
    } else {
        username = callingTokenInfo.username();
    }

    if (callingTokenInfo.username() != username && !m_userManager->userInfo(callingTokenInfo.username()).scopes().testFlag(Types::PermissionScopeAdmin)) {
        returns.insert("error", enumValueName(UserManager::UserErrorPermissionDenied));
        return createReply(returns);
    }

    UserInfo changedUserInfo = m_userManager->userInfo(username);

    QString email;
    if (params.contains("email")) {
        email = params.value("email").toString();
    } else {
        email = changedUserInfo.email();
    }
    QString displayName;
    if (params.contains("displayName")) {
        displayName = params.value("displayName").toString();
    } else {
        displayName = changedUserInfo.displayName();
    }
    UserManager::UserError status = m_userManager->setUserInfo(username, email, displayName);
    returns.insert("error", enumValueName(status));
    return createReply(returns);
}

}
