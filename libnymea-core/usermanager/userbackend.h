#ifndef USERBACKEND_H
#define USERBACKEND_H

#include <QObject>

#include "usermanager.h"

namespace nymeaserver {

class UserBackend : public QObject
{
    Q_OBJECT
public:
    explicit UserBackend(QObject *parent = nullptr);

    virtual UserManager::UserError createUser(const QString &username, const QString &password, Types::PermissionScopes scopes) = 0;
    virtual UserManager::UserError changePassword(const QString &username, const QString &newPassword) = 0;
    virtual UserManager::UserError removeUser(const QString &username) = 0;
    virtual UserManager::UserError setUserScopes(const QString &username, Types::PermissionScopes scopes) = 0;

signals:

};

}

#endif // USERBACKEND_H
