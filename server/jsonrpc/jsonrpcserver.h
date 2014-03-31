/****************************************************************************
 *                                                                          *
 *  This file is part of guh.                                               *
 *                                                                          *
 *  Guh is free software: you can redistribute it and/or modify             *
 *  it under the terms of the GNU General Public License as published by    *
 *  the Free Software Foundation, version 2 of the License.                 *
 *                                                                          *
 *  Guh is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *  GNU General Public License for more details.                            *
 *                                                                          *
 *  You should have received a copy of the GNU General Public License       *
 *  along with guh.  If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                          *
 ***************************************************************************/

#ifndef JSONRPCSERVER_H
#define JSONRPCSERVER_H

#include "deviceclass.h"
#include "action.h"
#include "event.h"
#include "jsonhandler.h"

#include <QObject>
#include <QVariantMap>
#include <QString>

#ifdef TESTING_ENABLED
class MockTcpServer;
#else
class TcpServer;
#endif
class Device;

class JsonRPCServer: public JsonHandler
{
    Q_OBJECT
public:
    JsonRPCServer(QObject *parent = 0);

    // JsonHandler API implementation
    QString name() const;
    Q_INVOKABLE QVariantMap Introspect(const QVariantMap &params) const;
    Q_INVOKABLE QVariantMap Version(const QVariantMap &params) const;
    Q_INVOKABLE QVariantMap SetNotificationStatus(const QVariantMap &params);

signals:
    void commandReceived(const QString &targetNamespace, const QString &command, const QVariantMap &params);

private slots:
    void clientConnected(const QUuid &clientId);
    void clientDisconnected(const QUuid &clientId);

    void processData(const QUuid &clientId, const QByteArray &jsonData);

private:
    void registerHandler(JsonHandler *handler);

    void sendResponse(const QUuid &clientId, int commandId, const QVariantMap &params = QVariantMap());
    void sendErrorResponse(const QUuid &clientId, int commandId, const QString &error);

private:
#ifdef TESTING_ENABLED
    MockTcpServer *m_tcpServer;
#else
    TcpServer *m_tcpServer;
#endif
    QHash<QString, JsonHandler*> m_handlers;

    // clientId, notificationsEnabled
    QHash<QUuid, bool> m_clients;
};

#endif
