/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2017 Michael Zanetti <michael.zanetti@guh.io>            *
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

#include "pushbuttonagent.h"

#include <QDBusMessage>
#include <QDBusObjectPath>
#include <QDebug>
#include <QTimer>

Q_LOGGING_CATEGORY(dcPushButtonAgent, "PushButtonAgent")

PushButtonAgent::PushButtonAgent(QObject *parent) : QObject(parent)
{
}

bool PushButtonAgent::init(QDBusConnection::BusType busType)
{
    QDBusConnection bus = busType == QDBusConnection::SessionBus ? QDBusConnection::sessionBus() : QDBusConnection::systemBus();

    bool result = bus.registerObject("/nymea/pushbuttonhandler", this, QDBusConnection::ExportScriptableContents);
    if (!result) {
        qCWarning(dcPushButtonAgent()) << "Error registering PushButton agent on D-Bus" << (busType == QDBusConnection::SessionBus ? "session" : "system") << "bus.";
        return false;
    }

    QDBusMessage message = QDBusMessage::createMethodCall("io.guh.nymead", "/io/guh/nymead/UserManager", "io.guh.nymead", "RegisterButtonAgent");
    message << QVariant::fromValue(QDBusObjectPath("/nymea/pushbuttonhandler"));
    QDBusMessage reply = bus.call(message);
    if (!reply.errorName().isEmpty()) {
        qCWarning(dcPushButtonAgent()) << "Error registering PushButton agent:" << reply.errorMessage();
        return false;
    }
    qCDebug(dcPushButtonAgent()) << "PushButton agent registered.";
    return true;
}

void PushButtonAgent::sendButtonPressed()
{
    qCDebug(dcPushButtonAgent()) << "Sending button pressed event.";
    emit PushButtonPressed();
}
