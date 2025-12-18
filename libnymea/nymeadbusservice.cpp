// SPDX-License-Identifier: LGPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2025, chargebyte austria GmbH
*
* This file is part of nymea.
*
* nymea is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* as published by the Free Software Foundation, either version 3
* of the License, or (at your option) any later version.
*
* nymea is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with nymea. If not, see <https://www.gnu.org/licenses/>.
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "nymeadbusservice.h"
#include "loggingcategories.h"

NYMEA_LOGGING_CATEGORY(dcDBus, "DBus")

namespace {
const QString kNymeaInterface = QStringLiteral("io.nymea.nymead");
const QString kLegacyInterface = QStringLiteral("io.guh.nymead");
const QString kNymeaNamespace = QStringLiteral("/io/nymea/");
const QString kLegacyNamespace = QStringLiteral("/io/guh/");
bool gLegacyServiceWarningLogged = false;
bool gLegacyObjectWarningLogged = false;
} // namespace

QDBusConnection::BusType NymeaDBusService::s_busType = QDBusConnection::SystemBus;

NymeaDBusService::NymeaDBusService(const QString &objectPath, QObject *parent)
    : QObject(parent)
    , m_connection(s_busType == QDBusConnection::SystemBus ? QDBusConnection::systemBus() : QDBusConnection::sessionBus())
{
    bool status = m_connection.registerService(kNymeaInterface);
    if (!status) {
        qCWarning(dcDBus()) << "Failed to register D-Bus service" << kNymeaInterface;
        return;
    }

    bool legacyServiceStatus = m_connection.registerService(kLegacyInterface);
    if (!legacyServiceStatus) {
        qCWarning(dcDBus()) << "Failed to register legacy D-Bus service" << kLegacyInterface << "- older clients might not be able to connect.";
    } else if (!gLegacyServiceWarningLogged) {
        qCWarning(dcDBus()) << "The D-Bus service" << kLegacyInterface << "is deprecated and kept only for backwards compatibility. Please migrate to" << kNymeaInterface << ".";
        gLegacyServiceWarningLogged = true;
    }

    QString normalizedObjectPath;
    foreach (const QString &part, objectPath.split(' ')) {
        normalizedObjectPath.append(part.at(0).toUpper());
        normalizedObjectPath.append(part.right(part.length() - 1));
    }

    QString nymeaObjectPath = normalizedObjectPath;
    QString legacyObjectPath;
    bool registerLegacyObject = false;

    if (normalizedObjectPath.contains(kNymeaNamespace)) {
        legacyObjectPath = normalizedObjectPath;
        legacyObjectPath.replace(kNymeaNamespace, kLegacyNamespace);
        registerLegacyObject = true;
    } else if (normalizedObjectPath.contains(kLegacyNamespace)) {
        legacyObjectPath = normalizedObjectPath;
        nymeaObjectPath = normalizedObjectPath;
        nymeaObjectPath.replace(kLegacyNamespace, kNymeaNamespace);
        registerLegacyObject = true;
    }

    status = m_connection.registerObject(nymeaObjectPath, kNymeaInterface, parent, QDBusConnection::ExportScriptableSlots);
    if (!status) {
        qCWarning(dcDBus()) << "Failed to register D-Bus object:" << nymeaObjectPath;
        return;
    }

    if (registerLegacyObject) {
        bool legacyObjectStatus = m_connection.registerObject(legacyObjectPath, kLegacyInterface, parent, QDBusConnection::ExportScriptableSlots);
        if (!legacyObjectStatus) {
            qCWarning(dcDBus()) << "Failed to register deprecated legacy D-Bus object:" << legacyObjectPath;
        } else if (!gLegacyObjectWarningLogged) {
            qCWarning(dcDBus()) << "The D-Bus interface" << kLegacyInterface << "is deprecated and only kept for backwards compatibility.";
            gLegacyObjectWarningLogged = true;
        }
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
