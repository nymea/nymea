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

#ifndef JSONRPCSERVERIMPLEMENTATION_H
#define JSONRPCSERVERIMPLEMENTATION_H

#include "jsonrpc/jsonrpcserver.h"
#include "jsonrpc/jsonhandler.h"
#include "usermanager/userinfo.h"
#include "usermanager/invitationinfo.h"
#include "transportinterface.h"

#include <QObject>
#include <QVariantMap>
#include <QString>
#include <QSet>
#include <QSslConfiguration>

class Thing;

namespace nymeaserver {

class JsonRPCServerImplementation: public JsonHandler, public JsonRPCServer
{
    Q_OBJECT
public:
    JsonRPCServerImplementation(const QSslConfiguration &sslConfiguration = QSslConfiguration(), QObject *parent = nullptr);

    // JsonHandler API implementation
    QString name() const override;
    Q_INVOKABLE JsonReply *Hello(const QVariantMap &params, const JsonContext &context);
    Q_INVOKABLE JsonReply *Introspect(const QVariantMap &params) const;
    Q_INVOKABLE JsonReply *Version(const QVariantMap &params) const;
    Q_INVOKABLE JsonReply *SetNotificationStatus(const QVariantMap &params, const JsonContext &context);

    Q_INVOKABLE JsonReply *CreateUser(const QVariantMap &params);
    Q_INVOKABLE JsonReply *Authenticate(const QVariantMap &params, const JsonContext &context);
    Q_INVOKABLE JsonReply *AuthenticateWithToken(const QVariantMap &params, const JsonContext &context);
    Q_INVOKABLE JsonReply *RequestPushButtonAuth(const QVariantMap &params, const JsonContext &context);
    Q_INVOKABLE JsonReply *KeepAlive(const QVariantMap &params);

signals:
    void PushButtonAuthFinished(const QUuid &clientId, const QVariantMap &params);

    // Server API
public:
    void registerTransportInterface(TransportInterface *interface);
    void unregisterTransportInterface(TransportInterface *interface);

    bool registerHandler(JsonHandler *handler) override;
    bool registerExperienceHandler(JsonHandler *handler, int majorVersion, int minorVersion) override;

private:
    QHash<QString, JsonHandler *> handlers() const;

    void sendResponse(TransportInterface *interface, const QUuid &clientId, int commandId, const QVariantMap &params = QVariantMap(), const QString &deprecationWarning = QString());
    void sendErrorResponse(TransportInterface *interface, const QUuid &clientId, int commandId, const QString &error);
    void sendUnauthorizedResponse(TransportInterface *interface, const QUuid &clientId, int commandId, const QString &error);

    // Centralizes persistent-token last-seen marking for Hello, Authenticate and
    // push-button completion. A no-op for an empty/invalid/expired token. Marks a given
    // token id at most once per client connection, even across A->B->A token switches.
    void markTokenSeenIfNewlyBound(const QUuid &clientId, const QByteArray &token);

    // Masks a top-level params.token value (the only field name ever used for a bearer
    // token - regular client tokens and one-time invitation secrets alike) before a raw
    // JSON payload is logged. Returns data unchanged if it doesn't parse as a JSON object.
    // Callers should guard with the relevant category's isDebugEnabled() first so the
    // parse/reserialize cost is only paid when the log line would actually be emitted.
    QByteArray redactSensitiveFields(const QByteArray &data) const;

    void processJsonPacket(TransportInterface *interface, const QUuid &clientId, const QByteArray &data);

private slots:
    void setup();

    void clientConnected(const QUuid &clientId);
    void clientDisconnected(const QUuid &clientId);

    void processData(const QUuid &clientId, const QByteArray &data);

    void sendNotification(const QVariantMap &params);
    void sendClientNotification(const QUuid &clientId, const QVariantMap &params);
    void sendClientNotification(const QVariantMap &params, const ThingId &thingId);
    void sendClientNotification(const QVariantMap &params, const nymeaserver::UserInfo &userInfo);

    void asyncReplyFinished();

    void onPushButtonAuthFinished(int transactionId, bool success, const QByteArray &token);

    // Disconnects every live connection authenticated with a token that was just
    // revoked or has logically expired. sendNotification() performs no token re-check,
    // so without this a revoked client's notification stream keeps flowing until it
    // disconnects on its own.
    void onTokenInvalidated(const QByteArray &token);

    // UsersHandler owns packing/emitting InvitationAdded/InvitationRemoved (so the
    // notification keeps the "Users." namespace), but has no client-token/transport
    // state of its own; these resolve the Admin-eligible recipients and drive one call
    // to UsersHandler per eligible client.
    void onInvitationAdded(const nymeaserver::InvitationInfo &invitation);
    void onInvitationRemoved(const QUuid &invitationId);

private:
    // Clients currently subscribed to the Users namespace whose bound token resolves,
    // through the same authoritative validity path used for authenticated requests, to a
    // user with PermissionScopeAdmin. Unauthenticated, non-admin, expired/revoked, and
    // authentication-disabled tokenless clients are never included even if subscribed.
    QList<QUuid> adminEligibleClientIds() const;

private:
    QVariantMap m_api;
    QHash<JsonHandler *, QString> m_experiences;
    QHash<QString, JsonHandler *> m_handlers;
    QHash<JsonReply *, TransportInterface *> m_asyncReplies;

    QHash<QUuid, TransportInterface *> m_clientTransports;
    QHash<QUuid, QByteArray> m_clientBuffers;
    QHash<QUuid, QStringList> m_clientNotifications;
    QHash<QUuid, QLocale> m_clientLocales;
    QHash<QUuid, QByteArray> m_clientTokens;
    // Token ids already marked as seen on each client connection, so repeated Hello
    // and ordinary requests never issue a second UPDATE for the same (client, token).
    QHash<QUuid, QSet<QUuid>> m_seenTokenIdsByClient;
    QHash<int, QUuid> m_pushButtonTransactions;
    QHash<QUuid, QTimer *> m_newConnectionWaitTimers;

    QHash<QString, JsonReply *> m_pairingRequests;

    int m_notificationId;

    QTimer m_connectionLockdownTimer;

    QString formatAssertion(const QString &targetNamespace, const QString &method, QMetaMethod::MethodType methodType, JsonHandler *handler, const QVariantMap &data) const;
};

}

#endif // JSONRPCSERVERIMPLEMENTATION_H

