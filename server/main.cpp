/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
 *                                                                         *
 *  This file is part of nymea.                                            *
 *                                                                         *
 *  nymea is free software: you can redistribute it and/or modify          *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 **
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with nymea. If not, see <http://www.gnu.org/licenses/>.          *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QLoggingCategory>
#include <QMessageLogger>
#include <QTranslator>
#include <QStringList>
#include <QTextStream>
#include <QDateTime>
#include <QtPlugin>
#include <QObject>
#include <QtDebug>
#include <QString>
#include <QFile>
#include <QDir>

#include "stdio.h"
#include "unistd.h"
#include "nymeacore.h"
#include "nymeaservice.h"
#include "nymeasettings.h"
#include "nymeadbusservice.h"
#include "nymeaapplication.h"
#include "loggingcategories.h"

static QFile s_logFile;

static const char *const normal = "\033[0m";
static const char *const warning = "\e[33m";
static const char *const error = "\e[31m";

using namespace nymeaserver;

static void consoleLogHandler(QtMsgType type, const QMessageLogContext& context, const QString& message)
{
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
        fprintf(stdout, "%s W | %s: %s%s\n", warning, context.category, message.toUtf8().data(), normal);
        break;
    case QtCriticalMsg:
        messageString = QString(" C %1 | %2: %3").arg(timeString).arg(context.category).arg(message);
        fprintf(stdout, "%s C | %s: %s%s\n", error, context.category, message.toUtf8().data(), normal);
        break;
    case QtFatalMsg:
        messageString = QString(" F %1 | %2: %3").arg(timeString).arg(context.category).arg(message);
        fprintf(stdout, "%s F | %s: %s%s\n", error, context.category, message.toUtf8().data(), normal);
        break;
    }
    fflush(stdout);

    if (s_logFile.isOpen()) {
        QTextStream textStream(&s_logFile);
        textStream << messageString << endl;
    }
}

int main(int argc, char *argv[])
{
    qInstallMessageHandler(consoleLogHandler);

    NymeaApplication application(argc, argv);
    application.setOrganizationName("nymea");
    application.setApplicationName("nymead");
    application.setApplicationVersion(NYMEA_VERSION_STRING);

    // logging filers for core and libnymea
    QStringList loggingFilters = {
        "Application",
        "Warnings",
        "DeviceManager",
        "RuleEngine",
        "RuleEngineDebug",
        "Hardware",
        "Bluetooth",
        "Connection",
        "LogEngine",
        "TcpServer",
        "TcpServerTraffic",
        "WebServer",
        "WebServerTraffic",
        "DebugServer",
        "WebSocketServer",
        "WebSocketServerTraffic",
        "JsonRpc",
        "JsonRpcTraffic",
        "Rest",
        "OAuth2",
        "TimeManager",
        "Coap",
        "Avahi",
        "UPnP",
        "Cloud",
        "CloudTraffic",
        "NetworkManager",
        "UserManager",
        "AWS",
        "AWSTraffic",
        "BluetoothServer",
        "BluetoothServerTraffic",
        "Mqtt"
    };

    QStringList loggingFiltersPlugins;
    foreach (const QJsonObject &pluginMetadata, DeviceManager::pluginsMetadata()) {
        QString pluginName = pluginMetadata.value("name").toString();
        loggingFiltersPlugins << pluginName.left(1).toUpper() + pluginName.mid(1);
    }

    // Translator for the server application
    QTranslator translator;
    // check if there are local translations
    if (!translator.load(QLocale::system(), application.applicationName(), "-", QDir(QCoreApplication::applicationDirPath() + "../../translations/").absolutePath(), ".qm"))
        if (!translator.load(QLocale::system(), application.applicationName(), "-", NymeaSettings::translationsPath(), ".qm"))
            qWarning(dcApplication()) << "Could not find nymead translations for" << QLocale::system().name() << endl << (QDir(QCoreApplication::applicationDirPath() + "../../translations/").absolutePath()) << endl << NymeaSettings::translationsPath();



    qApp->installTranslator(&translator);

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    QString applicationDescription = QCoreApplication::translate("nymea", "\nnymea is an open source IoT (Internet of Things) server, \n"
                                                                          "which allows to control a lot of different devices from many different \n"
                                                                          "manufacturers. With the powerful rule engine you are able to connect any \n"
                                                                          "device available in the system and create individual scenes and behaviors \n"
                                                                          "for your environment.\n\n");

    applicationDescription.append(QString("nymead %1 %2 2014-2018 guh GmbH\n"
                                          "Released under the GNU GENERAL PUBLIC LICENSE Version 2.\n\n"
                                          "API version: %3\n").arg(NYMEA_VERSION_STRING).arg(QChar(0xA9)).arg(JSON_PROTOCOL_VERSION));

    parser.setApplicationDescription(applicationDescription);

    QCommandLineOption foregroundOption(QStringList() << "n" << "no-daemon", QCoreApplication::translate("nymea", "Run nymead in the foreground, not as daemon."));
    parser.addOption(foregroundOption);

    QString debugDescription = QCoreApplication::translate("nymea", "Debug categories to enable. Prefix with \"No\" to disable. Suffix with \"Warnings\" to address warnings.\nExamples:\n-d AWSTraffic\n-d NoDeviceManager\n-d NoBluetoothWarnings\n\nCategories are:");

    loggingFilters.sort();
    foreach (const QString &filterName, loggingFilters)
        debugDescription += "\n- " + filterName;


    loggingFiltersPlugins.sort();
    debugDescription += "\n\nPlugin categories:\n";
    foreach (const QString &filterName, loggingFiltersPlugins)
        debugDescription += "\n- " + filterName;

    QCommandLineOption allOption(QStringList() << "p" << "print-all", QCoreApplication::translate("nymea", "Enables all debug categories except *Traffic and *Debug categories. Single debug categories can be disabled again with -d parameter."));
    parser.addOption(allOption);

    QCommandLineOption logOption({"l", "log"}, QCoreApplication::translate("nymea", "Specify a log file to write to, if this option is not specified, logs will be printed to the standard output."), "logfile", "/var/log/nymead.log");
    parser.addOption(logOption);

    QCommandLineOption dbusOption(QStringList() << "session", QCoreApplication::translate("nymea", "If specified, all D-Bus interfaces will be bound to the session bus instead of the system bus."));
    parser.addOption(dbusOption);

    QCommandLineOption debugOption(QStringList() << "d" << "debug-category", debugDescription, "[No]DebugCategory[Warnings]");
    parser.addOption(debugOption);

    parser.process(application);

    // Open the logfile, if any specified
    if (parser.isSet(logOption)) {
        QFileInfo fi(parser.value(logOption));
        QDir dir(fi.absolutePath());
        if (!dir.exists() && !dir.mkpath(dir.absolutePath())) {
            qWarning() << "Error opening log file" << parser.value(logOption);
            return 1;
        }
        s_logFile.setFileName(parser.value(logOption));
        if (!s_logFile.open(QFile::WriteOnly | QFile::Append)) {
            qWarning() << "Error opening log file" << parser.value(logOption);
            return 1;
        }
    }

    QStringList filterRules;
    if (parser.isSet(allOption)) {
        filterRules << "*.debug=true";
        filterRules << "*Traffic.debug=false";
        filterRules << "*Debug.debug=false";
    } else {
        filterRules << "*.debug=false";
    }

    // And allow overriding individual values
    foreach (QString debugArea, parser.values(debugOption)) {
        bool enable = !debugArea.startsWith("No");
        bool isWarning = debugArea.endsWith("Warnings");
        debugArea.remove(QRegExp("^No"));
        debugArea.remove(QRegExp("Warnings$"));
        if (loggingFilters.contains(debugArea) || loggingFiltersPlugins.contains(debugArea)) {
            filterRules.append(QString("%1.%2=%3").arg(debugArea).arg(isWarning ? "warning" : "debug").arg(enable ? "true": "false"));
        } else {
            qCWarning(dcApplication) << QCoreApplication::translate("nymea", "No such debug category:") << debugArea;
        }
    }
    QLoggingCategory::setFilterRules(filterRules.join('\n'));

    if (parser.isSet(dbusOption)) {
        NymeaDBusService::setBusType(QDBusConnection::SessionBus);
    }

    bool startForeground = parser.isSet(foregroundOption);
    if (startForeground) {
        // inform about userid
        uint userId = getuid();
        if (userId != 0) {
            // check if config directory for logfile exists
            if (!QDir().mkpath(NymeaSettings::settingsPath())) {
                fprintf(stdout, "Could not create nymea settings directory %s", qPrintable(NymeaSettings::settingsPath()));
                exit(EXIT_FAILURE);
            }
            qCInfo(dcApplication) << "=====================================";
            qCInfo(dcApplication) << "nymead" << NYMEA_VERSION_STRING << "started with user ID" << userId;
            qCInfo(dcApplication) << "=====================================";
        } else {
            qCInfo(dcApplication) << "=====================================";
            qCInfo(dcApplication) << "nymead" << NYMEA_VERSION_STRING << "started as root.";
            qCInfo(dcApplication) << "=====================================";
        }

        // If running in a snappy environment, print out some details about it.
        if (!qgetenv("SNAP").isEmpty()) {
            // Note: http://snapcraft.io/docs/reference/env
            qCInfo(dcApplication) << "Snap name       :" << qgetenv("SNAP_NAME");
            qCInfo(dcApplication) << "Snap version    :" << qgetenv("SNAP_VERSION");
            qCInfo(dcApplication) << "Snap directory  :" << qgetenv("SNAP");
            qCInfo(dcApplication) << "Snap app data   :" << qgetenv("SNAP_DATA");
            qCInfo(dcApplication) << "Snap user data  :" << qgetenv("SNAP_USER_DATA");
            qCInfo(dcApplication) << "Snap app common :" << qgetenv("SNAP_COMMON");
        }

        // create core instance
        NymeaCore::instance()->init();
        int ret = application.exec();
        if (s_logFile.isOpen()) {
            s_logFile.close();
        }
        return ret;
    }

    NymeaService service(argc, argv);
    int ret = service.exec();
    if (s_logFile.isOpen()) {
        s_logFile.close();
    }
    return ret;
}
