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
#include <QDebug>
#include <QObject>

#include "../../utils/pushbuttonagent.h"
#include "inputwatcher.h"

int main(int argc, char *argv[])
{
    QCommandLineParser parser;
    parser.addHelpOption();
    QCommandLineOption dbusOption(QStringList() << "session",
                                  QCoreApplication::translate("SimplePushButtonHandler",
                                                              "If specified, all D-Bus interfaces will be bound to the session bus instead of the system bus."));
    parser.addOption(dbusOption);

    QCoreApplication a(argc, argv);

    parser.process(a);

    PushButtonAgent agent;
    if (!agent.init(parser.isSet(dbusOption) ? QDBusConnection::SessionBus : QDBusConnection::SystemBus)) {
        return -1;
    }
    InputWatcher inputWatcher;
    QObject::connect(&inputWatcher, &InputWatcher::enterPressed, &agent, &PushButtonAgent::sendButtonPressed);

    qDebug() << "Use the Enter key as push button.";
    return a.exec();
}
