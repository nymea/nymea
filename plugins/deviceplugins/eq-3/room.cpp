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
