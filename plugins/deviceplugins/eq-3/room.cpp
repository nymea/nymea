/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
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

#include "room.h"

Room::Room(QObject *parent) :
    QObject(parent)
{
}

int Room::roomId()
{
    return m_roomId;
}

void Room::setRoomId(int roomId)
{
    m_roomId = roomId;
}

QString Room::roomName()
{
    return m_roomName;
}

void Room::setRoomName(QString roomName)
{
    m_roomName = roomName;
}

QByteArray Room::groupRfAddress()
{
    return m_groupRfAddress;
}

void Room::setGroupRfAddress(QByteArray groupRfAddress)
{
    m_groupRfAddress = groupRfAddress;
}
