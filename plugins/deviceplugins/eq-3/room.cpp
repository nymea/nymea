/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
 *                                                                         *
 *  This file is part of guh.                                              *
 *                                                                         *
 *  This library is free software; you can redistribute it and/or          *
 *  modify it under the terms of the GNU Lesser General Public             *
 *  License as published by the Free Software Foundation; either           *
 *  version 2.1 of the License, or (at your option) any later version.     *
 *                                                                         *
 *  This library is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
 *  Lesser General Public License for more details.                        *
 *                                                                         *
 *  You should have received a copy of the GNU Lesser General Public       *
 *  License along with this library; If not, see                           *
 *  <http://www.gnu.org/licenses/>.                                        *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "room.h"
#include "extern-plugininfo.h"

Room::Room(QObject *parent) :
    QObject(parent)
{
}

int Room::roomId() const
{
    return m_roomId;
}

void Room::setRoomId(const int &roomId)
{
    m_roomId = roomId;
}

QString Room::roomName() const
{
    return m_roomName;
}

void Room::setRoomName(const QString &roomName)
{
    m_roomName = roomName;
}

QByteArray Room::groupRfAddress() const
{
    return m_groupRfAddress;
}

void Room::setGroupRfAddress(const QByteArray &groupRfAddress)
{
    m_groupRfAddress = groupRfAddress;
}
