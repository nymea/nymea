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

#ifndef USERMANAGER_H
#define USERMANAGER_H

#include "tokeninfo.h"
#include "invitationinfo.h"
#include "userinfo.h"
#include "userinventoryitem.h"

#include <QObject>
#include <QSqlDatabase>
#include <QTimer>
#include <QSet>

class QSqlQuery;

namespace nymeaserver {

class PushButtonDBusService;

class UserManager : public QObject
{
    Q_OBJECT
public:
    enum UserError {
        UserErrorNoError,
        UserErrorBackendError,
        UserErrorInvalidUserId,
        UserErrorDuplicateUserId,
        UserErrorBadPassword,
        UserErrorTokenNotFound,
        UserErrorPermissionDenied,
        UserErrorInconsistantScopes,
        UserErrorInventoryItemNotFound,
        UserErrorDuplicateInventoryItem,
        UserErrorInvalidInventoryItem,
        UserErrorInvitationNotFound,
        UserErrorInvalidInvitationDuration,
        UserErrorInvitationsDisabled
    };
    Q_ENUM(UserError)

    explicit UserManager(const QString &dbName, QObject *parent = nullptr);

    // True if the database could not be opened/migrated and existing user data was
    // therefore left untouched instead of being rotated away. A UserManager in this state
    // must not be used to serve any request.
    bool initializationFailed() const;

    bool initRequired() const;
    UserInfoList users() const;

    UserError createUser(const QString &username, const QString &password, const QString &email, const QString &displayName, Types::PermissionScopes scopes, const QList<ThingId> &allowedThingIds = QList<ThingId>());
    UserError changePassword(const QString &username, const QString &newPassword);
    UserError removeUser(const QString &username);
    UserError setUserScopes(const QString &username, Types::PermissionScopes scopes, const QList<ThingId> &allowedThingIds = QList<ThingId>());
    UserError setUserInfo(const QString &username, const QString &email, const QString &displayName);

    bool pushButtonAuthAvailable() const;

    QByteArray authenticate(const QString &username, const QString &password, const QString &deviceName);
    int requestPushButtonAuth(const QString &deviceName);
    void cancelPushButtonAuth(int transactionId);

    UserInfo userInfo(const QString &username = QString()) const;
    TokenInfo tokenInfo(const QByteArray &token) const;
    TokenInfo tokenInfo(const QUuid &tokenId) const;
    QList<TokenInfo> tokens(const QString &username) const;
    UserInventoryItems userInventoryItems(const QString &username = QString(), const QString &type = QString()) const;
    UserInventoryItem userInventoryItem(const QUuid &inventoryItemId) const;
    UserInventoryItem findEnabledUserInventoryItem(const QString &type, const QString &payloadKey, const QVariant &payloadValue) const;

    UserError removeToken(const QUuid &tokenId);
    // Diagnostic only: callers keep an already validated connection authenticated even if
    // this returns false. Never pass the clear token value here, only its id.
    bool markTokenSeen(const QUuid &tokenId, const QDateTime &timestamp);
    UserError addUserInventoryItem(const QString &username, const QString &type, const QString &displayName, const QVariantMap &payload, bool enabled = true);
    UserError updateUserInventoryItem(const QUuid &inventoryItemId, const QString &displayName, const QVariantMap &payload, bool enabled);
    UserError removeUserInventoryItem(const QUuid &inventoryItemId);

    // validitySeconds and, when hasTokenValidity, tokenValiditySeconds must each be in
    // 1..2592000 (30 days) or UserErrorInvalidInvitationDuration is returned. oneTimeToken
    // and info are only populated on UserErrorNoError; the clear token is never stored,
    // only its hash.
    UserError createInvitation(const QString &username, uint validitySeconds,
                               bool hasTokenValidity, uint tokenValiditySeconds,
                               QByteArray &oneTimeToken, InvitationInfo &info);

    bool verifyToken(const QByteArray &token);

    bool hasRestrictedThingAccess(const QByteArray &token) const;
    bool accessToThingGranted(const ThingId &thingId, const QByteArray &token);
    bool accessToThingGranted(const ThingId &thingId, const QString &username) const;
    QList<ThingId> getAllowedThingIdsForToken(const QByteArray &token) const;

public slots:
    void onThingRemoved(const ThingId &thingId);

signals:
    void userAdded(const QString &username);
    void userRemoved(const QString &username);
    void userChanged(const QString &username);
    void pushButtonAuthFinished(int transactionId, bool success, const QByteArray &token);

    void userThingRestrictionsChanged(const nymeaserver::UserInfo &userInfo, const ThingId &thingId, bool accessGranted);

    // Emitted for a token exactly once per revocation/expiry, only after the removal has
    // committed. Live connections authenticated with this token must be disconnected;
    // without this, a revoked client's notification stream would otherwise keep flowing
    // until it disconnects on its own.
    void tokenInvalidated(const QByteArray &token);

    void invitationAdded(const nymeaserver::InvitationInfo &invitation);
    void invitationRemoved(const QUuid &invitationId);

private:
    bool initDB();
    void rotate(const QString &dbName);
    bool validateUsername(const QString &username) const;
    bool validatePassword(const QString &password) const;
    bool validateToken(const QByteArray &token) const;
    bool validateScopes(Types::PermissionScopes scopes) const;
    bool validateInventoryItem(const QString &type, const QVariantMap &payload) const;
    QByteArray serializeInventoryPayload(const QVariantMap &payload) const;
    QVariantMap deserializeInventoryPayload(const QByteArray &payload) const;
    bool enabledInventoryItemExists(const QString &type, const QString &payloadKey, const QVariant &payloadValue, const QUuid &ignoredInventoryItemId = QUuid()) const;
    UserInventoryItem inventoryItemFromQuery(const QSqlQuery &query) const;

    // Authoritative UTC handling for tokens.expirydate/lastseen, kept separate from the
    // existing timezone-less "yyyy-MM-dd hh:mm:ss" convention used by creationdate.
    QDateTime parseStoredUtcDateTime(const QVariant &value) const;
    QString formatUtcDateTimeForStorage(const QDateTime &value) const;
    // The one authoritative answer to "is this token still valid right now": absent
    // expirydate never expires; every other caller must go through this rather than
    // comparing timestamps itself.
    bool isTokenLogicallyExpired(const QVariant &expiryDateValue) const;

    // Identifies every now-expired token, emits tokenInvalidated() at most once per
    // token id (deduplicated across repeated purge attempts), and retries physical
    // deletion for rows that failed to delete on a previous pass.
    void purgeExpiredTokens();

    void dumpDBError(const QString &message);

    void evaluateAllowedThingsForUser();

private slots:
    void onPushButtonPressed();

    // Purges now-expired tokens, then (re-)arms m_expiryTimer for the nearest remaining
    // tokens.expirydate. Called after startup, after removeToken()/removeUser(), on the
    // timer's own fire, and on TimeManager::dateTimeChanged (wall-clock corrections).
    void rearmExpiryTimer();

private:
    QSqlDatabase m_db;
    bool m_hadUsersTableBeforeInit = false;
    bool m_initializationFailed = false;
    QTimer *m_expiryTimer = nullptr;
    // Token ids already reported via tokenInvalidated() while their physical deletion is
    // still being retried, so a persistently failing delete never re-emits the signal.
    QSet<QUuid> m_notifiedExpiredTokenIds;
    PushButtonDBusService *m_pushButtonDBusService = nullptr;
    int m_pushButtonTransactionIdCounter = 0;
    QPair<int, QString> m_pushButtonTransaction;

};

}

Q_DECLARE_METATYPE(nymeaserver::UserManager::UserError)

#endif // USERMANAGER_H
