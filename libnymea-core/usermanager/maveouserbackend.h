#ifndef MAVEOUSERBACKEND_H
#define MAVEOUSERBACKEND_H

#include <QObject>
#include "userbackend.h"

class MqttClient;

using namespace nymeaserver;

class MaveoUserBackend : public UserBackend
{
    Q_OBJECT
public:
    explicit MaveoUserBackend(QObject *parent = nullptr);

    bool initRequired() const override;

    UserManager::UserError createUser(const QString &username, const QString &password, const QString &email, const QString &displayName, Types::PermissionScopes scopes) override;
    UserManager::UserError changePassword(const QString &username, const QString &newPassword) override;
    UserManager::UserError removeUser(const QString &username) override;
    UserManager::UserError setUserScopes(const QString &username, Types::PermissionScopes scopes) override;

    UserInfo userInfo(const QString &username) const override;
    UserManager::UserError setUserInfo(const QString &username, const QString &email, const QString &displayName) override;

    UserInfoList users() const override;
    QByteArray authenticate(const QString &username, const QString &password, const QString &deviceName) override;

    QList<TokenInfo> tokens(const QString &username) const override;
    TokenInfo tokenInfo(const QByteArray &token) const override;
    TokenInfo tokenInfo(const QUuid &tokenId) const override;
    UserManager::UserError removeToken(const QUuid &tokenId) override;
    bool verifyToken(const QByteArray &token) override;

private slots:
    void connect2Aws();

    void onConnected();
    void onDisconnected();

private:
    MqttClient *m_client = nullptr;
};

#endif // MAVEOUSERBACKEND_H
