/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2016 Simon Stürz <simon.stuerz@guh.io>                   *
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

#include "networkconnection.h"
#include "dbus-interfaces.h"
#include "loggingcategories.h"

#include <QJsonDocument>

namespace guhserver {

NetworkConnection::NetworkConnection(const QDBusObjectPath &objectPath, QObject *parent) :
    QObject(parent),
    m_objectPath(objectPath)
{
    qDBusRegisterMetaType<ConnectionSettings>();

    QDBusInterface connectionInterface(serviceString, m_objectPath.path(), connectionsInterfaceString, QDBusConnection::systemBus());
    if(!connectionInterface.isValid()) {
        qCWarning(dcNetworkManager()) << "Invalid connection dbus interface";
        return;
    }

    QDBusMessage query = connectionInterface.call("GetSettings");
    if(query.type() != QDBusMessage::ReplyMessage) {
        qCWarning(dcNetworkManager()) << query.errorName() << query.errorMessage();
        return;
    }

    if (query.arguments().isEmpty())
        return;

    const QDBusArgument &argument = query.arguments().at(0).value<QDBusArgument>();
    ConnectionSettings settingsMap = qdbus_cast<ConnectionSettings>(argument);

    foreach (const QString &category, settingsMap.keys())
        if (category == "connection")
            qCDebug(dcNetworkManager()) << "    --> " << category << qUtf8Printable(QJsonDocument::fromVariant(settingsMap.value(category)).toJson(QJsonDocument::Indented));

}

QDBusObjectPath NetworkConnection::objectPath() const
{
    return m_objectPath;
}

}
