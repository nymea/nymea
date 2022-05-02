#include "userinfo.h"

#include <QMetaEnum>

UserInfo::UserInfo()
{

}

UserInfo::UserInfo(const QString &username):
    m_username(username)
{

}

QString UserInfo::username() const
{
    return m_username;
}

void UserInfo::setUsername(const QString &username)
{
    m_username = username;
}

QString UserInfo::email()
{
    return m_email;
}

void UserInfo::setEmail(const QString &email)
{
    m_email = email;
}

QString UserInfo::displayName() const
{
    return m_displayName;
}

void UserInfo::setDisplayName(const QString &displayName)
{
    m_displayName = displayName;
}

Types::PermissionScopes UserInfo::scopes() const
{
    return m_scopes;
}

void UserInfo::setScopes(Types::PermissionScopes scopes)
{
    m_scopes = scopes;
}

QVariant UserInfoList::get(int index) const
{
    return QVariant::fromValue(at(index));
}

void UserInfoList::put(const QVariant &variant)
{
    append(variant.value<UserInfo>());
}

