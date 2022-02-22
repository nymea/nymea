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
* GNU General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU General Public License as published by the Free Software
* Foundation, GNU version 3. This project is distributed in the hope that it
* will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
* of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
* Public License for more details.
*
* You should have received a copy of the GNU General Public License along with
* this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*!
    \class nymeaserver::UserManager
    \brief This class represents the manager for the users in nymead.

    \ingroup user
    \inmodule core

    The user manager is responsible for managing the user database, tokens and authentication. The user manager
    creates a user database where all relevant information will be stored.

    \sa TokenInfo, PushButtonDBusService
*/

/*! \enum nymeaserver::UserManager::UserError

    This enum represents the possible errors the \l{UserManager} can have.

    \value UserErrorNoError
        No error occurred. Everything is ok.4
    \value UserErrorBackendError
        Something went wrong in the manager. This is probably caused by a database error.
    \value UserErrorInvalidUserId
        The given user name is not valid.
    \value UserErrorDuplicateUserId
        The given user name already exits. Please use a different user name.
    \value UserErrorBadPassword
        The given password is to weak. Please use a stronger password.
    \value UserErrorTokenNotFound
        The given token is unknown to the UserManager.
    \value UserErrorPermissionDenied
        The permission is denied. Either invalid username, password or token.
*/

/*! \fn void nymeaserver::UserManager::pushButtonAuthFinished(int transactionId, bool success, const QByteArray &token);
    This signal is emitted when the push authentication for the given \a transactionId is finished.
    If \a success is true, the resulting \a token contains a non empty string.

    \sa requestPushButtonAuth
*/

#include "usermanager.h"
#include "nymeasettings.h"
#include "loggingcategories.h"
#include "nymeacore.h"
#include "builtinuserbackend.h"
#include "maveouserbackend.h"

#include <QUuid>
#include <QCryptographicHash>
#include <QSqlQuery>
#include <QSqlResult>
#include <QVariant>
#include <QSqlError>
#include <QRegExpValidator>
#include <QDateTime>
#include <QDebug>
#include <QFileInfo>

namespace nymeaserver {

/*! Constructs a new UserManager with the given \a dbName and \a parent. */
UserManager::UserManager(QObject *parent):
    QObject(parent)
{
    m_builtinBackend = new BuiltinUserBackend(this);
//    m_builtinBackend = new MaveoUserBackend(this);
    connect(m_builtinBackend, &BuiltinUserBackend::userAdded, this, &UserManager::userAdded);
    connect(m_builtinBackend, &BuiltinUserBackend::userRemoved, this, &UserManager::userRemoved);
    connect(m_builtinBackend, &BuiltinUserBackend::userChanged, this, &UserManager::userChanged);
    connect(m_builtinBackend, &BuiltinUserBackend::pushButtonAuthFinished, this, &UserManager::pushButtonAuthFinished);

}

/*! Will return true if the database is working fine but doesn't have any information on users whatsoever.
 *  That is, neither a user nor an anonymous token.
 *  This may be used to determine whether a first-time setup is required.
 */
bool UserManager::initRequired() const
{
    return m_builtinBackend->initRequired();
}

/*! Returns the list of user names for this UserManager. */
UserInfoList UserManager::users() const
{
    return m_builtinBackend->users();
}

/*! Creates a new user with the given \a username and \a password. Returns the \l UserError to inform about the result. */
UserManager::UserError UserManager::createUser(const QString &username, const QString &password, const QString &email, const QString &displayName, Types::PermissionScopes scopes)
{
    return m_builtinBackend->createUser(username, password, email, displayName, scopes);
}

UserManager::UserError UserManager::changePassword(const QString &username, const QString &newPassword)
{
    return m_builtinBackend->changePassword(username, newPassword);
}

UserManager::UserError UserManager::removeUser(const QString &username)
{
    return m_builtinBackend->removeUser(username);
}

UserManager::UserError UserManager::setUserScopes(const QString &username, Types::PermissionScopes scopes)
{
    return m_builtinBackend->setUserScopes(username, scopes);
}

UserManager::UserError UserManager::setUserInfo(const QString &username, const QString &email, const QString &displayName)
{
    return m_builtinBackend->setUserInfo(username, email, displayName);
}

/*! Returns true if the push button authentication is available for this system. */
bool UserManager::pushButtonAuthAvailable() const
{
    return m_builtinBackend->pushButtonAuthAvailable();
}

/*! Authenticated the given \a username with the given \a password for the \a deviceName. If the authentication was
    successful, the token will be returned, otherwise the return value will be an empty byte array.
*/
QByteArray UserManager::authenticate(const QString &username, const QString &password, const QString &deviceName)
{
    return m_builtinBackend->authenticate(username, password, deviceName);
}

/*! Start the push button authentication for the device with the given \a deviceName. Returns the transaction id as refference to the request. */
int UserManager::requestPushButtonAuth(const QString &deviceName)
{
    return m_builtinBackend->requestPushButtonAuth(deviceName);
}

/*! Cancel the push button authentication with the given \a transactionId.

    \sa requestPushButtonAuth
*/
void UserManager::cancelPushButtonAuth(int transactionId)
{
    return m_builtinBackend->cancelPushButtonAuth(transactionId);
}

/*! Request UserInfo.
 The UserInfo for the given username is returned.
*/
UserInfo UserManager::userInfo(const QString &username) const
{
    return m_builtinBackend->userInfo(username);
}

QList<TokenInfo> UserManager::tokens(const QString &username) const
{
    return m_builtinBackend->tokens(username);
}

TokenInfo UserManager::tokenInfo(const QByteArray &token) const
{
    return m_builtinBackend->tokenInfo(token);
}

TokenInfo UserManager::tokenInfo(const QUuid &tokenId) const
{
    return m_builtinBackend->tokenInfo(tokenId);
}

/*! Removes the token with the given \a tokenId. Returns \l{UserError} to inform about the result. */
UserManager::UserError UserManager::removeToken(const QUuid &tokenId)
{
    return m_builtinBackend->removeToken(tokenId);
}

/*! Returns true, if the given \a token is valid. */
bool UserManager::verifyToken(const QByteArray &token)
{
    return m_builtinBackend->verifyToken(token);
}

}
