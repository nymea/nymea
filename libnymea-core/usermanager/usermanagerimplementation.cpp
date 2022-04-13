/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2022, nymea GmbH
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

#include "usermanagerimplementation.h"
#include "nymeasettings.h"
#include "loggingcategories.h"
#include "nymeacore.h"
#include "builtinuserbackend.h"

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
#include <QCoreApplication>

namespace nymeaserver {

UserManagerImplementation::UserManagerImplementation(QObject *parent):
    UserManager(parent)
{
    foreach (const QString &path, pluginSearchDirs()) {
        QDir dir(path);
        qCDebug(dcPlatform) << "Loading platform plugins from:" << dir.absolutePath();
        foreach (const QString &entry, dir.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot)) {
            QFileInfo fi(path + "/" + entry);
            if (fi.isFile()) {
                if (entry.startsWith("libnymea_userbackendplugin") && entry.endsWith(".so")) {
                    loadBackendPlugin(path + "/" + entry);
                }
            } else if (fi.isDir()) {
                if (QFileInfo::exists(path + "/" + entry + "/libnymea_userbackendplugin" + entry + ".so")) {
                    loadBackendPlugin(path + "/" +  entry + "/libnymea_userbackendplugin" + entry + ".so");
                }
            }
            if (m_backend) {
                break;
            }
        }
        if (m_backend) {
            break;
        }
    }

    if (!m_backend) {
        qCInfo(dcUserManager()) << "Using builtin backend.";
        m_backend = new BuiltinUserBackend(this);
    } else {
        qCInfo(dcUserManager()) << "Using backend:" << m_backend->metaObject()->className();
    }

    connect(m_backend, &BuiltinUserBackend::userAdded, this, &UserManagerImplementation::userAdded);
    connect(m_backend, &BuiltinUserBackend::userRemoved, this, &UserManagerImplementation::userRemoved);
    connect(m_backend, &BuiltinUserBackend::userChanged, this, &UserManagerImplementation::userChanged);
    connect(m_backend, &BuiltinUserBackend::pushButtonAuthFinished, this, &UserManagerImplementation::pushButtonAuthFinished);

}

UserManager::Capabilities UserManagerImplementation::capabilities() const
{
    return m_backend->capabilities();
}

bool UserManagerImplementation::initRequired() const
{
    return m_backend->initRequired();
}

UserInfoList UserManagerImplementation::users() const
{
    return m_backend->users();
}

UserManagerImplementation::UserError UserManagerImplementation::createUser(const QString &username, const QString &password, const QString &email, const QString &displayName, Types::PermissionScopes scopes)
{
    return m_backend->createUser(username, password, email, displayName, scopes);
}

UserManagerImplementation::UserError UserManagerImplementation::changePassword(const QString &username, const QString &newPassword)
{
    return m_backend->changePassword(username, newPassword);
}

UserManagerImplementation::UserError UserManagerImplementation::removeUser(const QString &username)
{
    return m_backend->removeUser(username);
}

UserManagerImplementation::UserError UserManagerImplementation::setUserScopes(const QString &username, Types::PermissionScopes scopes)
{
    return m_backend->setUserScopes(username, scopes);
}

UserManagerImplementation::UserError UserManagerImplementation::setUserInfo(const QString &username, const QString &email, const QString &displayName)
{
    return m_backend->setUserInfo(username, email, displayName);
}

QByteArray UserManagerImplementation::authenticate(const QString &username, const QString &password, const QString &deviceName)
{
    return m_backend->authenticate(username, password, deviceName);
}

int UserManagerImplementation::requestPushButtonAuth(const QString &deviceName)
{
    return m_backend->requestPushButtonAuth(deviceName);
}

void UserManagerImplementation::cancelPushButtonAuth(int transactionId)
{
    return m_backend->cancelPushButtonAuth(transactionId);
}

UserInfo UserManagerImplementation::userInfo(const QString &username) const
{
    return m_backend->userInfo(username);
}

QList<TokenInfo> UserManagerImplementation::tokens(const QString &username) const
{
    return m_backend->tokens(username);
}

TokenInfo UserManagerImplementation::tokenInfo(const QByteArray &token) const
{
    return m_backend->tokenInfo(token);
}

TokenInfo UserManagerImplementation::tokenInfo(const QUuid &tokenId) const
{
    return m_backend->tokenInfo(tokenId);
}

UserManagerImplementation::UserError UserManagerImplementation::removeToken(const QUuid &tokenId)
{
    return m_backend->removeToken(tokenId);
}

bool UserManagerImplementation::verifyToken(const QByteArray &token)
{
    return m_backend->verifyToken(token);
}

QStringList UserManagerImplementation::pluginSearchDirs()
{
    QStringList searchDirs;
    QByteArray envPath = qgetenv("NYMEA_USERBACKEND_PLUGIN_PATH");
    if (!envPath.isEmpty()) {
        searchDirs << QString(envPath).split(':');
    }

    foreach (QString libraryPath, QCoreApplication::libraryPaths()) {
        searchDirs << libraryPath.replace("qt5", "nymea").replace("plugins", "userbackend");
    }
    foreach (QString libraryPath, QCoreApplication::libraryPaths()) {
        searchDirs << libraryPath.replace("plugins", "nymea/userbackend");
    }
    searchDirs << QCoreApplication::applicationDirPath() + "/../lib/nymea/userbackend/";
    searchDirs << QCoreApplication::applicationDirPath() + "/../userbackend/";
    searchDirs << QCoreApplication::applicationDirPath() + "/../../../userbackend/";
    return searchDirs;
}

void UserManagerImplementation::loadBackendPlugin(const QString &file)
{
    QPluginLoader loader;
    loader.setFileName(file);
    loader.setLoadHints(QLibrary::ResolveAllSymbolsHint);
    if (!loader.load()) {
        qCWarning(dcUserManager()) << loader.errorString();
        return;
    }
    m_backend = qobject_cast<UserBackend*>(loader.instance());
    if (!m_backend) {
        qCWarning(dcUserManager()) << "Could not get plugin instance of" << loader.fileName();
        loader.unload();
        return;
    }
    qCDebug(dcPlatform()) << "Loaded user backend plugin:" << loader.fileName();
    m_backend->setParent(this);
}

}
