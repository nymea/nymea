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
            qWarning(dcTranslations()) << "Could not find nymead translations for" << QLocale::system().name() << endl << (QDir(QCoreApplication::applicationDirPath() + "../../translations/").absolutePath()) << endl << NymeaSettings::translationsPath();



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

    QCommandLineOption logOption({"l", "log"}, QCoreApplication::translate("nymea", "Specify a log file to write to, if this option is not specified, logs will be printed to the standard output."), "logfile");
    parser.addOption(logOption);

    QCommandLineOption noColorOption({"c", "no-colors"}, QCoreApplication::translate("nymea", "Log output is colorized by default. Use this option to disable colors."));
    parser.addOption(noColorOption);

    QCommandLineOption dbusOption(QStringList() << "session", QCoreApplication::translate("nymea", "If specified, all D-Bus interfaces will be bound to the session bus instead of the system bus."));
    parser.addOption(dbusOption);

    QCommandLineOption debugOption(QStringList() << "d" << "debug-category", debugDescription, "[No]DebugCategory[Warnings]");
    parser.addOption(debugOption);

    parser.process(application);

    // Open the logfile, if any specified
    if (!initLogging(parser.value(logOption), !parser.isSet(noColorOption))) {
        qWarning() << "Error opening log file" << parser.value(logOption);
        return 1;
    }

    /* The logging rules will be evaluated sequentially
     *  1. All debug categories off
     *  2. Enable all debug categories if requested from command line (-p)
     *  3. The stored categories from the nymead.conf will be appended
     *  4. Add the individual command line params will be added (-d)
     *  5. QT_LOGGING_CONF
     *  6. QT_LOGGING_RULES
     *
     * The final filter rules will be set.
     */

    // 1. All debug categories off
    QStringList loggingRules;
    loggingRules << "*.debug=false";

    // 2. Enable all debug categories making sense if requested from command line (-p)
    if (parser.isSet(allOption)) {
        loggingRules << "*.debug=true";
        loggingRules << "*Traffic.debug=false";
        loggingRules << "*Debug.debug=false";
    }

    // 3. The stored categories from the nymead.conf will be appended
    NymeaSettings nymeaSettings(NymeaSettings::SettingsRoleGlobal);
    nymeaSettings.beginGroup("LoggingRules");
    foreach (const QString &category, nymeaSettings.childKeys()) {
        loggingRules << QString("%1=%2").arg(category).arg(nymeaSettings.value(category, "false").toString());
    }
    nymeaSettings.endGroup();

    // 4. Add the individual command line params will be added (-d)
    foreach (QString debugArea, parser.values(debugOption)) {
        bool enable = !debugArea.startsWith("No");
        bool isWarning = debugArea.endsWith("Warnings");
        debugArea.remove(QRegExp("^No"));
        debugArea.remove(QRegExp("Warnings$"));
        loggingRules.append(QString("%1.%2=%3").arg(debugArea).arg(isWarning ? "warning" : "debug").arg(enable ? "true": "false"));
//        if (loggingFilters.contains(debugArea) || loggingFiltersPlugins.contains(debugArea)) {
//        } else {
//            qCWarning(dcApplication) << QCoreApplication::translate("nymea", "No such debug category:") << debugArea;
//        }
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
        closeLogFile();
        return ret;
    }

    NymeaService service(argc, argv);
    int ret = service.exec();
    closeLogFile();
    return ret;
}
