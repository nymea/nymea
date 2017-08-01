/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
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
#include <QtDebug>
#include <QString>
#include <QFile>

#include "stdio.h"
#include "unistd.h"
#include "guhcore.h"
#include "guhservice.h"
#include "guhsettings.h"
#include "guhapplication.h"
#include "loggingcategories.h"

static QHash<QString, bool> s_loggingFilters;
static QFile s_logFile;

static const char *const normal = "\033[0m";
static const char *const warning = "\e[33m";
static const char *const error = "\e[31m";

using namespace guhserver;

static void loggingCategoryFilter(QLoggingCategory *category)
{
    if (s_loggingFilters.contains(category->categoryName())) {
        bool debugEnabled = s_loggingFilters.value(category->categoryName());
        category->setEnabled(QtDebugMsg, debugEnabled);
        category->setEnabled(QtWarningMsg, debugEnabled || s_loggingFilters.value("Warnings"));
    } else {
        category->setEnabled(QtDebugMsg, false);
        category->setEnabled(QtWarningMsg, s_loggingFilters.value("qml") || s_loggingFilters.value("Warnings"));
    }
}

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

    GuhApplication application(argc, argv);
    application.setOrganizationName("guh");
    application.setApplicationName("guhd");
    application.setApplicationVersion(GUH_VERSION_STRING);

    // logging filers for core and libguh
    s_loggingFilters.insert("Application", true);
    s_loggingFilters.insert("Warnings", true);
    s_loggingFilters.insert("DeviceManager", true);
    s_loggingFilters.insert("RuleEngine", true);
    s_loggingFilters.insert("Hardware", false);
    s_loggingFilters.insert("Connection", true);
    s_loggingFilters.insert("LogEngine", false);
    s_loggingFilters.insert("TcpServer", false);
    s_loggingFilters.insert("WebServer", false);
    s_loggingFilters.insert("WebSocketServer", false);
    s_loggingFilters.insert("JsonRpc", false);
    s_loggingFilters.insert("Rest", false);
    s_loggingFilters.insert("OAuth2", false);
    s_loggingFilters.insert("TimeManager", false);
    s_loggingFilters.insert("Coap", false);
    s_loggingFilters.insert("Avahi", false);
    s_loggingFilters.insert("Cloud", true);
    s_loggingFilters.insert("NetworkManager", true);
    s_loggingFilters.insert("UserManager", true);

    QHash<QString, bool> loggingFiltersPlugins;
    foreach (const QJsonObject &pluginMetadata, DeviceManager::pluginsMetadata()) {
        loggingFiltersPlugins.insert(pluginMetadata.value("idName").toString(), false);
    }

    // Translator for the server application
    QTranslator translator;
    // check if there are local translations
    if (!translator.load(QLocale::system(), application.applicationName(), "-", QDir(QCoreApplication::applicationDirPath() + "../../translations/").absolutePath(), ".qm"))
        if (!translator.load(QLocale::system(), application.applicationName(), "-", GuhSettings::translationsPath(), ".qm"))
            qWarning(dcApplication()) << "Could not find guhd translations for" << QLocale::system().name() << endl << (QDir(QCoreApplication::applicationDirPath() + "../../translations/").absolutePath()) << endl << GuhSettings::translationsPath();



    qApp->installTranslator(&translator);

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    QString applicationDescription = QCoreApplication::translate("main", "\nguh ( /[guːh]/ ) is an open source IoT (Internet of Things) server, \n"
                                             "which allows to control a lot of different devices from many different \n"
                                             "manufacturers. With the powerful rule engine you are able to connect any \n"
                                             "device available in the system and create individual scenes and behaviors \n"
                                             "for your environment.\n\n");

    applicationDescription.append(QString("guhd %1 %2 2014-2017 guh GmbH\n"
                                          "Released under the GNU GENERAL PUBLIC LICENSE Version 2.\n\n"
                                          "API version: %3\n").arg(GUH_VERSION_STRING).arg(QChar(0xA9)).arg(JSON_PROTOCOL_VERSION));

    parser.setApplicationDescription(applicationDescription);

    QCommandLineOption foregroundOption(QStringList() << "n" << "no-daemon", QCoreApplication::translate("main", "Run guhd in the foreground, not as daemon."));
    parser.addOption(foregroundOption);

    QString debugDescription = QCoreApplication::translate("main", "Debug categories to enable. Prefix with \"No\" to disable. Warnings from all categories will be printed unless explicitly muted with \"NoWarnings\". \n\nCategories are:");

    // create sorted loggingFiler list
    QStringList sortedFilterList = QStringList(s_loggingFilters.keys());
    sortedFilterList.sort();
    foreach (const QString &filterName, sortedFilterList)
        debugDescription += "\n- " + filterName + " (" + (s_loggingFilters.value(filterName) ? "yes" : "no") + ")";


    // create sorted plugin loggingFiler list
    QStringList sortedPluginList = QStringList(loggingFiltersPlugins.keys());
    sortedPluginList.sort();
    debugDescription += "\n\nPlugin categories:\n";
    foreach (const QString &filterName, sortedPluginList)
        debugDescription += "\n- " + filterName + " (" + (s_loggingFilters.value(filterName) ? "yes" : "no") + ")";


    QCommandLineOption allOption(QStringList() << "p" << "print-all", QCoreApplication::translate("main", "Enables all debug categories. This parameter overrides all debug category parameters."));
    parser.addOption(allOption);
    QCommandLineOption debugOption(QStringList() << "d" << "debug-category", debugDescription, "[No]DebugCategory");
    parser.addOption(debugOption);

    QCommandLineOption logOption({"l", "log"}, QCoreApplication::translate("main", "Specify a log file to write to, If this option is not specified, logs will be printed to the standard output."), "logfile", "/var/log/guhd.log");
    parser.addOption(logOption);

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

    // add plugin metadata to the static hash
    foreach (const QString &category, loggingFiltersPlugins.keys())
        s_loggingFilters.insert(category, false);

    // check debug area
    if (!parser.isSet(allOption)) {
        foreach (QString debugArea, parser.values(debugOption)) {
            bool enable = !debugArea.startsWith("No");
            debugArea.remove(QRegExp("^No"));
            if (s_loggingFilters.contains(debugArea)) {
                s_loggingFilters[debugArea] = enable;
            } else {
                qCWarning(dcApplication) << QCoreApplication::translate("main", "No such debug category:") << debugArea;
            }
        }
    } else {
        foreach (const QString &debugArea, s_loggingFilters.keys())
            s_loggingFilters[debugArea] = true;

    }
    QLoggingCategory::installFilter(loggingCategoryFilter);

    bool startForeground = parser.isSet(foregroundOption);
    if (startForeground) {
        // inform about userid
        int userId = getuid();
        if (userId != 0) {
            // check if config directory for logfile exists
            if (!QDir().mkpath(GuhSettings::settingsPath())) {
                fprintf(stdout, "Could not create guh settings directory %s", qPrintable(GuhSettings::settingsPath()));
                exit(EXIT_FAILURE);
            }
            qCDebug(dcApplication) << "=====================================";
            qCDebug(dcApplication) << "guhd" << GUH_VERSION_STRING << "started with user ID" << userId;
            qCDebug(dcApplication) << "=====================================";
        } else {
            qCDebug(dcApplication) << "=====================================";
            qCDebug(dcApplication) << "guhd" << GUH_VERSION_STRING << "started as root.";
            qCDebug(dcApplication) << "=====================================";
        }

#ifdef SNAPPY
        // Note: http://snapcraft.io/docs/reference/env
        qCDebug(dcApplication) << "Snap name       :" << qgetenv("SNAP_NAME");
        qCDebug(dcApplication) << "Snap version    :" << qgetenv("SNAP_VERSION");
        qCDebug(dcApplication) << "Snap directory  :" << qgetenv("SNAP");
        qCDebug(dcApplication) << "Snap app data   :" << qgetenv("SNAP_DATA");
        qCDebug(dcApplication) << "Snap user data  :" << qgetenv("SNAP_USER_DATA");
        qCDebug(dcApplication) << "Snap app common :" << qgetenv("SNAP_COMMON");
#endif

        // create core instance
        GuhCore::instance();
        int ret = application.exec();
        if (s_logFile.isOpen()) {
            s_logFile.close();
        }
        return ret;
    }

    GuhService service(argc, argv);
    int ret = service.exec();
    if (s_logFile.isOpen()) {
        s_logFile.close();
    }
    return ret;
}
