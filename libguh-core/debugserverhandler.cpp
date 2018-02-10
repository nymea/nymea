/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2018 Simon Stürz <simon.stuerz@guh.io>                   *
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

#include "guhcore.h"
#include "httpreply.h"
#include "guhsettings.h"
#include "httprequest.h"
#include "loggingcategories.h"
#include "debugserverhandler.h"

#include <QXmlStreamWriter>
#include <QCoreApplication>

namespace guhserver {

DebugServerHandler::DebugServerHandler(QObject *parent) : QObject(parent)
{

}

QByteArray DebugServerHandler::createDebugXmlDocument()
{
    QByteArray data;
    QXmlStreamWriter writer(&data);
    writer.setAutoFormatting(true);
    writer.writeStartDocument("1.0");
    writer.writeProcessingInstruction("DOCUMENT", "html");
    writer.writeComment("Auto generated html page from nymea");
    writer.writeStartElement("html");
    writer.writeAttribute("lang", GuhCore::instance()->configuration()->locale().name());

    writer.writeStartElement("head");

    writer.writeEmptyElement("meta");
    writer.writeAttribute("http-equiv", "Content-Type");
    writer.writeAttribute("content", "text/html; charset=utf-8");

    writer.writeEmptyElement("link");
    writer.writeAttribute("rel", "stylesheet");
    writer.writeAttribute("href", "/debug/styles.css");

    writer.writeTextElement("title", QCoreApplication::translate("main", "Debug nymea"));

    writer.writeEndElement(); // head

    writer.writeStartElement("body");

    // Welcome section
    writer.writeTextElement("h1", QCoreApplication::translate("main", "nymea debug interface"));
    writer.writeEmptyElement("hr");
    writer.writeTextElement("p", QCoreApplication::translate("main", "Welcome to the debug interface."));
    writer.writeTextElement("p", QCoreApplication::translate("main", "This debug interface was designed to provide an easy possibility to get helpful information about the running nymea server."));

    writer.writeEmptyElement("hr");
    writer.writeTextElement("h3", QCoreApplication::translate("main", "Warning"));
    writer.writeTextElement("p", QCoreApplication::translate("main", "Be aware that this debug interface is a security breach and offers access to the system log and therefore to possibly sensible data."));
    writer.writeTextElement("p", QCoreApplication::translate("main", "If you are not using this debug tools, than you should disable it."));
    writer.writeEmptyElement("hr");

    // System information section
    writer.writeTextElement("h2", QCoreApplication::translate("main", "Server information"));
    writer.writeEmptyElement("hr");

    writer.writeStartElement("table");
    //writer.writeAttribute("width", "100%");
    writer.writeAttribute("border", "1");

    writer.writeStartElement("col");
    writer.writeAttribute("align", "left");
    writer.writeEndElement(); // col

    writer.writeStartElement("col");
    writer.writeAttribute("align", "left");
    writer.writeEndElement(); // col

    QString userName = qgetenv("USER");
    if (userName.isEmpty())
        userName = qgetenv("USERNAME");

    writer.writeStartElement("tr");
    writer.writeTextElement("th", QCoreApplication::translate("main", "User"));
    writer.writeTextElement("td", userName);
    writer.writeEndElement(); // tr

    writer.writeStartElement("tr");
    writer.writeTextElement("th", QCoreApplication::translate("main", "Compiled with Qt version"));
    writer.writeTextElement("td", QT_VERSION_STR);
    writer.writeEndElement(); // tr

    writer.writeStartElement("tr");
    writer.writeTextElement("th", QCoreApplication::translate("main", "Qt runtime version"));
    writer.writeTextElement("td", qVersion());
    writer.writeEndElement(); // tr

    writer.writeStartElement("tr");
    writer.writeTextElement("th", QCoreApplication::translate("main", "Command"));
    writer.writeTextElement("td", QCoreApplication::arguments().join(' '));
    writer.writeEndElement(); // tr

    if (!qgetenv("SNAP").isEmpty()) {
        // Note: http://snapcraft.io/docs/reference/env

        writer.writeStartElement("tr");
        writer.writeTextElement("th", QCoreApplication::translate("main", "Snap name"));
        writer.writeTextElement("td", qgetenv("SNAP_NAME"));
        writer.writeEndElement(); // tr

        writer.writeStartElement("tr");
        writer.writeTextElement("th", QCoreApplication::translate("main", "Snap version"));
        writer.writeTextElement("td", qgetenv("SNAP_VERSION"));
        writer.writeEndElement(); // tr

        writer.writeStartElement("tr");
        writer.writeTextElement("th", QCoreApplication::translate("main", "Snap directory"));
        writer.writeTextElement("td", qgetenv("SNAP"));
        writer.writeEndElement(); // tr

        writer.writeStartElement("tr");
        writer.writeTextElement("th", QCoreApplication::translate("main", "Snap application data"));
        writer.writeTextElement("td", qgetenv("SNAP_DATA"));
        writer.writeEndElement(); // tr

        writer.writeStartElement("tr");
        writer.writeTextElement("th", QCoreApplication::translate("main", "Snap user data"));
        writer.writeTextElement("td", qgetenv("SNAP_USER_DATA"));
        writer.writeEndElement(); // tr

        writer.writeStartElement("tr");
        writer.writeTextElement("th", QCoreApplication::translate("main", "Snap common data"));
        writer.writeTextElement("td", qgetenv("SNAP_COMMON"));
        writer.writeEndElement(); // tr
    }


    writer.writeStartElement("tr");
    writer.writeTextElement("th", QCoreApplication::translate("main", "Server name"));
    writer.writeTextElement("td", GuhCore::instance()->configuration()->serverName());
    writer.writeEndElement(); // tr

    writer.writeStartElement("tr");
    writer.writeTextElement("th", QCoreApplication::translate("main", "Server version"));
    writer.writeTextElement("td", GUH_VERSION_STRING);
    writer.writeEndElement(); // tr

    writer.writeStartElement("tr");
    writer.writeTextElement("th", QCoreApplication::translate("main", "JSON-RPC version"));
    writer.writeTextElement("td", JSON_PROTOCOL_VERSION);
    writer.writeEndElement(); // tr

    writer.writeStartElement("tr");
    writer.writeTextElement("th", QCoreApplication::translate("main", "Language"));
    writer.writeTextElement("td", GuhCore::instance()->configuration()->locale().name() + " (" + GuhCore::instance()->configuration()->locale().nativeCountryName() + " - " + GuhCore::instance()->configuration()->locale().nativeLanguageName() + ")");
    writer.writeEndElement(); // tr

    writer.writeStartElement("tr");
    writer.writeTextElement("th", QCoreApplication::translate("main", "Timezone"));
    writer.writeTextElement("td", QString::fromUtf8(GuhCore::instance()->configuration()->timeZone()));
    writer.writeEndElement(); // tr

    writer.writeStartElement("tr");
    writer.writeTextElement("th", QCoreApplication::translate("main", "Server UUID"));
    writer.writeTextElement("td", GuhCore::instance()->configuration()->serverUuid().toString());
    writer.writeEndElement(); // tr

    writer.writeStartElement("tr");
    writer.writeTextElement("th", QCoreApplication::translate("main", "Settings path"));
    writer.writeTextElement("td", GuhSettings::settingsPath());
    writer.writeEndElement(); // tr

    writer.writeStartElement("tr");
    writer.writeTextElement("th", QCoreApplication::translate("main", "Translations path"));
    writer.writeTextElement("td", GuhSettings(GuhSettings::SettingsRoleGlobal).translationsPath());
    writer.writeEndElement(); // tr

    writer.writeStartElement("tr");
    writer.writeTextElement("th", QCoreApplication::translate("main", "Log database"));
    writer.writeTextElement("td", GuhSettings(GuhSettings::SettingsRoleGlobal).logPath());
    writer.writeEndElement(); // tr

    for (int i = 0; i < GuhCore::instance()->deviceManager()->pluginSearchDirs().count(); i++) {
        writer.writeStartElement("tr");
        writer.writeEndElement(); // tr

        if (i == 0) {
            writer.writeTextElement("th", QCoreApplication::translate("main", "Plugin paths"));
        } else {
            writer.writeTextElement("th", "");
        }
        writer.writeTextElement("td", QFileInfo(GuhCore::instance()->deviceManager()->pluginSearchDirs().at(i)).absoluteFilePath());
    }

    writer.writeEndElement(); // table


    // Downloads section
    writer.writeEmptyElement("hr");
    writer.writeTextElement("h2", QCoreApplication::translate("main", "Downloads"));
    writer.writeEmptyElement("hr");

    // Logs download section
    writer.writeTextElement("h3", QCoreApplication::translate("main", "Logs"));

    writer.writeStartElement("table");
    writer.writeAttribute("border", "1");


    writer.writeStartElement("tr");
    writer.writeTextElement("th", QCoreApplication::translate("main", "Log database"));
    writer.writeTextElement("td", GuhSettings::logPath());
    writer.writeStartElement("td");
    writer.writeStartElement("form");
    writer.writeAttribute("method", "get");
    writer.writeAttribute("action", "/debug/logdb.sql");
    writer.writeStartElement("button");
    writer.writeAttribute("type", "submit");
    writer.writeCharacters(QCoreApplication::translate("main", "Download"));
    writer.writeEndElement(); // button
    writer.writeEndElement(); // form
    writer.writeEndElement(); // td
    writer.writeEndElement(); // tr

    writer.writeStartElement("tr");
    writer.writeTextElement("th", QCoreApplication::translate("main", "System logs"));
    writer.writeTextElement("td", "/var/log/syslog");
    writer.writeStartElement("td");
    writer.writeStartElement("form");
    writer.writeAttribute("method", "get");
    writer.writeAttribute("action", "/debug/syslog");
    writer.writeStartElement("button");
    writer.writeAttribute("type", "submit");
    writer.writeCharacters(QCoreApplication::translate("main", "Download"));
    writer.writeEndElement(); // button
    writer.writeEndElement(); // form
    writer.writeEndElement(); // td
    writer.writeEndElement(); // tr

    // TODO: offer logfile download if specified in the command line options

    writer.writeEndElement(); // table


    // Settings download section
    writer.writeTextElement("h3", QCoreApplication::translate("main", "Settings"));

    writer.writeStartElement("table");
    writer.writeAttribute("border", "1");

    writer.writeStartElement("tr");
    writer.writeTextElement("th", QCoreApplication::translate("main", "Guhd settings"));
    writer.writeTextElement("td", GuhSettings(GuhSettings::SettingsRoleGlobal).fileName());
    writer.writeStartElement("td");
    writer.writeStartElement("form");
    writer.writeAttribute("method", "get");
    writer.writeAttribute("action", "/debug/settings/guhd");
    writer.writeStartElement("button");
    writer.writeAttribute("type", "submit");
    writer.writeCharacters(QCoreApplication::translate("main", "Download"));
    writer.writeEndElement(); // button
    writer.writeEndElement(); // form
    writer.writeEndElement(); // td
    writer.writeEndElement(); // tr

    writer.writeStartElement("tr");
    writer.writeTextElement("th", QCoreApplication::translate("main", "Device settings"));
    writer.writeTextElement("td", GuhSettings(GuhSettings::SettingsRoleDevices).fileName());
    writer.writeStartElement("td");
    writer.writeStartElement("form");
    writer.writeAttribute("method", "get");
    writer.writeAttribute("action", "/debug/settings/devices");
    writer.writeStartElement("button");
    writer.writeAttribute("type", "submit");
    writer.writeCharacters(QCoreApplication::translate("main", "Download"));
    writer.writeEndElement(); // button
    writer.writeEndElement(); // form
    writer.writeEndElement(); // td
    writer.writeEndElement(); // tr

    writer.writeStartElement("tr");
    writer.writeTextElement("th", QCoreApplication::translate("main", "Device state settings"));
    writer.writeTextElement("td", GuhSettings(GuhSettings::SettingsRoleDeviceStates).fileName());
    writer.writeStartElement("td");
    writer.writeStartElement("form");
    writer.writeAttribute("method", "get");
    writer.writeAttribute("action", "/debug/settings/devicestates");
    writer.writeStartElement("button");
    writer.writeAttribute("type", "submit");
    writer.writeCharacters(QCoreApplication::translate("main", "Download"));
    writer.writeEndElement(); // button
    writer.writeEndElement(); // form
    writer.writeEndElement(); // td
    writer.writeEndElement(); // tr


    writer.writeStartElement("tr");
    writer.writeTextElement("th", QCoreApplication::translate("main", "Rules settings"));
    writer.writeTextElement("td", GuhSettings(GuhSettings::SettingsRoleRules).fileName());
    writer.writeStartElement("td");
    writer.writeStartElement("form");
    writer.writeAttribute("method", "get");
    writer.writeAttribute("action", "/debug/settings/rules");
    writer.writeStartElement("button");
    writer.writeAttribute("type", "submit");
    writer.writeCharacters(QCoreApplication::translate("main", "Download"));
    writer.writeEndElement(); // button
    writer.writeEndElement(); // form
    writer.writeEndElement(); // td
    writer.writeEndElement(); // tr

    writer.writeStartElement("tr");
    writer.writeTextElement("th", QCoreApplication::translate("main", "Plugins settings"));
    writer.writeTextElement("td", GuhSettings(GuhSettings::SettingsRolePlugins).fileName());
    writer.writeStartElement("td");
    writer.writeStartElement("form");
    writer.writeAttribute("method", "get");
    writer.writeAttribute("action", "/debug/settings/plugins");
    writer.writeStartElement("button");
    writer.writeAttribute("type", "submit");
    writer.writeCharacters(QCoreApplication::translate("main", "Download"));
    writer.writeEndElement(); // button
    writer.writeEndElement(); // form
    writer.writeEndElement(); // td
    writer.writeEndElement(); // tr

    writer.writeEndElement(); // table

    writer.writeEmptyElement("hr");

    writer.writeStartElement("footer");
    writer.writeTextElement("p", QString("Copyright %1 2018 guh GmbH.").arg(QChar(0xA9)));
    writer.writeTextElement("p", QCoreApplication::translate("main", "Released under the GNU GENERAL PUBLIC LICENSE Version 2."));
    writer.writeEndElement(); // footer

    writer.writeEndElement(); // body

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
    writer.writeAttribute("lang", GuhCore::instance()->configuration()->locale().name());

    writer.writeStartElement("head");

    writer.writeEmptyElement("meta");
    writer.writeAttribute("http-equiv", "Content-Type");
    writer.writeAttribute("content", "text/html; charset=utf-8");
    writer.writeTextElement("title", QCoreApplication::translate("main", "Debug nymea"));

    writer.writeEndElement(); // head

    writer.writeStartElement("body");

    writer.writeTextElement("h1", QCoreApplication::translate("main", "Error") + QString(" %1").arg(static_cast<int>(statusCode)));
    writer.writeEmptyElement("hr");

    writer.writeTextElement("p", errorMessage);

    writer.writeEmptyElement("hr");

    writer.writeStartElement("footer");
    writer.writeTextElement("p", QString("Copyright %1 2018 guh GmbH.").arg(QChar(0xA9)));
    writer.writeTextElement("p", QCoreApplication::translate("main", "Released under the GNU GENERAL PUBLIC LICENSE Version 2."));
    writer.writeEndElement(); // footer

    writer.writeEndElement(); // body

    writer.writeEndElement(); // html

    return data;
}

QByteArray DebugServerHandler::loadResourceFile(const QString &resourceFileName)
{
    QFile resourceFile(QString(":%1").arg(resourceFileName));
    if (!resourceFile.open(QFile::ReadOnly | QFile::Text)) {
        qCWarning(dcWebServer()) << "Could not open resource file" << resourceFile.fileName();
        return QByteArray();
    }

    QTextStream inputStream(&resourceFile);
    return inputStream.readAll().toUtf8();
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
    QByteArray data = loadResourceFile(resourceFileName);

    // Create reply for resource file
    HttpReply *reply = RestResource::createSuccessReply();
    reply->setPayload(data);

    // Check content type
    if (resourceFileName.endsWith(".css")) {
        reply->setHeader(HttpReply::ContentTypeHeader, "text/css; charset=\"utf-8\";");
    } else if (resourceFileName.endsWith(".svg")) {
        reply->setHeader(HttpReply::ContentTypeHeader, "image/svg+xml; charset=\"utf-8\";");
    }

    return reply;
}


HttpReply *DebugServerHandler::processDebugRequest(const QString &requestPath)
{
    qCDebug(dcWebServer()) << "Debug request for" << requestPath;

    // Check if debug page request
    if (requestPath == "/debug" || requestPath == "/debug/") {
        qCDebug(dcWebServer()) << "Create debug interface page";
        // Fallback default debug page
        HttpReply *reply = RestResource::createSuccessReply();
        reply->setHeader(HttpReply::ContentTypeHeader, "text/html");
        reply->setPayload(createDebugXmlDocument());
        return reply;
    }

    // Check if this is a logdb requested
    if (requestPath.startsWith("/debug/logdb.sql")) {
        qCDebug(dcWebServer()) << "Loading" << GuhSettings::logPath();
        QFile logDatabaseFile(GuhSettings::logPath());
        if (!logDatabaseFile.exists()) {
            qCWarning(dcWebServer()) << "Could not read log database file for debug download" << GuhSettings::logPath() << "file does not exist.";
            HttpReply *reply = RestResource::createErrorReply(HttpReply::NotFound);
            reply->setHeader(HttpReply::ContentTypeHeader, "text/html");
            reply->setPayload(createErrorXmlDocument(HttpReply::NotFound, QCoreApplication::translate("main", "Could not find file") + " " + logDatabaseFile.fileName()));
            return reply;
        }

        if (!logDatabaseFile.open(QFile::ReadOnly)) {
            qCWarning(dcWebServer()) << "Could not read log database file for debug download" << GuhSettings::logPath();
            HttpReply *reply = RestResource::createErrorReply(HttpReply::Forbidden);
            reply->setHeader(HttpReply::ContentTypeHeader, "text/html");
            reply->setPayload(createErrorXmlDocument(HttpReply::NotFound, QCoreApplication::translate("main", "Could not open file") + " " + logDatabaseFile.fileName()));
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
        qCDebug(dcWebServer()) << "Loading" << syslogFileName;
        QFile syslogFile(syslogFileName);
        if (!syslogFile.exists()) {
            qCWarning(dcWebServer()) << "Could not read log database file for debug download" << syslogFileName << "file does not exist.";
            HttpReply *reply = RestResource::createErrorReply(HttpReply::NotFound);
            reply->setHeader(HttpReply::ContentTypeHeader, "text/html");
            reply->setPayload(createErrorXmlDocument(HttpReply::NotFound, QCoreApplication::translate("main", "Could not find file") + " " + syslogFileName));
            return reply;
        }

        if (!syslogFile.open(QFile::ReadOnly)) {
            qCWarning(dcWebServer()) << "Could not read syslog file for debug download" << syslogFileName;
            HttpReply *reply = RestResource::createErrorReply(HttpReply::Forbidden);
            reply->setHeader(HttpReply::ContentTypeHeader, "text/html");
            reply->setPayload(createErrorXmlDocument(HttpReply::NotFound, QCoreApplication::translate("main", "Could not open file") + " " + syslogFileName));
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
            QString settingsFileName = GuhSettings(GuhSettings::SettingsRoleDevices).fileName();
            qCDebug(dcWebServer()) << "Loading" << settingsFileName;
            QFile settingsFile(settingsFileName);
            if (!settingsFile.exists()) {
                qCWarning(dcWebServer()) << "Could not read file for debug download" << settingsFileName << "file does not exist.";
                HttpReply *reply = RestResource::createErrorReply(HttpReply::NotFound);
                reply->setHeader(HttpReply::ContentTypeHeader, "text/html");
                reply->setPayload(createErrorXmlDocument(HttpReply::NotFound, QCoreApplication::translate("main", "Could not find file") + " " + settingsFileName));
                return reply;
            }

            if (!settingsFile.open(QFile::ReadOnly)) {
                qCWarning(dcWebServer()) << "Could not read file for debug download" << settingsFileName;
                HttpReply *reply = RestResource::createErrorReply(HttpReply::Forbidden);
                reply->setHeader(HttpReply::ContentTypeHeader, "text/html");
                reply->setPayload(createErrorXmlDocument(HttpReply::NotFound, QCoreApplication::translate("main", "Could not open file") + " " + settingsFileName));
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
            QString settingsFileName = GuhSettings(GuhSettings::SettingsRoleRules).fileName();
            qCDebug(dcWebServer()) << "Loading" << settingsFileName;
            QFile settingsFile(settingsFileName);
            if (!settingsFile.exists()) {
                qCWarning(dcWebServer()) << "Could not read file for debug download" << settingsFileName << "file does not exist.";
                HttpReply *reply = RestResource::createErrorReply(HttpReply::NotFound);
                reply->setHeader(HttpReply::ContentTypeHeader, "text/html");
                reply->setPayload(createErrorXmlDocument(HttpReply::NotFound, QCoreApplication::translate("main", "Could not find file") + " " + settingsFileName));
                return reply;
            }

            if (!settingsFile.open(QFile::ReadOnly)) {
                qCWarning(dcWebServer()) << "Could not read file for debug download" << settingsFileName;
                HttpReply *reply = RestResource::createErrorReply(HttpReply::Forbidden);
                reply->setHeader(HttpReply::ContentTypeHeader, "text/html");
                reply->setPayload(createErrorXmlDocument(HttpReply::NotFound, QCoreApplication::translate("main", "Could not open file") + " " + settingsFileName));
                return reply;
            }

            QByteArray settingsFileData = settingsFile.readAll();
            settingsFile.close();

            HttpReply *reply = RestResource::createSuccessReply();
            reply->setHeader(HttpReply::ContentTypeHeader, "text/plain");
            reply->setPayload(settingsFileData);
            return reply;
        }

        if (requestPath.startsWith("/debug/settings/guhd")) {
            QString settingsFileName = GuhSettings(GuhSettings::SettingsRoleGlobal).fileName();
            qCDebug(dcWebServer()) << "Loading" << settingsFileName;
            QFile settingsFile(settingsFileName);
            if (!settingsFile.exists()) {
                qCWarning(dcWebServer()) << "Could not read file for debug download" << settingsFileName << "file does not exist.";
                HttpReply *reply = RestResource::createErrorReply(HttpReply::NotFound);
                reply->setHeader(HttpReply::ContentTypeHeader, "text/html");
                reply->setPayload(createErrorXmlDocument(HttpReply::NotFound, QCoreApplication::translate("main", "Could not find file") + " " + settingsFileName));
                return reply;
            }

            if (!settingsFile.open(QFile::ReadOnly)) {
                qCWarning(dcWebServer()) << "Could not read file for debug download" << settingsFileName;
                HttpReply *reply = RestResource::createErrorReply(HttpReply::Forbidden);
                reply->setHeader(HttpReply::ContentTypeHeader, "text/html");
                reply->setPayload(createErrorXmlDocument(HttpReply::NotFound, QCoreApplication::translate("main", "Could not open file") + " " + settingsFileName));
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
            QString settingsFileName = GuhSettings(GuhSettings::SettingsRoleDeviceStates).fileName();
            qCDebug(dcWebServer()) << "Loading" << settingsFileName;
            QFile settingsFile(settingsFileName);
            if (!settingsFile.exists()) {
                qCWarning(dcWebServer()) << "Could not read file for debug download" << settingsFileName << "file does not exist.";
                HttpReply *reply = RestResource::createErrorReply(HttpReply::NotFound);
                reply->setHeader(HttpReply::ContentTypeHeader, "text/html");
                reply->setPayload(createErrorXmlDocument(HttpReply::NotFound, QCoreApplication::translate("main", "Could not find file") + " " + settingsFileName));
                return reply;
            }

            if (!settingsFile.open(QFile::ReadOnly)) {
                qCWarning(dcWebServer()) << "Could not read file for debug download" << settingsFileName;
                HttpReply *reply = RestResource::createErrorReply(HttpReply::Forbidden);
                reply->setHeader(HttpReply::ContentTypeHeader, "text/html");
                reply->setPayload(createErrorXmlDocument(HttpReply::NotFound, QCoreApplication::translate("main", "Could not open file") + " " + settingsFileName));
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
            QString settingsFileName = GuhSettings(GuhSettings::SettingsRolePlugins).fileName();
            qCDebug(dcWebServer()) << "Loading" << settingsFileName;
            QFile settingsFile(settingsFileName);
            if (!settingsFile.exists()) {
                qCWarning(dcWebServer()) << "Could not read file for debug download" << settingsFileName << "file does not exist.";
                HttpReply *reply = RestResource::createErrorReply(HttpReply::NotFound);
                reply->setHeader(HttpReply::ContentTypeHeader, "text/html");
                reply->setPayload(createErrorXmlDocument(HttpReply::NotFound, QCoreApplication::translate("main", "Could not find file") + " " + settingsFileName));
                return reply;
            }

            if (!settingsFile.open(QFile::ReadOnly)) {
                qCWarning(dcWebServer()) << "Could not read file for debug download" << settingsFileName;
                HttpReply *reply = RestResource::createErrorReply(HttpReply::Forbidden);
                reply->setHeader(HttpReply::ContentTypeHeader, "text/html");
                reply->setPayload(createErrorXmlDocument(HttpReply::NotFound, QCoreApplication::translate("main", "Could not open file") + " " + settingsFileName));
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

    // Check if this is a resource file request
    if (resourceFileExits(requestPath)) {
        return processDebugFileRequest(requestPath);
    }

    // If nothing matches, redirect to /debug page
    qCWarning(dcWebServer()) << "Resource for debug interface not found. Redirecting to /debug";
    HttpReply *reply = RestResource::createErrorReply(HttpReply::PermanentRedirect);
    reply->setHeader(HttpReply::LocationHeader, "/debug");
    return reply;
}

}
