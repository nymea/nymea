#ifndef BUILTINUSERBACKEND_H
#define BUILTINUSERBACKEND_H

#include "usermanagemenent/userbackend.h"

#include <QObject>
#include <QSqlDatabase>

namespace nymeaserver
{

class PushButtonDBusService;

class BuiltinUserBackend : public UserBackend
{
    Q_OBJECT
public:
    explicit BuiltinUserBackend(QObject *parent = nullptr);

    UserManager::Capabilities capabilities() const override;
    bool initRequired() const override;

    UserInfoList users() const override;
    UserManager::UserError createUser(const QString &username, const QString &password, const QString &email, const QString &displayName, Types::PermissionScopes scopes) override;
    UserManager::UserError changePassword(const QString &username, const QString &newPassword) override;
    UserManager::UserError removeUser(const QString &username) override;
    UserManager::UserError setUserScopes(const QString &username, Types::PermissionScopes scopes) override;

    UserInfo userInfo(const QString &username) const override;
    UserManager::UserError setUserInfo(const QString &username, const QString &email, const QString &displayName) override;

    QByteArray authenticate(const QString &username, const QString &password, const QString &deviceName) override;

    QList<TokenInfo> tokens(const QString &username) const override;
    TokenInfo tokenInfo(const QByteArray &token) const override;
    TokenInfo tokenInfo(const QUuid &tokenId) const override;
    UserManager::UserError removeToken(const QUuid &tokenId) override;
    bool verifyToken(const QByteArray &token) override;

    bool pushButtonAuthAvailable() const override;
    int requestPushButtonAuth(const QString &deviceName) override;
    void cancelPushButtonAuth(int transactionId) override;

private slots:
    void onPushButtonPressed();

private:
    bool initDB();
    void rotate(const QString &dbName);

    bool validateUsername(const QString &username) const;
    bool validatePassword(const QString &password) const;
    bool validateToken(const QByteArray &token) const;

    void dumpDBError(const QString &message);

private:
    QSqlDatabase m_db;

    PushButtonDBusService *m_pushButtonDBusService = nullptr;
    int m_pushButtonTransactionIdCounter = 0;
    QPair<int, QString> m_pushButtonTransaction;

};

}

#endif // BUILTINUSERBACKEND_H
