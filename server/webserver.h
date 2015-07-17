/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stuerz <simon.stuerz@guh.guru>                *
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

#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <QObject>
#include <QHash>
#include <QDir>

#include "transportinterface.h"

class QTcpServer;
class QTcpSocket;
class QUuid;

// Note: Status codes according to HTTP 1.1: https://tools.ietf.org/html/rfc7231

namespace guhserver {

class WebServer :  public TransportInterface
{
    Q_OBJECT
public:
    enum RequestMethod {
        Get,
        Post,
        Put,
        Delete,
        Unhandled
    };

    explicit WebServer(QObject *parent = 0);
    ~WebServer();

    void sendData(const QUuid &clientId, const QVariantMap &data) override;
    void sendData(const QList<QUuid> &clients, const QVariantMap &data) override;

private:
    QTcpServer *m_server;

    QHash<QUuid, QTcpSocket *> m_clientList;
    bool m_enabled;
    qint16 m_port;
    QDir m_webinterfaceDir;

    bool verifyFile(QTcpSocket *socket, const QString &fileName);
    QString fileName(const QString &query);
    RequestMethod getRequestMethodType(const QString &methodString);

    void writeData(QTcpSocket *socket, const QByteArray &data);

private slots:
    void onNewConnection();
    void readClient();
    void discardClient();

public slots:
    bool startServer() override;
    bool stopServer() override;

};

}

#endif // WEBSERVER_H
