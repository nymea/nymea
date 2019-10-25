/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
 *  Copyright (C) 2014-2017 Michael Zanetti <michael.zanetti@guh.io>       *
 *                                                                         *
 *  This file is part of nymea.                                            *
 *                                                                         *
 *  nymea is free software: you can redistribute it and/or modify          *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  nymea is distributed in the hope that it will be useful,               *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with nymea. If not, see <http://www.gnu.org/licenses/>.          *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef JSONRPCSERVERIMPLEMENTATION_H
#define JSONRPCSERVERIMPLEMENTATION_H

#include "jsonrpc/jsonrpcserver.h"
#include "jsonrpc/jsonhandler.h"
#include "transportinterface.h"
#include "usermanager/usermanager.h"

#include "types/deviceclass.h"
#include "types/action.h"
#include "types/event.h"

#include <QObject>
#include <QVariantMap>
#include <QString>
#include <QSslConfiguration>

class Device;

namespace nymeaserver {

class JsonRPCServerImplementation: public JsonHandler, public JsonRPCServer
{
    Q_OBJECT
public:
    JsonRPCServerImplementation(const QSslConfiguration &sslConfiguration = QSslConfiguration(), QObject *parent = nullptr);

    // JsonHandler API implementation
    QString name() const;
    Q_INVOKABLE JsonReply *Hello(const QVariantMap &params);
    Q_INVOKABLE JsonReply *Introspect(const QVariantMap &params) const;
    Q_INVOKABLE JsonReply *Version(const QVariantMap &params) const;
    Q_INVOKABLE JsonReply *SetNotificationStatus(const QVariantMap &params);

    Q_INVOKABLE JsonReply *CreateUser(const QVariantMap &params);
    Q_INVOKABLE JsonReply *Authenticate(const QVariantMap &params);
    Q_INVOKABLE JsonReply *RequestPushButtonAuth(const QVariantMap &params);
    Q_INVOKABLE JsonReply *Tokens(const QVariantMap &params) const;
    Q_INVOKABLE JsonReply *RemoveToken(const QVariantMap &params);
    Q_INVOKABLE JsonReply *SetupCloudConnection(const QVariantMap &params);
    Q_INVOKABLE JsonReply *SetupRemoteAccess(const QVariantMap &params);
    Q_INVOKABLE JsonReply *IsCloudConnected(const QVariantMap &params);
    Q_INVOKABLE JsonReply *KeepAlive(const QVariantMap &params);

signals:
    void CloudConnectedChanged(const QVariantMap &map);
    void PushButtonAuthFinished(const QVariantMap &params);

    // Server API
public:
    void registerTransportInterface(TransportInterface *interface, bool authenticationRequired);
    void unregisterTransportInterface(TransportInterface *interface);

    bool registerHandler(JsonHandler *handler) override;

private:
    QHash<QString, JsonHandler *> handlers() const;

    void sendResponse(TransportInterface *interface, const QUuid &clientId, int commandId, const QVariantMap &params = QVariantMap());
    void sendErrorResponse(TransportInterface *interface, const QUuid &clientId, int commandId, const QString &error);
    void sendUnauthorizedResponse(TransportInterface *interface, const QUuid &clientId, int commandId, const QString &error);
    QVariantMap createWelcomeMessage(TransportInterface *interface, const QUuid &clientId) const;

    void processJsonPacket(TransportInterface *interface, const QUuid &clientId, const QByteArray &data);

private slots:
    void setup();

    void clientConnected(const QUuid &clientId);
    void clientDisconnected(const QUuid &clientId);

    void processData(const QUuid &clientId, const QByteArray &data);

    void sendNotification(const QVariantMap &params);

    void asyncReplyFinished();

    void pairingFinished(QString cognitoUserId, int status, const QString &message);
    void onCloudConnectionStateChanged();
    void onPushButtonAuthFinished(int transactionId, bool success, const QByteArray &token);

private:
    QVariantMap m_api;
    QMap<TransportInterface*, bool> m_interfaces; // Interface, authenticationRequired
    QHash<QString, JsonHandler *> m_handlers;
    QHash<JsonReply *, TransportInterface *> m_asyncReplies;

    QHash<QUuid, TransportInterface*> m_clientTransports;
    QHash<QUuid, QByteArray> m_clientBuffers;
    QHash<QUuid, QStringList> m_clientNotifications;
    QHash<QUuid, QLocale> m_clientLocales;
    QHash<int, QUuid> m_pushButtonTransactions;
    QHash<QUuid, QTimer*> m_newConnectionWaitTimers;

    QHash<QString, JsonReply*> m_pairingRequests;

    int m_notificationId;

    QString formatAssertion(const QString &targetNamespace, const QString &method, QMetaMethod::MethodType methodType, JsonHandler *handler, const QVariantMap &data) const;
};

}

#endif // JSONRPCSERVERIMPLEMENTATION_H

