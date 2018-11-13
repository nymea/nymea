/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2018 Simon Stürz <simon.stuerz@guh.io>                   *
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

#ifndef DEBUGSERVERHANDLER_H
#define DEBUGSERVERHANDLER_H

#include <QTimer>
#include <QObject>
#include <QProcess>
#include <QWebSocketServer>

#include "httpreply.h"

namespace nymeaserver {

class DebugServerHandler : public QObject
{
    Q_OBJECT
public:
    explicit DebugServerHandler(QObject *parent = nullptr);

    HttpReply *processDebugRequest(const QString &requestPath);

private:
    static QtMessageHandler s_oldLogMessageHandler;
    static QList<QWebSocket*> s_websocketClients;
    static void logMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &message);

    QTimer *m_timer = nullptr;
    QWebSocketServer *m_websocketServer = nullptr;

    QProcess *m_pingProcess = nullptr;
    HttpReply *m_pingReply = nullptr;

    QProcess *m_digProcess = nullptr;
    HttpReply *m_digReply = nullptr;

    QProcess *m_tracePathProcess = nullptr;
    HttpReply *m_tracePathReply = nullptr;

    QByteArray loadResourceData(const QString &resourceFileName);
    QString getResourceFileName(const QString &requestPath);
    bool resourceFileExits(const QString &requestPath);

    HttpReply *processDebugFileRequest(const QString &requestPath);

    QByteArray createDebugXmlDocument();
    QByteArray createErrorXmlDocument(HttpReply::HttpStatusCode statusCode, const QString &errorMessage);

private slots:
    void onTimeout();
    void onWebsocketClientConnected();
    void onWebsocketClientDisconnected();
    void onWebsocketClientError(QAbstractSocket::SocketError error);

    void onPingProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onDigProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onTracePathProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

};

}

#endif // DEBUGSERVERHANDLER_H
