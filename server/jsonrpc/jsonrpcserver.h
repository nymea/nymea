/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
 *  Copyright (C) 2014-2017 Michael Zanetti <michael.zanetti@guh.io>       *
 *                                                                         *
 *  This file is part of guh.                                              *
 *                                                                         *
 *  Guh is free software: you can redistribute it and/or modify            *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  Guh is distributed in the hope that it will be useful,                 *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with guh. If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef JSONRPCSERVER_H
#define JSONRPCSERVER_H

#include "plugin/deviceclass.h"
#include "jsonhandler.h"
#include "transportinterface.h"
#include "usermanager.h"

#include "types/action.h"
#include "types/event.h"

#include <QObject>
#include <QVariantMap>
#include <QString>

class Device;
class QSslConfiguration;

namespace guhserver {

class JsonRPCServer: public JsonHandler
{
    Q_OBJECT
public:
    JsonRPCServer(const QSslConfiguration &sslConfiguration = QSslConfiguration(), QObject *parent = 0);

    // JsonHandler API implementation
    QString name() const;
    Q_INVOKABLE JsonReply *Introspect(const QVariantMap &params) const;
    Q_INVOKABLE JsonReply *Version(const QVariantMap &params) const;
    Q_INVOKABLE JsonReply *SetNotificationStatus(const QVariantMap &params);

    Q_INVOKABLE JsonReply *CreateUser(const QVariantMap &params);
    Q_INVOKABLE JsonReply *Authenticate(const QVariantMap &params);

    QHash<QString, JsonHandler *> handlers() const;

    void registerTransportInterface(TransportInterface *interface, const bool &enabled = true);

private:
    void sendResponse(TransportInterface *interface, const QUuid &clientId, int commandId, const QVariantMap &params = QVariantMap());
    void sendErrorResponse(TransportInterface *interface, const QUuid &clientId, int commandId, const QString &error);
    void sendUnauthorizedResponse(TransportInterface *interface, const QUuid &clientId, int commandId, const QString &error);

private slots:
    void setup();

    void clientConnected(const QUuid &clientId);
    void clientDisconnected(const QUuid &clientId);

    void processData(const QUuid &clientId, const QByteArray &data);

    void sendNotification(const QVariantMap &params);

    void asyncReplyFinished();

private:
    QList<TransportInterface *> m_interfaces;
    QHash<QString, JsonHandler *> m_handlers;
    QHash<JsonReply *, TransportInterface *> m_asyncReplies;

    // clientId, notificationsEnabled
    QHash<QUuid, bool> m_clients;

    int m_notificationId;

    void registerHandler(JsonHandler *handler);
    QString formatAssertion(const QString &targetNamespace, const QString &method, JsonHandler *handler, const QVariantMap &data) const;
};

}

#endif // JSONRPCSERVER_H
