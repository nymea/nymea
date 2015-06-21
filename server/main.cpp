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
#include <QtPlugin>

#include "guhcore.h"
#include "guhservice.h"
#include "loggingcategories.h"

QHash<QString, bool> s_loggingFilters;

void loggingCategoryFilter(QLoggingCategory *category)
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

int main(int argc, char *argv[])
{
    //qInstallMessageHandler(myMessageOutput);
    QCoreApplication application(argc, argv);
    application.setOrganizationName("guh");
    application.setApplicationName("guhd");
    application.setApplicationVersion(GUH_VERSION_STRING);

    // filter for core and libguh
    s_loggingFilters.insert("Application", true);
    s_loggingFilters.insert("Warnings", true);
    s_loggingFilters.insert("DeviceManager", true);
    s_loggingFilters.insert("RuleEngine", true);
    s_loggingFilters.insert("Connection", true);
    s_loggingFilters.insert("JsonRpc", false);
    s_loggingFilters.insert("Hardware", false);
    s_loggingFilters.insert("LogEngine", false);

    // plugins
    #ifdef boblight
    s_loggingFilters.insert("Boblight", false);
    #endif
    s_loggingFilters.insert("CommandLauncher", false);
    s_loggingFilters.insert("RF433", false);
    s_loggingFilters.insert("DateTime", false);
    s_loggingFilters.insert("EQ-3", false);
    s_loggingFilters.insert("LgSmartTv", false);
    s_loggingFilters.insert("Lircd", false);
    s_loggingFilters.insert("MailNotification", false);
    s_loggingFilters.insert("Mock", false);
    s_loggingFilters.insert("Openweahtermap", false);
    s_loggingFilters.insert("PhilipsHue", false);
    s_loggingFilters.insert("Tune", false);
    s_loggingFilters.insert("UdpCommander", false);
    s_loggingFilters.insert("WakeOnLan", false);
    s_loggingFilters.insert("Wemo", false);
    s_loggingFilters.insert("WifiDetector", false);


    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    QString applicationDescription = QString("\nguh ( /[guːh]/ ) is an open source home automation server, which allows to\n"
                                  "control a lot of different devices from many different manufacturers.\n\n"
                                  "guhd %1 (C) 2014-2015 guh\n"
                                  "Released under the GNU GENERAL PUBLIC LICENSE Version 2.").arg(GUH_VERSION_STRING);

    parser.setApplicationDescription(applicationDescription);

    QCommandLineOption foregroundOption(QStringList() << "n" << "no-daemon", QCoreApplication::translate("main", "Run guhd in the foreground, not as daemon."));
    parser.addOption(foregroundOption);

    QString debugDescription = QString("Debug categories to enable. Prefix with \"No\" to disable. Warnings from all categories will be printed unless explicitly muted with \"NoWarnings\". \n\nCategories are:");
    QStringList sortedFilterList = QStringList(s_loggingFilters.keys());
    sortedFilterList.sort();
    foreach (const QString &filterName, sortedFilterList) {
        debugDescription += "\n- " + filterName + " (" + (s_loggingFilters.value(filterName) ? "yes" : "no") + ")";
    }
    QCommandLineOption debugOption(QStringList() << "d" << "debug", debugDescription, "[No]DebugCategory");
    parser.addOption(debugOption);

    parser.process(application);

    // check debug area
    foreach (QString debugArea, parser.values(debugOption)) {
        bool enable = !debugArea.startsWith("No");
        debugArea.remove(QRegExp("^No"));
        if (s_loggingFilters.contains(debugArea)) {
            s_loggingFilters[debugArea] = enable;
        } else {
            qCWarning(dcApplication) << "No such debug category:" << debugArea;
        }
    }
    QLoggingCategory::installFilter(loggingCategoryFilter);

    bool startForeground = parser.isSet(foregroundOption);
    if (startForeground) {
        GuhCore::instance()->setRunningMode(GuhCore::RunningModeApplication);
        return application.exec();
    }

    GuhService service(argc, argv);
    return service.exec();
}
