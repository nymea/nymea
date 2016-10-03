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

#ifndef NETWORKSETTINGS_H
#define NETWORKSETTINGS_H

#include <QObject>
#include <QDBusObjectPath>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusArgument>

#include "networkconnection.h"

namespace guhserver {

class NetworkConnection;

class NetworkSettings : public QObject
{
    Q_OBJECT
public:
    explicit NetworkSettings(QObject *parent = 0);

    QDBusObjectPath addConnection(const ConnectionSettings &settings);

private:
    QDBusInterface *m_settingsInterface;
    QHash<QDBusObjectPath, NetworkConnection *> m_connections;

    void loadConnections();

signals:

private slots:
    void connectionAdded(const QDBusObjectPath &objectPath);
    void connectionRemoved(const QDBusObjectPath &objectPath);
    void propertiesChanged(const QVariantMap &properties);

};

}

#endif // NETWORKSETTINGS_H
