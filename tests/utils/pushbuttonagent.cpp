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

PushButtonAgent::PushButtonAgent(QObject *parent) : QObject(parent)
{
}

bool PushButtonAgent::init(QDBusConnection::BusType busType)
{
    QDBusConnection bus = busType == QDBusConnection::SessionBus ? QDBusConnection::sessionBus() : QDBusConnection::systemBus();

    bool result = bus.registerObject("/guh/pushbuttonhandler", this, QDBusConnection::ExportScriptableContents);
    if (!result) {
        qDebug() << "Error registering PushButton agent on D-Bus.";
        return false;
    }

    QDBusMessage message = QDBusMessage::createMethodCall("io.guh.nymead", "/io/guh/nymead/UserManager", QString(), "RegisterButtonAgent");
    message << qVariantFromValue(QDBusObjectPath("/guh/pushbuttonhandler"));
    QDBusMessage reply = bus.call(message);
    if (!reply.errorName().isEmpty()) {
        qDebug() << "Error registering PushButton agent:" << reply.errorMessage();
        return false;
    }
    qDebug() << "PushButton agent registered.";
    return true;
}

void PushButtonAgent::sendButtonPressed()
{
    qDebug() << "Sending button pressed event.";
    emit PushButtonPressed();
}
