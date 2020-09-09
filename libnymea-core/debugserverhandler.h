/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2020, nymea GmbH
* Contact: contact@nymea.io
*
* This file is part of nymea.
* This project including source code and documentation is protected by
* copyright law, and remains the property of nymea GmbH. All rights, including
* reproduction, publication, editing and translation, are reserved. The use of
* this project is subject to the terms of a license agreement to be concluded
* with nymea GmbH in accordance with the terms of use of nymea GmbH, available
* under https://nymea.io/license
*
* GNU General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU General Public License as published by the Free Software
* Foundation, GNU version 3. This project is distributed in the hope that it
* will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
* of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
* Public License for more details.
*
* You should have received a copy of the GNU General Public License along with
* this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
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
