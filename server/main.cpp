// SPDX-License-Identifier: GPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2025, chargebyte austria GmbH
*
* This file is part of nymea.
*
* nymea is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* nymea is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with nymea. If not, see <https://www.gnu.org/licenses/>.
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QRegularExpression>
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
#include "version.h"

NYMEA_LOGGING_CATEGORY(dcApplication, "Application")

using namespace nymeaserver;


int main(int argc, char *argv[])
{
    NymeaApplication application(argc, argv);
    application.setOrganizationName("nymea");
    application.setApplicationName("nymead");
    application.setApplicationVersion(NYMEA_VERSION_STRING);

    // Logging filers for core + libnymea and plugins
    QStringList loggingFilters = NymeaCore::loggingFilters();
    QStringList loggingFiltersPlugins = NymeaCore::loggingFiltersPlugins();

    // Translator for the server application
    QTranslator translator;
    // check if there are local translations
    if (!translator.load(QLocale::system(), application.applicationName(), "-", QDir(QCoreApplication::applicationDirPath() + "../../translations/").absolutePath(), ".qm"))
        if (!translator.load(QLocale::system(), application.applicationName(), "-", NymeaSettings::translationsPath(), ".qm"))
            qCWarning(dcTranslations()) << "Could not find nymead translations for" << QLocale::system().name() << '\n' << (QDir(QCoreApplication::applicationDirPath() + "../../translations/").absolutePath()) << '\n' << NymeaSettings::translationsPath();



    qApp->installTranslator(&translator);

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    QString applicationDescription = QCoreApplication::translate("nymea", "\nnymea is an open source IoT (Internet of Things) server, \n"
                                                                          "which allows to control a lot of different devices from many different \n"
                                                                          "manufacturers. With the powerful rule engine you are able to connect any \n"
                                                                          "device available in the system and create individual scenes and behaviors \n"
                                                                          "for your environment.\n\n");

    applicationDescription.append(QString("nymead %1 %2 %3 nymea GmbH\n"
                                          "Released under the GNU GENERAL PUBLIC LICENSE Version 3.\n\n"
                                          "API version: %4\n").arg(NYMEA_VERSION_STRING).arg(QChar(0xA9)).arg(COPYRIGHT_YEAR_STRING).arg(JSON_PROTOCOL_VERSION));

    parser.setApplicationDescription(applicationDescription);

    QCommandLineOption foregroundOption({"n", "no-daemon"}, QCoreApplication::translate("nymea", "Run nymead in the foreground, not as daemon."));
    parser.addOption(foregroundOption);

    QCommandLineOption quietOption({"q", "quiet"}, QCoreApplication::translate("nymea", "Disables logging all debug, info and warning categories."));
    parser.addOption(quietOption);

    QCommandLineOption allOption({"p", "print-all"}, QCoreApplication::translate("nymea", "Enables all info and debug categories except *Traffic and *Debug categories."));
    parser.addOption(allOption);

    QCommandLineOption logOption({"l", "log"}, QCoreApplication::translate("nymea", "Specify a log file to write to, if this option is not specified, logs will be printed to the standard output."), "logfile");
    parser.addOption(logOption);

    QCommandLineOption noColorOption("no-colors", QCoreApplication::translate("nymea", "Log output is colorized by default. Use this option to disable colors."));
    parser.addOption(noColorOption);

    QCommandLineOption dbusOption("session", QCoreApplication::translate("nymea", "If specified, all D-Bus interfaces will be bound to the session bus instead of the system bus."));
    parser.addOption(dbusOption);

    QString debugDescription = QCoreApplication::translate("nymea", "Debug categories to enable. Prefix with \"No\" to disable. Suffix with \"Info\" or \"Warnings\" to address info and warning messages. Enabling a debug category will implicitly enable the according info category.\nExamples:\n-d ThingManager\n-d NoApplicationInfo\n\n");
    QCommandLineOption debugOption({"d", "debug-category"}, debugDescription, "[No]DebugCategory[Warning|Info]]");
    parser.addOption(debugOption);

    QCommandLineOption interfacesOption({"i", "interface"}, QCoreApplication::translate("nymea", "Additional interfaces to listen on. In nymea URI format (e.g. nymeas://127.0.0.2:7777). Note that such interfaces will not require any authentication as they are intended to be used for automated testing only."), "interfaceString");
    parser.addOption(interfacesOption);

    QCommandLineOption noLogDbOption({"m", "no-logengine"}, QCoreApplication::translate("nymea", "Disable the influx DB log engine."));
    parser.addOption(noLogDbOption);

    QCommandLineOption configurationOption({"c", "configuration"}, QCoreApplication::translate("nymea", "Uses the given <path> for storing configurations. Using this option will override the NYMEA_CONFIG_PATH environment variable."), "path");
    parser.addOption(configurationOption);

    parser.process(application);

    // Open the logfile, if any specified
    if (!initLogging(parser.value(logOption), !parser.isSet(noColorOption))) {
        qWarning() << "Error opening log file" << parser.value(logOption);
        return 1;
    }

    /* The logging rules will be evaluated sequentially
     *  1. All debug and info categories off (with -q also warnings)
     *  2. Enable all warning info and debug categories if requested from command line (-p)
     *  3. The stored categories from the nymead.conf will be appended
     *  4. Add the individual command line params will be added (-d)
     *  5. QT_LOGGING_CONF
     *  6. QT_LOGGING_RULES
     */

    // 1. All debug categories off
    QStringList loggingRules;
    loggingRules << "*.debug=false";
    loggingRules << "*.info=false";
    if (parser.isSet(quietOption)) {
        loggingRules << "*.warning=false";
    } else {
        loggingRules << "Application.info=true";
    }

    // 2. Enable all debug categories making sense if requested from command line (-p)
    if (parser.isSet(allOption)) {
        loggingRules << "*.debug=true";
        loggingRules << "*.info=true";
        loggingRules << "*.warning=true";
        loggingRules << "*Traffic.debug=false";
        loggingRules << "*Debug.debug=false";
    }

    // 3. The stored categories from the nymead.conf will be appended
    NymeaSettings nymeaSettings(NymeaSettings::SettingsRoleGlobal);
    nymeaSettings.beginGroup("LoggingRules");
    foreach (const QString &category, nymeaSettings.childKeys()) {
        bool enable = nymeaSettings.value(category, false).toBool();
        if (enable && category.endsWith("debug")) {
            loggingRules << QString("%1=%2").arg(category).arg("true");
            loggingRules << QString("%1=%2").arg(QString(category).replace(QRegularExpression("debug$"), "info")).arg("true");
            loggingRules << QString("%1=%2").arg(QString(category).replace(QRegularExpression("debug$"), "warning")).arg("true");
        } else {
            loggingRules << QString("%1=%2").arg(category).arg(nymeaSettings.value(category, "false").toString());
        }
    }
    nymeaSettings.endGroup();

    // 4. Add the individual command line params will be added (-d)
    foreach (QString debugArea, parser.values(debugOption)) {
        bool enable = true;
        bool isWarning = debugArea.endsWith("Warnings");
        bool isInfo = debugArea.endsWith("Info");
        if (QRegularExpression("^No[A-Z]").match(debugArea).hasMatch()) {
            debugArea.remove(QRegularExpression("^No"));
            enable = false;
        }
        debugArea.remove(QRegularExpression("(Warnings|Info)$"));
        if (enable && !isWarning && !isInfo) {
            loggingRules.append(QString("%1.%2=%3").arg(debugArea).arg("debug").arg("true"));
            loggingRules.append(QString("%1.%2=%3").arg(debugArea).arg("info").arg("true"));
            loggingRules.append(QString("%1.%2=%3").arg(debugArea).arg("warning").arg("true"));
        } else {
            loggingRules.append(QString("%1.%2=%3").arg(debugArea).arg(isWarning ? "warning" : (isInfo ? "info" : "debug")).arg(enable ? "true": "false"));
        }
    }


    // Finally set the rules for the logging
    QLoggingCategory::setFilterRules(loggingRules.join('\n'));

    // Parse DBus option
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
            qCInfo(dcApplication()) << "=====================================";
            qCInfo(dcApplication()) << "nymead" << NYMEA_VERSION_STRING << "started with user ID" << userId;
            qCInfo(dcApplication()) << "=====================================";
        } else {
            qCInfo(dcApplication()) << "=====================================";
            qCInfo(dcApplication()) << "nymead" << NYMEA_VERSION_STRING << "started as root.";
            qCInfo(dcApplication()) << "=====================================";
        }

        QStringList arguments;
        for (int i = 0; i < argc; i++) {
            arguments << QString(argv[i]);
        }
        qCInfo(dcApplication()) << "Started:" << arguments.takeFirst();
        qCInfo(dcApplication()) << "Parameters:" << arguments.join(' ');
        qCInfo(dcApplication()) << "Built with Qt:" << QT_VERSION_STR;
        qCInfo(dcApplication()) << "Run with Qt:" << qVersion();


        // If running in a snappy environment, print out some details about it.
        if (!qEnvironmentVariableIsEmpty("SNAP")) {
            // Note: http://snapcraft.io/docs/reference/env
            qCInfo(dcApplication()) << "Snap name       :" << qgetenv("SNAP_NAME");
            qCInfo(dcApplication()) << "Snap version    :" << qgetenv("SNAP_VERSION");
            qCInfo(dcApplication()) << "Snap directory  :" << qgetenv("SNAP");
            qCInfo(dcApplication()) << "Snap app data   :" << qgetenv("SNAP_DATA");
            qCInfo(dcApplication()) << "Snap user data  :" << qgetenv("SNAP_USER_DATA");
            qCInfo(dcApplication()) << "Snap app common :" << qgetenv("SNAP_COMMON");
        }

        if (parser.isSet(configurationOption)) {
            QString configPath = parser.value(configurationOption);
            qCInfo(dcApplication()) << "Using custom configuration localtion" << configPath;
            if (!qEnvironmentVariableIsEmpty("NYMEA_CONFIG_PATH")) {
                QString configPathEnv = qgetenv("NYMEA_CONFIG_PATH");
                if (configPathEnv != configPathEnv) {
                    qCWarning(dcApplication()) << "The configuration param is overriding the configured" << configPathEnv << "with" << configPath;
                }
            }

            qputenv("NYMEA_CONFIG_PATH", configPath.toUtf8());
        }

        // create core instance
        QObject::connect(NymeaCore::instance(), &NymeaCore::initialized, NymeaCore::instance(), [](){
            qCInfo(dcApplication()) << "The core is now up and running.";
        });

        NymeaCore::instance()->init(parser.values(interfacesOption), parser.isSet(noLogDbOption));

        int ret = application.exec();
        closeLogFile();
        return ret;
    }

    // FIXME: the background service should get the arguments too
    NymeaService service(argc, argv);
    int ret = service.exec();
    closeLogFile();
    return ret;
}
