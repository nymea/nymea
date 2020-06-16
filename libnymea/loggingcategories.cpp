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
* GNU Lesser General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU Lesser General Public License as published by the Free
* Software Foundation; version 3. This project is distributed in the hope that
* it will be useful, but WITHOUT ANY WARRANTY; without even the implied
* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "loggingcategories.h"
#include <QFileInfo>
#include <QDir>
#include <QDateTime>

QStringList s_nymeaLoggingCategories;

// FIXME: Those should eventually disappear from here
NYMEA_LOGGING_CATEGORY(dcPluginMetadata, "PluginMetadata")
NYMEA_LOGGING_CATEGORY(dcThing, "Thing")
NYMEA_LOGGING_CATEGORY(dcThingManager, "ThingManager")
NYMEA_LOGGING_CATEGORY(dcSystem, "System")
NYMEA_LOGGING_CATEGORY(dcPlatform, "Platform")
NYMEA_LOGGING_CATEGORY(dcPlatformUpdate, "PlatformUpdate")
NYMEA_LOGGING_CATEGORY(dcPlatformZeroConf, "PlatformZeroConf")
NYMEA_LOGGING_CATEGORY(dcExperiences, "Experiences")
NYMEA_LOGGING_CATEGORY(dcTimeManager, "TimeManager")
NYMEA_LOGGING_CATEGORY(dcRuleEngine, "RuleEngine")
NYMEA_LOGGING_CATEGORY(dcRuleEngineDebug, "RuleEngineDebug")
NYMEA_LOGGING_CATEGORY(dcScriptEngine, "ScriptEngine")
NYMEA_LOGGING_CATEGORY(dcHardware, "Hardware")
NYMEA_LOGGING_CATEGORY(dcLogEngine, "LogEngine")
NYMEA_LOGGING_CATEGORY(dcServerManager, "ServerManager")
NYMEA_LOGGING_CATEGORY(dcTcpServer, "TcpServer")
NYMEA_LOGGING_CATEGORY(dcTcpServerTraffic, "TcpServerTraffic")
NYMEA_LOGGING_CATEGORY(dcWebServer, "WebServer")
NYMEA_LOGGING_CATEGORY(dcWebServerTraffic, "WebServerTraffic")
NYMEA_LOGGING_CATEGORY(dcDebugServer, "DebugServer")
NYMEA_LOGGING_CATEGORY(dcWebSocketServer, "WebSocketServer")
NYMEA_LOGGING_CATEGORY(dcWebSocketServerTraffic, "WebSocketServerTraffic")
NYMEA_LOGGING_CATEGORY(dcJsonRpc, "JsonRpc")
NYMEA_LOGGING_CATEGORY(dcJsonRpcTraffic, "JsonRpcTraffic")
NYMEA_LOGGING_CATEGORY(dcRest, "Rest")
NYMEA_LOGGING_CATEGORY(dcOAuth2, "OAuth2")
NYMEA_LOGGING_CATEGORY(dcUpnp, "UPnP")
NYMEA_LOGGING_CATEGORY(dcBluetooth, "Bluetooth")
NYMEA_LOGGING_CATEGORY(dcCloud, "Cloud")
NYMEA_LOGGING_CATEGORY(dcCloudTraffic, "CloudTraffic")
NYMEA_LOGGING_CATEGORY(dcNetworkManager, "NetworkManager")
NYMEA_LOGGING_CATEGORY(dcUserManager, "UserManager")
NYMEA_LOGGING_CATEGORY(dcAWS, "AWS")
NYMEA_LOGGING_CATEGORY(dcAWSTraffic, "AWSTraffic")
NYMEA_LOGGING_CATEGORY(dcBluetoothServer, "BluetoothServer")
NYMEA_LOGGING_CATEGORY(dcBluetoothServerTraffic, "BluetoothServerTraffic")
NYMEA_LOGGING_CATEGORY(dcMqtt, "Mqtt")
NYMEA_LOGGING_CATEGORY(dcTranslations, "Translations")
NYMEA_LOGGING_CATEGORY(dcI2C, "I2C")


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
            qWarning() << "Error opening log file:" << fileName;
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
