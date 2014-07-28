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

