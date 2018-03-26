/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2017 Michael Zanetti <michael.zanetti@guh.io>            *
 *  Copyright (C) 2018 Simon Stürz <simon.stuerz@guh.io>                   *
 *                                                                         *
 *  This file is part of nymea.                                            *
 *                                                                         *
 *  nymea is free software: you can redistribute it and/or modify          *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  nymea is distributed in the hope that it will be useful,               *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with nymea. If not, see <http://www.gnu.org/licenses/>.          *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "nymeadbusservice.h"
#include "loggingcategories.h"

QDBusConnection::BusType NymeaDBusService::s_busType = QDBusConnection::SystemBus;

NymeaDBusService::NymeaDBusService(const QString &objectPath, QObject *parent):
    QObject(parent),
    m_connection(s_busType == QDBusConnection::SystemBus ? QDBusConnection::systemBus() : QDBusConnection::sessionBus())
{
    bool status = m_connection.registerService("io.guh.nymead");
    if (!status) {
        qCWarning(dcApplication()) << "Failed to register D-Bus service.";
        return;
    }
    QString finalObjectPath;
    foreach (const QString &part, objectPath.split(' ')) {
        finalObjectPath.append(part.at(0).toUpper());
        finalObjectPath.append(part.right(part.length() - 1));
    }
    status = m_connection.registerObject(finalObjectPath, "io.guh.nymead", parent, QDBusConnection::ExportScriptableSlots);
    if (!status) {
        qCWarning(dcApplication()) << "Failed to register D-Bus object:" << finalObjectPath;
        return;
    }
    m_isValid = true;
}

bool NymeaDBusService::isValid()
{
    return m_isValid;
}

QDBusConnection NymeaDBusService::connection() const
{
    return m_connection;
}

/*! This will configure all NymeaDBusService objects to be registered on the given \a busType.
 * Note that this will only affect instances created after this method has been called.
 */
void NymeaDBusService::setBusType(QDBusConnection::BusType busType)
{
    s_busType = busType;
}

