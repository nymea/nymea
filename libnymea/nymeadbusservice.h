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

#ifndef NYMEADBUSSERVICE_H
#define NYMEADBUSSERVICE_H

#include <QObject>
#include <QDBusConnection>
#include <QDBusContext>

class NymeaDBusService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "io.guh.nymead")

public:
    explicit NymeaDBusService(const QString &objectPath, QObject *parent = nullptr);

    static void setBusType(QDBusConnection::BusType busType);

    bool isValid();

protected:
    QDBusConnection connection() const;

private:
    static QDBusConnection::BusType s_busType;
    QDBusConnection m_connection;
    bool m_isValid = false;

};


#endif // NYMEADBUSSERVICE_H
