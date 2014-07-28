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

#ifndef ROOM_H
#define ROOM_H

#include <QObject>

class Room : public QObject
{
    Q_OBJECT
public:
    explicit Room(QObject *parent = 0);

    int roomId();
    void setRoomId(int roomId);

    QString roomName();
    void setRoomName(QString roomName);

    QByteArray groupRfAddress();
    void setGroupRfAddress(QByteArray groupRfAddress);

private:
    int m_roomId;
    QString m_roomName;
    QByteArray m_groupRfAddress;

signals:



public slots:

};

#endif // ROOM_H
