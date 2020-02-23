#include "userinfo.h"

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
