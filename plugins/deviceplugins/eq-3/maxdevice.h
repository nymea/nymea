#ifndef MAXDEVICE_H
#define MAXDEVICE_H

#include <QObject>

#include "room.h"

class MaxDevice : public QObject
{
    Q_OBJECT
public:
    explicit MaxDevice(QObject *parent = 0);

    enum MaxDeviceType{
        DeviceCube = 0,
        DeviceRadiatorThermostat = 1,
        DeviceRadiatorThermostatPlus = 2,
        DeviceWallThermostat = 3,
        DeviceWindowContact = 4,
        DeviceEcoButton = 5
    };

    int deviceType();
    void setDeviceType(int deviceType);

    QString deviceTypeString();

    QByteArray rfAddress();
    void setRfAddress(QByteArray rfAddress);

    QString serialNumber();
    void setSerialNumber(QString serialNumber);

    QString deviceName();
    void setDeviceName(QString deviceName);

    int roomId();
    void setRoomId(int roomId);

    Room *room();
    void setRoom(Room *room);
private:
    int m_deviceType;
    QString m_deviceTypeString;
    QByteArray m_rfAddress;
    QString m_serialNumber;
    QString m_deviceName;
    int m_roomId;
    Room *m_room;
    bool m_batteryOk;

signals:

public slots:

};

#endif // MAXDEVICE_H
