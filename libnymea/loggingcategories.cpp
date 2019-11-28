/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015-2018 Simon Stürz <simon.stuerz@guh.io>              *
 *                                                                         *
 *  This file is part of nymea.                                            *
 *                                                                         *
 *  This library is free software; you can redistribute it and/or          *
 *  modify it under the terms of the GNU Lesser General Public             *
 *  License as published by the Free Software Foundation; either           *
 *  version 2.1 of the License, or (at your option) any later version.     *
 *                                                                         *
 *  This library is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
 *  Lesser General Public License for more details.                        *
 *                                                                         *
 *  You should have received a copy of the GNU Lesser General Public       *
 *  License along with this library; If not, see                           *
 *  <http://www.gnu.org/licenses/>.                                        *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "loggingcategories.h"
#include <QFileInfo>
#include <QDir>
#include <QDateTime>

Q_LOGGING_CATEGORY(dcApplication, "Application")
Q_LOGGING_CATEGORY(dcPluginMetadata, "PluginMetadata")
Q_LOGGING_CATEGORY(dcDevice, "Device")
Q_LOGGING_CATEGORY(dcDeviceManager, "DeviceManager")
Q_LOGGING_CATEGORY(dcSystem, "System")
Q_LOGGING_CATEGORY(dcPlatform, "Platform")
Q_LOGGING_CATEGORY(dcPlatformUpdate, "PlatformUpdate")
Q_LOGGING_CATEGORY(dcPlatformZeroConf, "PlatformZeroConf")
Q_LOGGING_CATEGORY(dcExperiences, "Experiences")
Q_LOGGING_CATEGORY(dcTimeManager, "TimeManager")
Q_LOGGING_CATEGORY(dcRuleEngine, "RuleEngine")
Q_LOGGING_CATEGORY(dcRuleEngineDebug, "RuleEngineDebug")
Q_LOGGING_CATEGORY(dcScriptEngine, "ScriptEngine")
Q_LOGGING_CATEGORY(dcHardware, "Hardware")
Q_LOGGING_CATEGORY(dcLogEngine, "LogEngine")
Q_LOGGING_CATEGORY(dcServerManager, "ServerManager")
Q_LOGGING_CATEGORY(dcTcpServer, "TcpServer")
Q_LOGGING_CATEGORY(dcTcpServerTraffic, "TcpServerTraffic")
Q_LOGGING_CATEGORY(dcWebServer, "WebServer")
Q_LOGGING_CATEGORY(dcWebServerTraffic, "WebServerTraffic")
Q_LOGGING_CATEGORY(dcDebugServer, "DebugServer")
Q_LOGGING_CATEGORY(dcWebSocketServer, "WebSocketServer")
Q_LOGGING_CATEGORY(dcWebSocketServerTraffic, "WebSocketServerTraffic")
Q_LOGGING_CATEGORY(dcJsonRpc, "JsonRpc")
Q_LOGGING_CATEGORY(dcJsonRpcTraffic, "JsonRpcTraffic")
Q_LOGGING_CATEGORY(dcRest, "Rest")
Q_LOGGING_CATEGORY(dcOAuth2, "OAuth2")
Q_LOGGING_CATEGORY(dcUpnp, "UPnP")
Q_LOGGING_CATEGORY(dcBluetooth, "Bluetooth")
Q_LOGGING_CATEGORY(dcCloud, "Cloud")
Q_LOGGING_CATEGORY(dcCloudTraffic, "CloudTraffic")
Q_LOGGING_CATEGORY(dcNetworkManager, "NetworkManager")
Q_LOGGING_CATEGORY(dcUserManager, "UserManager")
Q_LOGGING_CATEGORY(dcAWS, "AWS")
Q_LOGGING_CATEGORY(dcAWSTraffic, "AWSTraffic")
Q_LOGGING_CATEGORY(dcBluetoothServer, "BluetoothServer")
Q_LOGGING_CATEGORY(dcBluetoothServerTraffic, "BluetoothServerTraffic")
Q_LOGGING_CATEGORY(dcMqtt, "Mqtt")
Q_LOGGING_CATEGORY(dcTranslations, "Translations")


static QFile s_logFile;
static bool s_useColors;
static QList<QtMessageHandler> s_handlers;

static const char *const normal = "\033[0m";
static const char *const warning = "\033[33m";
static const char *const error = "\033[31m";

void nymeaInstallMessageHandler(QtMessageHandler handler)
{
    s_handlers.append(handler);
}

void nymeaUninstallMessageHandler(QtMessageHandler handler)
{
    s_handlers.removeAll(handler);
}

void nymeaLogMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &message)
{
    // Copy message to all installed nymea handlers
    foreach (QtMessageHandler handler, s_handlers) {
        handler(type, context, message);
    }

    QString messageString;
    QString timeString = QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss.zzz");
    switch (type) {
#if (QT_VERSION >= QT_VERSION_CHECK(5, 5, 0))
    case QtInfoMsg:
        messageString = QString(" I %1 | %2: %3").arg(timeString).arg(context.category).arg(message);
        fprintf(stdout, " I | %s: %s\n", context.category, message.toUtf8().data());
        break;
#endif
    case QtDebugMsg:
        messageString = QString(" I %1 | %2: %3").arg(timeString).arg(context.category).arg(message);
        fprintf(stdout, " I | %s: %s\n", context.category, message.toUtf8().data());
        break;
    case QtWarningMsg:
        messageString = QString(" W %1 | %2: %3").arg(timeString).arg(context.category).arg(message);
        fprintf(stdout, "%s W | %s: %s%s\n", s_useColors ? warning : "", context.category, message.toUtf8().data(), s_useColors ? normal : "");
        break;
    case QtCriticalMsg:
        messageString = QString(" C %1 | %2: %3").arg(timeString).arg(context.category).arg(message);
        fprintf(stdout, "%s C | %s: %s%s\n", s_useColors ? error : "", context.category, message.toUtf8().data(), s_useColors ? error : "");
        break;
    case QtFatalMsg:
        messageString = QString(" F %1 | %2: %3").arg(timeString).arg(context.category).arg(message);
        fprintf(stdout, "%s F | %s: %s%s\n", s_useColors ? error : "", context.category, message.toUtf8().data(), s_useColors ? error: "");
        break;
    }
    fflush(stdout);

    if (s_logFile.isOpen()) {
        QTextStream textStream(&s_logFile);
        textStream << messageString << endl;
    }
}

bool initLogging(const QString &fileName, bool useColors)
{
    s_useColors = useColors;

    qInstallMessageHandler(nymeaLogMessageHandler);

    if (!fileName.isEmpty()) {
        QFileInfo fi(fileName);
        QDir dir(fi.absolutePath());
        if (!dir.exists() && !dir.mkpath(dir.absolutePath())) {
            qWarning() << "Error logfile path:" << fileName;
            return false;
        }
        s_logFile.setFileName(fileName);
        if (!s_logFile.open(QFile::WriteOnly | QFile::Append)) {
            qWarning() << "Error opening log file:" <<fileName;
            return false;
        }
    }
    return true;
}

void closeLogFile()
{
    if (s_logFile.isOpen()) {
        s_logFile.close();
    }
}
