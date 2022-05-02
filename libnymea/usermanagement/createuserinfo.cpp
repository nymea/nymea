#include "createuserinfo.h"

#include <QLoggingCategory>
Q_DECLARE_LOGGING_CATEGORY(dcUserManager)

CreateUserInfo::CreateUserInfo(const QString &username, const QString &password, const QString &email, const QString &displayName, Types::PermissionScopes scopes, QObject *parent):
    QObject(parent),
    m_username(username),
    m_password(password),
    m_email(email),
    m_displayName(displayName),
    m_scopes(scopes)

{

}

QString CreateUserInfo::username() const
{
    return m_username;
}

QString CreateUserInfo::password() const
{
    return m_password;
}

QString CreateUserInfo::email() const
{
    return m_email;
}

QString CreateUserInfo::displayName() const
{
    return m_displayName;
}

Types::PermissionScopes CreateUserInfo::scopes() const
{
    return m_scopes;
}

UserManager::UserError CreateUserInfo::status() const
{
    return m_status;
}

void CreateUserInfo::finish(UserManager::UserError status)
{
    if (m_finished) {
        qCWarning(dcUserManager()) << "CreateUserInfo already finished. Not finishing a second time.";
        return;
    }
    m_finished = true;
    m_status = status;
    QMetaObject::invokeMethod(this, "finished", Qt::QueuedConnection, Q_ARG(UserManager::UserError, status));
}
