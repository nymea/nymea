#ifndef CREATEUSERINFO_H
#define CREATEUSERINFO_H

#include <QObject>
#include "usermanager.h"

class CreateUserInfo : public QObject
{
    Q_OBJECT
public:
    explicit CreateUserInfo(const QString &username, const QString &password, const QString &email, const QString &displayName, Types::PermissionScopes scopes, QObject *parent = nullptr);

    QString username() const;
    QString password() const;
    QString email() const;
    QString displayName() const;
    Types::PermissionScopes scopes() const;
    UserManager::UserError status() const;

    void finish(UserManager::UserError status);

signals:
    void finished(UserManager::UserError status);

private:
    QString m_username;
    QString m_password;
    QString m_email;
    QString m_displayName;
    Types::PermissionScopes m_scopes = Types::PermissionScopeNone;

    bool m_finished = false;
    UserManager::UserError m_status = UserManager::UserErrorNoError;
};

#endif // CREATEUSERINFO_H
