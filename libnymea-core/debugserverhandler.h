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

#ifndef DEBUGSERVERHANDLER_H
#define DEBUGSERVERHANDLER_H

#include <QTimer>
#include <QObject>
#include <QProcess>
#include <QUrlQuery>
#include <QWebSocketServer>
#include <QMutex>

#include "debugreportgenerator.h"
#include "servers/httpreply.h"

namespace nymeaserver {

class DebugServerHandler : public QObject
{
    Q_OBJECT
public:
    explicit DebugServerHandler(QObject *parent = nullptr);

    HttpReply *processDebugRequest(const QString &requestPath, const QUrlQuery &requestQuery);

private:
    static QList<QWebSocket*> s_websocketClients;
    static void logMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &message);
    static QMutex s_loggingMutex;

    QWebSocketServer *m_websocketServer = nullptr;

    QProcess *m_pingProcess = nullptr;
    HttpReply *m_pingReply = nullptr;

    QProcess *m_digProcess = nullptr;
    HttpReply *m_digReply = nullptr;

    QProcess *m_tracePathProcess = nullptr;
    HttpReply *m_tracePathReply = nullptr;

    DebugReportGenerator *m_debugReportGenerator = nullptr;

    QByteArray loadResourceData(const QString &resourceFileName);
    QString getResourceFileName(const QString &requestPath);
    bool resourceFileExits(const QString &requestPath);

    HttpReply *processDebugFileRequest(const QString &requestPath);

    QByteArray createDebugXmlDocument();
    QByteArray createErrorXmlDocument(HttpReply::HttpStatusCode statusCode, const QString &errorMessage);

private slots:
    void onDebugServerEnabledChanged(bool enabled);

    void onWebsocketClientConnected();
    void onWebsocketClientDisconnected();
    void onWebsocketClientError(QAbstractSocket::SocketError error);

    void onPingProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onDigProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onTracePathProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onDebugReportGeneratorFinished(bool success);
    void onDebugReportGeneratorTimeout();
};

}

#endif // DEBUGSERVERHANDLER_H
