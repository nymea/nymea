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

#include "usershandler.h"
#include "usermanagemenent/usermanager.h"
//#include "usermanager/usermanagerimplementation.h"

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
    description = "Create a new user in the API with the given username and password. Use scopes to define the permissions for the new user. If no scopes are given, this user will be an admin user. Call Authenticate after this to obtain a device token for this user.";
    params.insert("username", enumValueName(String));
    params.insert("password", enumValueName(String));
    params.insert("o:email", enumValueName(String));
    params.insert("o:displayName", enumValueName(String));
    params.insert("o:scopes", flagRef<Types::PermissionScopes>());
    returns.insert("error", enumRef<UserManager::UserError>());
    registerMethod("CreateUser", description, params, returns);

    params.clear(); returns.clear();
    description = "Change the password for the currently logged in user.";
    params.insert("newPassword", enumValueName(String));
    returns.insert("error", enumRef<UserManager::UserError>());
    registerMethod("ChangePassword", description, params, returns);

    params.clear(); returns.clear();
    description = "Get info about the current token (the currently logged in user).";
    returns.insert("o:userInfo", objectRef<UserInfo>());
    returns.insert("error", enumRef<UserManager::UserError>());
    registerMethod("GetUserInfo", description, params, returns, Types::PermissionScopeNone);

    params.clear(); returns.clear();
    description = "Get all the tokens for the current user.";
    returns.insert("o:tokenInfoList", objectRef<TokenInfoList>());
    returns.insert("error", enumRef<UserManager::UserError>());
    registerMethod("GetTokens", description, params, returns);

    params.clear(); returns.clear();
    description = "Revoke access for a given token.";
    params.insert("tokenId", enumValueName(Uuid));
    returns.insert("error", enumRef<UserManager::UserError>());
    registerMethod("RemoveToken", description, params, returns);

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
    description = "Set the permissions (scopes) for a given user.";
    params.insert("username", enumValueName(String));
    params.insert("scopes", flagRef<Types::PermissionScopes>());
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

    UserManager::UserError status = m_userManager->createUser(username, password, email, displayName, scopes);

    QVariantMap returns;
    returns.insert("error", enumValueName<UserManager::UserError>(status));
    return createReply(returns);
}

JsonReply *UsersHandler::ChangePassword(const QVariantMap &params, const JsonContext &context)
{
    QVariantMap ret;

    QByteArray currentToken = context.token();
    if (currentToken.isEmpty()) {
        qCWarning(dcJsonRpc()) << "Cannot change password from an unauthenticated connection";
        ret.insert("error", enumValueName<UserManager::UserError>(UserManager::UserErrorPermissionDenied));
        return createReply(ret);
    }
    if (!m_userManager->verifyToken(currentToken)) {
        // Might happen if the client is connecting via an unauthenticated connection but tries to sneak in an invalid token
        qCWarning(dcJsonRpc()) << "Invalid token. Is this an unauthenticated connection?";
        ret.insert("error", enumValueName<UserManager::UserError>(UserManager::UserErrorPermissionDenied));
        return createReply(ret);
    }

    QString newPassword = params.value("newPassword").toString();

    TokenInfo tokenInfo = m_userManager->tokenInfo(currentToken);

    UserManager::UserError status = m_userManager->changePassword(tokenInfo.username(), newPassword);
    ret.insert("error", enumValueName<UserManager::UserError>(status));
    return createReply(ret);
}

JsonReply *UsersHandler::GetUserInfo(const QVariantMap &params, const JsonContext &context)
{
    Q_UNUSED(params)
    QVariantMap ret;

    QByteArray currentToken = context.token();
    if (currentToken.isEmpty()) {
        qCWarning(dcJsonRpc()) << "Cannot get user info from an unauthenticated connection";
        ret.insert("error", enumValueName<UserManager::UserError>(UserManager::UserErrorPermissionDenied));
        return createReply(ret);
    }
    if (!m_userManager->verifyToken(currentToken)) {
        // Might happen if the client is connecting via an unauthenticated connection but tries to sneak in an invalid token
        qCWarning(dcJsonRpc()) << "Invalid token. Is this an unauthenticated connection?";
        ret.insert("error", enumValueName<UserManager::UserError>(UserManager::UserErrorPermissionDenied));
        return createReply(ret);
    }

    TokenInfo tokenInfo = m_userManager->tokenInfo(currentToken);

    UserInfo userInfo = m_userManager->userInfo(tokenInfo.username());
    ret.insert("userInfo", pack(userInfo));
    ret.insert("error", enumValueName<UserManager::UserError>(UserManager::UserErrorNoError));
    return createReply(ret);
}

JsonReply *UsersHandler::GetTokens(const QVariantMap &params, const JsonContext &context)
{
    Q_UNUSED(params)
    QVariantMap ret;

    QByteArray currentToken = context.token();
    if (currentToken.isEmpty()) {
        qCWarning(dcJsonRpc()) << "Cannot fetch tokens for an unauthenticated connection";
        ret.insert("error", enumValueName<UserManager::UserError>(UserManager::UserErrorPermissionDenied));
        return createReply(ret);
    }
    if (!m_userManager->verifyToken(currentToken)) {
        // Might happen if the client is connecting via an unauthenticated connection but tries to sneak in an invalid token
        qCWarning(dcJsonRpc()) << "Invalid token. Is this an unauthenticated connection?";
        ret.insert("error", enumValueName<UserManager::UserError>(UserManager::UserErrorPermissionDenied));
        return createReply(ret);
    }

    TokenInfo tokenInfo = m_userManager->tokenInfo(currentToken);
    qCDebug(dcJsonRpc()) << "Fetching tokens for user" << currentToken << tokenInfo.username();
    QList<TokenInfo> tokens = m_userManager->tokens(tokenInfo.username());
    QVariantList retList;
    foreach (const TokenInfo &tokenInfo, tokens) {
        retList << pack(tokenInfo);
    }
    ret.insert("tokenInfoList", retList);
    ret.insert("error", enumValueName<UserManager::UserError>(UserManager::UserErrorNoError));
    return createReply(ret);
}

JsonReply *UsersHandler::RemoveToken(const QVariantMap &params, const JsonContext &context)
{
    QVariantMap ret;

    QByteArray currentToken = context.token();
    if (currentToken.isEmpty()) {
        qCWarning(dcJsonRpc()) << "Cannot remove a token from an unauthenticated connection.";
        ret.insert("error", enumValueName<UserManager::UserError>(UserManager::UserErrorPermissionDenied));
        return createReply(ret);
    }
    if (!m_userManager->verifyToken(currentToken)) {
        // Might happen if the client is connecting via an unauthenticated connection but tries to sneak in an invalid token
        qCWarning(dcJsonRpc()) << "Invalid token. Is this an unauthenticated connection?";
        ret.insert("error", enumValueName<UserManager::UserError>(UserManager::UserErrorPermissionDenied));
        return createReply(ret);
    }
    QUuid tokenId = params.value("tokenId").toUuid();
    TokenInfo tokenToRemove = m_userManager->tokenInfo(tokenId);
    if (tokenToRemove.id().isNull()) {
        qCWarning(dcJsonRpc()) << "Token with ID" << tokenId << "not found";
        ret.insert("error", enumValueName<UserManager::UserError>(UserManager::UserErrorTokenNotFound));
        return createReply(ret);
    }

    TokenInfo currentTokenInfo = m_userManager->tokenInfo(currentToken);
    if (currentTokenInfo.username() != tokenToRemove.username()) {
        qCWarning(dcJsonRpc()) << "Cannot remove a token from another user!";
        ret.insert("error", enumValueName<UserManager::UserError>(UserManager::UserErrorPermissionDenied));
        return createReply(ret);
    }

    qCDebug(dcJsonRpc()) << "Removing token" << tokenId << "for user" << currentTokenInfo.username();

    UserManager::UserError error = m_userManager->removeToken(tokenId);
    ret.insert("error", enumValueName<UserManager::UserError>(error));
    return createReply(ret);
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
    QString username = params.value("username").toString();
    Types::PermissionScopes scopes = Types::scopesFromStringList(params.value("scopes").toStringList());
    UserManager::UserError error = m_userManager->setUserScopes(username, scopes);
    QVariantMap returns;
    returns.insert("error", enumValueName<UserManager::UserError>(error));
    return createReply(returns);
}

JsonReply *UsersHandler::SetUserInfo(const QVariantMap &params, const JsonContext &context)
{
    QVariantMap ret;

    TokenInfo callingTokenInfo = m_userManager->tokenInfo(context.token());
    QString username;

    if (params.contains("username")) {
        username = params.value("username").toString();
    } else {
        username = callingTokenInfo.username();
    }

    if (callingTokenInfo.username() != username && !m_userManager->userInfo(callingTokenInfo.username()).scopes().testFlag(Types::PermissionScopeAdmin)) {
        ret.insert("error", enumValueName(UserManager::UserErrorPermissionDenied));
        return createReply(ret);
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
    ret.insert("error", enumValueName(status));
    return createReply(ret);
}

}
