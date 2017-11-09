/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2017 Michael Zanetti <michael.zanetti@guh.io>            *
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

#include "guhdbusservice.h"

namespace guhserver {

QDBusConnection GuhDBusService::s_connection = QDBusConnection::systemBus();

GuhDBusService::GuhDBusService(const QString &objectPath, UserManager *parent) : QObject(parent)
{
    bool status = s_connection.registerService("io.guh.nymead");
    if (!status) {
        qCWarning(dcApplication) << "Failed to register D-Bus service.";
        return;
    }
    status = s_connection.registerObject(objectPath, this, QDBusConnection::ExportScriptableContents);
    if (!status) {
        qCWarning(dcApplication()) << "Failed to register D-Bus object.";
        return;
    }
    m_isValid = true;
}

bool GuhDBusService::isValid()
{
    return m_isValid;
}

QDBusConnection GuhDBusService::connection() const
{
    return s_connection;
}

void GuhDBusService::setBusType(QDBusConnection::BusType busType)
{
    if (busType == QDBusConnection::SessionBus) {
        s_connection = QDBusConnection::sessionBus();
    } else {
        s_connection = QDBusConnection::systemBus();
    }
}

}
