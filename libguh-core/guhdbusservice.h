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

#ifndef GUHDBUSSERVICE_H
#define GUHDBUSSERVICE_H

#include <QObject>
#include <QDBusConnection>
#include "guhcore.h"

namespace guhserver {

class GuhDBusService : public QObject, public QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "io.guh.nymead")

public:
    explicit GuhDBusService(const QString &objectPath, UserManager *parent = nullptr);

    static void setBusType(QDBusConnection::BusType busType);

    bool isValid();

protected:
    QDBusConnection connection() const;

private:
    static QDBusConnection s_connection;
    bool m_isValid = false;

};

}

#endif // GUHDBUSSERVICE_H
