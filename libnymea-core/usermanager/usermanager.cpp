// SPDX-License-Identifier: LGPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2025, chargebyte austria GmbH
*
* This file is part of nymea.
*
* nymea is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* as published by the Free Software Foundation, either version 3
* of the License, or (at your option) any later version.
*
* nymea is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with nymea. If not, see <https://www.gnu.org/licenses/>.
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
#include "loggingcategories.h"
#include "pushbuttondbusservice.h"
#include "nymeacore.h"

#include <QUuid>
#include <QCryptographicHash>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QSqlQuery>
#include <QVariant>
#include <QSqlError>
#include <QDateTime>
#include <QDebug>
#include <QFileInfo>
#include <QRegularExpression>

namespace nymeaserver {

/*! Constructs a new UserManager with the given \a dbName and \a parent. */
UserManager::UserManager(const QString &dbName, QObject *parent):
    QObject(parent)
{
    m_db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), "users");
    m_db.setDatabaseName(dbName);

    qCDebug(dcUserManager()) << "Opening user database" << m_db.databaseName();

    if (!m_db.isValid()) {
        qCWarning(dcUserManager()) << "The database is not valid:" << m_db.lastError().driverText() << m_db.lastError().databaseText();
        rotate(m_db.databaseName());
    }

    if (!initDB()) {
        if (m_hadUsersTableBeforeInit) {
            // A users table with potentially real accounts/tokens already existed before this
            // attempt failed (e.g. a migration step broke partway through). Rotating here would
            // silently make every existing user inaccessible. Leave the file exactly as it is and
            // fail closed instead; see UserManager::initializationFailed().
            qCCritical(dcUserManager()) << "Error initializing an existing user database. Refusing to touch it; leaving it in place for diagnosis.";
            m_initializationFailed = true;
        } else {
            qCWarning(dcUserManager()) << "Error initializing user database. Trying to correct it.";
            if (QFileInfo::exists(m_db.databaseName())) {
                rotate(m_db.databaseName());
                if (!initDB()) {
                    qCCritical(dcUserManager()) << "Error fixing user database. Giving up. Users can't be stored.";
                    m_initializationFailed = true;
                }
            } else {
                m_initializationFailed = true;
            }
        }
    }

    m_pushButtonDBusService = new PushButtonDBusService("/io/nymea/nymead/UserManager", this);
    connect(m_pushButtonDBusService, &PushButtonDBusService::pushButtonPressed, this, &UserManager::onPushButtonPressed);
    m_pushButtonTransaction = QPair<int, QString>(-1, QString());

    if (!m_initializationFailed) {
        m_expiryTimer = new QTimer(this);
        m_expiryTimer->setSingleShot(true);
        connect(m_expiryTimer, &QTimer::timeout, this, &UserManager::rearmExpiryTimer);
        // Guarded: a standalone UserManager constructed without a fully initialized
        // NymeaCore (as in isolated DB-loading tests) has no TimeManager to react to.
        if (NymeaCore::instance()->timeManager()) {
            connect(NymeaCore::instance()->timeManager(), &TimeManager::dateTimeChanged, this, &UserManager::rearmExpiryTimer);
        }
        rearmExpiryTimer();
    }
}

/*! Returns true if the user database could not be opened or migrated. In that case the
    existing file (if any) was deliberately left untouched instead of being rotated away,
    and this UserManager must not be used to serve any request. */
bool UserManager::initializationFailed() const
{
    return m_initializationFailed;
}

/*! Resolves invitation availability for the process lifetime. Passing \a available as
    false transactionally purges every pending invitation with no notification; if that
    purge fails, this sets initializationFailed() so the caller aborts startup instead of
    serving with a partially-disabled feature. */
void UserManager::setInvitationsAvailable(bool available)
{
    m_invitationsAvailable = available;
    if (available)
        return;

    QSqlQuery query(m_db);
    if (!query.exec("DELETE FROM invitations;") || query.lastError().isValid()) {
        qCCritical(dcUserManager()) << "Failed to purge pending invitations on disabled startup:" << query.lastError().databaseText() << query.lastError().driverText();
        m_initializationFailed = true;
        return;
    }
    qCDebug(dcUserManager()) << "Invitations disabled: purged" << query.numRowsAffected() << "pending invitation(s), no notification emitted.";
}

/*! Returns whether invitations are currently available, as resolved once at startup by
    setInvitationsAvailable(). */
bool UserManager::invitationsAvailable() const
{
    return m_invitationsAvailable;
}

/*! Will return true if the database is working fine but doesn't have any information on users whatsoever.
 *  That is, neither a user nor an anonymous token.
 *  This may be used to determine whether a first-time setup is required.
 */
bool UserManager::initRequired() const
{
    QString getTokensQuery = QString("SELECT id, username, creationdate, deviceName FROM tokens;");
    QSqlQuery resultQuery(m_db);
    if (!resultQuery.exec(getTokensQuery)) {
        qCWarning(dcUserManager()) << "Unable to execute SQL query" << getTokensQuery << m_db.lastError().databaseText() << m_db.lastError().driverText();
        return false;
    }

    if (m_db.lastError().type() != QSqlError::NoError) {
        qCWarning(dcUserManager) << "Query for tokens failed:" << m_db.lastError().databaseText() << m_db.lastError().driverText() << getTokensQuery;
        // Note: do not return true in case the database access fails.
        return false;
    }

    return users().isEmpty() && !resultQuery.first();
}

/*! Returns the list of user names for this UserManager. */
UserInfoList UserManager::users() const
{
    UserInfoList users;
    QString userQuery("SELECT * FROM users;");
    QSqlQuery resultQuery(m_db);
    if (!resultQuery.exec(userQuery)) {
        qCWarning(dcUserManager()) << "Unable to execute SQL query" << userQuery << m_db.lastError().databaseText() << m_db.lastError().driverText();
        return users;
    }

    while (resultQuery.next()) {
        UserInfo info = UserInfo(resultQuery.value("username").toString());
        info.setEmail(resultQuery.value("email").toString());
        info.setDisplayName(resultQuery.value("displayName").toString());
        info.setScopes(Types::scopesFromStringList(resultQuery.value("scopes").toString().split(',')));
        info.setAllowedThingIds(Types::thingIdsFromStringList(resultQuery.value("allowedThingIds").toString().split(',')));
        users.append(info);
    }
    return users;
}

/*! Creates a new user with the given \a username and \a password. Returns the \l UserError to inform about the result. */
UserManager::UserError UserManager::createUser(const QString &username, const QString &password, const QString &email, const QString &displayName, Types::PermissionScopes scopes, const QList<ThingId> &allowedThingIds)
{
    if (!validateUsername(username)) {
        qCWarning(dcUserManager) << "Error creating user. Invalid username:" << username;
        return UserErrorInvalidUserId;
    }

    if (!validatePassword(password)) {
        qCWarning(dcUserManager) << "Password failed character validation. Must contain a letter, a number and a special charactar. Minimum length: 8";
        return UserErrorBadPassword;
    }

    if (!validateScopes(scopes)) {
        // The method warns about he specific validation
        return UserErrorInconsistantScopes;
    }

    // Verify thing IDs, if there is no thing with this id, we don't save it and it will not be verified.
    // We don't return an error, the thing might have dissapeared
    QList<ThingId> thingIds;
    ThingManager *thingManager = NymeaCore::instance()->thingManager();
    if (!thingManager) {
        qCWarning(dcUserManager()) << "Cannot validate allowed things for user" << username
                                   << "because thing manager is not available yet. Skipping validation.";
        thingIds = allowedThingIds;
    } else {
        foreach (const ThingId &thingId, allowedThingIds) {
            if (thingManager->configuredThings().findById(thingId) == nullptr) {
                qCWarning(dcUserManager()) << "Cannot set user scope for" << username << "because there is no thing with ID ";
            } else {
                thingIds.append(thingId);
            }
        }
    }

    QSqlQuery checkForDuplicateUserQuery(m_db);
    checkForDuplicateUserQuery.prepare("SELECT * FROM users WHERE lower(username) = :username;");
    checkForDuplicateUserQuery.bindValue(":username", username.toLower());
    // Note: We're using toLower() on the username mainly for the reason that in old versions the username used to be an email address
    checkForDuplicateUserQuery.exec();
    if (checkForDuplicateUserQuery.first()) {
        qCWarning(dcUserManager) << "Username" << username << "already in use";
        return UserErrorDuplicateUserId;
    }

    static QRegularExpression bracketsRe("[{}]");
    QByteArray salt = QUuid::createUuid().toString().remove(bracketsRe).toUtf8();
    QByteArray hashedPassword = QCryptographicHash::hash(QString(password + salt).toUtf8(), QCryptographicHash::Sha512).toBase64();
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO users(username, email, displayName, password, salt, scopes, allowedThingIds)"
                  "VALUES(:username, :email, :displayName, :password, :salt, :scopes, :allowedThingIds);");

    query.bindValue(":username", username.toLower());
    query.bindValue(":email", email);
    query.bindValue(":displayName", displayName);
    query.bindValue(":password", QString::fromUtf8(hashedPassword));
    query.bindValue(":salt", QString::fromUtf8(salt));
    query.bindValue(":scopes", Types::scopesToStringList(scopes).join(','));
    query.bindValue(":allowedThingIds", Types::thingIdsToStringList(thingIds).join(','));
    query.exec();
    if (query.lastError().type() != QSqlError::NoError) {
        qCWarning(dcUserManager) << "Error creating user:" << query.lastError().databaseText() << query.lastError().driverText();
        return UserErrorBackendError;
    }

    qCInfo(dcUserManager()) << "New user" << username << "added to the system with permissions:" << Types::scopesToStringList(scopes);
    emit userAdded(username);
    return UserErrorNoError;
}

UserManager::UserError UserManager::changePassword(const QString &username, const QString &newPassword)
{
    if (!validateUsername(username)) {
        qCWarning(dcUserManager) << "Invalid username:" << username;
        return UserErrorInvalidUserId;
    }

    if (!validatePassword(newPassword)) {
        qCWarning(dcUserManager) << "Password failed character validation. Must contain a letter, a number and a special charactar. Minimum length: 8";
        return UserErrorBadPassword;
    }

    QString checkForUserExistingQueryString = QString("SELECT * FROM users WHERE lower(username) = \"%1\";").arg(username.toLower());
    QSqlQuery checkForUserExistingQuery(m_db);
    if (!checkForUserExistingQuery.exec(checkForUserExistingQueryString)) {
        qCWarning(dcUserManager()) << "Unable to execute SQL query" << checkForUserExistingQueryString << m_db.lastError().databaseText() << m_db.lastError().driverText();
        return UserErrorBackendError;
    }

    if (!checkForUserExistingQuery.first()) {
        qCWarning(dcUserManager) << "Username does not exist.";
        return UserErrorInvalidUserId;
    }

    // Update the password
    QByteArray salt = QUuid::createUuid().toString().remove(QRegularExpression("[{}]")).toUtf8();
    QByteArray hashedPassword = QCryptographicHash::hash(QString(newPassword + salt).toUtf8(), QCryptographicHash::Sha512).toBase64();

    QSqlQuery updatePasswordQuery(m_db);
    updatePasswordQuery.prepare("UPDATE users SET password = :password, salt = :salt WHERE lower(username) = :username;");
    updatePasswordQuery.bindValue(":password", QString::fromUtf8(hashedPassword));
    updatePasswordQuery.bindValue(":salt", QString::fromUtf8(salt));
    updatePasswordQuery.bindValue(":username", username.toLower());

    if (!updatePasswordQuery.exec()) {
        qCWarning(dcUserManager()) << "Unable to execute SQL query" << updatePasswordQuery.executedQuery() << m_db.lastError().databaseText() << m_db.lastError().driverText();
        return UserErrorBackendError;
    }

    if (m_db.lastError().type() != QSqlError::NoError) {
        qCWarning(dcUserManager) << "Error updating password for user:" << m_db.lastError().databaseText() << m_db.lastError().driverText();
        return UserErrorBackendError;
    }

    qCDebug(dcUserManager()) << "Password updated for user" << username;
    return UserErrorNoError;
}

/*! Removes \a username and every invitation/token/inventory row belonging to them in one
    transaction. Any statement or commit failure rolls back everything and emits nothing;
    invitation removals, token session revocations, and userRemoved are only emitted after
    a successful commit. */
UserManager::UserError UserManager::removeUser(const QString &username)
{
    if (!m_db.transaction()) {
        dumpDBError("Error starting transaction for removing user.");
        return UserErrorBackendError;
    }

    QSqlQuery selectTokensQuery(m_db);
    selectTokensQuery.prepare("SELECT token FROM tokens WHERE lower(username) = :username;");
    selectTokensQuery.bindValue(":username", username.toLower());
    if (!selectTokensQuery.exec()) {
        qCWarning(dcUserManager()) << "Unable to execute SQL query" << selectTokensQuery.lastQuery() << m_db.lastError().databaseText() << m_db.lastError().driverText();
        m_db.rollback();
        return UserErrorBackendError;
    }
    QList<QByteArray> removedTokenValues;
    while (selectTokensQuery.next())
        removedTokenValues << selectTokensQuery.value("token").toString().toUtf8();

    QSqlQuery selectInvitationsQuery(m_db);
    selectInvitationsQuery.prepare("SELECT id FROM invitations WHERE lower(username) = :username;");
    selectInvitationsQuery.bindValue(":username", username.toLower());
    if (!selectInvitationsQuery.exec()) {
        qCWarning(dcUserManager()) << "Unable to execute SQL query" << selectInvitationsQuery.lastQuery() << m_db.lastError().databaseText() << m_db.lastError().driverText();
        m_db.rollback();
        return UserErrorBackendError;
    }
    QList<QUuid> removedInvitationIds;
    while (selectInvitationsQuery.next())
        removedInvitationIds << QUuid(selectInvitationsQuery.value("id").toString());

    QSqlQuery dropUserQuery(m_db);
    dropUserQuery.prepare("DELETE FROM users WHERE lower(username) = :username;");
    dropUserQuery.bindValue(":username", username.toLower());
    if (!dropUserQuery.exec()) {
        qCWarning(dcUserManager()) << "Unable to execute SQL query" << dropUserQuery.lastQuery() << m_db.lastError().databaseText() << m_db.lastError().driverText();
        m_db.rollback();
        return UserErrorBackendError;
    }

    if (dropUserQuery.numRowsAffected() == 0) {
        m_db.rollback();
        return UserErrorInvalidUserId;
    }

    QSqlQuery dropInvitationsQuery(m_db);
    dropInvitationsQuery.prepare("DELETE FROM invitations WHERE lower(username) = :username;");
    dropInvitationsQuery.bindValue(":username", username.toLower());
    if (!dropInvitationsQuery.exec()) {
        qCWarning(dcUserManager()) << "Unable to execute SQL query" << dropInvitationsQuery.lastQuery() << m_db.lastError().databaseText() << m_db.lastError().driverText();
        m_db.rollback();
        return UserErrorBackendError;
    }

    QSqlQuery dropTokensQuery(m_db);
    dropTokensQuery.prepare("DELETE FROM tokens WHERE lower(username) = :username;");
    dropTokensQuery.bindValue(":username", username.toLower());
    if (!dropTokensQuery.exec()) {
        qCWarning(dcUserManager()) << "Unable to execute SQL query" << dropTokensQuery.lastQuery() << m_db.lastError().databaseText() << m_db.lastError().driverText();
        m_db.rollback();
        return UserErrorBackendError;
    }

    QSqlQuery dropInventoryQuery(m_db);
    dropInventoryQuery.prepare("DELETE FROM userInventory WHERE lower(username) = :username;");
    dropInventoryQuery.bindValue(":username", username.toLower());
    if (!dropInventoryQuery.exec()) {
        qCWarning(dcUserManager()) << "Unable to delete user inventory for user" << username << dropInventoryQuery.lastError().databaseText() << dropInventoryQuery.lastError().driverText();
        m_db.rollback();
        return UserErrorBackendError;
    }

    if (!m_db.commit()) {
        dumpDBError("Error committing user removal transaction.");
        m_db.rollback();
        return UserErrorBackendError;
    }

    foreach (const QUuid &invitationId, removedInvitationIds)
        emit invitationRemoved(invitationId);
    foreach (const QByteArray &tokenValue, removedTokenValues)
        emit tokenInvalidated(tokenValue);
    emit userRemoved(username);
    if (!removedTokenValues.isEmpty() && m_expiryTimer)
        rearmExpiryTimer();
    return UserErrorNoError;
}

UserManager::UserError UserManager::setUserScopes(const QString &username, Types::PermissionScopes scopes, const QList<ThingId> &allowedThingIds)
{
    if (!validateScopes(scopes)) {
        // The method warns about he specific validation
        return UserErrorInconsistantScopes;
    }

    // Verify thing IDs, if there is no thing with this id, we don't save it and it will not be verified.
    // We don't return an error, the thing might have dissapeared
    QList<ThingId> thingIds;
    ThingManager *thingManager = NymeaCore::instance()->thingManager();
    if (!thingManager) {
        qCWarning(dcUserManager()) << "Cannot validate allowed things for user" << username
                                   << "because thing manager is not available yet. Skipping validation.";
        thingIds = allowedThingIds;
    } else {
        foreach (const ThingId &thingId, allowedThingIds) {
            if (thingManager->configuredThings().findById(thingId) == nullptr) {
                qCWarning(dcUserManager()) << "The user" << username << "should have access to thing with ID"
                                           << thingId.toString() << "but there is no such thing. Ignoring value.";
            } else {
                thingIds.append(thingId);
            }
        }
    }

    QList<ThingId> thingsAppeared;
    QList<ThingId> thingsDisappeared;

    // Get the current allowed things
    if (!scopes.testFlag(Types::PermissionScopeAccessAllThings)) {

        // Restricted thing access, let's notify this user if any things appeared or dissapeard for the user
        UserInfo currentUserInfo = userInfo(username);

        // Get new appeared things for this user
        foreach (const ThingId &thingId, thingIds) {
            if (currentUserInfo.allowedThingIds().contains(thingId))
                continue;

            qCDebug(dcUserManager()) << "Thing with ID" << thingId.toString() << "now allowed for this user any more. Notify user" << username << "that thing appeared.";
            thingsAppeared.append(thingId);
        }

        // Get disappeared things for this user
        foreach (const ThingId &thingId, currentUserInfo.allowedThingIds()) {
            if (thingIds.contains(thingId))
                continue;

            qCDebug(dcUserManager()) << "Thing with ID" << thingId.toString() << "not allowed for this user any more. Notify user" << username << "that thing dissappeared.";
            thingsDisappeared.append(thingId);
        }
    }

    QString scopesString = Types::scopesToStringList(scopes).join(',');
    QString allowedThingIdsString = Types::thingIdsToStringList(thingIds).join(',');

    qCDebug(dcUserManager()) << "Updating scopes of user" << username << "Scopes:" << scopes << "Allowed things:" << allowedThingIds;

    QSqlQuery setScopesQuery(m_db);
    setScopesQuery.prepare("UPDATE users SET scopes = :scopes, allowedThingIds = :allowedThingIds WHERE username = :username;");
    setScopesQuery.bindValue(":username", username);
    setScopesQuery.bindValue(":scopes", scopesString);
    setScopesQuery.bindValue(":allowedThingIds", allowedThingIdsString);
    if (!setScopesQuery.exec()) {
        qCWarning(dcUserManager()) << "Error updating scopes for user" << username << setScopesQuery.lastError().databaseText() << setScopesQuery.lastError().driverText();
        return UserErrorBackendError;
    }

    emit userChanged(username);

    // Notify after updating the user information
    UserInfo ui = userInfo(username);
    foreach (const ThingId &thingId, thingsAppeared)
        emit userThingRestrictionsChanged(ui, thingId, true);

    foreach (const ThingId &thingId, thingsDisappeared)
        emit userThingRestrictionsChanged(ui, thingId, false);

    return UserErrorNoError;
}

UserManager::UserError UserManager::setUserInfo(const QString &username, const QString &email, const QString &displayName)
{
    QSqlQuery query(m_db);
    query.prepare("UPDATE users SET email = :email, displayName = :displayName WHERE username = :username;");
    query.bindValue(":email", email);
    query.bindValue(":displayName", displayName);
    query.bindValue(":username", username);
    query.exec();
    if (query.lastError().type() != QSqlError::NoError) {
        qCWarning(dcUserManager()) << "Error updating user info for user" << username << query.lastError().databaseText() << query.lastError().driverText() << query.executedQuery();
        return UserErrorBackendError;
    }
    emit userChanged(username);
    return UserErrorNoError;
}

/*! Returns true if the push button authentication is available for this system. */
bool UserManager::pushButtonAuthAvailable() const
{
    return m_pushButtonDBusService->agentAvailable();
}

/*! Authenticated the given \a username with the given \a password for the \a deviceName. If the authentication was
    successful, the token will be returned, otherwise the return value will be an empty byte array.
*/
QByteArray UserManager::authenticate(const QString &username, const QString &password, const QString &deviceName)
{
    if (!validateUsername(username)) {
        qCWarning(dcUserManager) << "Authenticate: Username did not pass validation:" << username;
        return QByteArray();
    }

    QSqlQuery passwordQuery(m_db);
    passwordQuery.prepare("SELECT password, salt FROM users WHERE lower(username) = :username;");
    passwordQuery.bindValue(":username", username.toLower());
    passwordQuery.exec();
    if (!passwordQuery.first()) {
        qCWarning(dcUserManager) << "No such username" << username;
        return QByteArray();
    }

    QByteArray salt = passwordQuery.value("salt").toByteArray();
    QByteArray hashedPassword = passwordQuery.value("password").toByteArray();

    if (hashedPassword != QCryptographicHash::hash(QString(password + salt).toUtf8(), QCryptographicHash::Sha512).toBase64()) {
        qCWarning(dcUserManager) << "Authentication error for user:" << username;
        return QByteArray();
    }

    QByteArray token = QCryptographicHash::hash(QUuid::createUuid().toByteArray(), QCryptographicHash::Sha256).toBase64();

    QSqlQuery storeTokenQuery(m_db);
    storeTokenQuery.prepare("INSERT INTO tokens (id, username, token, creationdate, devicename)"
                            "VALUES (:id, :username, :token, :creationdate, :devicename)");
    storeTokenQuery.bindValue(":id", QUuid::createUuid().toString());
    storeTokenQuery.bindValue(":username", username.toLower());
    storeTokenQuery.bindValue(":token", QString::fromUtf8(token));
    storeTokenQuery.bindValue(":creationdate", NymeaCore::instance()->timeManager()->currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    storeTokenQuery.bindValue(":devicename", deviceName);

    if (!storeTokenQuery.exec()) {
        qCWarning(dcUserManager()) << "Unable to execute SQL query" << storeTokenQuery.lastQuery() << m_db.lastError().databaseText() << m_db.lastError().driverText();
        return QByteArray();
    }

    if (m_db.lastError().type() != QSqlError::NoError) {
        qCWarning(dcUserManager) << "Error storing token in DB:" << m_db.lastError().databaseText() << m_db.lastError().driverText();
        return QByteArray();
    }

    return token;
}

/*! Start the push button authentication for the device with the given \a deviceName. Returns the transaction id as refference to the request. */
int UserManager::requestPushButtonAuth(const QString &deviceName)
{
    if (m_pushButtonTransaction.first != -1) {
        qCWarning(dcUserManager()) << "PushButton authentication already in progress for device" << m_pushButtonTransaction.second << ". Cancelling...";
        cancelPushButtonAuth(m_pushButtonTransaction.first);
    }

    qCDebug(dcUserManager()) << "Starting PushButton authentication for device" << deviceName;
    int transactionId = ++m_pushButtonTransactionIdCounter;
    m_pushButtonTransaction = QPair<int, QString>(transactionId, deviceName);
    return transactionId;
}

/*! Cancel the push button authentication with the given \a transactionId.

    \sa requestPushButtonAuth
*/
void UserManager::cancelPushButtonAuth(int transactionId)
{
    if (m_pushButtonTransaction.first == -1) {
        qCWarning(dcUserManager()) << "No PushButton transaction in progress. Nothing to cancel.";
        return;
    }
    if (m_pushButtonTransaction.first != transactionId) {
        qCWarning(dcUserManager()) << "PushButton transaction" << transactionId << "not in progress. Cannot cancel.";
        return;
    }
    qCDebug(dcUserManager()) << "Cancelling PushButton transaction for device:" << m_pushButtonTransaction.second;
    emit pushButtonAuthFinished(m_pushButtonTransaction.first, false, QByteArray());
    m_pushButtonTransaction.first = -1;

}

/*! Request UserInfo.
 The UserInfo for the given username is returned.
*/
UserInfo UserManager::userInfo(const QString &username) const
{
    QSqlQuery getUserQuery(m_db);
    getUserQuery.prepare("SELECT * FROM users WHERE lower(username) = :username;");
    getUserQuery.bindValue(":username", username);
    if (!getUserQuery.exec()) {
        qCWarning(dcUserManager()) << "Unable to execute SQL query" << getUserQuery.lastQuery() << m_db.lastError().databaseText() << m_db.lastError().driverText();
        return UserInfo();
    }

    if (m_db.lastError().type() != QSqlError::NoError) {
        qCWarning(dcUserManager) << "Query for user" << username << "failed:" << getUserQuery.lastQuery() << m_db.lastError().databaseText() << m_db.lastError().driverText();
        return UserInfo();
    }

    if (!getUserQuery.first())
        return UserInfo();

    UserInfo userInfo = UserInfo(getUserQuery.value("username").toString());
    userInfo.setEmail(getUserQuery.value("email").toString());
    userInfo.setDisplayName(getUserQuery.value("displayName").toString());
    userInfo.setScopes(Types::scopesFromStringList(getUserQuery.value("scopes").toString().split(',')));
    userInfo.setAllowedThingIds(Types::thingIdsFromStringList(getUserQuery.value("allowedThingIds").toString().split(',')));
    return userInfo;
}

QList<TokenInfo> UserManager::tokens(const QString &username) const
{
    QList<TokenInfo> ret;

    QSqlQuery query(m_db);
    query.prepare("SELECT id, username, creationdate, deviceName, expirydate, lastseen FROM tokens WHERE lower(username) = :username;");
    query.bindValue(":username", username.toLower());
    query.exec();
    if (m_db.lastError().type() != QSqlError::NoError) {
        qCWarning(dcUserManager) << "Query for tokens failed:" << query.lastError().databaseText() << query.lastError().driverText() << query.executedQuery();
        return ret;
    }

    while (query.next()) {
        if (isTokenLogicallyExpired(query.value("expirydate")))
            continue;
        ret << TokenInfo(query.value("id").toUuid(), query.value("username").toString(), query.value("creationdate").toDateTime(), query.value("devicename").toString(),
                          parseStoredUtcDateTime(query.value("expirydate")), parseStoredUtcDateTime(query.value("lastseen")));
    }
    return ret;
}

UserInventoryItems UserManager::userInventoryItems(const QString &username, const QString &type) const
{
    UserInventoryItems ret;

    QStringList conditions;
    if (!username.isEmpty())
        conditions << "lower(username) = :username";
    if (!type.isEmpty())
        conditions << "type = :type";

    QSqlQuery query(m_db);
    query.prepare(QString("SELECT * FROM userInventory%1 ORDER BY username, type, displayName;")
                  .arg(conditions.isEmpty() ? QString() : QString(" WHERE %1").arg(conditions.join(" AND "))));
    if (!username.isEmpty())
        query.bindValue(":username", username.toLower());
    if (!type.isEmpty())
        query.bindValue(":type", type);

    if (!query.exec()) {
        qCWarning(dcUserManager()) << "Unable to query user inventory" << query.lastError().databaseText() << query.lastError().driverText();
        return ret;
    }

    while (query.next())
        ret.append(inventoryItemFromQuery(query));

    return ret;
}

UserInventoryItem UserManager::userInventoryItem(const QUuid &inventoryItemId) const
{
    if (inventoryItemId.isNull())
        return UserInventoryItem();

    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM userInventory WHERE id = :id;");
    query.bindValue(":id", inventoryItemId.toString());
    if (!query.exec()) {
        qCWarning(dcUserManager()) << "Unable to query user inventory item" << inventoryItemId << query.lastError().databaseText() << query.lastError().driverText();
        return UserInventoryItem();
    }

    if (!query.first())
        return UserInventoryItem();

    return inventoryItemFromQuery(query);
}

UserInventoryItem UserManager::findEnabledUserInventoryItem(const QString &type, const QString &payloadKey, const QVariant &payloadValue) const
{
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM userInventory WHERE type = :type AND enabled = 1;");
    query.bindValue(":type", type);
    if (!query.exec()) {
        qCWarning(dcUserManager()) << "Unable to query enabled user inventory items" << query.lastError().databaseText() << query.lastError().driverText();
        return UserInventoryItem();
    }

    while (query.next()) {
        UserInventoryItem item = inventoryItemFromQuery(query);
        if (item.payload().value(payloadKey) == payloadValue)
            return item;
    }

    return UserInventoryItem();
}

TokenInfo UserManager::tokenInfo(const QByteArray &token) const
{
    if (token.isEmpty()) {
        return TokenInfo();
    }

    if (!validateToken(token)) {
        qCWarning(dcUserManager()) << "Token did not pass validation:" << token;
        return TokenInfo();
    }

    QSqlQuery getTokenQuery(m_db);
    getTokenQuery.prepare("SELECT id, username, creationdate, deviceName, expirydate, lastseen FROM tokens WHERE token = :token;");
    getTokenQuery.bindValue(":token", QString::fromUtf8(token));
    if (!getTokenQuery.exec()) {
        qCWarning(dcUserManager()) << "Unable to execute SQL query" << getTokenQuery.lastQuery() << m_db.lastError().databaseText() << m_db.lastError().driverText();
        return TokenInfo();
    }

    if (m_db.lastError().type() != QSqlError::NoError) {
        qCWarning(dcUserManager) << "Query for token failed:" << getTokenQuery.lastQuery() << m_db.lastError().databaseText() << m_db.lastError().driverText();
        return TokenInfo();
    }

    if (!getTokenQuery.first())
        return TokenInfo();

    if (isTokenLogicallyExpired(getTokenQuery.value("expirydate")))
        return TokenInfo();

    return TokenInfo(getTokenQuery.value("id").toUuid(), getTokenQuery.value("username").toString(), getTokenQuery.value("creationdate").toDateTime(), getTokenQuery.value("devicename").toString(),
                      parseStoredUtcDateTime(getTokenQuery.value("expirydate")), parseStoredUtcDateTime(getTokenQuery.value("lastseen")));
}

TokenInfo UserManager::tokenInfo(const QUuid &tokenId) const
{
    QSqlQuery getTokenQuery(m_db);
    getTokenQuery.prepare("SELECT id, username, creationdate, deviceName, expirydate, lastseen FROM tokens WHERE id = :id;");
    getTokenQuery.bindValue(":id", tokenId.toString());
    if (!getTokenQuery.exec()) {
        qCWarning(dcUserManager()) << "Unable to execute SQL query" << getTokenQuery.lastQuery() << m_db.lastError().databaseText() << m_db.lastError().driverText();
        return TokenInfo();
    }

    if (m_db.lastError().type() != QSqlError::NoError) {
        qCWarning(dcUserManager) << "Query for token failed:" << getTokenQuery.lastQuery() << m_db.lastError().databaseText() << m_db.lastError().driverText();
        return TokenInfo();
    }

    if (!getTokenQuery.first()) {
        return TokenInfo();
    }

    if (isTokenLogicallyExpired(getTokenQuery.value("expirydate")))
        return TokenInfo();

    return TokenInfo(getTokenQuery.value("id").toUuid(), getTokenQuery.value("username").toString(), getTokenQuery.value("creationdate").toDateTime(), getTokenQuery.value("devicename").toString(),
                      parseStoredUtcDateTime(getTokenQuery.value("expirydate")), parseStoredUtcDateTime(getTokenQuery.value("lastseen")));
}

/*! Removes the token with the given \a tokenId. Returns \l{UserError} to inform about the result. */
UserManager::UserError UserManager::removeToken(const QUuid &tokenId)
{
    QSqlQuery selectTokenQuery(m_db);
    selectTokenQuery.prepare("SELECT token FROM tokens WHERE id = :id;");
    selectTokenQuery.bindValue(":id", tokenId.toString());
    if (!selectTokenQuery.exec() || !selectTokenQuery.first()) {
        qCWarning(dcUserManager) << "Tried to remove token, but the token could not be found in the DB.";
        return UserErrorTokenNotFound;
    }
    QByteArray tokenValue = selectTokenQuery.value("token").toString().toUtf8();

    QSqlQuery removeTokenQuery(m_db);
    removeTokenQuery.prepare("DELETE FROM tokens WHERE id = :id;");
    removeTokenQuery.bindValue(":id", tokenId.toString());

    if (!removeTokenQuery.exec()) {
        qCWarning(dcUserManager()) << "Unable to execute SQL query" << removeTokenQuery.lastQuery() << m_db.lastError().databaseText() << m_db.lastError().driverText();
        return UserErrorBackendError;
    }

    if (m_db.lastError().type() != QSqlError::NoError) {
        qCWarning(dcUserManager) << "Removing token failed:" << removeTokenQuery.lastQuery() << m_db.lastError().databaseText() << m_db.lastError().driverText();
        return UserErrorBackendError;
    }

    if (removeTokenQuery.numRowsAffected() != 1) {
        qCWarning(dcUserManager) << "Tried to remove token, but the token could not be found in the DB.";
        return UserErrorTokenNotFound;
    }

    qCDebug(dcUserManager) << "Token" << tokenId << "removed from DB";
    emit tokenInvalidated(tokenValue);
    if (m_expiryTimer)
        rearmExpiryTimer();
    return UserErrorNoError;
}

/*! Marks the token with the given \a tokenId as last seen at \a timestamp, a single prepared
    UPDATE by token id. The clear token value is never received or logged here. The result is
    diagnostic only: callers must keep an already validated connection authenticated even if
    this returns false. */
bool UserManager::markTokenSeen(const QUuid &tokenId, const QDateTime &timestamp)
{
    QSqlQuery query(m_db);
    query.prepare("UPDATE tokens SET lastseen = :lastseen WHERE id = :id;");
    query.bindValue(":lastseen", formatUtcDateTimeForStorage(timestamp));
    query.bindValue(":id", tokenId.toString());

    if (!query.exec()) {
        qCWarning(dcUserManager()) << "Unable to mark token seen for token id" << tokenId << query.lastError().databaseText() << query.lastError().driverText();
        return false;
    }

    if (query.numRowsAffected() != 1) {
        qCWarning(dcUserManager()) << "Marking token seen affected" << query.numRowsAffected() << "rows for token id" << tokenId;
        return false;
    }

    return true;
}

UserManager::UserError UserManager::addUserInventoryItem(const QString &username, const QString &type, const QString &displayName, const QVariantMap &payload, bool enabled)
{
    if (!validateUsername(username) || !userInfo(username.toLower()).isValid()) {
        qCWarning(dcUserManager()) << "Error adding user inventory item. Invalid username:" << username;
        return UserErrorInvalidUserId;
    }

    if (!validateInventoryItem(type, payload))
        return UserErrorInvalidInventoryItem;

    if (enabled && type == "rfidTag" && enabledInventoryItemExists(type, "tagHash", payload.value("tagHash"))) {
        qCWarning(dcUserManager()) << "Refusing duplicate enabled RFID tag hash for user" << username;
        return UserErrorDuplicateInventoryItem;
    }

    const QByteArray serializedPayload = serializeInventoryPayload(payload);
    if (serializedPayload.isEmpty() && !payload.isEmpty())
        return UserErrorInvalidInventoryItem;

    QSqlQuery query(m_db);
    query.prepare("INSERT INTO userInventory(id, username, type, displayName, enabled, payload) "
                  "VALUES(:id, :username, :type, :displayName, :enabled, :payload);");
    query.bindValue(":id", QUuid::createUuid().toString());
    query.bindValue(":username", username.toLower());
    query.bindValue(":type", type);
    query.bindValue(":displayName", displayName);
    query.bindValue(":enabled", enabled ? 1 : 0);
    query.bindValue(":payload", QString::fromUtf8(serializedPayload));
    if (!query.exec()) {
        qCWarning(dcUserManager()) << "Error adding user inventory item:" << query.lastError().databaseText() << query.lastError().driverText();
        return UserErrorBackendError;
    }

    emit userChanged(username.toLower());
    return UserErrorNoError;
}

UserManager::UserError UserManager::updateUserInventoryItem(const QUuid &inventoryItemId, const QString &displayName, const QVariantMap &payload, bool enabled)
{
    UserInventoryItem item = userInventoryItem(inventoryItemId);
    if (!item.isValid())
        return UserErrorInventoryItemNotFound;

    if (!validateInventoryItem(item.type(), payload))
        return UserErrorInvalidInventoryItem;

    if (enabled && item.type() == "rfidTag" && enabledInventoryItemExists(item.type(), "tagHash", payload.value("tagHash"), inventoryItemId)) {
        qCWarning(dcUserManager()) << "Refusing duplicate enabled RFID tag hash for inventory item" << inventoryItemId;
        return UserErrorDuplicateInventoryItem;
    }

    const QByteArray serializedPayload = serializeInventoryPayload(payload);
    if (serializedPayload.isEmpty() && !payload.isEmpty())
        return UserErrorInvalidInventoryItem;

    QSqlQuery query(m_db);
    query.prepare("UPDATE userInventory SET displayName = :displayName, enabled = :enabled, payload = :payload WHERE id = :id;");
    query.bindValue(":id", inventoryItemId.toString());
    query.bindValue(":displayName", displayName);
    query.bindValue(":enabled", enabled ? 1 : 0);
    query.bindValue(":payload", QString::fromUtf8(serializedPayload));
    if (!query.exec()) {
        qCWarning(dcUserManager()) << "Error updating user inventory item:" << query.lastError().databaseText() << query.lastError().driverText();
        return UserErrorBackendError;
    }

    emit userChanged(item.username());
    return UserErrorNoError;
}

UserManager::UserError UserManager::removeUserInventoryItem(const QUuid &inventoryItemId)
{
    UserInventoryItem item = userInventoryItem(inventoryItemId);
    if (!item.isValid())
        return UserErrorInventoryItemNotFound;

    QSqlQuery query(m_db);
    query.prepare("DELETE FROM userInventory WHERE id = :id;");
    query.bindValue(":id", inventoryItemId.toString());
    if (!query.exec()) {
        qCWarning(dcUserManager()) << "Error removing user inventory item:" << query.lastError().databaseText() << query.lastError().driverText();
        return UserErrorBackendError;
    }

    emit userChanged(item.username());
    return UserErrorNoError;
}

/*! Creates a one-time invitation for \a username, valid for \a validitySeconds (the
    invitation's own absolute expiry, 1..2592000). When \a hasTokenValidity is true, the
    regular token minted on redemption additionally expires \a tokenValiditySeconds after
    a successful redemption (also 1..2592000); otherwise the redeemed token never expires.
    On success, the clear one-time token is returned via \a oneTimeToken (this is the only
    place it ever leaves the server - only its hash is stored) and \a info is populated. */
UserManager::UserError UserManager::createInvitation(const QString &username, uint validitySeconds,
                                                       bool hasTokenValidity, uint tokenValiditySeconds,
                                                       QByteArray &oneTimeToken, InvitationInfo &info)
{
    if (!m_invitationsAvailable)
        return UserErrorInvitationsDisabled;

    if (validitySeconds < 1 || validitySeconds > 2592000)
        return UserErrorInvalidInvitationDuration;
    if (hasTokenValidity && (tokenValiditySeconds < 1 || tokenValiditySeconds > 2592000))
        return UserErrorInvalidInvitationDuration;

    QSqlQuery userExistsQuery(m_db);
    userExistsQuery.prepare("SELECT username FROM users WHERE lower(username) = :username;");
    userExistsQuery.bindValue(":username", username.toLower());
    if (!userExistsQuery.exec() || !userExistsQuery.first()) {
        qCWarning(dcUserManager()) << "Cannot create invitation for unknown user" << username;
        return UserErrorInvalidUserId;
    }

    QByteArray clearToken = QCryptographicHash::hash(QUuid::createUuid().toByteArray(), QCryptographicHash::Sha256).toBase64();
    QByteArray tokenHash = QCryptographicHash::hash(clearToken, QCryptographicHash::Sha256).toBase64();
    QUuid invitationId = QUuid::createUuid();
    QDateTime creationTime = NymeaCore::instance()->timeManager()->currentDateTime().toUTC();
    QDateTime expiryTime = creationTime.addSecs(validitySeconds);

    QSqlQuery insertQuery(m_db);
    insertQuery.prepare("INSERT INTO invitations (id, username, tokenhash, creationdate, expirydate, tokenvalidityduration) "
                         "VALUES (:id, :username, :tokenhash, :creationdate, :expirydate, :tokenvalidityduration);");
    insertQuery.bindValue(":id", invitationId.toString());
    insertQuery.bindValue(":username", username.toLower());
    insertQuery.bindValue(":tokenhash", QString::fromUtf8(tokenHash));
    insertQuery.bindValue(":creationdate", formatUtcDateTimeForStorage(creationTime));
    insertQuery.bindValue(":expirydate", formatUtcDateTimeForStorage(expiryTime));
    insertQuery.bindValue(":tokenvalidityduration", hasTokenValidity ? QVariant(tokenValiditySeconds) : QVariant());
    if (!insertQuery.exec() || insertQuery.lastError().isValid()) {
        qCWarning(dcUserManager()) << "Unable to create invitation" << insertQuery.lastError().databaseText() << insertQuery.lastError().driverText();
        return UserErrorBackendError;
    }

    info = InvitationInfo(invitationId, username.toLower(), creationTime, expiryTime,
                           hasTokenValidity ? QVariant(tokenValiditySeconds) : QVariant());
    oneTimeToken = clearToken;
    emit invitationAdded(info);
    return UserErrorNoError;
}

/*! Returns all invitations in \a result, optionally filtered by \a username (all when
    empty). Expired invitations are purged before listing, so the result never contains
    one. */
UserManager::UserError UserManager::invitations(QList<InvitationInfo> &result, const QString &username)
{
    if (!m_invitationsAvailable)
        return UserErrorInvitationsDisabled;

    purgeExpiredInvitations();

    QSqlQuery query(m_db);
    if (username.isEmpty()) {
        query.prepare("SELECT id, username, creationdate, expirydate, tokenvalidityduration FROM invitations;");
    } else {
        query.prepare("SELECT id, username, creationdate, expirydate, tokenvalidityduration FROM invitations WHERE lower(username) = :username;");
        query.bindValue(":username", username.toLower());
    }
    if (!query.exec()) {
        qCWarning(dcUserManager()) << "Unable to query invitations" << query.lastError().databaseText() << query.lastError().driverText();
        return UserErrorBackendError;
    }

    while (query.next()) {
        QVariant tokenValidityDuration = query.value("tokenvalidityduration").isNull()
                ? QVariant() : QVariant(query.value("tokenvalidityduration").toUInt());
        result << InvitationInfo(QUuid(query.value("id").toString()), query.value("username").toString(),
                                  parseStoredUtcDateTime(query.value("creationdate")),
                                  parseStoredUtcDateTime(query.value("expirydate")),
                                  tokenValidityDuration);
    }
    return UserErrorNoError;
}

/*! Removes the pending invitation with the given \a invitationId. */
UserManager::UserError UserManager::removeInvitation(const QUuid &invitationId)
{
    if (!m_invitationsAvailable)
        return UserErrorInvitationsDisabled;

    QSqlQuery query(m_db);
    query.prepare("DELETE FROM invitations WHERE id = :id;");
    query.bindValue(":id", invitationId.toString());
    if (!query.exec()) {
        qCWarning(dcUserManager()) << "Unable to remove invitation" << query.lastError().databaseText() << query.lastError().driverText();
        return UserErrorBackendError;
    }
    if (query.numRowsAffected() != 1) {
        qCWarning(dcUserManager()) << "Tried to remove invitation, but it could not be found in the DB.";
        return UserErrorInvitationNotFound;
    }

    emit invitationRemoved(invitationId);
    return UserErrorNoError;
}

void UserManager::purgeExpiredInvitations()
{
    QSqlQuery selectQuery(m_db);
    if (!selectQuery.exec("SELECT id, expirydate FROM invitations;")) {
        qCWarning(dcUserManager()) << "Unable to query for expired invitations" << selectQuery.lastError().databaseText() << selectQuery.lastError().driverText();
        return;
    }

    QList<QUuid> expiredIds;
    while (selectQuery.next()) {
        // isTokenLogicallyExpired() is a plain "now >= this UTC timestamp" check; it is
        // not actually token-specific and applies equally to invitations.expirydate.
        if (isTokenLogicallyExpired(selectQuery.value("expirydate")))
            expiredIds << QUuid(selectQuery.value("id").toString());
    }

    foreach (const QUuid &invitationId, expiredIds) {
        QSqlQuery deleteQuery(m_db);
        deleteQuery.prepare("DELETE FROM invitations WHERE id = :id;");
        deleteQuery.bindValue(":id", invitationId.toString());
        if (deleteQuery.exec() && deleteQuery.numRowsAffected() == 1) {
            emit invitationRemoved(invitationId);
        } else {
            qCWarning(dcUserManager()) << "Failed to purge expired invitation" << invitationId << "- will retry on the next check.";
        }
    }
}

/*! Redeems the one-time invitation identified by \a oneTimeToken for the given
    \a deviceName, returning the resulting regular client token on success. Returns an
    empty QByteArray for every failure (malformed input, unknown/already-used/expired
    token, or a database error) without distinguishing between them, so this never
    provides a state oracle. Lookup, expiry check, invitation deletion, and client-token
    insertion happen in one SQLite transaction: any failure rolls back the whole
    operation, leaving the invitation redeemable and letting no client token escape. */
QByteArray UserManager::redeemInvitation(const QByteArray &oneTimeToken, const QString &deviceName)
{
    // Disabled collapses into the same empty-result shape as every other failure: it
    // must never reveal whether a supplied token once existed.
    if (!m_invitationsAvailable)
        return QByteArray();

    // Before any hash or query: reject malformed input up front.
    if (!isCanonicalInvitationToken(oneTimeToken))
        return QByteArray();
    if (!isValidInvitationDeviceName(deviceName))
        return QByteArray();

    QByteArray tokenHash = QCryptographicHash::hash(oneTimeToken, QCryptographicHash::Sha256).toBase64();

    if (!m_db.transaction()) {
        dumpDBError("Error starting transaction for invitation redemption.");
        return QByteArray();
    }

    QSqlQuery selectQuery(m_db);
    selectQuery.prepare("SELECT id, username, expirydate, tokenvalidityduration FROM invitations WHERE tokenhash = :tokenhash;");
    selectQuery.bindValue(":tokenhash", QString::fromUtf8(tokenHash));
    if (!selectQuery.exec() || !selectQuery.first()) {
        m_db.rollback();
        return QByteArray();
    }

    QUuid invitationId = QUuid(selectQuery.value("id").toString());
    QString username = selectQuery.value("username").toString();
    QVariant expiryDateValue = selectQuery.value("expirydate");
    QVariant tokenValidityDurationValue = selectQuery.value("tokenvalidityduration");

    if (isTokenLogicallyExpired(expiryDateValue)) {
        // Expired: still delete it and commit that deletion within this same
        // transaction so it doesn't linger looking redeemable, but redemption fails.
        QSqlQuery deleteExpiredQuery(m_db);
        deleteExpiredQuery.prepare("DELETE FROM invitations WHERE id = :id;");
        deleteExpiredQuery.bindValue(":id", invitationId.toString());
        if (!deleteExpiredQuery.exec() || deleteExpiredQuery.numRowsAffected() != 1 || !m_db.commit()) {
            m_db.rollback();
            return QByteArray();
        }
        emit invitationRemoved(invitationId);
        return QByteArray();
    }

    QSqlQuery deleteInvitationQuery(m_db);
    deleteInvitationQuery.prepare("DELETE FROM invitations WHERE id = :id;");
    deleteInvitationQuery.bindValue(":id", invitationId.toString());
    if (!deleteInvitationQuery.exec() || deleteInvitationQuery.numRowsAffected() != 1) {
        // Gone already (a concurrent/earlier redemption or purge won the race).
        m_db.rollback();
        return QByteArray();
    }

    QByteArray clientToken = QCryptographicHash::hash(QUuid::createUuid().toByteArray(), QCryptographicHash::Sha256).toBase64();
    QDateTime localNow = NymeaCore::instance()->timeManager()->currentDateTime();
    QVariant clientTokenExpiry;
    if (tokenValidityDurationValue.isValid() && !tokenValidityDurationValue.isNull()) {
        // Measured from this successful redemption, never from invitation creation.
        QDateTime expiry = localNow.toUTC().addSecs(tokenValidityDurationValue.toUInt());
        clientTokenExpiry = formatUtcDateTimeForStorage(expiry);
    }

    QSqlQuery insertTokenQuery(m_db);
    insertTokenQuery.prepare("INSERT INTO tokens (id, username, token, creationdate, devicename, expirydate, lastseen) "
                             "VALUES (:id, :username, :token, :creationdate, :devicename, :expirydate, NULL);");
    insertTokenQuery.bindValue(":id", QUuid::createUuid().toString());
    insertTokenQuery.bindValue(":username", username);
    insertTokenQuery.bindValue(":token", QString::fromUtf8(clientToken));
    // Same timezone-less local-time convention already used by authenticate()'s
    // tokens.creationdate write; only expirydate/lastseen use the UTC-safe convention.
    insertTokenQuery.bindValue(":creationdate", localNow.toString("yyyy-MM-dd hh:mm:ss"));
    insertTokenQuery.bindValue(":devicename", deviceName);
    insertTokenQuery.bindValue(":expirydate", clientTokenExpiry);
    if (!insertTokenQuery.exec() || insertTokenQuery.lastError().isValid()) {
        m_db.rollback();
        return QByteArray();
    }

    if (!m_db.commit()) {
        dumpDBError("Error committing invitation redemption transaction.");
        m_db.rollback();
        return QByteArray();
    }

    emit invitationRemoved(invitationId);
    if (clientTokenExpiry.isValid() && m_expiryTimer)
        rearmExpiryTimer();
    return clientToken;
}

/*! Returns true, if the given \a token is valid. */
bool UserManager::verifyToken(const QByteArray &token)
{
    if (!validateToken(token)) {
        qCWarning(dcUserManager) << "Token failed character validation" << token;
        return false;
    }

    QSqlQuery getTokenQuery(m_db);
    getTokenQuery.prepare("SELECT * FROM tokens WHERE token = :token;");
    getTokenQuery.bindValue(":token", QString::fromUtf8(token));

    if (!getTokenQuery.exec()) {
        qCWarning(dcUserManager()) << "Unable to execute SQL query" << getTokenQuery.lastQuery() << m_db.lastError().databaseText() << m_db.lastError().driverText();
        return false;
    }

    if (m_db.lastError().type() != QSqlError::NoError) {
        qCWarning(dcUserManager) << "Query for token failed:" << getTokenQuery.lastQuery() << m_db.lastError().databaseText() << m_db.lastError().driverText() << getTokenQuery.lastQuery();
        return false;
    }

    if (!getTokenQuery.first()) {
        qCDebug(dcUserManager) << "Authorization failed for token" << token;
        return false;
    }

    if (isTokenLogicallyExpired(getTokenQuery.value("expirydate"))) {
        qCDebug(dcUserManager) << "Token has expired" << token;
        return false;
    }

    //qCDebug(dcUserManager) << "Token authorized for user" << result.value("username").toString();
    return true;
}

bool UserManager::hasRestrictedThingAccess(const QByteArray &token) const
{
    UserInfo ui = userInfo(tokenInfo(token).username());
    return !ui.scopes().testFlag(Types::PermissionScopeAccessAllThings);
}

bool UserManager::accessToThingGranted(const ThingId &thingId, const QByteArray &token)
{
    if (!hasRestrictedThingAccess(token))
        return true;

    return getAllowedThingIdsForToken(token).contains(thingId);
}

bool UserManager::accessToThingGranted(const ThingId &thingId, const QString &username) const
{
    UserInfo ui = userInfo(username);
    if (!ui.isValid())
        return false;

    if (ui.scopes().testFlag(Types::PermissionScopeAccessAllThings))
        return true;

    return ui.allowedThingIds().contains(thingId);
}

QList<ThingId> UserManager::getAllowedThingIdsForToken(const QByteArray &token) const
{
    return userInfo(tokenInfo(token).username()).allowedThingIds();
}

void UserManager::onThingRemoved(const ThingId &thingId)
{
    // If a thing has been removed from the system, clean up any thing based permissions
    foreach (const UserInfo &userInfo, users()) {
        if (userInfo.allowedThingIds().contains(thingId)) {
            QList<ThingId> allowedThingIds = userInfo.allowedThingIds();
            allowedThingIds.removeAll(thingId);

            if (setUserScopes(userInfo.username(), userInfo.scopes(), allowedThingIds) != UserErrorNoError) {
                qCWarning(dcUserManager()) << "Failed to remove thing with ID" << thingId.toString() << "from allowed things of user" << userInfo.username();
            } else {
                qCDebug(dcUserManager()) << "Removed thing with ID" << thingId.toString() << "from allowed things of user" << userInfo.username();
            }
        }
    }
}

bool UserManager::initDB()
{
    m_db.close();

    if (!m_db.open()) {
        dumpDBError("Can't open user database. Init failed.");
        return false;
    }

    // Recorded on every attempt (fresh install and migration retries alike) so the
    // constructor can tell a fresh/corrupt file (nothing to lose, safe to rotate) apart
    // from an existing database that already had real accounts (must not be rotated away).
    m_hadUsersTableBeforeInit = m_db.tables().contains("users");

    int currentVersion = -1;
    int newVersion = 5;

    if (m_db.tables().contains("metadata")) {
        QSqlQuery query(m_db);
        if (!query.exec("SELECT data FROM metadata WHERE key = 'version';")) {
            qCWarning(dcUserManager()) << "Unable to execute SQL query" << query.executedQuery() << m_db.lastError().databaseText() << m_db.lastError().driverText();
        } else if (query.next()) {
            currentVersion = query.value("data").toInt();
            qCInfo(dcUserManager()) << "Current database version is" << currentVersion;
            if (currentVersion == newVersion) {
                qCInfo(dcUserManager()) << "The database version is up to date";
            }
        }
    }

    if (!m_db.tables().contains("users")) {
        qCDebug(dcUserManager()) << "No \"users\" table found. Creating the table...";
        QSqlQuery query(m_db);
        if (!query.exec("CREATE TABLE users (username VARCHAR(40) UNIQUE PRIMARY KEY, email VARCHAR(40), displayName VARCHAR(40), password VARCHAR(100), salt VARCHAR(100), scopes TEXT, allowedThingIds TEXT);") || m_db.lastError().isValid()) {
            dumpDBError("Error initializing user database (table users).");
            m_db.close();
            return false;
        }
    } else {
        if (currentVersion < 1) {
            qCDebug(dcUserManager()) << "Start user table database migration to version 1";
            QSqlQuery query = QSqlQuery(m_db);
            if (!query.exec("ALTER TABLE users ADD COLUMN scopes TEXT;") || m_db.lastError().isValid()) {
                dumpDBError("Error migrating user database (table users).");
                m_db.close();
                return false;
            }

            // Migrated existing users from before multiuser support are admins by default
            query = QSqlQuery(m_db);
            query.prepare("UPDATE users SET scopes = ?;");
            query.addBindValue(Types::scopesToStringList(Types::PermissionScopeAdmin).join(','));

            if (!query.exec() || query.lastError().isValid()) {
                dumpDBError("Error migrating user database (updating existing users).");
                m_db.close();
                return false;
            }

            query = QSqlQuery(m_db);
            if (!query.exec("ALTER TABLE users ADD COLUMN email VARCHAR(40);") || m_db.lastError().isValid()) {
                dumpDBError("Error migrating user database (table users).");
                m_db.close();
                return false;
            }

            query = QSqlQuery(m_db);
            if (!query.exec("ALTER TABLE users ADD COLUMN displayName VARCHAR(40);") || m_db.lastError().isValid()) {
                dumpDBError("Error migrating user database (table users).");
                m_db.close();
                return false;
            }

            // Up until schema 1, username was an email. Copy it to initialize the email field.
            query = QSqlQuery(m_db);
            if (!query.exec("UPDATE users SET email = username;") || m_db.lastError().isValid()) {
                dumpDBError("Error migrating user database (table users).");
                m_db.close();
                return false;
            }

            qCDebug(dcUserManager()) << "Migrated successfully users table to database version 1";
        }

        if (currentVersion < 2) {
            // - Add new "allowedThingIds" row into the users table
            // - New permission has been added "PermissionScopeAccessAllThings", the existing users require
            //   all this permission in order to have an unchainged behavior
            qCDebug(dcUserManager()) << "Migrating user table to version 2";

            // - Add new "allowedThingIds" row into the users table, it remains is empty at this point
            QSqlQuery query = QSqlQuery(m_db);
            if (!query.exec("ALTER TABLE users ADD COLUMN allowedThingIds TEXT;") || m_db.lastError().isValid()) {
                dumpDBError("Error migrating user database (table users).");
                m_db.close();
                return false;
            }

            if (!m_db.transaction()) {
                dumpDBError("Error starting transaction for migrating user database (table users).");
                return false;
            }

            QSqlQuery selectQuery(m_db);
            if (!selectQuery.exec("SELECT username, scopes FROM users")) {
                dumpDBError("Select failed: " + selectQuery.lastError().text());
                return false;
            }

            QSqlQuery updateQuery(m_db);
            updateQuery.prepare("UPDATE users SET scopes = :scopes WHERE username = :username");
            while (selectQuery.next()) {
                QString username = selectQuery.value("username").toString();
                Types::PermissionScopes scopes = Types::scopesFromStringList(selectQuery.value("scopes").toString().split(','));

                // In case this is an admin, make sure we store only the Admin scope
                if (!scopes.testFlag(Types::PermissionScopeAdmin)) {
                    scopes.setFlag(Types::PermissionScopeAccessAllThings);
                }

                updateQuery.bindValue(":scopes", Types::scopesToStringList(scopes).join(','));
                updateQuery.bindValue(":username", username);

                if (!updateQuery.exec()) {
                    qCWarning(dcUserManager()) << "Update failed for username" << username << ":" << updateQuery.lastError().text();
                    m_db.rollback();
                    return false;
                }
            }

            if (!m_db.commit()) {
                dumpDBError("Error migrating user database (table users) to version 2. Rollback.");
                m_db.rollback();
                return false;
            }

            qCDebug(dcUserManager()) << "Migrated successfully users table to database version 2";
        }
    }

    if (!m_db.tables().contains("userInventory")) {
        qCDebug(dcUserManager()) << "No \"userInventory\" table found. Creating the table...";
        QSqlQuery query(m_db);
        if (!query.exec("CREATE TABLE userInventory (id VARCHAR(40) UNIQUE PRIMARY KEY, username VARCHAR(40), type VARCHAR(40), displayName VARCHAR(100), enabled INTEGER, payload TEXT);") || m_db.lastError().isValid()) {
            dumpDBError("Error initializing user database (table userInventory).");
            m_db.close();
            return false;
        }
    }

    if (!m_db.tables().contains("tokens")) {
        qCDebug(dcUserManager()) << "No \"tokens\" table found. Creating the table...";
        QSqlQuery query(m_db);
        if (!query.exec("CREATE TABLE tokens (id VARCHAR(40) UNIQUE, username VARCHAR(40), token VARCHAR(100) UNIQUE, creationdate DATETIME, devicename VARCHAR(40), expirydate DATETIME, lastseen DATETIME);") || m_db.lastError().isValid()) {
            dumpDBError("Error initializing user database (table tokens)");
            m_db.close();
            return false;
        }
    } else if (currentVersion < 4) {
        // NULL = never expires / not yet observed. Existing tokens are left with both
        // unset rather than backfilled from creationdate. Both columns are added in one
        // transaction so a failure never leaves the table with only one of them, which
        // would otherwise wedge every future migration attempt on a duplicate-column error.
        qCDebug(dcUserManager()) << "Migrating tokens table to database version 4";
        if (!m_db.transaction()) {
            dumpDBError("Error starting transaction for migrating user database (table tokens).");
            m_db.close();
            return false;
        }
        QSqlQuery query(m_db);
        if (!query.exec("ALTER TABLE tokens ADD COLUMN expirydate DATETIME;") || m_db.lastError().isValid()) {
            dumpDBError("Error migrating user database (table tokens, expirydate).");
            m_db.rollback();
            m_db.close();
            return false;
        }
        query = QSqlQuery(m_db);
        if (!query.exec("ALTER TABLE tokens ADD COLUMN lastseen DATETIME;") || m_db.lastError().isValid()) {
            dumpDBError("Error migrating user database (table tokens, lastseen).");
            m_db.rollback();
            m_db.close();
            return false;
        }
        if (!m_db.commit()) {
            dumpDBError("Error migrating user database (table tokens) to version 4. Rollback.");
            m_db.rollback();
            m_db.close();
            return false;
        }
        qCDebug(dcUserManager()) << "Migrated successfully tokens table to database version 4";
    }

    if (!m_db.tables().contains("invitations")) {
        qCDebug(dcUserManager()) << "No \"invitations\" table found. Creating the table...";
        QSqlQuery query(m_db);
        if (!query.exec("CREATE TABLE invitations (id VARCHAR(40) UNIQUE, username VARCHAR(40), tokenhash VARCHAR(100) UNIQUE, creationdate DATETIME, expirydate DATETIME NOT NULL, tokenvalidityduration INTEGER);") || m_db.lastError().isValid()) {
            dumpDBError("Error initializing user database (table invitations)");
            m_db.close();
            return false;
        }
    }

    if (!m_db.tables().contains("metadata")) {
        qCDebug(dcUserManager()) << "No \"metadata\" table found. Creating the table...";
        QSqlQuery query(m_db);
        if (!query.exec("CREATE TABLE metadata (key VARCHAR(10), data VARCHAR(40));") || m_db.lastError().isValid()) {
            dumpDBError("Error setting up user database (table metadata)!");
            m_db.close();
            return false;
        }

        query = QSqlQuery(m_db);
        query.prepare("INSERT INTO metadata (key, data) VALUES ('version', :version);");
        query.bindValue(":version", newVersion);
        if (!query.exec() || m_db.lastError().isValid()) {
            dumpDBError("Error setting up user database (setting version metadata)!");
            m_db.close();
            return false;
        }
    } else {
        // All migrations have been done
        if (currentVersion < newVersion) {
            QSqlQuery query(m_db);
            query.prepare("UPDATE metadata SET data = :version WHERE key = 'version'");
            query.bindValue(":version", newVersion);
            if (!query.exec() || m_db.lastError().isValid()) {
                dumpDBError("Error updating database version");
                m_db.close();
                return false;
            }
            qCInfo(dcUserManager()) << "Finished database migration to version" << newVersion;
        }
    }

    qCDebug(dcUserManager()) << "User database initialized successfully";
    return true;
}

void UserManager::rotate(const QString &dbName)
{
    int index = 1;
    while (QFileInfo::exists(QString("%1.%2").arg(dbName).arg(index)))
        index++;

    qCDebug(dcUserManager()) << "Backing up old database file to" << QString("%1.%2").arg(dbName).arg(index);
    QFile f(dbName);
    if (!f.rename(QString("%1.%2").arg(dbName).arg(index))) {
        qCWarning(dcUserManager()) << "Error backing up old database.";
    } else {
        qCDebug(dcUserManager()) << "Successfully moved old database";
    }
}

bool UserManager::validateUsername(const QString &username) const
{
    static QRegularExpression validator("[a-zA-Z0-9_\\.+-@]{3,}");
    return validator.match(username).hasMatch();
}

bool UserManager::validatePassword(const QString &password) const
{
    if (password.length() < 8)
        return false;

    static QRegularExpression lowerRe("[a-z]");
    if (!password.contains(lowerRe))
        return false;

    static QRegularExpression upperRe("[A-Z]");
    if (!password.contains(upperRe))
        return false;

    static QRegularExpression numbersRe("[0-9]");
    if (!password.contains(numbersRe))
        return false;

    return true;
}

bool UserManager::validateToken(const QByteArray &token) const
{
    static QRegularExpression validator(QRegularExpression("(^[a-zA-Z0-9_\\.+-/=]+$)"));
    return validator.match(token).hasMatch();
}

bool UserManager::isCanonicalInvitationToken(const QByteArray &token) const
{
    if (token.length() != 44)
        return false;
    if (token.count('=') != 1 || !token.endsWith('='))
        return false;

    QByteArray decoded = QByteArray::fromBase64(token, QByteArray::Base64Encoding | QByteArray::AbortOnBase64DecodingErrors);
    if (decoded.length() != 32)
        return false;

    // Reject non-canonical encodings a lenient decoder might still accept (e.g. non-zero
    // padding bits) by requiring an exact re-encode round trip.
    if (decoded.toBase64() != token)
        return false;

    return true;
}

bool UserManager::isValidInvitationDeviceName(const QString &deviceName) const
{
    QByteArray utf8 = deviceName.toUtf8();
    if (utf8.isEmpty() || utf8.length() > 40)
        return false;

    foreach (const QChar &character, deviceName) {
        if (character.category() == QChar::Other_Control || character.category() == QChar::Other_Format)
            return false;
    }

    return true;
}

bool UserManager::validateScopes(Types::PermissionScopes scopes) const
{
    if (scopes.testFlag(Types::PermissionScopeAdmin) || scopes == Types::PermissionScopeNone || scopes == Types::PermissionScopeControlThings)
        return true;

    if (scopes.testFlag(Types::PermissionScopeConfigureThings)) {
        if (!scopes.testFlag(Types::PermissionScopeControlThings) ||
            !scopes.testFlag(Types::PermissionScopeAccessAllThings)) {
            qCWarning(dcUserManager()) << "Invalid scopes combination. If a user can configure things he must have access to all things and must be able to control them.";
            return false;
        }
    }

    // Note: if access to all things, there are no restrictions
    if (!scopes.testFlag(Types::PermissionScopeAccessAllThings)) {
        if (scopes.testFlag(Types::PermissionScopeControlThings) ||
            scopes.testFlag(Types::PermissionScopeConfigureRules)||
            scopes.testFlag(Types::PermissionScopeExecuteRules)) {
            qCWarning(dcUserManager()) << "Invalid scopes combination. If a user has not access to all things, he can not configure them or create/execute rules.";
            return false;
        }
    }

    if (scopes.testFlag(Types::PermissionScopeExecuteRules)) {
        if (!scopes.testFlag(Types::PermissionScopeAccessAllThings)) {
            qCWarning(dcUserManager()) << "Invalid scopes combination. If a user can execute rules, he must have access to all things.";
            return false;
        }
    }

    if (scopes.testFlag(Types::PermissionScopeConfigureRules)) {
        if (!scopes.testFlag(Types::PermissionScopeAccessAllThings) ||
            !scopes.testFlag(Types::PermissionScopeExecuteRules)) {
            qCWarning(dcUserManager()) << "Invalid scopes combination. If a user can create rules, he must have access to all things and be able to execute them.";
            return false;
        }
    }

    return true;
}

bool UserManager::validateInventoryItem(const QString &type, const QVariantMap &payload) const
{
    if (type.isEmpty() || type.length() > 40)
        return false;

    const QByteArray serializedPayload = serializeInventoryPayload(payload);
    if ((serializedPayload.isEmpty() && !payload.isEmpty()) || serializedPayload.size() > 8192)
        return false;

    if (type == "rfidTag") {
        const QString tagHash = payload.value("tagHash").toString();
        if (tagHash.isEmpty() || !tagHash.startsWith("sha256:"))
            return false;

        if (payload.contains("code") || payload.contains("rawCode"))
            return false;

        if (payload.contains("profile") && payload.value("profile").type() != QVariant::Map)
            return false;
    }

    return true;
}

QByteArray UserManager::serializeInventoryPayload(const QVariantMap &payload) const
{
    QJsonDocument document = QJsonDocument::fromVariant(payload);
    if (!document.isObject())
        return QByteArray();

    return document.toJson(QJsonDocument::Compact);
}

QVariantMap UserManager::deserializeInventoryPayload(const QByteArray &payload) const
{
    if (payload.isEmpty())
        return QVariantMap();

    QJsonParseError error;
    QJsonDocument document = QJsonDocument::fromJson(payload, &error);
    if (error.error != QJsonParseError::NoError || !document.isObject()) {
        qCWarning(dcUserManager()) << "Unable to parse user inventory payload:" << error.errorString();
        return QVariantMap();
    }

    return document.toVariant().toMap();
}

bool UserManager::enabledInventoryItemExists(const QString &type, const QString &payloadKey, const QVariant &payloadValue, const QUuid &ignoredInventoryItemId) const
{
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM userInventory WHERE type = :type AND enabled = 1;");
    query.bindValue(":type", type);
    if (!query.exec()) {
        qCWarning(dcUserManager()) << "Unable to check duplicate user inventory item" << query.lastError().databaseText() << query.lastError().driverText();
        return false;
    }

    while (query.next()) {
        const QUuid id(query.value("id").toString());
        if (!ignoredInventoryItemId.isNull() && id == ignoredInventoryItemId)
            continue;

        const QVariantMap payload = deserializeInventoryPayload(query.value("payload").toByteArray());
        if (payload.value(payloadKey) == payloadValue)
            return true;
    }

    return false;
}

UserInventoryItem UserManager::inventoryItemFromQuery(const QSqlQuery &query) const
{
    UserInventoryItem item(QUuid(query.value("id").toString()));
    item.setUsername(query.value("username").toString());
    item.setType(query.value("type").toString());
    item.setDisplayName(query.value("displayName").toString());
    item.setEnabled(query.value("enabled").toBool());
    item.setPayload(deserializeInventoryPayload(query.value("payload").toByteArray()));
    return item;
}

/*! Parses a tokens.expirydate/lastseen column value read back from the database. An
    invalid/NULL \a value means "never expires"/"not yet observed" and yields an invalid
    QDateTime. A populated value is always UTC, regardless of the local timezone. */
QDateTime UserManager::parseStoredUtcDateTime(const QVariant &value) const
{
    if (value.isNull())
        return QDateTime();

    QDateTime dateTime = QDateTime::fromString(value.toString(), Qt::ISODate);
    dateTime.setTimeSpec(Qt::UTC);
    return dateTime;
}

/*! Formats \a value for storage in tokens.expirydate/lastseen, always as UTC ISO 8601.
    An invalid \a value must not be passed here; store NULL directly instead. */
QString UserManager::formatUtcDateTimeForStorage(const QDateTime &value) const
{
    return value.toUTC().toString(Qt::ISODate);
}

/*! The one authoritative answer to whether a token identified by its raw \a
    expiryDateValue column value is still valid right now. An absent/NULL value never
    expires. Every caller that needs to know if a token is still valid must go through
    this rather than comparing timestamps itself.

    Deliberately uses QDateTime::currentDateTimeUtc() rather than TimeManager: this is a
    stateless per-request comparison, not a scheduled/simulatable timer, and it must work
    from a standalone UserManager as well as from a fully initialized NymeaCore. */
bool UserManager::isTokenLogicallyExpired(const QVariant &expiryDateValue) const
{
    QDateTime expiryTime = parseStoredUtcDateTime(expiryDateValue);
    if (!expiryTime.isValid())
        return false;

    return QDateTime::currentDateTimeUtc() >= expiryTime;
}

void UserManager::purgeExpiredTokens()
{
    QSqlQuery selectQuery(m_db);
    if (!selectQuery.exec("SELECT id, token, expirydate FROM tokens WHERE expirydate IS NOT NULL;")) {
        qCWarning(dcUserManager()) << "Unable to query for expired tokens" << selectQuery.lastError().databaseText() << selectQuery.lastError().driverText();
        return;
    }

    QList<QPair<QUuid, QByteArray>> expired;
    while (selectQuery.next()) {
        if (isTokenLogicallyExpired(selectQuery.value("expirydate")))
            expired << qMakePair(QUuid(selectQuery.value("id").toString()), selectQuery.value("token").toString().toUtf8());
    }

    foreach (const auto &pair, expired) {
        const QUuid &tokenId = pair.first;

        if (!m_notifiedExpiredTokenIds.contains(tokenId)) {
            // Emitted immediately on first observation, independent of whether the
            // physical delete below succeeds.
            m_notifiedExpiredTokenIds.insert(tokenId);
            emit tokenInvalidated(pair.second);
        }

        QSqlQuery deleteQuery(m_db);
        deleteQuery.prepare("DELETE FROM tokens WHERE id = :id;");
        deleteQuery.bindValue(":id", tokenId.toString());
        if (deleteQuery.exec() && deleteQuery.numRowsAffected() == 1) {
            m_notifiedExpiredTokenIds.remove(tokenId);
        } else {
            qCWarning(dcUserManager()) << "Failed to purge expired token" << tokenId << "- will retry on the next check.";
        }
    }
}

void UserManager::rearmExpiryTimer()
{
    purgeExpiredTokens();

    QSqlQuery query(m_db);
    if (!query.exec("SELECT MIN(expirydate) AS nextExpiry FROM tokens WHERE expirydate IS NOT NULL;") || !query.first() || query.value("nextExpiry").isNull()) {
        m_expiryTimer->stop();
        return;
    }

    QDateTime nextExpiry = parseStoredUtcDateTime(query.value("nextExpiry"));
    qint64 msecsUntilExpiry = QDateTime::currentDateTimeUtc().msecsTo(nextExpiry);

    // Never pass a potentially overflowing interval to QTimer (its int parameter maxes
    // out around 24.8 days): re-check at least this often regardless of how far away the
    // actual deadline is, recalculating against current UTC time on every wake-up.
    constexpr qint64 maxIntervalMs = 24LL * 60 * 60 * 1000;
    qint64 boundedMsecs = qBound<qint64>(0, msecsUntilExpiry, maxIntervalMs);
    m_expiryTimer->start(static_cast<int>(boundedMsecs));
}

void UserManager::dumpDBError(const QString &message)
{
    qCCritical(dcUserManager) << message << "Driver error:" << m_db.lastError().driverText() << "Database error:" << m_db.lastError().databaseText();
}

void UserManager::evaluateAllowedThingsForUser()
{

}

void UserManager::onPushButtonPressed()
{
    if (m_pushButtonTransaction.first == -1) {
        qCDebug(dcUserManager()) << "PushButton pressed without a client waiting for it. Ignoring the signal.";
        return;
    }

    // Creating a user without username and password. It won't be able to log in via user/password
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM users WHERE username = \"\";");
    query.exec();
    if (!query.next()) {
        qCDebug(dcUserManager()) << "Creating token admin user";
        QSqlQuery query(m_db);
        query.prepare("INSERT INTO users(username, password, salt, scopes) values(?, ?, ?, ?);");
        query.addBindValue("");
        query.addBindValue("");
        query.addBindValue("");
        query.addBindValue(Types::scopeToString(Types::PermissionScopeAdmin));
        query.exec();
        if (query.lastError().type() != QSqlError::NoError) {
            qCWarning(dcUserManager) << "Error creating push button user:" << query.lastError().databaseText() << query.lastError().driverText();
        }
    }

    QByteArray token = QCryptographicHash::hash(QUuid::createUuid().toByteArray(), QCryptographicHash::Sha256).toBase64();
    QString storeTokenQueryString = QString("INSERT INTO tokens(id, username, token, creationdate, devicename) VALUES(\"%1\", \"%2\", \"%3\", \"%4\", \"%5\");")
                                        .arg(QUuid::createUuid().toString())
                                        .arg("")
                                        .arg(QString::fromUtf8(token))
                                        .arg(NymeaCore::instance()->timeManager()->currentDateTime().toString("yyyy-MM-dd hh:mm:ss"))
                                        .arg(m_pushButtonTransaction.second);

    QSqlQuery storeTokenQuery(m_db);
    if (!storeTokenQuery.exec(storeTokenQueryString) || m_db.lastError().type() != QSqlError::NoError) {
        qCWarning(dcUserManager()) << "Error storing token in DB:" << m_db.lastError().databaseText() << m_db.lastError().driverText();
        qCWarning(dcUserManager()) << "PushButton Auth failed.";
        emit pushButtonAuthFinished(m_pushButtonTransaction.first, false, QByteArray());
    } else {
        qCDebug(dcUserManager()) << "PushButton Auth succeeded.";
        emit pushButtonAuthFinished(m_pushButtonTransaction.first, true, token);
    }

    m_pushButtonTransaction.first = -1;
}

}
