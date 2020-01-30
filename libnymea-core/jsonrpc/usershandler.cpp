#include "usershandler.h"
#include "usermanager/usermanager.h"
#include "usermanager/userinfo.h"

#include "loggingcategories.h"

namespace nymeaserver {

UsersHandler::UsersHandler(UserManager *userManager, QObject *parent):
    JsonHandler(parent),
    m_userManager(userManager)
{
    registerObject<UserInfo>();
    registerObject<TokenInfo, TokenInfoList>();

    QVariantMap params, returns;
    QString description;

    params.clear(); returns.clear();
    description = "Create a new user in the API. Currently this is only allowed to be called once when a new nymea instance is set up. Call Authenticate after this to obtain a device token for this user.";
    params.insert("username", enumValueName(String));
    params.insert("password", enumValueName(String));
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
    registerMethod("GetUserInfo", description, params, returns);

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

}

QString UsersHandler::name() const
{
    return "Users";
}

JsonReply *UsersHandler::CreateUser(const QVariantMap &params)
{
    QString username = params.value("username").toString();
    QString password = params.value("password").toString();

    UserManager::UserError status = m_userManager->createUser(username, password);

    QVariantMap returns;
    returns.insert("error", enumValueName<UserManager::UserError>(status));
    return createReply(returns);
}

JsonReply *UsersHandler::ChangePassword(const QVariantMap &params)
{
    QVariantMap ret;

    QByteArray currentToken = property("token").toByteArray();
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
    QString username = m_userManager->userInfo(currentToken).username();

    UserManager::UserError status = m_userManager->changePassword(username, newPassword);
    ret.insert("error", enumValueName<UserManager::UserError>(status));
    return createReply(ret);
}

JsonReply *UsersHandler::Authenticate(const QVariantMap &params)
{
    QString username = params.value("username").toString();
    QString password = params.value("password").toString();
    QString deviceName = params.value("deviceName").toString();

    QByteArray token = m_userManager->authenticate(username, password, deviceName);
    QVariantMap ret;
    ret.insert("success", !token.isEmpty());
    if (!token.isEmpty()) {
        ret.insert("token", token);
    }
    return createReply(ret);
}

JsonReply *UsersHandler::RequestPushButtonAuth(const QVariantMap &params)
{
    QString deviceName = params.value("deviceName").toString();
    QUuid clientId = this->property("clientId").toUuid();

    int transactionId = m_userManager->requestPushButtonAuth(deviceName);
    m_pushButtonTransactions.insert(transactionId, clientId);

    QVariantMap data;
    data.insert("transactionId", transactionId);
    // TODO: return false if pushbutton auth is disabled in settings
    data.insert("success", true);
    return createReply(data);
}

JsonReply *UsersHandler::GetUserInfo(const QVariantMap &params)
{
    Q_UNUSED(params)
    QVariantMap ret;

    QByteArray currentToken = property("token").toByteArray();
    if (currentToken.isEmpty()) {
        qCWarning(dcJsonRpc()) << "Cannot get user info form an unauthenticated connection";
        ret.insert("error", enumValueName<UserManager::UserError>(UserManager::UserErrorPermissionDenied));
        return createReply(ret);
    }
    if (!m_userManager->verifyToken(currentToken)) {
        // Might happen if the client is connecting via an unauthenticated connection but tries to sneak in an invalid token
        qCWarning(dcJsonRpc()) << "Invalid token. Is this an unauthenticated connection?";
        ret.insert("error", enumValueName<UserManager::UserError>(UserManager::UserErrorPermissionDenied));
        return createReply(ret);
    }

    UserInfo userInfo = m_userManager->userInfo(currentToken);
    ret.insert("userInfo", pack(userInfo));
    ret.insert("error", enumValueName<UserManager::UserError>(UserManager::UserErrorNoError));
    return createReply(ret);
}

JsonReply *UsersHandler::GetTokens(const QVariantMap &params)
{
    Q_UNUSED(params)
    QVariantMap ret;

    QByteArray currentToken = property("token").toByteArray();
    if (currentToken.isEmpty()) {
        qCWarning(dcJsonRpc()) << "Cannot fetch tokens form an unauthenticated connection";
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
    qCDebug(dcJsonRpc()) << "Fetching tokens for user" << tokenInfo.username();
    QList<TokenInfo> tokens = m_userManager->tokens(tokenInfo.username());
    QVariantList retList;
    foreach (const TokenInfo &tokenInfo, tokens) {
        retList << pack(tokenInfo);
    }
    ret.insert("tokenInfoList", retList);
    ret.insert("error", enumValueName<UserManager::UserError>(UserManager::UserErrorNoError));
    return createReply(ret);
}

JsonReply *UsersHandler::RemoveToken(const QVariantMap &params)
{
    QVariantMap ret;

    QByteArray currentToken = property("token").toByteArray();
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

}
