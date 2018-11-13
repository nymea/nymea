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

#include "nymeacore.h"
#include "httpreply.h"
#include "nymeasettings.h"
#include "httprequest.h"
#include "loggingcategories.h"
#include "debugserverhandler.h"
#include "stdio.h"

#include <QXmlStreamWriter>
#include <QCoreApplication>
#include <QMessageLogger>


QtMessageHandler DebugServerHandler::s_oldLogMessageHandler = nullptr;
QList<QWebSocket*> DebugServerHandler::s_websocketClients;

namespace nymeaserver {

DebugServerHandler::DebugServerHandler(QObject *parent) :
    QObject(parent)
{
    m_websocketServer = new QWebSocketServer("Debug server", QWebSocketServer::NonSecureMode, this);
    connect(m_websocketServer, &QWebSocketServer::newConnection, this, &DebugServerHandler::onWebsocketClientConnected);

    // FIXME: enable disable server with debug server
    if (!m_websocketServer->listen(QHostAddress::Any, 2626)) {
        qCWarning(dcWebServer()) << "DebugServer: The debug server websocket interface could not listen on" << m_websocketServer->serverUrl().toString();
    }
    qCDebug(dcWebServer()) << "DebugServer: Started debug server websocket interface on" << m_websocketServer->serverUrl().toString();

    m_timer = new QTimer(this);
    m_timer->setSingleShot(false);
    m_timer->setInterval(1000);
    connect(m_timer, &QTimer::timeout, this, &DebugServerHandler::onTimeout);

    //m_timer->start();

    s_oldLogMessageHandler = qInstallMessageHandler(&logMessageHandler);
}

HttpReply *DebugServerHandler::processDebugRequest(const QString &requestPath)
{
    qCDebug(dcWebServer()) << "DebugServer: Debug request for" << requestPath;

    // Check if debug page request
    if (requestPath == "/debug" || requestPath == "/debug/") {
        qCDebug(dcWebServer()) << "DebugServer: Create debug interface page";
        // Fallback default debug page
        HttpReply *reply = RestResource::createSuccessReply();
        reply->setHeader(HttpReply::ContentTypeHeader, "text/html");
        reply->setPayload(createDebugXmlDocument());
        return reply;
    }

    // Check if this is a logdb requested
    if (requestPath.startsWith("/debug/logdb.sql")) {
        qCDebug(dcWebServer()) << "DebugServer: Loading" << NymeaCore::instance()->configuration()->logDBName();
        QFile logDatabaseFile(NymeaCore::instance()->configuration()->logDBName());
        if (!logDatabaseFile.exists()) {
            qCWarning(dcWebServer()) << "DebugServer: Could not read log database file for debug download" << NymeaCore::instance()->configuration()->logDBName() << "file does not exist.";
            HttpReply *reply = RestResource::createErrorReply(HttpReply::NotFound);
            reply->setHeader(HttpReply::ContentTypeHeader, "text/html");
            //: The HTTP error message of the debug interface. The %1 represents the file name.
            reply->setPayload(createErrorXmlDocument(HttpReply::NotFound, tr("Could not find file \"%1\".").arg(logDatabaseFile.fileName())));
            return reply;
        }

        if (!logDatabaseFile.open(QFile::ReadOnly)) {
            qCWarning(dcWebServer()) << "DebugServer: Could not read log database file for debug download" << NymeaCore::instance()->configuration()->logDBName();
            HttpReply *reply = RestResource::createErrorReply(HttpReply::Forbidden);
            reply->setHeader(HttpReply::ContentTypeHeader, "text/html");
            //: The HTTP error message of the debug interface. The %1 represents the file name.
            reply->setPayload(createErrorXmlDocument(HttpReply::NotFound, tr("Could not open file \"%1\".").arg(logDatabaseFile.fileName())));
            return reply;
        }

        QByteArray logDatabaseRawData = logDatabaseFile.readAll();
        logDatabaseFile.close();

        HttpReply *reply = RestResource::createSuccessReply();
        reply->setHeader(HttpReply::ContentTypeHeader, "application/sql");
        reply->setPayload(logDatabaseRawData);
        return reply;
    }


    // Check if this is a syslog requested
    if (requestPath.startsWith("/debug/syslog")) {
        QString syslogFileName = "/var/log/syslog";
        qCDebug(dcWebServer()) << "DebugServer: Loading" << syslogFileName;
        QFile syslogFile(syslogFileName);
        if (!syslogFile.exists()) {
            qCWarning(dcWebServer()) << "DebugServer: Could not read log database file for debug download" << syslogFileName << "file does not exist.";
            HttpReply *reply = RestResource::createErrorReply(HttpReply::NotFound);
            reply->setHeader(HttpReply::ContentTypeHeader, "text/html");
            reply->setPayload(createErrorXmlDocument(HttpReply::NotFound, tr("Could not find file \"%1\".").arg(syslogFileName)));
            return reply;
        }

        if (!syslogFile.open(QFile::ReadOnly)) {
            qCWarning(dcWebServer()) << "DebugServer: Could not read syslog file for debug download" << syslogFileName;
            HttpReply *reply = RestResource::createErrorReply(HttpReply::Forbidden);
            reply->setHeader(HttpReply::ContentTypeHeader, "text/html");
            reply->setPayload(createErrorXmlDocument(HttpReply::NotFound, tr("Could not open file \"%1\".").arg(syslogFileName)));
            return reply;
        }

        QByteArray syslogFileData = syslogFile.readAll();
        syslogFile.close();

        HttpReply *reply = RestResource::createSuccessReply();
        reply->setHeader(HttpReply::ContentTypeHeader, "text/plain");
        reply->setPayload(syslogFileData);
        return reply;
    }

    // Check if this is a settings request
    if (requestPath.startsWith("/debug/settings")) {
        if (requestPath.startsWith("/debug/settings/devices")) {
            QString settingsFileName = NymeaSettings(NymeaSettings::SettingsRoleDevices).fileName();
            qCDebug(dcWebServer()) << "DebugServer: Loading" << settingsFileName;
            QFile settingsFile(settingsFileName);
            if (!settingsFile.exists()) {
                qCWarning(dcWebServer()) << "DebugServer: Could not read file for debug download" << settingsFileName << "file does not exist.";
                HttpReply *reply = RestResource::createErrorReply(HttpReply::NotFound);
                reply->setHeader(HttpReply::ContentTypeHeader, "text/html");
                reply->setPayload(createErrorXmlDocument(HttpReply::NotFound, tr("Could not find file \"%1\".").arg(settingsFileName)));
                return reply;
            }

            if (!settingsFile.open(QFile::ReadOnly)) {
                qCWarning(dcWebServer()) << "DebugServer: Could not read file for debug download" << settingsFileName;
                HttpReply *reply = RestResource::createErrorReply(HttpReply::Forbidden);
                reply->setHeader(HttpReply::ContentTypeHeader, "text/html");
                reply->setPayload(createErrorXmlDocument(HttpReply::NotFound, tr("Could not open file \"%1\".").arg(settingsFileName)));
                return reply;
            }

            QByteArray settingsFileData = settingsFile.readAll();
            settingsFile.close();

            HttpReply *reply = RestResource::createSuccessReply();
            reply->setHeader(HttpReply::ContentTypeHeader, "text/plain");
            reply->setPayload(settingsFileData);
            return reply;
        }

        if (requestPath.startsWith("/debug/settings/rules")) {
            QString settingsFileName = NymeaSettings(NymeaSettings::SettingsRoleRules).fileName();
            qCDebug(dcWebServer()) << "DebugServer: Loading" << settingsFileName;
            QFile settingsFile(settingsFileName);
            if (!settingsFile.exists()) {
                qCWarning(dcWebServer()) << "DebugServer: Could not read file for debug download" << settingsFileName << "file does not exist.";
                HttpReply *reply = RestResource::createErrorReply(HttpReply::NotFound);
                reply->setHeader(HttpReply::ContentTypeHeader, "text/html");
                reply->setPayload(createErrorXmlDocument(HttpReply::NotFound, tr("Could not find file \"%1\".").arg(settingsFileName)));
                return reply;
            }

            if (!settingsFile.open(QFile::ReadOnly)) {
                qCWarning(dcWebServer()) << "DebugServer: Could not read file for debug download" << settingsFileName;
                HttpReply *reply = RestResource::createErrorReply(HttpReply::Forbidden);
                reply->setHeader(HttpReply::ContentTypeHeader, "text/html");
                reply->setPayload(createErrorXmlDocument(HttpReply::NotFound, tr("Could not open file \"%1\".").arg(settingsFileName)));
                return reply;
            }

            QByteArray settingsFileData = settingsFile.readAll();
            settingsFile.close();

            HttpReply *reply = RestResource::createSuccessReply();
            reply->setHeader(HttpReply::ContentTypeHeader, "text/plain");
            reply->setPayload(settingsFileData);
            return reply;
        }

        if (requestPath.startsWith("/debug/settings/nymead")) {
            QString settingsFileName = NymeaSettings(NymeaSettings::SettingsRoleGlobal).fileName();
            qCDebug(dcWebServer()) << "DebugServer: Loading" << settingsFileName;
            QFile settingsFile(settingsFileName);
            if (!settingsFile.exists()) {
                qCWarning(dcWebServer()) << "DebugServer: Could not read file for debug download" << settingsFileName << "file does not exist.";
                HttpReply *reply = RestResource::createErrorReply(HttpReply::NotFound);
                reply->setHeader(HttpReply::ContentTypeHeader, "text/html");
                reply->setPayload(createErrorXmlDocument(HttpReply::NotFound, tr("Could not find file \"%1\".").arg(settingsFileName)));
                return reply;
            }

            if (!settingsFile.open(QFile::ReadOnly)) {
                qCWarning(dcWebServer()) << "DebugServer: Could not read file for debug download" << settingsFileName;
                HttpReply *reply = RestResource::createErrorReply(HttpReply::Forbidden);
                reply->setHeader(HttpReply::ContentTypeHeader, "text/html");
                reply->setPayload(createErrorXmlDocument(HttpReply::NotFound, tr("Could not open file \"%1\".").arg(settingsFileName)));
                return reply;
            }

            QByteArray settingsFileData = settingsFile.readAll();
            settingsFile.close();

            HttpReply *reply = RestResource::createSuccessReply();
            reply->setHeader(HttpReply::ContentTypeHeader, "text/plain");
            reply->setPayload(settingsFileData);
            return reply;
        }

        if (requestPath.startsWith("/debug/settings/devicestates")) {
            QString settingsFileName = NymeaSettings(NymeaSettings::SettingsRoleDeviceStates).fileName();
            qCDebug(dcWebServer()) << "DebugServer: Loading" << settingsFileName;
            QFile settingsFile(settingsFileName);
            if (!settingsFile.exists()) {
                qCWarning(dcWebServer()) << "DebugServer: Could not read file for debug download" << settingsFileName << "file does not exist.";
                HttpReply *reply = RestResource::createErrorReply(HttpReply::NotFound);
                reply->setHeader(HttpReply::ContentTypeHeader, "text/html");
                reply->setPayload(createErrorXmlDocument(HttpReply::NotFound, tr("Could not find file \"%1\".").arg(settingsFileName)));
                return reply;
            }

            if (!settingsFile.open(QFile::ReadOnly)) {
                qCWarning(dcWebServer()) << "DebugServer: Could not read file for debug download" << settingsFileName;
                HttpReply *reply = RestResource::createErrorReply(HttpReply::Forbidden);
                reply->setHeader(HttpReply::ContentTypeHeader, "text/html");
                reply->setPayload(createErrorXmlDocument(HttpReply::NotFound, tr("Could not open file \"%1\".").arg(settingsFileName)));
                return reply;
            }

            QByteArray settingsFileData = settingsFile.readAll();
            settingsFile.close();

            HttpReply *reply = RestResource::createSuccessReply();
            reply->setHeader(HttpReply::ContentTypeHeader, "text/plain");
            reply->setPayload(settingsFileData);
            return reply;
        }

        if (requestPath.startsWith("/debug/settings/plugins")) {
            QString settingsFileName = NymeaSettings(NymeaSettings::SettingsRolePlugins).fileName();
            qCDebug(dcWebServer()) << "DebugServer: Loading" << settingsFileName;
            QFile settingsFile(settingsFileName);
            if (!settingsFile.exists()) {
                qCWarning(dcWebServer()) << "DebugServer: Could not read file for debug download" << settingsFileName << "file does not exist.";
                HttpReply *reply = RestResource::createErrorReply(HttpReply::NotFound);
                reply->setHeader(HttpReply::ContentTypeHeader, "text/html");
                reply->setPayload(createErrorXmlDocument(HttpReply::NotFound, tr("Could not find file \"%1\".").arg(settingsFileName)));
                return reply;
            }

            if (!settingsFile.open(QFile::ReadOnly)) {
                qCWarning(dcWebServer()) << "DebugServer: Could not read file for debug download" << settingsFileName;
                HttpReply *reply = RestResource::createErrorReply(HttpReply::Forbidden);
                reply->setHeader(HttpReply::ContentTypeHeader, "text/html");
                reply->setPayload(createErrorXmlDocument(HttpReply::NotFound, tr("Could not open file \"%1\".").arg(settingsFileName)));
                return reply;
            }

            QByteArray settingsFileData = settingsFile.readAll();
            settingsFile.close();

            HttpReply *reply = RestResource::createSuccessReply();
            reply->setHeader(HttpReply::ContentTypeHeader, "text/plain");
            reply->setPayload(settingsFileData);
            return reply;
        }
    }

    if (requestPath.startsWith("/debug/ping")) {
        // Only one ping process should run
        if (m_pingProcess || m_pingReply)
            return RestResource::createErrorReply(HttpReply::InternalServerError);

        qCDebug(dcWebServer()) << "DebugServer: Start ping nymea.io process";
        m_pingProcess = new QProcess(this);
        m_pingProcess->setProcessChannelMode(QProcess::MergedChannels);
        connect(m_pingProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(onPingProcessFinished(int,QProcess::ExitStatus)));
        m_pingProcess->start("ping", { "-c", "4", "nymea.io" } );

        m_pingReply = RestResource::createAsyncReply();
        return m_pingReply;
    }

    if (requestPath.startsWith("/debug/dig")) {
        // Only one dig process should run
        if (m_digProcess || m_digReply)
            return RestResource::createErrorReply(HttpReply::InternalServerError);

        qCDebug(dcWebServer()) << "DebugServer: Start dig nymea.io process";
        m_digProcess = new QProcess(this);
        m_digProcess->setProcessChannelMode(QProcess::MergedChannels);
        connect(m_digProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(onDigProcessFinished(int,QProcess::ExitStatus)));
        m_digProcess->start("dig", { "nymea.io" } );

        m_digReply = RestResource::createAsyncReply();
        return m_digReply;
    }

    if (requestPath.startsWith("/debug/tracepath")) {
        // Only one tracepath process should run
        if (m_tracePathProcess || m_tracePathReply)
            return RestResource::createErrorReply(HttpReply::InternalServerError);

        qCDebug(dcWebServer()) << "DebugServer: Start tracepath nymea.io process";
        m_tracePathProcess = new QProcess(this);
        m_tracePathProcess->setProcessChannelMode(QProcess::MergedChannels);
        connect(m_tracePathProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(onTracePathProcessFinished(int,QProcess::ExitStatus)));
        m_tracePathProcess->start("tracepath", { "nymea.io" } );

        m_tracePathReply = RestResource::createAsyncReply();
        return m_tracePathReply;
    }


    // Check if this is a resource file request
    if (resourceFileExits(requestPath)) {
        return processDebugFileRequest(requestPath);
    }

    // If nothing matches, redirect to /debug page
    qCWarning(dcWebServer()) << "DebugServer: Resource for debug interface not found. Redirecting to /debug";
    HttpReply *reply = RestResource::createErrorReply(HttpReply::PermanentRedirect);
    reply->setHeader(HttpReply::LocationHeader, "/debug");
    return reply;
}

void DebugServerHandler::logMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &message) {
  s_oldLogMessageHandler(type, context, message);

  foreach (QWebSocket *client, s_websocketClients) {
      client->sendTextMessage(message + "\n");
  }
}

QByteArray DebugServerHandler::loadResourceData(const QString &resourceFileName)
{
    QFile resourceFile(QString(":%1").arg(resourceFileName));
    if (!resourceFile.open(QFile::ReadOnly | QFile::Text)) {
        qCWarning(dcWebServer()) << "DebugServer: Could not open resource file" << resourceFile.fileName();
        return QByteArray();
    }

    return resourceFile.readAll();
}

QString DebugServerHandler::getResourceFileName(const QString &requestPath)
{
    return QString(requestPath).remove("/debug");
}

bool DebugServerHandler::resourceFileExits(const QString &requestPath)
{
    QFile resourceFile(QString(":%1").arg(getResourceFileName(requestPath)));
    return resourceFile.exists();
}

HttpReply *DebugServerHandler::processDebugFileRequest(const QString &requestPath)
{
    // Here we already know that the resource file exists
    QString resourceFileName = getResourceFileName(requestPath);
    QByteArray data = loadResourceData(resourceFileName);

    // Create reply for resource file
    HttpReply *reply = RestResource::createSuccessReply();
    reply->setPayload(data);

    // Check content type
    if (resourceFileName.endsWith(".css")) {
        reply->setHeader(HttpReply::ContentTypeHeader, "text/css; charset=\"utf-8\";");
    } else if (resourceFileName.endsWith(".svg")) {
        reply->setHeader(HttpReply::ContentTypeHeader, "image/svg+xml; charset=\"utf-8\";");
    } else if (resourceFileName.endsWith(".js")) {
        reply->setHeader(HttpReply::ContentTypeHeader, "text/javascript; charset=\"utf-8\";");
    } else if (resourceFileName.endsWith(".png")) {
        reply->setHeader(HttpReply::ContentTypeHeader, "image/png");
    }

    return reply;
}

QByteArray DebugServerHandler::createDebugXmlDocument()
{
    QByteArray data;
    QXmlStreamWriter writer(&data);
    writer.setAutoFormatting(true);
    writer.writeStartDocument("1.0");
    writer.writeProcessingInstruction("DOCUMENT", "html");
    writer.writeComment("Auto generated html page from nymea server");

    writer.writeStartElement("html");
    writer.writeAttribute("lang", NymeaCore::instance()->configuration()->locale().name());

    // Head
    writer.writeStartElement("head");

    writer.writeEmptyElement("meta");
    writer.writeAttribute("http-equiv", "Content-Type");
    writer.writeAttribute("content", "text/html; charset=utf-8");

    writer.writeEmptyElement("link");
    writer.writeAttribute("rel", "stylesheet");
    writer.writeAttribute("href", "/debug/styles.css");

    writer.writeStartElement("script");
    writer.writeAttribute("type", "application/javascript");
    writer.writeAttribute("src", "/debug/script.js");
    writer.writeCharacters("");
    writer.writeEndElement(); // script

    // Favicons
    writer.writeEmptyElement("link");
    writer.writeAttribute("rel", "icon");
    writer.writeAttribute("type", "image/png");
    writer.writeAttribute("sizes", "16x16");
    writer.writeAttribute("href", "/debug/favicons/favicon-16x16.png");

    writer.writeEmptyElement("link");
    writer.writeAttribute("rel", "icon");
    writer.writeAttribute("type", "image/png");
    writer.writeAttribute("sizes", "32x32");
    writer.writeAttribute("href", "/debug/favicons/favicon-32x32.png");

    writer.writeEmptyElement("link");
    writer.writeAttribute("rel", "icon");
    writer.writeAttribute("type", "image/png");
    writer.writeAttribute("sizes", "64x64");
    writer.writeAttribute("href", "/debug/favicons/favicon-64x64.png");

    writer.writeEmptyElement("link");
    writer.writeAttribute("rel", "icon");
    writer.writeAttribute("type", "image/png");
    writer.writeAttribute("sizes", "128x128");
    writer.writeAttribute("href", "/debug/favicons/favicon-128.png");

    writer.writeEmptyElement("link");
    writer.writeAttribute("rel", "icon");
    writer.writeAttribute("type", "image/png");
    writer.writeAttribute("sizes", "196x196");
    writer.writeAttribute("href", "/debug/favicons/favicon-196x196.png");


    //: The header title of the debug server interface
    writer.writeTextElement("title", tr("Debug nymea"));

    writer.writeEndElement(); // head

    // Container
    writer.writeStartElement("div");
    writer.writeAttribute("class", "container");

    // Header
    writer.writeStartElement("div");
    writer.writeAttribute("class", "header");
    writer.writeStartElement("h1");
    writer.writeEmptyElement("img");
    writer.writeAttribute("src", "/debug/logo.svg");
    writer.writeAttribute("class", "nymea-main-logo");

    //: The main title of the debug server interface
    writer.writeCharacters(tr("nymea debug interface"));
    writer.writeEndElement(); // h1
    writer.writeEndElement(); // div header

    // Body
    writer.writeStartElement("div");
    writer.writeAttribute("class", "body");

    //: The welcome message of the debug interface
    writer.writeTextElement("p", tr("Welcome to the debug interface."));
    writer.writeTextElement("p", tr("This debug interface was designed to provide an easy possibility to get helpful information about the running nymea server."));

    // Warning
    writer.writeStartElement("div");
    writer.writeAttribute("class", "warning");
    // Warning image
    writer.writeStartElement("div");
    writer.writeAttribute("class", "warning-image-area");
    writer.writeEmptyElement("img");
    writer.writeAttribute("class", "warning-image");
    writer.writeAttribute("src", "/debug/warning.svg");
    writer.writeEndElement(); // div warning image
    // Warning message
    writer.writeStartElement("div");
    writer.writeAttribute("class", "warning-message");
    //: The warning message of the debug interface
    writer.writeCharacters(tr("Be aware that this debug interface is a security risk and could offer access to sensible data."));
    writer.writeEndElement(); // div warning message
    writer.writeEndElement(); // div warning


    writer.writeEmptyElement("hr");

    // System information section
    //: The server information section of the debug interface
    writer.writeTextElement("h2", tr("Server information"));
    writer.writeEmptyElement("hr");

    writer.writeStartElement("table");

    writer.writeStartElement("tr");
    //: The user name in the server infromation section of the debug interface
    writer.writeTextElement("th", tr("User"));
    writer.writeTextElement("td", qgetenv("USER"));
    writer.writeEndElement(); // tr

    writer.writeStartElement("tr");
    //: The Qt build version description in the server infromation section of the debug interface
    writer.writeTextElement("th", tr("Compiled with Qt version"));
    writer.writeTextElement("td", QT_VERSION_STR);
    writer.writeEndElement(); // tr

    writer.writeStartElement("tr");
    //: The Qt runtime version description in the server infromation section of the debug interface
    writer.writeTextElement("th", tr("Qt runtime version"));
    writer.writeTextElement("td", qVersion());
    writer.writeEndElement(); // tr

    writer.writeStartElement("tr");
    //: The command description in the server infromation section of the debug interface
    writer.writeTextElement("th", tr("Command"));
    writer.writeTextElement("td", QCoreApplication::arguments().join(' '));
    writer.writeEndElement(); // tr

    if (!qgetenv("SNAP").isEmpty()) {
        // Note: http://snapcraft.io/docs/reference/env

        writer.writeStartElement("tr");
        //: The snap name description in the server infromation section of the debug interface
        writer.writeTextElement("th", tr("Snap name"));
        writer.writeTextElement("td", qgetenv("SNAP_NAME"));
        writer.writeEndElement(); // tr

        writer.writeStartElement("tr");
        //: The snap version description in the server infromation section of the debug interface
        writer.writeTextElement("th", tr("Snap version"));
        writer.writeTextElement("td", qgetenv("SNAP_VERSION"));
        writer.writeEndElement(); // tr

        writer.writeStartElement("tr");
        //: The snap directory description in the server infromation section of the debug interface
        writer.writeTextElement("th", tr("Snap directory"));
        writer.writeTextElement("td", qgetenv("SNAP"));
        writer.writeEndElement(); // tr

        writer.writeStartElement("tr");
        //: The snap application data description in the server infromation section of the debug interface
        writer.writeTextElement("th", tr("Snap application data"));
        writer.writeTextElement("td", qgetenv("SNAP_DATA"));
        writer.writeEndElement(); // tr

        writer.writeStartElement("tr");
        //: The snap user data description in the server infromation section of the debug interface
        writer.writeTextElement("th", tr("Snap user data"));
        writer.writeTextElement("td", qgetenv("SNAP_USER_DATA"));
        writer.writeEndElement(); // tr

        writer.writeStartElement("tr");
        //: The snap common data description in the server infromation section of the debug interface
        writer.writeTextElement("th", tr("Snap common data"));
        writer.writeTextElement("td", qgetenv("SNAP_COMMON"));
        writer.writeEndElement(); // tr
    }


    writer.writeStartElement("tr");
    //: The server name description in the server infromation section of the debug interface
    writer.writeTextElement("th", tr("Server name"));
    writer.writeTextElement("td", NymeaCore::instance()->configuration()->serverName());
    writer.writeEndElement(); // tr

    writer.writeStartElement("tr");
    //: The server version description in the server infromation section of the debug interface
    writer.writeTextElement("th", tr("Server version"));
    writer.writeTextElement("td", NYMEA_VERSION_STRING);
    writer.writeEndElement(); // tr

    writer.writeStartElement("tr");
    //: The API version description in the server infromation section of the debug interface
    writer.writeTextElement("th", tr("JSON-RPC version"));
    writer.writeTextElement("td", JSON_PROTOCOL_VERSION);
    writer.writeEndElement(); // tr

    writer.writeStartElement("tr");
    //: The language description in the server infromation section of the debug interface
    writer.writeTextElement("th", tr("Language"));
    writer.writeTextElement("td", NymeaCore::instance()->configuration()->locale().name() + " (" + NymeaCore::instance()->configuration()->locale().nativeCountryName() + " - " + NymeaCore::instance()->configuration()->locale().nativeLanguageName() + ")");
    writer.writeEndElement(); // tr

    writer.writeStartElement("tr");
    //: The timezone description in the server infromation section of the debug interface
    writer.writeTextElement("th", tr("Timezone"));
    writer.writeTextElement("td", QString::fromUtf8(NymeaCore::instance()->configuration()->timeZone()));
    writer.writeEndElement(); // tr

    writer.writeStartElement("tr");
    //: The server id description in the server infromation section of the debug interface
    writer.writeTextElement("th", tr("Server UUID"));
    writer.writeTextElement("td", NymeaCore::instance()->configuration()->serverUuid().toString());
    writer.writeEndElement(); // tr

    writer.writeStartElement("tr");
    //: The settings path description in the server infromation section of the debug interface
    writer.writeTextElement("th", tr("Settings path"));
    writer.writeTextElement("td", NymeaSettings::settingsPath());
    writer.writeEndElement(); // tr

    writer.writeStartElement("tr");
    //: The translation path description in the server infromation section of the debug interface
    writer.writeTextElement("th", tr("Translations path"));
    writer.writeTextElement("td", NymeaSettings(NymeaSettings::SettingsRoleGlobal).translationsPath());
    writer.writeEndElement(); // tr

    for (int i = 0; i < NymeaCore::instance()->deviceManager()->pluginSearchDirs().count(); i++) {
        writer.writeStartElement("tr");
        writer.writeEndElement(); // tr

        if (i == 0) {
            //: The plugins path description in the server infromation section of the debug interface
            writer.writeTextElement("th", tr("Plugin paths"));
        } else {
            writer.writeTextElement("th", "");
        }
        writer.writeTextElement("td", QFileInfo(NymeaCore::instance()->deviceManager()->pluginSearchDirs().at(i)).absoluteFilePath());
    }

    writer.writeEndElement(); // table


    // Downloads section
    writer.writeEmptyElement("hr");
    //: The downloads section of the debug interface
    writer.writeTextElement("h2", tr("Downloads"));

    // Logs download section
    writer.writeEmptyElement("hr");
    //: The download logs section of the debug interface
    writer.writeTextElement("h3", tr("Logs"));
    writer.writeEmptyElement("hr");

    // Download row
    writer.writeStartElement("div");
    writer.writeAttribute("class", "download-row");

    writer.writeStartElement("div");
    writer.writeAttribute("class", "download-name-column");
    //: The log databse download description of the debug interface
    writer.writeTextElement("p", tr("Log database"));
    writer.writeEndElement(); // div download-name-column

    if (QFileInfo(NymeaCore::instance()->configuration()->logDBName()).exists()) {
        writer.writeStartElement("div");
        writer.writeAttribute("class", "download-path-column");
        writer.writeTextElement("p", NymeaCore::instance()->configuration()->logDBName());
        writer.writeEndElement(); // div download-path-column
    }

    writer.writeStartElement("div");
    writer.writeAttribute("class", "download-button-column");
    writer.writeStartElement("form");
    writer.writeAttribute("class", "download-button");
    writer.writeStartElement("button");
    writer.writeAttribute("class", "button");
    writer.writeAttribute("type", "button");
    if (!QFile::exists(NymeaCore::instance()->configuration()->logDBName())) {
        writer.writeAttribute("disabled", "disabled");
    }
    writer.writeAttribute("onClick", "downloadFile('/debug/logdb.sql', 'logdb.sql')");
    //: The download button description of the debug interface
    writer.writeCharacters(tr("Download"));
    writer.writeEndElement(); // button
    writer.writeEndElement(); // form
    writer.writeEndElement(); // div download-button-column

    writer.writeEndElement(); // div download-row

    // Download row
    writer.writeStartElement("div");
    writer.writeAttribute("class", "download-row");

    writer.writeStartElement("div");
    writer.writeAttribute("class", "download-name-column");
    //: The syslog download description of the debug interface
    writer.writeTextElement("p", tr("System logs"));
    writer.writeEndElement(); // div download-name-column

    writer.writeStartElement("div");
    writer.writeAttribute("class", "download-path-column");
    writer.writeTextElement("p", "/var/log/syslog");
    writer.writeEndElement(); // div download-path-column

    writer.writeStartElement("div");
    writer.writeAttribute("class", "download-button-column");
    writer.writeStartElement("form");
    writer.writeAttribute("class", "download-button");
    writer.writeStartElement("button");
    writer.writeAttribute("class", "button");
    writer.writeAttribute("type", "button");
    writer.writeAttribute("onClick", "downloadFile('/debug/syslog', 'syslog.log')");
    writer.writeCharacters(tr("Download"));
    writer.writeEndElement(); // button
    writer.writeEndElement(); // form
    writer.writeEndElement(); // div download-button-column

    writer.writeStartElement("div");
    writer.writeAttribute("class", "show-button-column");
    writer.writeStartElement("form");
    writer.writeAttribute("class", "show-button");
    writer.writeStartElement("button");
    writer.writeAttribute("class", "button");
    writer.writeAttribute("type", "button");
    writer.writeAttribute("onClick", "window.open('/debug/syslog', '_blank')");
    writer.writeCharacters(tr("Show"));
    writer.writeEndElement(); // button
    writer.writeEndElement(); // form
    writer.writeEndElement(); // div show-button-column

    writer.writeEndElement(); // div download-row


    // Settings download section
    writer.writeEmptyElement("hr");
    //: The settings download section title of the debug interface
    writer.writeTextElement("h3", tr("Settings"));
    writer.writeEmptyElement("hr");

    // Download row
    writer.writeStartElement("div");
    writer.writeAttribute("class", "download-row");

    writer.writeStartElement("div");
    writer.writeAttribute("class", "download-name-column");
    //: The nymead settings download description of the debug interface
    writer.writeTextElement("p", tr("nymead settings"));
    writer.writeEndElement(); // div download-name-column

    writer.writeStartElement("div");
    writer.writeAttribute("class", "download-path-column");
    writer.writeTextElement("p", NymeaSettings(NymeaSettings::SettingsRoleGlobal).fileName());
    writer.writeEndElement(); // div download-path-column

    writer.writeStartElement("div");
    writer.writeAttribute("class", "download-button-column");
    writer.writeStartElement("form");
    writer.writeAttribute("class", "download-button");
    writer.writeStartElement("button");
    writer.writeAttribute("class", "button");
    writer.writeAttribute("type", "button");
    if (!QFile::exists(NymeaSettings(NymeaSettings::SettingsRoleGlobal).fileName())) {
        writer.writeAttribute("disabled", "disabled");
    }
    writer.writeAttribute("onClick", "downloadFile('/debug/settings/nymead', 'nymead.conf')");
    writer.writeCharacters(tr("Download"));
    writer.writeEndElement(); // button
    writer.writeEndElement(); // form
    writer.writeEndElement(); // div download-button-column

    writer.writeStartElement("div");
    writer.writeAttribute("class", "show-button-column");
    writer.writeStartElement("form");
    writer.writeAttribute("class", "show-button");
    writer.writeStartElement("button");
    writer.writeAttribute("class", "button");
    writer.writeAttribute("type", "button");
    if (!QFile::exists(NymeaSettings(NymeaSettings::SettingsRoleGlobal).fileName())) {
        writer.writeAttribute("disabled", "disabled");
    }
    writer.writeAttribute("onClick", "window.open('/debug/settings/nymead', '_blank')");
    writer.writeCharacters(tr("Show"));
    writer.writeEndElement(); // button
    writer.writeEndElement(); // form
    writer.writeEndElement(); // div show-button-column

    writer.writeEndElement(); // div download-row


    // Download row
    writer.writeStartElement("div");
    writer.writeAttribute("class", "download-row");

    writer.writeStartElement("div");
    writer.writeAttribute("class", "download-name-column");
    //: The device settings download description of the debug interface
    writer.writeTextElement("p", tr("Device settings"));
    writer.writeEndElement(); // div download-name-column

    writer.writeStartElement("div");
    writer.writeAttribute("class", "download-path-column");
    writer.writeTextElement("p", NymeaSettings(NymeaSettings::SettingsRoleDevices).fileName());
    writer.writeEndElement(); // div download-path-column

    writer.writeStartElement("div");
    writer.writeAttribute("class", "download-button-column");
    writer.writeStartElement("form");
    writer.writeAttribute("class", "download-button");
    writer.writeStartElement("button");
    writer.writeAttribute("class", "button");
    writer.writeAttribute("type", "button");
    if (!QFile::exists(NymeaSettings(NymeaSettings::SettingsRoleDevices).fileName())) {
        writer.writeAttribute("disabled", "disabled");
    }
    writer.writeAttribute("onClick", "downloadFile('/debug/settings/devices', 'devices.conf')");
    writer.writeCharacters(tr("Download"));
    writer.writeEndElement(); // button
    writer.writeEndElement(); // form
    writer.writeEndElement(); // div download-button-column

    writer.writeStartElement("div");
    writer.writeAttribute("class", "show-button-column");
    writer.writeStartElement("form");
    writer.writeAttribute("class", "show-button");
    writer.writeStartElement("button");
    writer.writeAttribute("class", "button");
    writer.writeAttribute("type", "button");
    if (!QFile::exists(NymeaSettings(NymeaSettings::SettingsRoleDevices).fileName())) {
        writer.writeAttribute("disabled", "true");
    }
    writer.writeAttribute("onClick", "window.open('/debug/settings/devices', '_blank')");
    writer.writeCharacters(tr("Show"));
    writer.writeEndElement(); // button
    writer.writeEndElement(); // form
    writer.writeEndElement(); // div show-button-column

    writer.writeEndElement(); // div download-row


    // Download row
    writer.writeStartElement("div");
    writer.writeAttribute("class", "download-row");

    writer.writeStartElement("div");
    writer.writeAttribute("class", "download-name-column");
    //: The device states settings download description of the debug interface
    writer.writeTextElement("p", tr("Device states settings"));
    writer.writeEndElement(); // div download-name-column

    writer.writeStartElement("div");
    writer.writeAttribute("class", "download-path-column");
    writer.writeTextElement("p", NymeaSettings(NymeaSettings::SettingsRoleDeviceStates).fileName());
    writer.writeEndElement(); // div download-path-column

    writer.writeStartElement("div");
    writer.writeAttribute("class", "download-button-column");
    writer.writeStartElement("form");
    writer.writeAttribute("class", "download-button");
    writer.writeStartElement("button");
    writer.writeAttribute("class", "button");
    writer.writeAttribute("type", "button");
    if (!QFile::exists(NymeaSettings(NymeaSettings::SettingsRoleDeviceStates).fileName())) {
        writer.writeAttribute("disabled", "true");
    }
    writer.writeAttribute("onClick", "downloadFile('/debug/settings/devicestates', 'devicestates.conf')");
    writer.writeCharacters(tr("Download"));
    writer.writeEndElement(); // button
    writer.writeEndElement(); // form
    writer.writeEndElement(); // div download-button-column

    writer.writeStartElement("div");
    writer.writeAttribute("class", "show-button-column");
    writer.writeStartElement("form");
    writer.writeAttribute("class", "show-button");
    writer.writeStartElement("button");
    writer.writeAttribute("class", "button");
    writer.writeAttribute("type", "button");
    if (!QFile::exists(NymeaSettings(NymeaSettings::SettingsRoleDeviceStates).fileName())) {
        writer.writeAttribute("disabled", "true");
    }
    writer.writeAttribute("onClick", "window.open('/debug/settings/devicestates', '_blank')");
    writer.writeCharacters(tr("Show"));
    writer.writeEndElement(); // button
    writer.writeEndElement(); // form
    writer.writeEndElement(); // div show-button-column

    writer.writeEndElement(); // div download-row


    // Download row
    writer.writeStartElement("div");
    writer.writeAttribute("class", "download-row");

    writer.writeStartElement("div");
    writer.writeAttribute("class", "download-name-column");
    //: The rules settings download description of the debug interface
    writer.writeTextElement("p", tr("Rules settings"));
    writer.writeEndElement(); // div download-name-column

    writer.writeStartElement("div");
    writer.writeAttribute("class", "download-path-column");
    writer.writeTextElement("p", NymeaSettings(NymeaSettings::SettingsRoleRules).fileName());
    writer.writeEndElement(); // div download-path-column

    writer.writeStartElement("div");
    writer.writeAttribute("class", "download-button-column");
    writer.writeStartElement("form");
    writer.writeAttribute("class", "download-button");
    writer.writeStartElement("button");
    writer.writeAttribute("class", "button");
    writer.writeAttribute("type", "button");
    if (!QFile::exists(NymeaSettings(NymeaSettings::SettingsRoleRules).fileName())) {
        writer.writeAttribute("disabled", "true");
    }
    writer.writeAttribute("onClick", "downloadFile('/debug/settings/rules', 'rules.conf')");
    writer.writeCharacters(tr("Download"));
    writer.writeEndElement(); // button
    writer.writeEndElement(); // form
    writer.writeEndElement(); // div download-button-column

    writer.writeStartElement("div");
    writer.writeAttribute("class", "show-button-column");
    writer.writeStartElement("form");
    writer.writeAttribute("class", "show-button");
    writer.writeStartElement("button");
    writer.writeAttribute("class", "button");
    writer.writeAttribute("type", "button");
    if (!QFile::exists(NymeaSettings(NymeaSettings::SettingsRoleRules).fileName())) {
        writer.writeAttribute("disabled", "true");
    }
    writer.writeAttribute("onClick", "window.open('/debug/settings/rules', '_blank')");
    writer.writeCharacters(tr("Show"));
    writer.writeEndElement(); // button
    writer.writeEndElement(); // form
    writer.writeEndElement(); // div show-button-column

    writer.writeEndElement(); // div download-row


    // Download row
    writer.writeStartElement("div");
    writer.writeAttribute("class", "download-row");

    writer.writeStartElement("div");
    writer.writeAttribute("class", "download-name-column");
    //: The plugins settings download description of the debug interface
    writer.writeTextElement("p", tr("Plugins settings"));
    writer.writeEndElement(); // div download-name-column

    writer.writeStartElement("div");
    writer.writeAttribute("class", "download-path-column");
    writer.writeTextElement("p", NymeaSettings(NymeaSettings::SettingsRolePlugins).fileName());
    writer.writeEndElement(); // div download-path-column

    writer.writeStartElement("div");
    writer.writeAttribute("class", "download-button-column");
    writer.writeStartElement("form");
    writer.writeAttribute("class", "download-button");
    writer.writeStartElement("button");
    writer.writeAttribute("class", "button");
    writer.writeAttribute("type", "button");
    if (!QFile::exists(NymeaSettings(NymeaSettings::SettingsRolePlugins).fileName())) {
        writer.writeAttribute("disabled", "true");
    }
    writer.writeAttribute("onClick", "downloadFile('/debug/settings/plugins', 'plugins.conf')");
    writer.writeCharacters(tr("Download"));
    writer.writeEndElement(); // button
    writer.writeEndElement(); // form
    writer.writeEndElement(); // div download-button-column

    writer.writeStartElement("div");
    writer.writeAttribute("class", "show-button-column");
    writer.writeStartElement("form");
    writer.writeAttribute("class", "show-button");
    writer.writeStartElement("button");
    writer.writeAttribute("class", "button");
    writer.writeAttribute("type", "button");
    if (!QFile::exists(NymeaSettings(NymeaSettings::SettingsRolePlugins).fileName())) {
        writer.writeAttribute("disabled", "true");
    }
    writer.writeAttribute("onClick", "window.open('/debug/settings/plugins', '_blank')");
    writer.writeCharacters(tr("Show"));
    writer.writeEndElement(); // button
    writer.writeEndElement(); // form
    writer.writeEndElement(); // div show-button-column

    writer.writeEndElement(); // div download-row


    // Network section
    writer.writeStartElement("div");
    writer.writeAttribute("class", "network");
    writer.writeEmptyElement("hr");
    //: The network section of the debug interface
    writer.writeTextElement("h2", tr("Network"));

    //: The network section description of the debug interface
    writer.writeTextElement("p", tr("This section allows you to perform different network connectivity tests in order "
                                    "to find out if the device nymea is running on is online and has full network connectivity."));


    // Ping section
    writer.writeEmptyElement("hr");
    //: The ping section of the debug interface
    writer.writeTextElement("h3", tr("Ping nymea.io"));
    writer.writeEmptyElement("hr");

    // Start ping button
    writer.writeStartElement("button");
    writer.writeAttribute("class", "button");
    writer.writeAttribute("type", "button");
    writer.writeAttribute("id", "pingButton");
    writer.writeAttribute("onClick", "startPingTest()");
    //: The ping button text of the debug interface
    writer.writeCharacters(tr("Start ping test"));
    writer.writeEndElement(); // button

    // Ping output
    writer.writeStartElement("textarea");
    writer.writeAttribute("class", "console-textarea");
    writer.writeAttribute("id", "pingTextArea");
    writer.writeAttribute("readonly", "readonly");
    writer.writeAttribute("rows", "12");
    writer.writeCharacters("");
    writer.writeEndElement(); // textarea


    // Dig section
    writer.writeEmptyElement("hr");
    //: The ping section of the debug interface
    writer.writeTextElement("h3", tr("DNS lookup for nymea.io"));
    writer.writeEmptyElement("hr");

    // Start dig button
    writer.writeStartElement("button");
    writer.writeAttribute("class", "button");
    writer.writeAttribute("type", "button");
    writer.writeAttribute("id", "digButton");
    writer.writeAttribute("onClick", "startDigTest()");
    //: The ping button text of the debug interface
    writer.writeCharacters(tr("Start DNS lookup test"));
    writer.writeEndElement(); // button

    // Dig output
    writer.writeStartElement("textarea");
    writer.writeAttribute("class", "console-textarea");
    writer.writeAttribute("id", "digTextArea");
    writer.writeAttribute("readonly", "readonly");
    writer.writeAttribute("rows", "21");
    writer.writeCharacters("");
    writer.writeEndElement(); // textarea

    // Dig section
    writer.writeEmptyElement("hr");
    //: The ping section of the debug interface
    writer.writeTextElement("h3", tr("Trace path nymea.io"));
    writer.writeEmptyElement("hr");

    // Start tracepath button
    writer.writeStartElement("button");
    writer.writeAttribute("class", "button");
    writer.writeAttribute("type", "button");
    writer.writeAttribute("id", "tracePathButton");
    writer.writeAttribute("onClick", "startTracePathTest()");
    //: The ping button text of the debug interface
    writer.writeCharacters(tr("Start trace path test"));
    writer.writeEndElement(); // button

    // Dig output
    writer.writeStartElement("textarea");
    writer.writeAttribute("class", "console-textarea");
    writer.writeAttribute("id", "tracePathTextArea");
    writer.writeAttribute("readonly", "readonly");
    writer.writeAttribute("rows", "20");
    writer.writeCharacters("");
    writer.writeEndElement(); // textarea

    writer.writeEndElement(); // div network

    // Logs stream
    writer.writeStartElement("div");
    writer.writeAttribute("class", "logstream");
    writer.writeEmptyElement("hr");
    //: The network section of the debug interface
    writer.writeTextElement("h2", tr("Server debug log stream"));
    writer.writeEmptyElement("hr");

    // Start stream button
    writer.writeStartElement("button");
    writer.writeAttribute("class", "button");
    writer.writeAttribute("type", "button");
    writer.writeAttribute("id", "connectWebsocketButton");
    writer.writeAttribute("onClick", "connectWebsocket()");
    //: The connect button for the log stream of the debug interface
    writer.writeCharacters(tr("Connect stream"));
    writer.writeEndElement(); // button

    // Stop stream button
    writer.writeStartElement("button");
    writer.writeAttribute("class", "button");
    writer.writeAttribute("type", "button");
    writer.writeAttribute("id", "disconnectWebsocketButton");
    writer.writeAttribute("onClick", "disconnectWebsocket()");
    writer.writeAttribute("disabled", "true");
    //: The disconnect button for the log stream of the debug interface
    writer.writeCharacters(tr("Disconnect stream"));
    writer.writeEndElement(); // button


    // Dig output
    writer.writeStartElement("textarea");
    writer.writeAttribute("class", "console-textarea");
    writer.writeAttribute("id", "logsTextArea");
    writer.writeAttribute("readonly", "readonly");
    writer.writeAttribute("rows", "30");
    writer.writeCharacters("");
    writer.writeEndElement(); // textarea


    writer.writeEndElement(); // div body

    // Footer
    writer.writeStartElement("div");
    writer.writeAttribute("class", "footer");
    writer.writeTextElement("p", QString("Copyright %1 2018 guh GmbH.").arg(QChar(0xA9)));
    //: The footer license note of the debug interface
    writer.writeTextElement("p", tr("Released under the GNU GENERAL PUBLIC LICENSE Version 2."));
    writer.writeEndElement(); // div footer

    writer.writeEndElement(); // div container

    writer.writeEndElement(); // html

    return data;
}

QByteArray DebugServerHandler::createErrorXmlDocument(HttpReply::HttpStatusCode statusCode, const QString &errorMessage)
{
    QByteArray data;
    QXmlStreamWriter writer(&data);
    writer.setAutoFormatting(true);
    writer.writeStartDocument("1.0");
    writer.writeComment("Live generated html page from nymea");
    writer.writeStartElement("html");
    writer.writeAttribute("lang", NymeaCore::instance()->configuration()->locale().name());

    // Head
    writer.writeStartElement("head");

    writer.writeEmptyElement("meta");
    writer.writeAttribute("http-equiv", "Content-Type");
    writer.writeAttribute("content", "text/html; charset=utf-8");

    writer.writeEmptyElement("link");
    writer.writeAttribute("rel", "stylesheet");
    writer.writeAttribute("href", "/debug/styles.css");

    writer.writeTextElement("title", tr("Debug nymea"));

    writer.writeEndElement(); // head

    // Container
    writer.writeStartElement("div");
    writer.writeAttribute("class", "container");

    // Header
    writer.writeStartElement("div");
    writer.writeAttribute("class", "header");
    writer.writeTextElement("p", " ");
    //: The HTTP error message of the debug interface. The %1 represents the error code ie.e 404
    writer.writeTextElement("h1", tr("Error  %1").arg(static_cast<int>(statusCode)));
    writer.writeEndElement(); // div header

    // Body
    writer.writeStartElement("div");
    writer.writeAttribute("class", "body");

    // Warning
    writer.writeStartElement("div");
    writer.writeAttribute("class", "warning");
    // Warning image
    writer.writeStartElement("div");
    writer.writeAttribute("class", "warning-image-area");
    writer.writeEmptyElement("img");
    writer.writeAttribute("class", "warning-image");
    writer.writeAttribute("src", "/debug/warning.svg");
    writer.writeEndElement(); // div warning image
    // Warning message
    writer.writeStartElement("div");
    writer.writeAttribute("class", "warning-message");
    writer.writeCharacters(errorMessage);
    writer.writeEndElement(); // div warning message
    writer.writeEndElement(); // div warning

    writer.writeEndElement(); // div body

    // Footer
    writer.writeStartElement("div");
    writer.writeAttribute("class", "footer");
    writer.writeTextElement("p", QString("Copyright %1 2018 guh GmbH.").arg(QChar(0xA9)));
    writer.writeTextElement("p", tr("Released under the GNU GENERAL PUBLIC LICENSE Version 2."));
    writer.writeEndElement(); // div footer

    writer.writeEndElement(); // div container
    writer.writeEndElement(); // html

    return data;
}

void DebugServerHandler::onTimeout()
{
    foreach (QWebSocket *client, s_websocketClients) {
        client->sendTextMessage("Hallo!\n");
    }
}

void DebugServerHandler::onWebsocketClientConnected()
{
    QWebSocket *client = m_websocketServer->nextPendingConnection();
    s_websocketClients.append(client);
    qCDebug(dcWebServer()) << "DebugServer: New websocket client connected:" << client->peerAddress().toString();

    connect(client, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onWebsocketClientError(QAbstractSocket::SocketError)));
    connect(client, &QWebSocket::disconnected, this, &DebugServerHandler::onWebsocketClientDisconnected);
}

void DebugServerHandler::onWebsocketClientDisconnected()
{
    QWebSocket *client = static_cast<QWebSocket *>(sender());
    qCDebug(dcWebServer()) << "DebugServer: Websocket client disconnected" << client->peerAddress().toString();
    s_websocketClients.removeAll(client);
    client->deleteLater();
}

void DebugServerHandler::onWebsocketClientError(QAbstractSocket::SocketError error)
{
    QWebSocket *client = static_cast<QWebSocket *>(sender());
    qCWarning(dcWebServer()) << "DebugServer: Websocket client error" << client->peerAddress().toString() << error << client->errorString();
}

void DebugServerHandler::onPingProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qCDebug(dcWebServer()) << "DebugServer: Ping process finished" << exitCode << exitStatus;
    QByteArray processOutput = m_pingProcess->readAll();
    qCDebug(dcWebServer()) << "DebugServer: Ping output:" << endl << qUtf8Printable(processOutput);

    m_pingReply->setPayload(processOutput);
    m_pingReply->setHttpStatusCode(HttpReply::Ok);
    m_pingReply->finished();
    m_pingReply = nullptr;

    m_pingProcess->deleteLater();
    m_pingProcess = nullptr;
}

void DebugServerHandler::onDigProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qCDebug(dcWebServer()) << "DebugServer: Dig process finished" << exitCode << exitStatus;
    QByteArray processOutput = m_digProcess->readAll();
    qCDebug(dcWebServer()) << "DebugServer: Dig output:" << endl << qUtf8Printable(processOutput);

    m_digReply->setPayload(processOutput);
    m_digReply->setHttpStatusCode(HttpReply::Ok);
    m_digReply->finished();
    m_digReply = nullptr;

    m_digProcess->deleteLater();
    m_digProcess = nullptr;
}

void DebugServerHandler::onTracePathProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qCDebug(dcWebServer()) << "DebugServer: Tracepath process finished" << exitCode << exitStatus;
    QByteArray processOutput = m_tracePathProcess->readAll();
    qCDebug(dcWebServer()) << "DebugServer: Tracepath output:" << endl << qUtf8Printable(processOutput);

    m_tracePathReply->setPayload(processOutput);
    m_tracePathReply->setHttpStatusCode(HttpReply::Ok);
    m_tracePathReply->finished();
    m_tracePathReply = nullptr;

    m_tracePathProcess->deleteLater();
    m_tracePathProcess = nullptr;
}


}
