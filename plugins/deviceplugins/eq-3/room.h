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
