#include "maxdevice.h"

MaxDevice::MaxDevice(QObject *parent) :
    QObject(parent)
{
}

int MaxDevice::deviceType()
{
    return m_deviceType;
}

void MaxDevice::setDeviceType(int deviceType)
{
    m_deviceType = deviceType;

    switch (m_deviceType) {
    case DeviceCube:
        m_deviceTypeString = "Cube";
        break;
    case DeviceRadiatorThermostat:
        m_deviceTypeString = "Radiator Thermostat";
        break;
    case DeviceRadiatorThermostatPlus:
        m_deviceTypeString = "Radiator Thermostat Plus";
        break;
    case DeviceEcoButton:
        m_deviceTypeString = "Eco Button";
        break;
    case DeviceWindowContact:
        m_deviceTypeString = "Window Contact";
        break;
    case DeviceWallThermostat:
        m_deviceTypeString = "Wall Thermostat";
        break;
    default:
        m_deviceTypeString = "-";
        break;
    }
}

QString MaxDevice::deviceTypeString()
{
    return m_deviceTypeString;
}

QByteArray MaxDevice::rfAddress()
{
    return m_rfAddress;
}

void MaxDevice::setRfAddress(QByteArray rfAddress)
{
    m_rfAddress = rfAddress;
}

QString MaxDevice::serialNumber()
{
    return m_serialNumber;
}

void MaxDevice::setSerialNumber(QString serialNumber)
{
    m_serialNumber = serialNumber;
}

QString MaxDevice::deviceName()
{
    return m_deviceName;
}

void MaxDevice::setDeviceName(QString deviceName)
{
    m_deviceName = deviceName;
}

int MaxDevice::roomId()
{
    return m_roomId;
}

void MaxDevice::setRoomId(int roomId)
{
    m_roomId = roomId;
}

Room *MaxDevice::room()
{
    return m_room;
}

void MaxDevice::setRoom(Room *room)
{
    m_room = room;
}

