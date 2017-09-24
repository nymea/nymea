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

#ifndef DBUSINTERFACES_H
#define DBUSINTERFACES_H

#include <QString>

namespace guhserver {

static const QString serviceString("org.freedesktop.NetworkManager");

static const QString pathString("/org/freedesktop/NetworkManager");
static const QString settingsPathString("/org/freedesktop/NetworkManager/Settings");

static const QString deviceInterfaceString("org.freedesktop.NetworkManager.Device");
static const QString wirelessInterfaceString("org.freedesktop.NetworkManager.Device.Wireless");
static const QString wiredInterfaceString("org.freedesktop.NetworkManager.Device.Wired");
static const QString accessPointInterfaceString("org.freedesktop.NetworkManager.AccessPoint");
static const QString settingsInterfaceString("org.freedesktop.NetworkManager.Settings");
static const QString connectionsInterfaceString("org.freedesktop.NetworkManager.Settings.Connection");

}

#endif // DBUSINTERFACES_H
