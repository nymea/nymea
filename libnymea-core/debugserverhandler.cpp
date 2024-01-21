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

#include "nymeacore.h"
#include "servers/httprequest.h"
#include "servers/httpreply.h"
#include "nymeasettings.h"
#include "loggingcategories.h"
#include "debugserverhandler.h"
#include "nymeaconfiguration.h"
#include "stdio.h"
#include "version.h"

#include <QXmlStreamWriter>
#include <QCoreApplication>
#include <QMessageLogger>
#include <QJsonDocument>
#include <QXmlStreamWriter>
#include <QCoreApplication>
#include <QFileInfo>
#include <QWebSocket>
#include <QPair>
#include <QHostInfo>


namespace nymeaserver {

QList<QWebSocket*> DebugServerHandler::s_websocketClients;
QMutex DebugServerHandler::s_loggingMutex;

DebugServerHandler::DebugServerHandler(QObject *parent) :
    QObject(parent)
{
    connect(NymeaCore::instance()->configuration(), &NymeaConfiguration::debugServerEnabledChanged, this, &DebugServerHandler::onDebugServerEnabledChanged);
    onDebugServerEnabledChanged(NymeaCore::instance()->configuration()->debugServerEnabled());
}

HttpReply *DebugServerHandler::processDebugRequest(const QString &requestPath, const QUrlQuery &requestQuery)
{
    qCDebug(dcDebugServer()) << "Debug request for" << requestPath;

    // Check if debug page request
    if (requestPath == "/debug" || requestPath == "/debug/") {
        qCDebug(dcDebugServer()) << "Create debug interface page";
        // Fallback default debug page
        HttpReply *reply = HttpReply::createSuccessReply();
        reply->setHeader(HttpReply::ContentTypeHeader, "text/html");
        reply->setPayload(createDebugXmlDocument());
        return reply;
    }

    // Check if this is a syslog requested
    if (requestPath.startsWith("/debug/syslog")) {
        QString syslogFileName = "/var/log/syslog";
        qCDebug(dcDebugServer()) << "Loading" << syslogFileName;
        QFile syslogFile(syslogFileName);
        if (!syslogFile.exists()) {
            qCWarning(dcDebugServer()) << "Could not read log database file for debug download" << syslogFileName << "file does not exist.";
            HttpReply *reply = HttpReply::createErrorReply(HttpReply::NotFound);
            reply->setHeader(HttpReply::ContentTypeHeader, "text/html");
            reply->setPayload(createErrorXmlDocument(HttpReply::NotFound, tr("Could not find file \"%1\".").arg(syslogFileName)));
            return reply;
        }

        if (!syslogFile.open(QFile::ReadOnly)) {
            qCWarning(dcDebugServer()) << "Could not read syslog file for debug download" << syslogFileName;
            HttpReply *reply = HttpReply::createErrorReply(HttpReply::Forbidden);
            reply->setHeader(HttpReply::ContentTypeHeader, "text/html");
            reply->setPayload(createErrorXmlDocument(HttpReply::NotFound, tr("Could not open file \"%1\".").arg(syslogFileName)));
            return reply;
        }

        QByteArray syslogFileData = syslogFile.readAll();
        syslogFile.close();

        HttpReply *reply = HttpReply::createSuccessReply();
        reply->setHeader(HttpReply::ContentTypeHeader, "text/plain");
        reply->setPayload(syslogFileData);
        return reply;
    }

    // Check if this is a settings request
    if (requestPath.startsWith("/debug/settings")) {
        if (requestPath.startsWith("/debug/settings/things")) {
            QString settingsFileName = NymeaSettings(NymeaSettings::SettingsRoleThings).fileName();
            qCDebug(dcDebugServer()) << "Loading" << settingsFileName;
            QFile settingsFile(settingsFileName);
            if (!settingsFile.exists()) {
                qCWarning(dcDebugServer()) << "Could not read file for debug download" << settingsFileName << "file does not exist.";
                HttpReply *reply = HttpReply::createErrorReply(HttpReply::NotFound);
                reply->setHeader(HttpReply::ContentTypeHeader, "text/html");
                reply->setPayload(createErrorXmlDocument(HttpReply::NotFound, tr("Could not find file \"%1\".").arg(settingsFileName)));
                return reply;
            }

            if (!settingsFile.open(QFile::ReadOnly)) {
                qCWarning(dcDebugServer()) << "Could not read file for debug download" << settingsFileName;
                HttpReply *reply = HttpReply::createErrorReply(HttpReply::Forbidden);
                reply->setHeader(HttpReply::ContentTypeHeader, "text/html");
                reply->setPayload(createErrorXmlDocument(HttpReply::NotFound, tr("Could not open file \"%1\".").arg(settingsFileName)));
                return reply;
            }

            QByteArray settingsFileData = settingsFile.readAll();
            settingsFile.close();

            HttpReply *reply = HttpReply::createSuccessReply();
            reply->setHeader(HttpReply::ContentTypeHeader, "text/plain");
            reply->setPayload(settingsFileData);
            return reply;
        }

        if (requestPath.startsWith("/debug/settings/rules")) {
            QString settingsFileName = NymeaSettings(NymeaSettings::SettingsRoleRules).fileName();
            qCDebug(dcDebugServer()) << "Loading" << settingsFileName;
            QFile settingsFile(settingsFileName);
            if (!settingsFile.exists()) {
                qCWarning(dcDebugServer()) << "Could not read file for debug download" << settingsFileName << "file does not exist.";
                HttpReply *reply = HttpReply::createErrorReply(HttpReply::NotFound);
                reply->setHeader(HttpReply::ContentTypeHeader, "text/html");
                reply->setPayload(createErrorXmlDocument(HttpReply::NotFound, tr("Could not find file \"%1\".").arg(settingsFileName)));
                return reply;
            }

            if (!settingsFile.open(QFile::ReadOnly)) {
                qCWarning(dcDebugServer()) << "Could not read file for debug download" << settingsFileName;
                HttpReply *reply = HttpReply::createErrorReply(HttpReply::Forbidden);
                reply->setHeader(HttpReply::ContentTypeHeader, "text/html");
                reply->setPayload(createErrorXmlDocument(HttpReply::NotFound, tr("Could not open file \"%1\".").arg(settingsFileName)));
                return reply;
            }

            QByteArray settingsFileData = settingsFile.readAll();
            settingsFile.close();

            HttpReply *reply = HttpReply::createSuccessReply();
            reply->setHeader(HttpReply::ContentTypeHeader, "text/plain");
            reply->setPayload(settingsFileData);
            return reply;
        }

        if (requestPath.startsWith("/debug/settings/nymead")) {
            QString settingsFileName = NymeaSettings(NymeaSettings::SettingsRoleGlobal).fileName();
            qCDebug(dcDebugServer()) << "Loading" << settingsFileName;
            QFile settingsFile(settingsFileName);
            if (!settingsFile.exists()) {
                qCWarning(dcDebugServer()) << "Could not read file for debug download" << settingsFileName << "file does not exist.";
                HttpReply *reply = HttpReply::createErrorReply(HttpReply::NotFound);
                reply->setHeader(HttpReply::ContentTypeHeader, "text/html");
                reply->setPayload(createErrorXmlDocument(HttpReply::NotFound, tr("Could not find file \"%1\".").arg(settingsFileName)));
                return reply;
            }

            if (!settingsFile.open(QFile::ReadOnly)) {
                qCWarning(dcDebugServer()) << "Could not read file for debug download" << settingsFileName;
                HttpReply *reply = HttpReply::createErrorReply(HttpReply::Forbidden);
                reply->setHeader(HttpReply::ContentTypeHeader, "text/html");
                reply->setPayload(createErrorXmlDocument(HttpReply::NotFound, tr("Could not open file \"%1\".").arg(settingsFileName)));
                return reply;
            }

            QByteArray settingsFileData = settingsFile.readAll();
            settingsFile.close();

            HttpReply *reply = HttpReply::createSuccessReply();
            reply->setHeader(HttpReply::ContentTypeHeader, "text/plain");
            reply->setPayload(settingsFileData);
            return reply;
        }


        if (requestPath.startsWith("/debug/settings/plugins")) {
            QString settingsFileName = NymeaSettings(NymeaSettings::SettingsRolePlugins).fileName();
            qCDebug(dcDebugServer()) << "Loading" << settingsFileName;
            QFile settingsFile(settingsFileName);
            if (!settingsFile.exists()) {
                qCWarning(dcDebugServer()) << "Could not read file for debug download" << settingsFileName << "file does not exist.";
                HttpReply *reply = HttpReply::createErrorReply(HttpReply::NotFound);
                reply->setHeader(HttpReply::ContentTypeHeader, "text/html");
                reply->setPayload(createErrorXmlDocument(HttpReply::NotFound, tr("Could not find file \"%1\".").arg(settingsFileName)));
                return reply;
            }

            if (!settingsFile.open(QFile::ReadOnly)) {
                qCWarning(dcDebugServer()) << "Could not read file for debug download" << settingsFileName;
                HttpReply *reply = HttpReply::createErrorReply(HttpReply::Forbidden);
                reply->setHeader(HttpReply::ContentTypeHeader, "text/html");
                reply->setPayload(createErrorXmlDocument(HttpReply::NotFound, tr("Could not open file \"%1\".").arg(settingsFileName)));
                return reply;
            }

            QByteArray settingsFileData = settingsFile.readAll();
            settingsFile.close();

            HttpReply *reply = HttpReply::createSuccessReply();
            reply->setHeader(HttpReply::ContentTypeHeader, "text/plain");
            reply->setPayload(settingsFileData);
            return reply;
        }

        if (requestPath.startsWith("/debug/settings/tags")) {
            QString settingsFileName = NymeaSettings(NymeaSettings::SettingsRoleTags).fileName();
            qCDebug(dcDebugServer()) << "Loading" << settingsFileName;
            QFile settingsFile(settingsFileName);
            if (!settingsFile.exists()) {
                qCWarning(dcDebugServer()) << "Could not read file for debug download" << settingsFileName << "file does not exist.";
                HttpReply *reply = HttpReply::createErrorReply(HttpReply::NotFound);
                reply->setHeader(HttpReply::ContentTypeHeader, "text/html");
                reply->setPayload(createErrorXmlDocument(HttpReply::NotFound, tr("Could not find file \"%1\".").arg(settingsFileName)));
                return reply;
            }

            if (!settingsFile.open(QFile::ReadOnly)) {
                qCWarning(dcDebugServer()) << "Could not read file for debug download" << settingsFileName;
                HttpReply *reply = HttpReply::createErrorReply(HttpReply::Forbidden);
                reply->setHeader(HttpReply::ContentTypeHeader, "text/html");
                reply->setPayload(createErrorXmlDocument(HttpReply::NotFound, tr("Could not open file \"%1\".").arg(settingsFileName)));
                return reply;
            }

            QByteArray settingsFileData = settingsFile.readAll();
            settingsFile.close();

            HttpReply *reply = HttpReply::createSuccessReply();
            reply->setHeader(HttpReply::ContentTypeHeader, "text/plain");
            reply->setPayload(settingsFileData);
            return reply;
        }
    }

    if (requestPath.startsWith("/debug/settings/mqttpolicies")) {
        QString settingsFileName = NymeaSettings(NymeaSettings::SettingsRoleMqttPolicies).fileName();
        qCDebug(dcDebugServer()) << "Loading" << settingsFileName;
        QFile settingsFile(settingsFileName);
        if (!settingsFile.exists()) {
            qCWarning(dcDebugServer()) << "Could not read file for debug download" << settingsFileName << "file does not exist.";
            HttpReply *reply = HttpReply::createErrorReply(HttpReply::NotFound);
            reply->setHeader(HttpReply::ContentTypeHeader, "text/html");
            reply->setPayload(createErrorXmlDocument(HttpReply::NotFound, tr("Could not find file \"%1\".").arg(settingsFileName)));
            return reply;
        }

        if (!settingsFile.open(QFile::ReadOnly)) {
            qCWarning(dcDebugServer()) << "Could not read file for debug download" << settingsFileName;
            HttpReply *reply = HttpReply::createErrorReply(HttpReply::Forbidden);
            reply->setHeader(HttpReply::ContentTypeHeader, "text/html");
            reply->setPayload(createErrorXmlDocument(HttpReply::NotFound, tr("Could not open file \"%1\".").arg(settingsFileName)));
            return reply;
        }

        QByteArray settingsFileData = settingsFile.readAll();
        settingsFile.close();

        HttpReply *reply = HttpReply::createSuccessReply();
        reply->setHeader(HttpReply::ContentTypeHeader, "text/plain");
        reply->setPayload(settingsFileData);
        return reply;
    }

    if (requestPath.startsWith("/debug/settings/ioconnections")) {
        QString settingsFileName = NymeaSettings(NymeaSettings::SettingsRoleIOConnections).fileName();
        qCDebug(dcDebugServer()) << "Loading" << settingsFileName;
        QFile settingsFile(settingsFileName);
        if (!settingsFile.exists()) {
            qCWarning(dcDebugServer()) << "Could not read file for debug download" << settingsFileName << "file does not exist.";
            HttpReply *reply = HttpReply::createErrorReply(HttpReply::NotFound);
            reply->setHeader(HttpReply::ContentTypeHeader, "text/html");
            reply->setPayload(createErrorXmlDocument(HttpReply::NotFound, tr("Could not find file \"%1\".").arg(settingsFileName)));
            return reply;
        }

        if (!settingsFile.open(QFile::ReadOnly)) {
            qCWarning(dcDebugServer()) << "Could not read file for debug download" << settingsFileName;
            HttpReply *reply = HttpReply::createErrorReply(HttpReply::Forbidden);
            reply->setHeader(HttpReply::ContentTypeHeader, "text/html");
            reply->setPayload(createErrorXmlDocument(HttpReply::NotFound, tr("Could not open file \"%1\".").arg(settingsFileName)));
            return reply;
        }

        QByteArray settingsFileData = settingsFile.readAll();
        settingsFile.close();

        HttpReply *reply = HttpReply::createSuccessReply();
        reply->setHeader(HttpReply::ContentTypeHeader, "text/plain");
        reply->setPayload(settingsFileData);
        return reply;
    }

    if (requestPath.startsWith("/debug/ping")) {
        // Only one ping process should run
        if (m_pingProcess || m_pingReply)
            return HttpReply::createErrorReply(HttpReply::InternalServerError);

        qCDebug(dcDebugServer()) << "Start ping nymea.io process";
        m_pingProcess = new QProcess(this);
        m_pingProcess->setProcessChannelMode(QProcess::MergedChannels);
        connect(m_pingProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(onPingProcessFinished(int,QProcess::ExitStatus)));
        m_pingProcess->start("ping", { "-c", "4", "nymea.io" } );

        m_pingReply = HttpReply::createAsyncReply();
        connect(m_pingReply, &HttpReply::finished, this, [this](){
            m_pingReply = nullptr;
        });
        return m_pingReply;
    }

    if (requestPath.startsWith("/debug/dig")) {
        // Only one dig process should run
        if (m_digProcess || m_digReply)
            return HttpReply::createErrorReply(HttpReply::InternalServerError);

        qCDebug(dcDebugServer()) << "Start dig nymea.io process";
        m_digProcess = new QProcess(this);
        m_digProcess->setProcessChannelMode(QProcess::MergedChannels);
        connect(m_digProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(onDigProcessFinished(int,QProcess::ExitStatus)));
        m_digProcess->start("dig", { "nymea.io" } );

        m_digReply = HttpReply::createAsyncReply();
        connect(m_digReply, &HttpReply::finished, this, [this](){
            m_digReply = nullptr;
        });
        return m_digReply;
    }

    if (requestPath.startsWith("/debug/tracepath")) {
        // Only one tracepath process should run
        if (m_tracePathProcess || m_tracePathReply)
            return HttpReply::createErrorReply(HttpReply::InternalServerError);

        qCDebug(dcDebugServer()) << "Start tracepath nymea.io process";
        m_tracePathProcess = new QProcess(this);
        m_tracePathProcess->setProcessChannelMode(QProcess::MergedChannels);
        connect(m_tracePathProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(onTracePathProcessFinished(int,QProcess::ExitStatus)));
        m_tracePathProcess->start("tracepath", { "nymea.io" } );

        m_tracePathReply = HttpReply::createAsyncReply();
        connect(m_tracePathReply, &HttpReply::finished, this, [this](){
            m_tracePathReply = nullptr;
        });
        return m_tracePathReply;
    }

    if (requestPath.startsWith("/debug/logging-categories")) {

        if (requestQuery.isEmpty()) {
            // Return the list of debug category settings
            NymeaSettings settings(NymeaSettings::SettingsRoleGlobal);
            settings.beginGroup("LoggingRules");

            qCDebug(dcDebugServer()) << "Request logging categories list";
            QVariantMap dataMap;
            QVariantMap loggingCategories;
            foreach (const QString &loggingCategory, NymeaCore::loggingFilters()) {
                QString level = "critical";
                if (settings.value(QString("%1.warning").arg(loggingCategory), true).toBool()) {
                    level = "warning";
                }
                if (settings.value(QString("%1.info").arg(loggingCategory), true).toBool()) {
                    level = "info";
                }
                if (settings.value(QString("%1.debug").arg(loggingCategory), false).toBool()) {
                    level = "debug";
                }
                loggingCategories.insert(loggingCategory, level);
            }
            dataMap.insert("loggingCategories", loggingCategories);
            QVariantMap loggingCategoriesPlugins;
            foreach (const QString &loggingCategory, NymeaCore::loggingFiltersPlugins()) {
                QString level = "critical";
                if (settings.value(QString("%1.warning").arg(loggingCategory), true).toBool()) {
                    level = "warning";
                }
                if (settings.value(QString("%1.info").arg(loggingCategory), true).toBool()) {
                    level = "info";
                }
                if (settings.value(QString("%1.debug").arg(loggingCategory), false).toBool()) {
                    level = "debug";
                }
                loggingCategoriesPlugins.insert(loggingCategory, level);
            }
            dataMap.insert("loggingCategoriesPlugins", loggingCategoriesPlugins);

            settings.endGroup();

            HttpReply *reply = HttpReply::createSuccessReply();
            reply->setPayload(QJsonDocument::fromVariant(dataMap).toJson(QJsonDocument::Indented));
            return reply;
        } else {
            NymeaSettings settings(NymeaSettings::SettingsRoleGlobal);
            settings.beginGroup("LoggingRules");
            for (int i = 0; i < requestQuery.queryItems().count(); i++) {
                QString category = requestQuery.queryItems().at(i).first;
                if (!NymeaCore::loggingFilters().contains(category) && !NymeaCore::loggingFiltersPlugins().contains(category)) {
                    qCWarning(dcDebugServer()) << "Invalid logging category in request query" << requestQuery.toString() << category;
                    continue;
                }

                QString level = QVariant(requestQuery.queryItems().at(i).second).toString();
                qCDebug(dcDebugServer()) << "Logging category" << category << level;
                if (level == "debug") {
                    settings.setValue(QString("%1.debug").arg(category), true);
                    settings.setValue(QString("%1.info").arg(category), true);
                    settings.setValue(QString("%1.warning").arg(category), true);
                } else if (level == "info") {
                    settings.setValue(QString("%1.debug").arg(category), false);
                    settings.setValue(QString("%1.info").arg(category), true);
                    settings.setValue(QString("%1.warning").arg(category), true);
                } else if (level == "warning"){
                    settings.setValue(QString("%1.debug").arg(category), false);
                    settings.setValue(QString("%1.info").arg(category), false);
                    settings.setValue(QString("%1.warning").arg(category), true);
                } else {
                    settings.setValue(QString("%1.debug").arg(category), false);
                    settings.setValue(QString("%1.info").arg(category), false);
                    settings.setValue(QString("%1.warning").arg(category), false);
                }
            }

            // Update logging filter rules according to the nw settings
            QStringList loggingRules;
            loggingRules << "*.debug=false";
            // Load the rules from nymead.conf file and append them to the rules
            foreach (const QString &category, settings.childKeys()) {
                loggingRules << QString("%1=%2").arg(category).arg(settings.value(category, "false").toString());
            }
            settings.endGroup();
            QLoggingCategory::setFilterRules(loggingRules.join('\n'));

            return HttpReply::createSuccessReply();
        }
    }

    if (requestPath.startsWith("/debug/report")) {

        // The client can poll this url in order to get information about the current report generating process.
        // If there is currently no report generated, start generating it and inform client that there is a report on the way (204)
        // If there is already a report generation in progress, inform the client that it's not ready yet (204)
        // If the report is ready, return information of the file and the client will start downloading it (202 and file informations).

        // Check if download request or generate request
        if (requestQuery.hasQueryItem("filename")) {
            QString fileName = requestQuery.queryItemValue("filename");
            qCDebug(dcDebugServer()) << "Report download request for" << fileName;

            if (!m_debugReportGenerator) {
                qCWarning(dcDebugServer()) << "There is currently no debug report generator. The requested file does not exist.";
                return HttpReply::createErrorReply(HttpReply::NotFound);
            }

            if (m_debugReportGenerator->reportFileName() != fileName) {
                qCWarning(dcDebugServer()) << "The requested file is not the file from the current debug report generator" << m_debugReportGenerator->reportFileName() << "!=" << fileName;
                return HttpReply::createErrorReply(HttpReply::NotFound);

            }

            // Everything looks good, send the requested debug report
            HttpReply *downloadReportReply = HttpReply::createSuccessReply();
            downloadReportReply->setPayload(m_debugReportGenerator->reportFileData());
            downloadReportReply->setHeader(HttpReply::ContentTypeHeader, "application/tar+gzip;");
            return downloadReportReply;
        } else {
            // Generate or poll request
            if (!m_debugReportGenerator) {
                qCDebug(dcDebugServer()) << "Create new debug report generator and start generating report...";
                m_debugReportGenerator = new DebugReportGenerator(this);
                connect(m_debugReportGenerator, &DebugReportGenerator::finished, this, &DebugServerHandler::onDebugReportGeneratorFinished);
                connect(m_debugReportGenerator, &DebugReportGenerator::timeout, this, &DebugServerHandler::onDebugReportGeneratorTimeout);
                m_debugReportGenerator->generateReport();
                // Note: no content will bring the client to poll this report
                return HttpReply::createErrorReply(HttpReply::NoContent);
            } else {
                // There is a running generator, check if the report is ready
                if (!m_debugReportGenerator->isReady()) {
                    qCDebug(dcDebugServer()) << "Report is not ready yet";
                    // Note: no content tells the client the report is not ready yet
                    return HttpReply::createErrorReply(HttpReply::NoContent);
                } else {
                    if (m_debugReportGenerator->isValid()) {
                        // Success, the debug report is ready and valid
                        QVariantMap reportInformation;
                        reportInformation.insert("fileName", m_debugReportGenerator->reportFileName());
                        reportInformation.insert("fileSize", m_debugReportGenerator->reportFileData().size());
                        reportInformation.insert("md5sum", m_debugReportGenerator->md5Sum());

                        HttpReply * httpReply = HttpReply::createSuccessReply();
                        httpReply->setHttpStatusCode(HttpReply::Ok);
                        httpReply->setHeader(HttpReply::ContentTypeHeader, "application/json; charset=\"utf-8\";");
                        httpReply->setPayload(QJsonDocument::fromVariant(reportInformation).toJson(QJsonDocument::Indented));
                        return httpReply;
                    } else {
                        qCWarning(dcDebugServer()) << "The debug report generator finished with error.";
                        m_debugReportGenerator->deleteLater();
                        m_debugReportGenerator = nullptr;
                        return HttpReply::createErrorReply(HttpReply::InternalServerError);
                    }
                }
            }
        }
    }

    // Check if this is a resource file request
    if (resourceFileExits(requestPath)) {
        return processDebugFileRequest(requestPath);
    }

    // If nothing matches, redirect to /debug page
    qCWarning(dcDebugServer()) << "Resource for debug interface not found. Redirecting to /debug";
    HttpReply *reply = HttpReply::createErrorReply(HttpReply::PermanentRedirect);
    reply->setHeader(HttpReply::LocationHeader, "/debug");
    return reply;
}

void DebugServerHandler::logMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &message)
{
    QString finalMessage;
    switch (type) {
    case QtDebugMsg:
        finalMessage = QString(" I | %1: %2\n").arg(context.category).arg(message);
        break;
    case QtInfoMsg:
        finalMessage = QString(" I | %1: %2\n").arg(context.category).arg(message);
        break;
    case QtWarningMsg:
        finalMessage = QString(" W | %1: %2\n").arg(context.category).arg(message);
        break;
    case QtCriticalMsg:
        finalMessage = QString(" C | %1: %2\n").arg(context.category).arg(message);
        break;
    case QtFatalMsg:
        finalMessage = QString(" F | %1: %2\n").arg(context.category).arg(message);
        break;
    }

    QMutexLocker locker(&s_loggingMutex);
    foreach (QWebSocket *client, s_websocketClients) {
        client->sendTextMessage(finalMessage);
    }
}

QByteArray DebugServerHandler::loadResourceData(const QString &resourceFileName)
{
    QFile resourceFile(QString(":%1").arg(resourceFileName));
    if (!resourceFile.open(QFile::ReadOnly)) {
        qCWarning(dcDebugServer()) << "Could not open resource file" << resourceFile.fileName();
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
    HttpReply *reply = HttpReply::createSuccessReply();
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

void DebugServerHandler::onDebugServerEnabledChanged(bool enabled)
{
    if (enabled) {
        m_websocketServer = new QWebSocketServer("Debug server", QWebSocketServer::NonSecureMode, this);
        connect(m_websocketServer, &QWebSocketServer::newConnection, this, &DebugServerHandler::onWebsocketClientConnected);
        if (!m_websocketServer->listen(QHostAddress::Any, 2626)) {
            qCWarning(dcDebugServer()) << "The debug server websocket interface could not listen on" << m_websocketServer->serverUrl().toString();
            m_websocketServer->deleteLater();
            m_websocketServer = nullptr;
            return;
        }

        qCDebug(dcDebugServer()) << "The debug server websocket interface has been started on" << m_websocketServer->serverUrl().toString();
    } else {
        if (m_websocketServer) {
            m_websocketServer->close();
            qCDebug(dcDebugServer()) << "The debug server websocket interface has been closed" << m_websocketServer->serverUrl().toString();
            m_websocketServer->deleteLater();
            m_websocketServer = nullptr;
        }
    }
}

void DebugServerHandler::onWebsocketClientConnected()
{
    QWebSocket *client = m_websocketServer->nextPendingConnection();

    if (s_websocketClients.isEmpty()) {
        qCDebug(dcDebugServer()) << "Install debug message handler for live logs.";
        //QLoggingCategory::setFilterRules("*.debug=true");
        nymeaInstallMessageHandler(&logMessageHandler);
    }

    s_websocketClients.append(client);
    qCDebug(dcDebugServer()) << "New websocket client connected:" << client->peerAddress().toString();

    connect(client, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onWebsocketClientError(QAbstractSocket::SocketError)));
    connect(client, &QWebSocket::disconnected, this, &DebugServerHandler::onWebsocketClientDisconnected);
}

void DebugServerHandler::onWebsocketClientDisconnected()
{
    QWebSocket *client = static_cast<QWebSocket *>(sender());
    qCDebug(dcDebugServer()) << "Websocket client disconnected" << client->peerAddress().toString();
    s_websocketClients.removeAll(client);
    client->deleteLater();

    if (s_websocketClients.isEmpty()) {
        qCDebug(dcDebugServer()) << "Uninstalling debug message handler for live logs.";
        nymeaUninstallMessageHandler(&logMessageHandler);
    }
}

void DebugServerHandler::onWebsocketClientError(QAbstractSocket::SocketError error)
{
    QWebSocket *client = static_cast<QWebSocket *>(sender());
    qCWarning(dcDebugServer()) << "Websocket client error" << client->peerAddress().toString() << error << client->errorString();
}

void DebugServerHandler::onPingProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qCDebug(dcDebugServer()) << "Ping process finished" << exitCode << exitStatus;
    QByteArray processOutput = m_pingProcess->readAll();
    qCDebug(dcDebugServer()) << "Ping output:" << Qt::endl << qUtf8Printable(processOutput);

    if (m_pingReply) {
        m_pingReply->setPayload(processOutput);
        m_pingReply->setHttpStatusCode(HttpReply::Ok);
        m_pingReply->finished();
        m_pingReply = nullptr;
    }

    m_pingProcess->deleteLater();
    m_pingProcess = nullptr;
}

void DebugServerHandler::onDigProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qCDebug(dcDebugServer()) << "Dig process finished" << exitCode << exitStatus;
    QByteArray processOutput = m_digProcess->readAll();
    qCDebug(dcDebugServer()) << "Dig output:" << Qt::endl << qUtf8Printable(processOutput);

    if (m_digReply) {
        m_digReply->setPayload(processOutput);
        m_digReply->setHttpStatusCode(HttpReply::Ok);
        m_digReply->finished();
        m_digReply = nullptr;
    }

    m_digProcess->deleteLater();
    m_digProcess = nullptr;
}

void DebugServerHandler::onTracePathProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qCDebug(dcDebugServer()) << "Tracepath process finished" << exitCode << exitStatus;
    QByteArray processOutput = m_tracePathProcess->readAll();
    qCDebug(dcDebugServer()) << "Tracepath output:" << Qt::endl << qUtf8Printable(processOutput);

    if (m_tracePathReply) {
        m_tracePathReply->setPayload(processOutput);
        m_tracePathReply->setHttpStatusCode(HttpReply::Ok);
        m_tracePathReply->finished();
        m_tracePathReply = nullptr;
    }

    m_tracePathProcess->deleteLater();
    m_tracePathProcess = nullptr;
}

void DebugServerHandler::onDebugReportGeneratorFinished(bool success)
{
    DebugReportGenerator *debugReportGenerator = static_cast<DebugReportGenerator *>(sender());
    qCDebug(dcDebugServer()) << "Report generation finished" << (success ? "successfully" : "with error") << debugReportGenerator->reportFileName();
}

void DebugServerHandler::onDebugReportGeneratorTimeout()
{
    qCWarning(dcDebugServer()) << "Debug report expired.";
    if (m_debugReportGenerator) {
        m_debugReportGenerator->deleteLater();
        m_debugReportGenerator = nullptr;
    }
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

    // Default favicons
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
    writer.writeAttribute("sizes", "96x96");
    writer.writeAttribute("href", "/debug/favicons/favicon-96x96.png");

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

    // Apple favicons
    writer.writeEmptyElement("link");
    writer.writeAttribute("rel", "apple-touch-icon-precomposed");
    writer.writeAttribute("sizes", "57x57");
    writer.writeAttribute("href", "/debug/favicons/apple-touch-icon-57x57.png");

    writer.writeEmptyElement("link");
    writer.writeAttribute("rel", "apple-touch-icon-precomposed");
    writer.writeAttribute("sizes", "60x60");
    writer.writeAttribute("href", "/debug/favicons/apple-touch-icon-60x60.png");

    writer.writeEmptyElement("link");
    writer.writeAttribute("rel", "apple-touch-icon-precomposed");
    writer.writeAttribute("sizes", "72x72");
    writer.writeAttribute("href", "/debug/favicons/apple-touch-icon-72x72.png");

    writer.writeEmptyElement("link");
    writer.writeAttribute("rel", "apple-touch-icon-precomposed");
    writer.writeAttribute("sizes", "76x76");
    writer.writeAttribute("href", "/debug/favicons/apple-touch-icon-76x76.png");

    writer.writeEmptyElement("link");
    writer.writeAttribute("rel", "apple-touch-icon-precomposed");
    writer.writeAttribute("sizes", "114x114");
    writer.writeAttribute("href", "/debug/favicons/apple-touch-icon-114x114.png");

    writer.writeEmptyElement("link");
    writer.writeAttribute("rel", "apple-touch-icon-precomposed");
    writer.writeAttribute("sizes", "120x120");
    writer.writeAttribute("href", "/debug/favicons/apple-touch-icon-144x144.png");

    writer.writeEmptyElement("link");
    writer.writeAttribute("rel", "apple-touch-icon-precomposed");
    writer.writeAttribute("sizes", "144x144");
    writer.writeAttribute("href", "/debug/favicons/apple-touch-icon-144x144.png");

    writer.writeEmptyElement("link");
    writer.writeAttribute("rel", "apple-touch-icon-precomposed");
    writer.writeAttribute("sizes", "152x152");
    writer.writeAttribute("href", "/debug/favicons/apple-touch-icon-152x152.png");

    // Microsoft favicons
    writer.writeEmptyElement("meta");
    writer.writeAttribute("name", "nymea");
    writer.writeAttribute("content", "&nbsp;");

    writer.writeEmptyElement("meta");
    writer.writeAttribute("name", "msapplication-TileColor");
    writer.writeAttribute("content", "#FFFFFF");

    writer.writeEmptyElement("meta");
    writer.writeAttribute("name", "msapplication-TileImage");
    writer.writeAttribute("content", "/debug/favicons/mstile-144x144.png");

    writer.writeEmptyElement("meta");
    writer.writeAttribute("name", "msapplication-square70x70logo");
    writer.writeAttribute("content", "/debug/favicons/mstile-70x70.png");

    writer.writeEmptyElement("meta");
    writer.writeAttribute("name", "msapplication-square150x150logo");
    writer.writeAttribute("content", "/debug/favicons/mstile-150x150.png");

    writer.writeEmptyElement("meta");
    writer.writeAttribute("name", "msapplication-wide310x150logo");
    writer.writeAttribute("content", "/debug/favicons/mstile-310x150.png");

    writer.writeEmptyElement("meta");
    writer.writeAttribute("name", "msapplication-square310x310logo");
    writer.writeAttribute("content", "/debug/favicons/mstile-310x310.png");

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

    // Tab bar
    writer.writeStartElement("div");
    writer.writeAttribute("class", "tab");

    writer.writeStartElement("button");
    writer.writeAttribute("class", "tablinks");
    writer.writeAttribute("id", "informationTabButton");
    writer.writeAttribute("onclick", "selectSection(event, 'information-section')");
    //: The name of the section tab in the debug server interface
    writer.writeCharacters(tr("Information"));
    writer.writeEndElement(); // button

    writer.writeStartElement("button");
    writer.writeAttribute("class", "tablinks");
    writer.writeAttribute("id", "downloadsTabButton");
    writer.writeAttribute("onclick", "selectSection(event, 'downloads-section')");
    //: The name of the section tab in the debug server interface
    writer.writeCharacters(tr("Downloads"));
    writer.writeEndElement(); // button

    writer.writeStartElement("button");
    writer.writeAttribute("class", "tablinks");
    writer.writeAttribute("id", "networkTabButton");
    writer.writeAttribute("onclick", "selectSection(event, 'network-section')");
    //: The name of the section tab in the debug server interface
    writer.writeCharacters(tr("Network"));
    writer.writeEndElement(); // button

    writer.writeStartElement("button");
    writer.writeAttribute("class", "tablinks");
    writer.writeAttribute("id", "logsTabButton");
    writer.writeAttribute("onclick", "selectSection(event, 'logs-section')");
    //: The name of the section tab in the debug server interface
    writer.writeCharacters(tr("Logs"));
    writer.writeEndElement(); // button

    writer.writeEndElement(); // tab

    // Body
    writer.writeStartElement("div");
    writer.writeAttribute("class", "body");

    // ---------------------------------------------------------------------------
    writer.writeStartElement("div");
    writer.writeAttribute("class", "tabcontent");
    writer.writeAttribute("id", "information-section");

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
    writer.writeCharacters(tr("Please note that this debug interface may allow accessing sensitive data about the nymea system and connected devices and services. It is recommended to disable it again when not needed any more."));
    writer.writeEndElement(); // div warning message
    writer.writeEndElement(); // div warning


    // Server information section
    writer.writeEmptyElement("hr");
    //: The server information section of the debug interface
    writer.writeTextElement("h2", tr("Server information"));
    writer.writeEmptyElement("hr");

    writer.writeStartElement("table");

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

    writer.writeStartElement("tr");
    //: The user name in the server infromation section of the debug interface
    writer.writeTextElement("th", tr("User"));
    writer.writeTextElement("td", qgetenv("USER"));
    writer.writeEndElement(); // tr

    writer.writeStartElement("tr");
    //: The command description in the server infromation section of the debug interface
    writer.writeTextElement("th", tr("Command"));
    writer.writeTextElement("td", QCoreApplication::arguments().join(' '));
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

    writer.writeEndElement(); // table


    // System information section
    writer.writeEmptyElement("hr");
    //: The system information section of the debug interface
    writer.writeTextElement("h2", tr("System information"));
    writer.writeEmptyElement("hr");

    writer.writeStartElement("table");

    writer.writeStartElement("tr");
    //: The command description in the server infromation section of the debug interface
    writer.writeTextElement("th", tr("Hostname"));
#if (QT_VERSION < QT_VERSION_CHECK(5, 6, 0))
    writer.writeTextElement("td", QHostInfo::localHostName());
#else
    writer.writeTextElement("td", QSysInfo::machineHostName());
#endif

    writer.writeEndElement(); // tr

    writer.writeStartElement("tr");
    //: The command description in the server infromation section of the debug interface
    writer.writeTextElement("th", tr("Architecture"));
    writer.writeTextElement("td", QSysInfo::currentCpuArchitecture());
    writer.writeEndElement(); // tr

    writer.writeStartElement("tr");
    //: The command description in the server infromation section of the debug interface
    writer.writeTextElement("th", tr("Kernel type"));
    writer.writeTextElement("td", QSysInfo::kernelType());
    writer.writeEndElement(); // tr

    writer.writeStartElement("tr");
    //: The command description in the server infromation section of the debug interface
    writer.writeTextElement("th", tr("Kernel version"));
    writer.writeTextElement("td", QSysInfo::kernelVersion());
    writer.writeEndElement(); // tr

    writer.writeStartElement("tr");
    //: The command description in the server infromation section of the debug interface
    writer.writeTextElement("th", tr("Product type"));
    writer.writeTextElement("td", QSysInfo::productType());
    writer.writeEndElement(); // tr

    writer.writeStartElement("tr");
    //: The command description in the server infromation section of the debug interface
    writer.writeTextElement("th", tr("Product version"));
    writer.writeTextElement("td", QSysInfo::productVersion());
    writer.writeEndElement(); // tr

    writer.writeEndElement(); // table

    // Generate report
    writer.writeEmptyElement("hr");
    //: In the server information section of the debug interface
    writer.writeTextElement("h2", tr("Generate report"));
    writer.writeEmptyElement("hr");

    writer.writeTextElement("p", tr("If you want to provide all the debug information to a developer, you can generate a report file, "
                                    "which contains all information needed for reproducing a system and get information about possible problems."));

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
    writer.writeCharacters(tr("Please note that the generated debug report may contain sensitive data about the nymea system and connected devices and services."));
    writer.writeEndElement(); // div warning message
    writer.writeEndElement(); // div warning

    // Generate report button
    writer.writeStartElement("button");
    writer.writeAttribute("class", "button");
    writer.writeAttribute("type", "button");
    writer.writeAttribute("id", "generateReportButton");
    writer.writeAttribute("onClick", "generateReport()");
    //: The generate debug report button text of the debug interface
    writer.writeCharacters(tr("Generate report file"));
    writer.writeEndElement(); // button

    // Logs output
    writer.writeStartElement("textarea");
    writer.writeAttribute("class", "console-textarea");
    writer.writeAttribute("id", "generateReportTextArea");
    writer.writeAttribute("readonly", "readonly");
    writer.writeAttribute("rows", "5");
    writer.writeCharacters("");
    writer.writeEndElement(); // textarea

    writer.writeEndElement(); // information-section

    // ---------------------------------------------------------------------------
    writer.writeStartElement("div");
    writer.writeAttribute("class", "tabcontent");
    writer.writeAttribute("id", "downloads-section");

    // Downloads section
    writer.writeEmptyElement("hr");
    //: The downloads section of the debug interface
    writer.writeTextElement("h2", tr("Downloads"));

    // Logs download section
    writer.writeEmptyElement("hr");
    //: The download logs section of the debug interface
    writer.writeTextElement("h3", tr("Logs"));
    writer.writeEmptyElement("hr");

    // Download row syslog
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
    writer.writeAttribute("onClick", "showFile('/debug/syslog')");
    writer.writeCharacters(tr("Show"));
    writer.writeEndElement(); // button
    writer.writeEndElement(); // form
    writer.writeEndElement(); // div show-button-column

    writer.writeEndElement(); // div download-row


    // Settings download section global
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
    writer.writeAttribute("onClick", "showFile('/debug/settings/nymead')");
    writer.writeCharacters(tr("Show"));
    writer.writeEndElement(); // button
    writer.writeEndElement(); // form
    writer.writeEndElement(); // div show-button-column

    writer.writeEndElement(); // div download-row


    // Download row things
    writer.writeStartElement("div");
    writer.writeAttribute("class", "download-row");

    writer.writeStartElement("div");
    writer.writeAttribute("class", "download-name-column");
    //: The thing settings download description of the debug interface
    writer.writeTextElement("p", tr("Thing settings"));
    writer.writeEndElement(); // div download-name-column

    writer.writeStartElement("div");
    writer.writeAttribute("class", "download-path-column");
    writer.writeTextElement("p", NymeaSettings(NymeaSettings::SettingsRoleThings).fileName());
    writer.writeEndElement(); // div download-path-column

    writer.writeStartElement("div");
    writer.writeAttribute("class", "download-button-column");
    writer.writeStartElement("form");
    writer.writeAttribute("class", "download-button");
    writer.writeStartElement("button");
    writer.writeAttribute("class", "button");
    writer.writeAttribute("type", "button");
    if (!QFile::exists(NymeaSettings(NymeaSettings::SettingsRoleThings).fileName())) {
        writer.writeAttribute("disabled", "disabled");
    }
    writer.writeAttribute("onClick", "downloadFile('/debug/settings/things', 'things.conf')");
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
    if (!QFile::exists(NymeaSettings(NymeaSettings::SettingsRoleThings).fileName())) {
        writer.writeAttribute("disabled", "true");
    }
    writer.writeAttribute("onClick", "showFile('/debug/settings/things')");
    writer.writeCharacters(tr("Show"));
    writer.writeEndElement(); // button
    writer.writeEndElement(); // form
    writer.writeEndElement(); // div show-button-column

    writer.writeEndElement(); // div download-row


    // Download row rules
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
    writer.writeAttribute("onClick", "showFile('/debug/settings/rules')");
    writer.writeCharacters(tr("Show"));
    writer.writeEndElement(); // button
    writer.writeEndElement(); // form
    writer.writeEndElement(); // div show-button-column

    writer.writeEndElement(); // div download-row


    // Download row plugins
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
    writer.writeAttribute("onClick", "showFile('/debug/settings/plugins')");
    writer.writeCharacters(tr("Show"));
    writer.writeEndElement(); // button
    writer.writeEndElement(); // form
    writer.writeEndElement(); // div show-button-column
    writer.writeEndElement(); // div download-row



    // Download row tags
    writer.writeStartElement("div");
    writer.writeAttribute("class", "download-row");

    writer.writeStartElement("div");
    writer.writeAttribute("class", "download-name-column");
    //: The tag settings download description of the debug interface
    writer.writeTextElement("p", tr("Tag settings"));
    writer.writeEndElement(); // div download-name-column

    writer.writeStartElement("div");
    writer.writeAttribute("class", "download-path-column");
    writer.writeTextElement("p", NymeaSettings(NymeaSettings::SettingsRoleTags).fileName());
    writer.writeEndElement(); // div download-path-column

    writer.writeStartElement("div");
    writer.writeAttribute("class", "download-button-column");
    writer.writeStartElement("form");
    writer.writeAttribute("class", "download-button");
    writer.writeStartElement("button");
    writer.writeAttribute("class", "button");
    writer.writeAttribute("type", "button");
    if (!QFile::exists(NymeaSettings(NymeaSettings::SettingsRoleTags).fileName())) {
        writer.writeAttribute("disabled", "true");
    }
    writer.writeAttribute("onClick", "downloadFile('/debug/settings/tags', 'tags.conf')");
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
    if (!QFile::exists(NymeaSettings(NymeaSettings::SettingsRoleTags).fileName())) {
        writer.writeAttribute("disabled", "true");
    }
    writer.writeAttribute("onClick", "showFile('/debug/settings/tags')");
    writer.writeCharacters(tr("Show"));
    writer.writeEndElement(); // button
    writer.writeEndElement(); // form
    writer.writeEndElement(); // div show-button-column
    writer.writeEndElement(); // div download-row


    // Download row MQTT policies
    writer.writeStartElement("div");
    writer.writeAttribute("class", "download-row");

    writer.writeStartElement("div");
    writer.writeAttribute("class", "download-name-column");
    //: The MQTT policies download description of the debug interface
    writer.writeTextElement("p", tr("MQTT policies"));
    writer.writeEndElement(); // div download-name-column

    writer.writeStartElement("div");
    writer.writeAttribute("class", "download-path-column");
    writer.writeTextElement("p", NymeaSettings(NymeaSettings::SettingsRoleMqttPolicies).fileName());
    writer.writeEndElement(); // div download-path-column

    writer.writeStartElement("div");
    writer.writeAttribute("class", "download-button-column");
    writer.writeStartElement("form");
    writer.writeAttribute("class", "download-button");
    writer.writeStartElement("button");
    writer.writeAttribute("class", "button");
    writer.writeAttribute("type", "button");
    if (!QFile::exists(NymeaSettings(NymeaSettings::SettingsRoleMqttPolicies).fileName())) {
        writer.writeAttribute("disabled", "true");
    }
    writer.writeAttribute("onClick", "downloadFile('/debug/settings/mqttpolicies', 'mqttpolicies.conf')");
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
    if (!QFile::exists(NymeaSettings(NymeaSettings::SettingsRoleMqttPolicies).fileName())) {
        writer.writeAttribute("disabled", "true");
    }
    writer.writeAttribute("onClick", "showFile('/debug/settings/mqttpolicies')");
    writer.writeCharacters(tr("Show"));
    writer.writeEndElement(); // button
    writer.writeEndElement(); // form
    writer.writeEndElement(); // div show-button-column

    writer.writeEndElement(); // div download-row


    // Download row IO connections
    writer.writeStartElement("div");
    writer.writeAttribute("class", "download-row");

    writer.writeStartElement("div");
    writer.writeAttribute("class", "download-name-column");
    //: The MQTT policies download description of the debug interface
    writer.writeTextElement("p", tr("IO Connections"));
    writer.writeEndElement(); // div download-name-column

    writer.writeStartElement("div");
    writer.writeAttribute("class", "download-path-column");
    writer.writeTextElement("p", NymeaSettings(NymeaSettings::SettingsRoleIOConnections).fileName());
    writer.writeEndElement(); // div download-path-column

    writer.writeStartElement("div");
    writer.writeAttribute("class", "download-button-column");
    writer.writeStartElement("form");
    writer.writeAttribute("class", "download-button");
    writer.writeStartElement("button");
    writer.writeAttribute("class", "button");
    writer.writeAttribute("type", "button");
    if (!QFile::exists(NymeaSettings(NymeaSettings::SettingsRoleMqttPolicies).fileName())) {
        writer.writeAttribute("disabled", "true");
    }
    writer.writeAttribute("onClick", "downloadFile('/debug/settings/ioconnections', 'ioconnections.conf')");
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
    if (!QFile::exists(NymeaSettings(NymeaSettings::SettingsRoleMqttPolicies).fileName())) {
        writer.writeAttribute("disabled", "true");
    }
    writer.writeAttribute("onClick", "showFile('/debug/settings/ioconnections')");
    writer.writeCharacters(tr("Show"));
    writer.writeEndElement(); // button
    writer.writeEndElement(); // form
    writer.writeEndElement(); // div show-button-column

    writer.writeEndElement(); // div download-row

    writer.writeEndElement(); // downloads-section


    // ---------------------------------------------------------------------------
    writer.writeStartElement("div");
    writer.writeAttribute("class", "tabcontent");
    writer.writeAttribute("id", "network-section");

    // Network section
    writer.writeStartElement("div");
    writer.writeAttribute("class", "network");
    writer.writeEmptyElement("hr");
    //: The network section of the debug interface
    writer.writeTextElement("h2", tr("Network"));

    //: The network section description of the debug interface
    writer.writeTextElement("p", tr("This section allows you to perform different network connectivity tests in order "
                                    "to find out if the device where nymea is running has full network connectivity."));

    // Ping section
    writer.writeEmptyElement("hr");
    //: The ping section of the debug interface
    writer.writeTextElement("h3", tr("Ping"));
    writer.writeEmptyElement("hr");

    writer.writeTextElement("p", tr("This test makes four ping attempts to the nymea.io server."));

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
    //: The DNS lookup section of the debug interface
    writer.writeTextElement("h3", tr("DNS lookup"));
    writer.writeEmptyElement("hr");

    writer.writeTextElement("p", tr("This test makes a dynamic name server lookup for nymea.io."));


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

    // Trace section
    writer.writeEmptyElement("hr");
    //: The trace section of the debug interface
    writer.writeTextElement("h3", tr("Trace path"));
    writer.writeEmptyElement("hr");

    writer.writeTextElement("p", tr("This test shows the trace path from the nymea device to the nymea.io server."));

    // Start tracepath button
    writer.writeStartElement("button");
    writer.writeAttribute("class", "button");
    writer.writeAttribute("type", "button");
    writer.writeAttribute("id", "tracePathButton");
    writer.writeAttribute("onClick", "startTracePathTest()");
    //: The trace path button text of the debug interface
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

    writer.writeEndElement(); // network-section


    // ---------------------------------------------------------------------------
    writer.writeStartElement("div");
    writer.writeAttribute("class", "tabcontent");
    writer.writeAttribute("id", "logs-section");

    // Logs stream
    writer.writeStartElement("div");
    writer.writeAttribute("class", "logstream");
    writer.writeEmptyElement("hr");
    //: The network section of the debug interface
    writer.writeTextElement("h2", tr("Server live logs"));
    writer.writeEmptyElement("hr");

    writer.writeTextElement("p", tr("This section allows you to see the live logs of the nymea server."));

    writer.writeStartElement("div");
    writer.writeAttribute("class", "log-buttons");

    // Toggle log button
    writer.writeStartElement("button");
    writer.writeAttribute("class", "button");
    writer.writeAttribute("type", "button");
    writer.writeAttribute("id", "toggleLogsButton");
    writer.writeAttribute("onClick", "toggleWebsocketConnection()");
    //: The connect button for the log stream of the debug interface
    writer.writeCharacters(tr("Start logs"));
    writer.writeEndElement(); // button

    // Copy log content button
    writer.writeStartElement("button");
    writer.writeAttribute("class", "button");
    writer.writeAttribute("type", "button");
    writer.writeAttribute("id", "copyLogsButton");
    writer.writeAttribute("onClick", "copyLogsContent()");
    writer.writeEmptyElement("img");
    writer.writeAttribute("class", "tool-image");
    writer.writeAttribute("src", "/debug/edit-copy.svg");
    writer.writeEndElement(); // button

    // Copy log content button
    writer.writeStartElement("button");
    writer.writeAttribute("class", "button");
    writer.writeAttribute("type", "button");
    writer.writeAttribute("id", "clearLogsButton");
    writer.writeAttribute("onClick", "clearLogsContent()");
    writer.writeEmptyElement("img");
    writer.writeAttribute("class", "tool-image");
    writer.writeAttribute("src", "/debug/delete.svg");
    writer.writeEndElement(); // button

    writer.writeEndElement(); // div log-buttons


    // Logs output
    writer.writeStartElement("textarea");
    writer.writeAttribute("class", "console-textarea");
    writer.writeAttribute("id", "logsTextArea");
    writer.writeAttribute("readonly", "readonly");
    writer.writeAttribute("rows", "30");
    writer.writeCharacters("");
    writer.writeEndElement(); // textarea

    writer.writeEmptyElement("hr");
    //: The network section of the debug interface
    writer.writeTextElement("h2", tr("Logging filters"));
    writer.writeEmptyElement("hr");

    writer.writeStartElement("div");
    writer.writeAttribute("class", "categories-area");

    QStringList loggingCategories = NymeaCore::loggingFilters();
    loggingCategories.sort();

    QHash<QString, QString> categoryMap = {
        {"debug", "🐞 Debug"},
        {"info", "ℹ️ Info"},
        {"warning", "⚠️ Warning"},
        {"critical", "🔥 Critical"}
    };
    foreach (const QString &loggingCategory, loggingCategories) {
        writer.writeStartElement("div");
        writer.writeAttribute("class", "debug-category");
        writer.writeTextElement("p", loggingCategory);
        writer.writeStartElement("label");
        writer.writeStartElement("select");
        writer.writeAttribute("class", "debug-select");
        writer.writeAttribute("onchange", QString("toggleLoggingCategory('%1', this)").arg(loggingCategory));
        writer.writeAttribute("id", QString("debug-category-%1").arg(loggingCategory));
        foreach (const QString &option, QStringList({"debug", "info", "warning", "critical"})) {
            writer.writeStartElement("option");
            writer.writeAttribute("value", option);
            writer.writeCharacters(categoryMap.value(option));
            writer.writeEndElement();
        }
        writer.writeEndElement(); // select
        writer.writeEndElement(); // label
        writer.writeEndElement(); // div debug-category
    }

    writer.writeEndElement(); // div categories-area

    writer.writeEmptyElement("hr");
    //: The network section of the debug interface
    writer.writeTextElement("h2", tr("Logging filters plugins"));
    writer.writeEmptyElement("hr");

    writer.writeStartElement("div");
    writer.writeAttribute("class", "categories-area");

    QStringList loggingCategoriesPlugins = NymeaCore::loggingFiltersPlugins();
    loggingCategoriesPlugins.sort();
    foreach (const QString &loggingCategory, loggingCategoriesPlugins) {
        writer.writeStartElement("div");
        writer.writeAttribute("class", "debug-category");
        writer.writeTextElement("p", loggingCategory);
        writer.writeStartElement("label");
        writer.writeStartElement("select");
        writer.writeAttribute("class", "debug-select");
        writer.writeAttribute("onchange", QString("toggleLoggingCategory('%1', this)").arg(loggingCategory));
        writer.writeAttribute("id", QString("debug-category-%1").arg(loggingCategory));
        foreach (const QString &option, QStringList({"debug", "info", "warning", "critical"})) {
            writer.writeStartElement("option");
            writer.writeAttribute("value", option);
            writer.writeCharacters(categoryMap.value(option));
            writer.writeEndElement();
        }
        writer.writeEndElement(); // select
        writer.writeEndElement(); // label
        writer.writeEndElement(); // div debug-category
    }

    writer.writeEndElement(); // div categories-area

    writer.writeEndElement(); // logs-section

    writer.writeEndElement(); // div body

    // Footer
    writer.writeStartElement("div");
    writer.writeAttribute("class", "footer");
    writer.writeTextElement("p", QString("Copyright %1 %2 nymea GmbH.").arg(QChar(0xA9)).arg(COPYRIGHT_YEAR_STRING));
    //: The footer license note of the debug interface
    writer.writeTextElement("p", tr("Released under the GNU GENERAL PUBLIC LICENSE Version 3."));
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
    writer.writeTextElement("p", QString("Copyright %1 %2 nymea GmbH.").arg(QChar(0xA9)).arg(COPYRIGHT_YEAR_STRING));
    writer.writeTextElement("p", tr("Released under the GNU GENERAL PUBLIC LICENSE Version 2."));
    writer.writeEndElement(); // div footer

    writer.writeEndElement(); // div container
    writer.writeEndElement(); // html

    return data;
}

}
