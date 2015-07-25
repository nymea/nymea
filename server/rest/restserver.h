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

#ifndef RESTSERVER_H
#define RESTSERVER_H

#include <QObject>

#include "webserver.h"
#include "jsonhandler.h"

class HttpRequest;
class HttpReply;

namespace guhserver {

class RestServer : public QObject
{
    Q_OBJECT
public:
    explicit RestServer(QObject *parent = 0);

private:
    WebServer *m_webserver;
    QList<QUuid> m_clientList;
    QHash<QUuid, JsonReply *> m_asyncReplies;

signals:
    void httpReplyReady(const HttpReply &httpReply);

private slots:
    void clientConnected(const QUuid &clientId);
    void clientDisconnected(const QUuid &clientId);
    
    void processHttpRequest(const QUuid &clientId, const HttpRequest &request);
    void asyncReplyFinished();

};

}

#endif // RESTSERVER_H
