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

#include <QCoreApplication>
#include <QDebug>
#include <QObject>
#include <QCommandLineParser>
#include <QCommandLineOption>

#include "../../utils/pushbuttonagent.h"
#include "inputwatcher.h"

int main(int argc, char *argv[])
{
    QCommandLineParser parser;
    parser.addHelpOption();
    QCommandLineOption dbusOption(QStringList() << "session", QCoreApplication::translate("SimplePushButtonHandler", "If specified, all D-Bus interfaces will be bound to the session bus instead of the system bus."));
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
