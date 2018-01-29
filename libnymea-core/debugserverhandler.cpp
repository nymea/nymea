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
#include "nymeasettings.h"
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
    writer.writeComment("Auto generated html page from nymea server");
    writer.writeStartElement("html");
    writer.writeAttribute("lang", GuhCore::instance()->configuration()->locale().name());

    // Head
    writer.writeStartElement("head");

    writer.writeEmptyElement("meta");
    writer.writeAttribute("http-equiv", "Content-Type");
    writer.writeAttribute("content", "text/html; charset=utf-8");

    writer.writeEmptyElement("link");
    writer.writeAttribute("rel", "stylesheet");
    writer.writeAttribute("href", "/debug/styles.css");

    //: The header title of the debug server interface
    writer.writeTextElement("title", QCoreApplication::translate("main", "Debug nymea"));

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
    writer.writeAttribute("class", "guh-main-logo");

    //: The main title of the debug server interface
    writer.writeCharacters(QCoreApplication::translate("main", "nymea debug interface"));
    writer.writeEndElement(); // h1
    writer.writeEndElement(); // div header

    // Body
    writer.writeStartElement("div");
    writer.writeAttribute("class", "body");

    //: The welcome message of the debug interface
    writer.writeTextElement("p", QCoreApplication::translate("main", "Welcome to the debug interface."));
    writer.writeTextElement("p", QCoreApplication::translate("main", "This debug interface was designed to provide an easy possibility to get helpful information about the running nymea server."));

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
    writer.writeCharacters(QCoreApplication::translate("main", "Be aware that this debug interface is a security risk and could offer access to sensible data."));
    writer.writeEndElement(); // div warning message
    writer.writeEndElement(); // div warning


    writer.writeEmptyElement("hr");

    // System information section
    //: The server information section of the debug interface
    writer.writeTextElement("h2", QCoreApplication::translate("main", "Server information"));
    writer.writeEmptyElement("hr");

    writer.writeStartElement("table");

    writer.writeStartElement("tr");
    //: The user name in the server infromation section of the debug interface
    writer.writeTextElement("th", QCoreApplication::translate("main", "User"));
    writer.writeTextElement("td", qgetenv("USER"));
    writer.writeEndElement(); // tr

    writer.writeStartElement("tr");
    //: The Qt build version description in the server infromation section of the debug interface
    writer.writeTextElement("th", QCoreApplication::translate("main", "Compiled with Qt version"));
    writer.writeTextElement("td", QT_VERSION_STR);
    writer.writeEndElement(); // tr

    writer.writeStartElement("tr");
    //: The Qt runtime version description in the server infromation section of the debug interface
    writer.writeTextElement("th", QCoreApplication::translate("main", "Qt runtime version"));
    writer.writeTextElement("td", qVersion());
    writer.writeEndElement(); // tr

    writer.writeStartElement("tr");
    //: The command description in the server infromation section of the debug interface
    writer.writeTextElement("th", QCoreApplication::translate("main", "Command"));
    writer.writeTextElement("td", QCoreApplication::arguments().join(' '));
    writer.writeEndElement(); // tr

    if (!qgetenv("SNAP").isEmpty()) {
        // Note: http://snapcraft.io/docs/reference/env

        writer.writeStartElement("tr");
        //: The snap name description in the server infromation section of the debug interface
        writer.writeTextElement("th", QCoreApplication::translate("main", "Snap name"));
        writer.writeTextElement("td", qgetenv("SNAP_NAME"));
        writer.writeEndElement(); // tr

        writer.writeStartElement("tr");
        //: The snap version description in the server infromation section of the debug interface
        writer.writeTextElement("th", QCoreApplication::translate("main", "Snap version"));
        writer.writeTextElement("td", qgetenv("SNAP_VERSION"));
        writer.writeEndElement(); // tr

        writer.writeStartElement("tr");
        //: The snap directory description in the server infromation section of the debug interface
        writer.writeTextElement("th", QCoreApplication::translate("main", "Snap directory"));
        writer.writeTextElement("td", qgetenv("SNAP"));
        writer.writeEndElement(); // tr

        writer.writeStartElement("tr");
        //: The snap application data description in the server infromation section of the debug interface
        writer.writeTextElement("th", QCoreApplication::translate("main", "Snap application data"));
        writer.writeTextElement("td", qgetenv("SNAP_DATA"));
        writer.writeEndElement(); // tr

        writer.writeStartElement("tr");
        //: The snap user data description in the server infromation section of the debug interface
        writer.writeTextElement("th", QCoreApplication::translate("main", "Snap user data"));
        writer.writeTextElement("td", qgetenv("SNAP_USER_DATA"));
        writer.writeEndElement(); // tr

        writer.writeStartElement("tr");
        //: The snap common data description in the server infromation section of the debug interface
        writer.writeTextElement("th", QCoreApplication::translate("main", "Snap common data"));
        writer.writeTextElement("td", qgetenv("SNAP_COMMON"));
        writer.writeEndElement(); // tr
    }


    writer.writeStartElement("tr");
    //: The server name description in the server infromation section of the debug interface
    writer.writeTextElement("th", QCoreApplication::translate("main", "Server name"));
    writer.writeTextElement("td", GuhCore::instance()->configuration()->serverName());
    writer.writeEndElement(); // tr

    writer.writeStartElement("tr");
    //: The server version description in the server infromation section of the debug interface
    writer.writeTextElement("th", QCoreApplication::translate("main", "Server version"));
    writer.writeTextElement("td", NYMEA_VERSION_STRING);
    writer.writeEndElement(); // tr

    writer.writeStartElement("tr");
    //: The API version description in the server infromation section of the debug interface
    writer.writeTextElement("th", QCoreApplication::translate("main", "JSON-RPC version"));
    writer.writeTextElement("td", JSON_PROTOCOL_VERSION);
    writer.writeEndElement(); // tr

    writer.writeStartElement("tr");
    //: The language description in the server infromation section of the debug interface
    writer.writeTextElement("th", QCoreApplication::translate("main", "Language"));
    writer.writeTextElement("td", GuhCore::instance()->configuration()->locale().name() + " (" + GuhCore::instance()->configuration()->locale().nativeCountryName() + " - " + GuhCore::instance()->configuration()->locale().nativeLanguageName() + ")");
    writer.writeEndElement(); // tr

    writer.writeStartElement("tr");
    //: The timezone description in the server infromation section of the debug interface
    writer.writeTextElement("th", QCoreApplication::translate("main", "Timezone"));
    writer.writeTextElement("td", QString::fromUtf8(GuhCore::instance()->configuration()->timeZone()));
    writer.writeEndElement(); // tr

    writer.writeStartElement("tr");
    //: The server id description in the server infromation section of the debug interface
    writer.writeTextElement("th", QCoreApplication::translate("main", "Server UUID"));
    writer.writeTextElement("td", GuhCore::instance()->configuration()->serverUuid().toString());
    writer.writeEndElement(); // tr

    writer.writeStartElement("tr");
    //: The settings path description in the server infromation section of the debug interface
    writer.writeTextElement("th", QCoreApplication::translate("main", "Settings path"));
    writer.writeTextElement("td", NymeaSettings::settingsPath());
    writer.writeEndElement(); // tr

    writer.writeStartElement("tr");
    //: The translation path description in the server infromation section of the debug interface
    writer.writeTextElement("th", QCoreApplication::translate("main", "Translations path"));
    writer.writeTextElement("td", NymeaSettings(NymeaSettings::SettingsRoleGlobal).translationsPath());
    writer.writeEndElement(); // tr

    writer.writeStartElement("tr");
    //: The log database path description in the server infromation section of the debug interface
    writer.writeTextElement("th", QCoreApplication::translate("main", "Log database"));
    writer.writeTextElement("td", NymeaSettings(NymeaSettings::SettingsRoleGlobal).logPath());
    writer.writeEndElement(); // tr

    for (int i = 0; i < GuhCore::instance()->deviceManager()->pluginSearchDirs().count(); i++) {
        writer.writeStartElement("tr");
        writer.writeEndElement(); // tr

        if (i == 0) {
            //: The plugins path description in the server infromation section of the debug interface
            writer.writeTextElement("th", QCoreApplication::translate("main", "Plugin paths"));
        } else {
            writer.writeTextElement("th", "");
        }
        writer.writeTextElement("td", QFileInfo(GuhCore::instance()->deviceManager()->pluginSearchDirs().at(i)).absoluteFilePath());
    }

    writer.writeEndElement(); // table


    // Downloads section
    writer.writeEmptyElement("hr");
    //: The downloads section of the debug interface
    writer.writeTextElement("h2", QCoreApplication::translate("main", "Downloads"));

    // Logs download section
    writer.writeEmptyElement("hr");
    //: The download logs section of the debug interface
    writer.writeTextElement("h3", QCoreApplication::translate("main", "Logs"));
    writer.writeEmptyElement("hr");

    // Download row
    writer.writeStartElement("div");
    writer.writeAttribute("class", "download-row");

    writer.writeStartElement("div");
    writer.writeAttribute("class", "download-name-column");
    //: The log databse download description of the debug interface
    writer.writeTextElement("p", QCoreApplication::translate("main", "Log database"));
    writer.writeEndElement(); // div download-name-column

    writer.writeStartElement("div");
    writer.writeAttribute("class", "download-path-column");
    writer.writeTextElement("p", NymeaSettings::logPath());
    writer.writeEndElement(); // div download-path-column

    writer.writeStartElement("div");
    writer.writeAttribute("class", "download-button-column");
    writer.writeStartElement("form");
    writer.writeAttribute("class", "download-button");
    writer.writeAttribute("method", "get");
    writer.writeAttribute("action", "/debug/logdb.sql");
    writer.writeStartElement("button");
    writer.writeAttribute("class", "button");
    writer.writeAttribute("type", "submit");
    //: The download button description of the debug interface
    writer.writeCharacters(QCoreApplication::translate("main", "Download"));
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
    writer.writeTextElement("p", QCoreApplication::translate("main", "System logs"));
    writer.writeEndElement(); // div download-name-column

    writer.writeStartElement("div");
    writer.writeAttribute("class", "download-path-column");
    writer.writeTextElement("p", "/var/log/syslog");
    writer.writeEndElement(); // div download-path-column

    writer.writeStartElement("div");
    writer.writeAttribute("class", "download-button-column");
    writer.writeStartElement("form");
    writer.writeAttribute("class", "download-button");
    writer.writeAttribute("method", "get");
    writer.writeAttribute("action", "/debug/syslog");
    writer.writeStartElement("button");
    writer.writeAttribute("class", "button");
    writer.writeAttribute("type", "submit");
    writer.writeCharacters(QCoreApplication::translate("main", "Download"));
    writer.writeEndElement(); // button
    writer.writeEndElement(); // form
    writer.writeEndElement(); // div download-button-column

    writer.writeEndElement(); // div download-row


    // Settings download section
    writer.writeEmptyElement("hr");
    //: The settings download section title of the debug interface
    writer.writeTextElement("h3", QCoreApplication::translate("main", "Settings"));
    writer.writeEmptyElement("hr");

    // Download row
    writer.writeStartElement("div");
    writer.writeAttribute("class", "download-row");

    writer.writeStartElement("div");
    writer.writeAttribute("class", "download-name-column");
    //: The nymead settings download description of the debug interface
    writer.writeTextElement("p", QCoreApplication::translate("main", "nymead settings"));
    writer.writeEndElement(); // div download-name-column

    writer.writeStartElement("div");
    writer.writeAttribute("class", "download-path-column");
    writer.writeTextElement("p", NymeaSettings(NymeaSettings::SettingsRoleGlobal).fileName());
    writer.writeEndElement(); // div download-path-column

    writer.writeStartElement("div");
    writer.writeAttribute("class", "download-button-column");
    writer.writeStartElement("form");
    writer.writeAttribute("class", "download-button");
    writer.writeAttribute("method", "get");
    writer.writeAttribute("action", "/debug/settings/nymead");
    writer.writeStartElement("button");
    writer.writeAttribute("class", "button");
    writer.writeAttribute("type", "submit");
    writer.writeCharacters(QCoreApplication::translate("main", "Download"));
    writer.writeEndElement(); // button
    writer.writeEndElement(); // form
    writer.writeEndElement(); // div download-button-column

    writer.writeEndElement(); // div download-row


    // Download row
    writer.writeStartElement("div");
    writer.writeAttribute("class", "download-row");

    writer.writeStartElement("div");
    writer.writeAttribute("class", "download-name-column");
    //: The device settings download description of the debug interface
    writer.writeTextElement("p", QCoreApplication::translate("main", "Device settings"));
    writer.writeEndElement(); // div download-name-column

    writer.writeStartElement("div");
    writer.writeAttribute("class", "download-path-column");
    writer.writeTextElement("p", NymeaSettings(NymeaSettings::SettingsRoleDevices).fileName());
    writer.writeEndElement(); // div download-path-column

    writer.writeStartElement("div");
    writer.writeAttribute("class", "download-button-column");
    writer.writeStartElement("form");
    writer.writeAttribute("class", "download-button");
    writer.writeAttribute("method", "get");
    writer.writeAttribute("action", "/debug/settings/devices");
    writer.writeStartElement("button");
    writer.writeAttribute("class", "button");
    writer.writeAttribute("type", "submit");
    writer.writeCharacters(QCoreApplication::translate("main", "Download"));
    writer.writeEndElement(); // button
    writer.writeEndElement(); // form
    writer.writeEndElement(); // div download-button-column

    writer.writeEndElement(); // div download-row


    // Download row
    writer.writeStartElement("div");
    writer.writeAttribute("class", "download-row");

    writer.writeStartElement("div");
    writer.writeAttribute("class", "download-name-column");
    //: The device states settings download description of the debug interface
    writer.writeTextElement("p", QCoreApplication::translate("main", "Device states settings"));
    writer.writeEndElement(); // div download-name-column

    writer.writeStartElement("div");
    writer.writeAttribute("class", "download-path-column");
    writer.writeTextElement("p", NymeaSettings(NymeaSettings::SettingsRoleDeviceStates).fileName());
    writer.writeEndElement(); // div download-path-column

    writer.writeStartElement("div");
    writer.writeAttribute("class", "download-button-column");
    writer.writeStartElement("form");
    writer.writeAttribute("class", "download-button");
    writer.writeAttribute("method", "get");
    writer.writeAttribute("action", "/debug/settings/devicestates");
    writer.writeStartElement("button");
    writer.writeAttribute("class", "button");
    writer.writeAttribute("type", "submit");
    writer.writeCharacters(QCoreApplication::translate("main", "Download"));
    writer.writeEndElement(); // button
    writer.writeEndElement(); // form
    writer.writeEndElement(); // div download-button-column

    writer.writeEndElement(); // div download-row


    // Download row
    writer.writeStartElement("div");
    writer.writeAttribute("class", "download-row");

    writer.writeStartElement("div");
    writer.writeAttribute("class", "download-name-column");
    //: The rules settings download description of the debug interface
    writer.writeTextElement("p", QCoreApplication::translate("main", "Rules settings"));
    writer.writeEndElement(); // div download-name-column

    writer.writeStartElement("div");
    writer.writeAttribute("class", "download-path-column");
    writer.writeTextElement("p", NymeaSettings(NymeaSettings::SettingsRoleRules).fileName());
    writer.writeEndElement(); // div download-path-column

    writer.writeStartElement("div");
    writer.writeAttribute("class", "download-button-column");
    writer.writeStartElement("form");
    writer.writeAttribute("class", "download-button");
    writer.writeAttribute("method", "get");
    writer.writeAttribute("action", "/debug/settings/rules");
    writer.writeStartElement("button");
    writer.writeAttribute("class", "button");
    writer.writeAttribute("type", "submit");
    writer.writeCharacters(QCoreApplication::translate("main", "Download"));
    writer.writeEndElement(); // button
    writer.writeEndElement(); // form
    writer.writeEndElement(); // div download-button-column

    writer.writeEndElement(); // div download-row


    // Download row
    writer.writeStartElement("div");
    writer.writeAttribute("class", "download-row");

    writer.writeStartElement("div");
    writer.writeAttribute("class", "download-name-column");
    //: The plugins settings download description of the debug interface
    writer.writeTextElement("p", QCoreApplication::translate("main", "Plugins settings"));
    writer.writeEndElement(); // div download-name-column

    writer.writeStartElement("div");
    writer.writeAttribute("class", "download-path-column");
    writer.writeTextElement("p", NymeaSettings(NymeaSettings::SettingsRolePlugins).fileName());
    writer.writeEndElement(); // div download-path-column

    writer.writeStartElement("div");
    writer.writeAttribute("class", "download-button-column");
    writer.writeStartElement("form");
    writer.writeAttribute("class", "download-button");
    writer.writeAttribute("method", "get");
    writer.writeAttribute("action", "/debug/settings/plugins");
    writer.writeStartElement("button");
    writer.writeAttribute("class", "button");
    writer.writeAttribute("type", "submit");
    writer.writeCharacters(QCoreApplication::translate("main", "Download"));
    writer.writeEndElement(); // button
    writer.writeEndElement(); // form
    writer.writeEndElement(); // div download-button-column

    writer.writeEndElement(); // div download-row

    writer.writeEndElement(); // div body


    // Footer
    writer.writeStartElement("div");
    writer.writeAttribute("class", "footer");
    writer.writeTextElement("p", QString("Copyright %1 2018 guh GmbH.").arg(QChar(0xA9)));
    //: The footer license note of the debug interface
    writer.writeTextElement("p", QCoreApplication::translate("main", "Released under the GNU GENERAL PUBLIC LICENSE Version 2."));
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
    writer.writeAttribute("lang", GuhCore::instance()->configuration()->locale().name());

    // Head
    writer.writeStartElement("head");

    writer.writeEmptyElement("meta");
    writer.writeAttribute("http-equiv", "Content-Type");
    writer.writeAttribute("content", "text/html; charset=utf-8");

    writer.writeEmptyElement("link");
    writer.writeAttribute("rel", "stylesheet");
    writer.writeAttribute("href", "/debug/styles.css");

    writer.writeTextElement("title", QCoreApplication::translate("main", "Debug nymea"));

    writer.writeEndElement(); // head

    // Container
    writer.writeStartElement("div");
    writer.writeAttribute("class", "container");

    // Header
    writer.writeStartElement("div");
    writer.writeAttribute("class", "header");
    writer.writeTextElement("p", " ");
    //: The HTTP error message of the debug interface. The %1 represents the error code ie.e 404
    writer.writeTextElement("h1", QCoreApplication::translate("main", "Error  %1").arg(static_cast<int>(statusCode)));
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
    writer.writeTextElement("p", QCoreApplication::translate("main", "Released under the GNU GENERAL PUBLIC LICENSE Version 2."));
    writer.writeEndElement(); // div footer

    writer.writeEndElement(); // div container

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
        qCDebug(dcWebServer()) << "Loading" << NymeaSettings::logPath();
        QFile logDatabaseFile(NymeaSettings::logPath());
        if (!logDatabaseFile.exists()) {
            qCWarning(dcWebServer()) << "Could not read log database file for debug download" << NymeaSettings::logPath() << "file does not exist.";
            HttpReply *reply = RestResource::createErrorReply(HttpReply::NotFound);
            reply->setHeader(HttpReply::ContentTypeHeader, "text/html");
            //: The HTTP error message of the debug interface. The %1 represents the file name.
            reply->setPayload(createErrorXmlDocument(HttpReply::NotFound, QCoreApplication::translate("main", "Could not find file \"%1\".").arg(logDatabaseFile.fileName())));
            return reply;
        }

        if (!logDatabaseFile.open(QFile::ReadOnly)) {
            qCWarning(dcWebServer()) << "Could not read log database file for debug download" << NymeaSettings::logPath();
            HttpReply *reply = RestResource::createErrorReply(HttpReply::Forbidden);
            reply->setHeader(HttpReply::ContentTypeHeader, "text/html");
            //: The HTTP error message of the debug interface. The %1 represents the file name.
            reply->setPayload(createErrorXmlDocument(HttpReply::NotFound, QCoreApplication::translate("main", "Could not open file \"%1\".").arg(logDatabaseFile.fileName())));
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
            reply->setPayload(createErrorXmlDocument(HttpReply::NotFound, QCoreApplication::translate("main", "Could not find file \"%1\".").arg(syslogFileName)));
            return reply;
        }

        if (!syslogFile.open(QFile::ReadOnly)) {
            qCWarning(dcWebServer()) << "Could not read syslog file for debug download" << syslogFileName;
            HttpReply *reply = RestResource::createErrorReply(HttpReply::Forbidden);
            reply->setHeader(HttpReply::ContentTypeHeader, "text/html");
            reply->setPayload(createErrorXmlDocument(HttpReply::NotFound, QCoreApplication::translate("main", "Could not open file \"%1\".").arg(syslogFileName)));
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
            qCDebug(dcWebServer()) << "Loading" << settingsFileName;
            QFile settingsFile(settingsFileName);
            if (!settingsFile.exists()) {
                qCWarning(dcWebServer()) << "Could not read file for debug download" << settingsFileName << "file does not exist.";
                HttpReply *reply = RestResource::createErrorReply(HttpReply::NotFound);
                reply->setHeader(HttpReply::ContentTypeHeader, "text/html");
                reply->setPayload(createErrorXmlDocument(HttpReply::NotFound, QCoreApplication::translate("main", "Could not find file \"%1\".").arg(settingsFileName)));
                return reply;
            }

            if (!settingsFile.open(QFile::ReadOnly)) {
                qCWarning(dcWebServer()) << "Could not read file for debug download" << settingsFileName;
                HttpReply *reply = RestResource::createErrorReply(HttpReply::Forbidden);
                reply->setHeader(HttpReply::ContentTypeHeader, "text/html");
                reply->setPayload(createErrorXmlDocument(HttpReply::NotFound, QCoreApplication::translate("main", "Could not open file \"%1\".").arg(settingsFileName)));
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
            qCDebug(dcWebServer()) << "Loading" << settingsFileName;
            QFile settingsFile(settingsFileName);
            if (!settingsFile.exists()) {
                qCWarning(dcWebServer()) << "Could not read file for debug download" << settingsFileName << "file does not exist.";
                HttpReply *reply = RestResource::createErrorReply(HttpReply::NotFound);
                reply->setHeader(HttpReply::ContentTypeHeader, "text/html");
                reply->setPayload(createErrorXmlDocument(HttpReply::NotFound, QCoreApplication::translate("main", "Could not find file \"%1\".").arg(settingsFileName)));
                return reply;
            }

            if (!settingsFile.open(QFile::ReadOnly)) {
                qCWarning(dcWebServer()) << "Could not read file for debug download" << settingsFileName;
                HttpReply *reply = RestResource::createErrorReply(HttpReply::Forbidden);
                reply->setHeader(HttpReply::ContentTypeHeader, "text/html");
                reply->setPayload(createErrorXmlDocument(HttpReply::NotFound, QCoreApplication::translate("main", "Could not open file \"%1\".").arg(settingsFileName)));
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
            qCDebug(dcWebServer()) << "Loading" << settingsFileName;
            QFile settingsFile(settingsFileName);
            if (!settingsFile.exists()) {
                qCWarning(dcWebServer()) << "Could not read file for debug download" << settingsFileName << "file does not exist.";
                HttpReply *reply = RestResource::createErrorReply(HttpReply::NotFound);
                reply->setHeader(HttpReply::ContentTypeHeader, "text/html");
                reply->setPayload(createErrorXmlDocument(HttpReply::NotFound, QCoreApplication::translate("main", "Could not find file \"%1\".").arg(settingsFileName)));
                return reply;
            }

            if (!settingsFile.open(QFile::ReadOnly)) {
                qCWarning(dcWebServer()) << "Could not read file for debug download" << settingsFileName;
                HttpReply *reply = RestResource::createErrorReply(HttpReply::Forbidden);
                reply->setHeader(HttpReply::ContentTypeHeader, "text/html");
                reply->setPayload(createErrorXmlDocument(HttpReply::NotFound, QCoreApplication::translate("main", "Could not open file \"%1\".").arg(settingsFileName)));
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
            qCDebug(dcWebServer()) << "Loading" << settingsFileName;
            QFile settingsFile(settingsFileName);
            if (!settingsFile.exists()) {
                qCWarning(dcWebServer()) << "Could not read file for debug download" << settingsFileName << "file does not exist.";
                HttpReply *reply = RestResource::createErrorReply(HttpReply::NotFound);
                reply->setHeader(HttpReply::ContentTypeHeader, "text/html");
                reply->setPayload(createErrorXmlDocument(HttpReply::NotFound, QCoreApplication::translate("main", "Could not find file \"%1\".").arg(settingsFileName)));
                return reply;
            }

            if (!settingsFile.open(QFile::ReadOnly)) {
                qCWarning(dcWebServer()) << "Could not read file for debug download" << settingsFileName;
                HttpReply *reply = RestResource::createErrorReply(HttpReply::Forbidden);
                reply->setHeader(HttpReply::ContentTypeHeader, "text/html");
                reply->setPayload(createErrorXmlDocument(HttpReply::NotFound, QCoreApplication::translate("main", "Could not open file \"%1\".").arg(settingsFileName)));
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
            qCDebug(dcWebServer()) << "Loading" << settingsFileName;
            QFile settingsFile(settingsFileName);
            if (!settingsFile.exists()) {
                qCWarning(dcWebServer()) << "Could not read file for debug download" << settingsFileName << "file does not exist.";
                HttpReply *reply = RestResource::createErrorReply(HttpReply::NotFound);
                reply->setHeader(HttpReply::ContentTypeHeader, "text/html");
                reply->setPayload(createErrorXmlDocument(HttpReply::NotFound, QCoreApplication::translate("main", "Could not find file \"%1\".").arg(settingsFileName)));
                return reply;
            }

            if (!settingsFile.open(QFile::ReadOnly)) {
                qCWarning(dcWebServer()) << "Could not read file for debug download" << settingsFileName;
                HttpReply *reply = RestResource::createErrorReply(HttpReply::Forbidden);
                reply->setHeader(HttpReply::ContentTypeHeader, "text/html");
                reply->setPayload(createErrorXmlDocument(HttpReply::NotFound, QCoreApplication::translate("main", "Could not open file \"%1\".").arg(settingsFileName)));
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
