/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stuerz <simon.stuerz@guh.guru>                *
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

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QLoggingCategory>
#include <QCommandLineOption>
#include <QMessageLogger>
#include <QStringList>
#include <QTextStream>
#include <QDateTime>
#include <QtPlugin>
#include <QtDebug>
#include <QFile>

#include "stdio.h"
#include "unistd.h"
#include "guhcore.h"
#include "guhservice.h"
#include "guhsettings.h"
#include "loggingcategories.h"

static QHash<QString, bool> s_loggingFilters;

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
    case QtDebugMsg:
        messageString = QString(" I %1 | %2: %3").arg(timeString).arg(context.category).arg(message);
        fprintf(stdout, " I | %s: %s\n", context.category, message.toUtf8().data());
        break;
    case QtWarningMsg:
        messageString = QString(" W %1 | %2: %3").arg(timeString).arg(context.category).arg(message);
        fprintf(stdout, " W | %s: %s\n", context.category, message.toUtf8().data());
        break;
    case QtCriticalMsg:
        messageString = QString(" C %1 | %2: %3").arg(timeString).arg(context.category).arg(message);
        fprintf(stdout, " C | %s: %s\n", context.category, message.toUtf8().data());
        break;
    case QtFatalMsg:
        messageString = QString(" F %1 | %2: %3").arg(timeString).arg(context.category).arg(message);
        fprintf(stdout, " F | %s: %s\n", context.category, message.toUtf8().data());
        break;
    }

    QFile logFile(GuhSettings::consoleLogPath());
    if (!logFile.open(QIODevice::WriteOnly | QIODevice::Append)) {
        fprintf(stdout, " W | Application: Could not open logfile.\n");
        return;
    }
    QTextStream textStream(&logFile);
    textStream << messageString << endl;
    logFile.close();
}


int main(int argc, char *argv[])
{
    qInstallMessageHandler(consoleLogHandler);

    QCoreApplication application(argc, argv);
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
    s_loggingFilters.insert("WebServer", true);
    s_loggingFilters.insert("WebSocketServer", true);
    s_loggingFilters.insert("JsonRpc", true);
    s_loggingFilters.insert("Rest", true);
    s_loggingFilters.insert("OAuth2", false);

    QHash<QString, bool> loggingFiltersPlugins;
    foreach (const QJsonObject &pluginMetadata, DeviceManager::pluginsMetadata()) {
        loggingFiltersPlugins.insert(pluginMetadata.value("idName").toString(), false);
    }

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    QString applicationDescription = QString("\nguh ( /[guːh]/ ) is an open source IoT (Internet of Things) server, \n"
                                             "which allows to control a lot of different devices from many different. \n"
                                             "manufacturers. With the powerful rule engine you are able to connect any \n"
                                             "device available in the system and create individual scenes and behaviors \n"
                                             "for your environment.\n\n"
                                             "guhd %1 (C) 2014-2015 guh\n"
                                             "Released under the GNU GENERAL PUBLIC LICENSE Version 2.\n\n"
                                             "API version: %2\n").arg(GUH_VERSION_STRING).arg(JSON_PROTOCOL_VERSION);

    parser.setApplicationDescription(applicationDescription);

    QCommandLineOption foregroundOption(QStringList() << "n" << "no-daemon", QCoreApplication::translate("main", "Run guhd in the foreground, not as daemon."));
    parser.addOption(foregroundOption);

    QString debugDescription = QString("Debug categories to enable. Prefix with \"No\" to disable. Warnings from all categories will be printed unless explicitly muted with \"NoWarnings\". \n\nCategories are:");

    // create sorted loggingFiler list
    QStringList sortedFilterList = QStringList(s_loggingFilters.keys());
    sortedFilterList.sort();
    foreach (const QString &filterName, sortedFilterList) {
        debugDescription += "\n- " + filterName + " (" + (s_loggingFilters.value(filterName) ? "yes" : "no") + ")";
    }
    // create sorted plugin loggingFiler list
    QStringList sortedPluginList = QStringList(loggingFiltersPlugins.keys());
    sortedPluginList.sort();
    debugDescription += "\n\nPlugin categories:\n";
    foreach (const QString &filterName, sortedPluginList) {
        debugDescription += "\n- " + filterName + " (" + (s_loggingFilters.value(filterName) ? "yes" : "no") + ")";
    }

    QCommandLineOption allOption(QStringList() << "p" << "print-all", "Enables all debug categories. This parameter overrides all debug category parameters.");
    parser.addOption(allOption);
    QCommandLineOption debugOption(QStringList() << "d" << "debug-category", debugDescription, "[No]DebugCategory");
    parser.addOption(debugOption);

    parser.process(application);

    // add plugin metadata to the static hash
    foreach (const QString &category, loggingFiltersPlugins.keys()) {
        s_loggingFilters.insert(category, false);
    }

    // check debug area
    if (!parser.isSet(allOption)) {
        foreach (QString debugArea, parser.values(debugOption)) {
            bool enable = !debugArea.startsWith("No");
            debugArea.remove(QRegExp("^No"));
            if (s_loggingFilters.contains(debugArea)) {
                s_loggingFilters[debugArea] = enable;
            } else {
                qCWarning(dcApplication) << "No such debug category:" << debugArea;
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
            qCDebug(dcApplication) << "=====================================";
            qCDebug(dcApplication) << "guhd" << GUH_VERSION_STRING << "started with user ID" << userId;
            qCDebug(dcApplication) << "=====================================";
        } else {
            qCDebug(dcApplication) << "=====================================";
            qCDebug(dcApplication) << "guhd" << GUH_VERSION_STRING << "started as root.";
            qCDebug(dcApplication) << "=====================================";
        }

        // create core instance
        GuhCore::instance()->setRunningMode(GuhCore::RunningModeApplication);
        return application.exec();
    }

    GuhService service(argc, argv);
    return service.exec();
}
