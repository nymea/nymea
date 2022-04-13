#ifndef USERBACKEND_H
#define USERBACKEND_H

#include <QObject>

#include "typeutils.h"
#include "usermanager.h"

class UserBackend : public QObject
{
    Q_OBJECT
public:
    explicit UserBackend(QObject *parent = nullptr);
    virtual ~UserBackend() = default;

    virtual UserManager::Capabilities capabilities() const = 0;
    virtual bool initRequired() const = 0;

    virtual UserManager::UserError createUser(const QString &username, const QString &password, const QString &email, const QString &displayName, Types::PermissionScopes scopes) = 0;
    virtual UserManager::UserError changePassword(const QString &username, const QString &newPassword) = 0;
    virtual UserManager::UserError removeUser(const QString &username) = 0;
    virtual UserManager::UserError setUserScopes(const QString &username, Types::PermissionScopes scopes) = 0;

    virtual UserInfo userInfo(const QString &username) const = 0;
    virtual UserManager::UserError setUserInfo(const QString &username, const QString &email, const QString &displayName) = 0;

    virtual UserInfoList users() const = 0;
    virtual QByteArray authenticate(const QString &username, const QString &password, const QString &deviceName) = 0;

    virtual QList<TokenInfo> tokens(const QString &username) const = 0;
    virtual TokenInfo tokenInfo(const QByteArray &token) const = 0;
    virtual TokenInfo tokenInfo(const QUuid &tokenId) const = 0;
    virtual UserManager::UserError removeToken(const QUuid &tokenId) = 0;
    virtual bool verifyToken(const QByteArray &token) = 0;

    virtual bool pushButtonAuthAvailable() const;
    virtual int requestPushButtonAuth(const QString &deviceName);
    virtual void cancelPushButtonAuth(int transactionId);

signals:
    void userAdded(const QString &username);
    void userRemoved(const QString &username);
    void userChanged(const QString &username);
    void pushButtonAuthFinished(int transactionId, bool success, const QByteArray &token);

};

Q_DECLARE_INTERFACE(UserBackend, "io.nymea.UserBackend")


#endif // USERBACKEND_H
