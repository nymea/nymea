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

#include "maxcube.h"

MaxCube::MaxCube(QObject *parent, QString serialNumber, QHostAddress hostAdress, quint16 port):
    QTcpSocket(parent), m_serialNumber(serialNumber), m_hostAddress(hostAdress), m_port(port)
{

    m_cubeInitialized = false;

    connect(this,SIGNAL(connected()),this,SLOT(connected()));
    connect(this,SIGNAL(disconnected()),this,SLOT(disconnected()));
    connect(this,SIGNAL(readyRead()),this,SLOT(readData()));
    connect(this,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(error(QAbstractSocket::SocketError)));

    connect(this,SIGNAL(cubeDataAvailable(QByteArray)),this,SLOT(processCubeData(QByteArray)));
}

QString MaxCube::serialNumber() const
{
    return m_serialNumber;
}

void MaxCube::setSerialNumber(const QString &serialNumber)
{
    m_serialNumber = serialNumber;
}

QByteArray MaxCube::rfAddress() const
{
    return m_rfAddress;
}

void MaxCube::setRfAddress(const QByteArray &rfAddress)
{
    m_rfAddress = rfAddress;
}

int MaxCube::firmware() const
{
    return m_firmware;
}

void MaxCube::setFirmware(const int &firmware)
{
    m_firmware = firmware;
}

QHostAddress MaxCube::hostAddress() const
{
    return m_hostAddress;
}

void MaxCube::setHostAddress(const QHostAddress &hostAddress)
{
    m_hostAddress = hostAddress;
}

quint16 MaxCube::port() const
{
    return m_port;
}

void MaxCube::setPort(const quint16 &port)
{
    m_port = port;
}

QByteArray MaxCube::httpConnectionId() const
{
    return m_httpConnectionId;
}

void MaxCube::setHttpConnectionId(const QByteArray &httpConnectionId)
{
    m_httpConnectionId = httpConnectionId;
}

int MaxCube::freeMemorySlots() const
{
    return m_freeMemorySlots;
}

void MaxCube::setFreeMemorySlots(const int &freeMemorySlots)
{
    m_freeMemorySlots = freeMemorySlots;
}

QDateTime MaxCube::cubeDateTime() const
{
    return m_cubeDateTime;
}

void MaxCube::setCubeDateTime(const QDateTime &cubeDateTime)
{
    m_cubeDateTime = cubeDateTime;
}

bool MaxCube::portalEnabeld() const
{
    return m_portalEnabeld;
}

QList<MaxDevice *> MaxCube::deviceList()
{
    return m_deviceList;
}

QList<Room *> MaxCube::roomList()
{
    return m_roomList;
}

void MaxCube::connectToCube()
{
    connectToHost(m_hostAddress,m_port);
}

void MaxCube::disconnectFromCube()
{
    disconnectFromHost();
}

bool MaxCube::sendData(QByteArray data)
{
    if(write(data) < 0){
        return false;
    }
    return true;
}

void MaxCube::decodeHelloMessage(QByteArray data)
{
    QList<QByteArray> list = data.split(',');
    m_cubeDateTime = calculateDateTime(list.at(7),list.at(8));

    m_rfAddress = list.at(1);
    m_firmware = list.at(2).toInt();

    qDebug() << "====================================================";
    qDebug() << "               HELLO message:";
    qDebug() << "====================================================";
    qDebug() << "           serial number | " << list.at(0);
    qDebug() << "        RF address (hex) | " << list.at(1);
    qDebug() << "                firmware | " << QString::number(list.at(2).toInt());
    qDebug() << "      HTTP connection id | " << list.at(4);
    qDebug() << "        duty cycle (hex) | " << list.at(5);
    qDebug() << " free memory slots (hex) | " << list.at(6);
    qDebug() << "               Cube date | " << m_cubeDateTime.date().toString("dd.MM.yyyy");
    qDebug() << "               Cube time | " << m_cubeDateTime.time().toString("HH:mm");
    qDebug() << "         State Cube Time | " << list.at(9);
    qDebug() << "             NTP counter | " << list.at(10);
}

void MaxCube::decodeMetadataMessage(QByteArray data)
{
    QList<QByteArray> list = data.left(data.length()-2).split(',');
    QByteArray dataDecoded = QByteArray::fromBase64(list.at(2));
    qDebug() << "====================================================";
    qDebug() << "               METADATA message:";
    qDebug() << "====================================================";

    // parse room list
    int roomCount = dataDecoded.toHex().mid(4,2).toInt(0,16);

    QByteArray roomRawData = dataDecoded.toHex();
    roomRawData = roomRawData.right(roomRawData.length()-6);

    for(int i = 0; i < roomCount; i++){
        Room *room = new Room(this);
        room->setRoomId(roomRawData.left(2).toInt(0,16));
        int roomNameLength = roomRawData.mid(2,2).toInt(0,16);
        room->setRoomName(QByteArray::fromHex(roomRawData.mid(4,roomNameLength*2)));
        room->setGroupRfAddress(roomRawData.mid(roomNameLength*2 + 4, 6));
        m_roomList.append(room);
        roomRawData = roomRawData.right(roomRawData.length() - ((roomNameLength*2) + 10));
    }
    qDebug() << "-------------------------|-------------------------";
    qDebug() << "found " << m_roomList.count() << "rooms";
    qDebug() << "-------------------------|-------------------------";

    foreach (Room *room, m_roomList) {
        qDebug() << "               Room Name | " << room->roomName();
        qDebug() << "                 Room ID | " << room->roomId();
        qDebug() << "        Group RF Address | " << room->groupRfAddress();
        qDebug() << "-------------------------|-------------------------";
    }

    // parse device list
    int deviceCount = roomRawData.left(2).toInt(0,16);
    QByteArray deviceRawData = roomRawData.right(roomRawData.length() - 2);

    for(int i = 0; i < deviceCount; i++){
        MaxDevice* device = new MaxDevice(this);
        device->setDeviceType(deviceRawData.left(2).toInt(0,16));
        device->setRfAddress(deviceRawData.mid(2,6));
        device->setSerialNumber(QByteArray::fromHex(deviceRawData.mid(8,20)));
        int deviceNameLenght = deviceRawData.mid(28,2).toInt(0,16);
        device->setDeviceName(QByteArray::fromHex(deviceRawData.mid(30,deviceNameLenght*2)));
        device->setRoomId(deviceRawData.mid(30 + deviceNameLenght*2,2).toInt(0,16));

        deviceRawData = deviceRawData.right(deviceRawData.length() - (32 + deviceNameLenght*2));
        m_deviceList.append(device);
    }

    qDebug() << "-------------------------|-------------------------";
    qDebug() << "found " << m_deviceList.count() << "devices";
    qDebug() << "-------------------------|-------------------------";

    foreach (MaxDevice *device, m_deviceList) {
        qDebug() << "             Device Name | " << device->deviceName();
        qDebug() << "            Serial Number| " << device->serialNumber();
        qDebug() << "      Device Type String | " << device->deviceTypeString();
        qDebug() << "        RF address (hex) | " << device->rfAddress();
        qDebug() << "                 Room ID | " << device->roomId();
        qDebug() << "-------------------------|-------------------------";
    }

    // set room data for each device
    foreach (MaxDevice *device, m_deviceList) {
        foreach (Room * room, m_roomList) {
            if(device->roomId() == room->roomId()){
                device->setRoomName(room->roomName());
            }
        }
    }

    emit deviceListReady(m_deviceList);
    m_cubeInitialized = true;
}

void MaxCube::decodeConfigMessage(QByteArray data)
{
    QList<QByteArray> list = data.split(',');
    if(list.count() < 2){
        return;
    }
    QByteArray rfAddress = list.at(0);
    QByteArray dataRaw = QByteArray::fromBase64(list.at(1)).toHex();
    int lengthData = dataRaw.left(2).toInt(0,16);
    if(rfAddress != dataRaw.mid(2,6)){
        qDebug() << "ERROR: rf addresses not equal!";
    }
    int deviceType = dataRaw.mid(8,2).toInt(0,16);
    //QByteArray unknown = dataRaw.mid(12,6);

    QByteArray serialNumber = QByteArray::fromHex(dataRaw.mid(16,20));
    qDebug() << "====================================================";
    qDebug() << "               CONFIG message:";
    qDebug() << "====================================================";
    qDebug() << "           Serial Number | " << serialNumber;
    qDebug() << "             device Type | " << deviceTypeString(deviceType);
    qDebug() << "        RF address (hex) | " << rfAddress;
    qDebug() << "             data length | " << lengthData;
    qDebug() << "-------------------------|-------------------------";

    switch (deviceType) {
    case MaxDevice::DeviceCube:{

        m_portalEnabeld = (bool)dataRaw.mid(36,2).toInt(0,16);

        qDebug() << "          portal enabled | " << m_portalEnabeld;
        qDebug() << "              portal URL | " << QString(QByteArray::fromHex(dataRaw.mid(170,68)));
        qDebug() << "               time zone | " << QString(QByteArray::fromHex(dataRaw.mid(428,6)));
        qDebug() << "      summer/winter time | " << QString(QByteArray::fromHex(dataRaw.mid(452,8)));
        break;
    }
    case MaxDevice::DeviceRadiatorThermostat:{
        int roomId = dataRaw.mid(10,2).toInt(0,16);
        int firmware = dataRaw.mid(12,2).toInt(0,16);
        double confortTemp = (double)dataRaw.mid(36,2).toInt(0,16) / 2.0;
        double ecoTemp = (double)dataRaw.mid(38,2).toInt(0,16) / 2.0;
        double maxSetPointTemp = (double)dataRaw.mid(40,2).toInt(0,16) / 2.0;
        double minSetPointTemp = (double)dataRaw.mid(42,2).toInt(0,16) / 2.0;
        double offsetTemp = (double)(dataRaw.mid(44,2).toInt(0,16) / 2.0 ) - 3.5;
        double windowOpenTemp = (double)dataRaw.mid(46,2).toInt(0,16)/2.0;
        int windowOpenDuration = dataRaw.mid(48,2).toInt(0,16);
        // boost code
        QByteArray boostDurationCode = fillBin(QByteArray::number(dataRaw.mid(50,2).toInt(0,16),2),8);
        int boostDuration = boostDurationCode.left(3).toInt(0,2) * 5;
        int valveValue = boostDurationCode.right(5).toInt(0,2) * 5;

        // day of week an time
        QByteArray dowTime = fillBin(QByteArray::number(dataRaw.mid(52,2).toInt(0,16),2),8);
        QString weekDay = weekDayString(dowTime.left(3).toInt(0,2));
        QTime time = QTime(dowTime.right(5).toInt(0,2),0);

        double valveMaximumSettings = (double)dataRaw.mid(54,2).toInt(0,16)*(double)100.0/255.0;
        double valveOffset = (double)dataRaw.mid(56,2).toInt(0,16)*100.0/255.0;

        qDebug() << "                 Room ID | " << roomId;
        qDebug() << "                firmware | " << firmware;
        qDebug() << "           Confort Temp. | " << confortTemp << "C";
        qDebug() << "               Eco Temp. | " << ecoTemp << "C";
        qDebug() << "    Max. Set Point Temp. | " << maxSetPointTemp << "C";
        qDebug() << "    Min. Set Point Temp. | " << minSetPointTemp << "C";
        qDebug() << "            Temp. Offset | " << offsetTemp << "C";
        qDebug() << "       Window Open Temp. | " << windowOpenTemp << "C";
        qDebug() << "   Window Open Duration  | " << windowOpenDuration << "min";
        qDebug() << "             Boost code  | " << boostDurationCode;
        qDebug() << "         Boost Duration  | " << boostDuration << "min";
        qDebug() << "             Valve value | " << valveValue << "%";
        qDebug() << "               dow code  | " << dowTime;
        qDebug() << "     disclaiming run day | " << weekDay;
        qDebug() << "    disclaiming run time | " << time.toString("HH:mm");
        qDebug() << "  Valve Maximum Settings | " << valveMaximumSettings << "%";
        qDebug() << "            Valve Offset | " << valveOffset << "%";
        parseWeeklyProgram(dataRaw.right(dataRaw.length() - 58));

        break;
    }
    case MaxDevice::DeviceRadiatorThermostatPlus:
        break;
    case MaxDevice::DeviceWallThermostat:{
        int roomId = dataRaw.mid(10,2).toInt(0,16);
        int firmware = dataRaw.mid(12,2).toInt(0,16);
        double confortTemp = (double)dataRaw.mid(36,2).toInt(0,16) / 2.0;
        double ecoTemp = (double)dataRaw.mid(38,2).toInt(0,16)/2.0;
        double maxSetPointTemp = (double)dataRaw.mid(40,2).toInt(0,16)/2.0;
        double minSetPointTemp = (double)dataRaw.mid(42,2).toInt(0,16)/2.0;

        qDebug() << "                 Room ID | " << roomId;
        qDebug() << "                firmware | " << firmware;
        qDebug() << "           Confort Temp. | " << confortTemp;
        qDebug() << "               Eco Temp. | " << ecoTemp;
        qDebug() << "    Max. Set Point Temp. | " << maxSetPointTemp;
        qDebug() << "    Min. Set Point Temp. | " << minSetPointTemp;

        parseWeeklyProgram(dataRaw.right(dataRaw.length() - 44));
        break;
    }
    case MaxDevice::DeviceEcoButton:
        break;
    case MaxDevice::DeviceWindowContact:
        break;
    default:
        qWarning() << "unknown device type: " << deviceType;
        break;
    }
}

void MaxCube::decodeDevicelistMessage(QByteArray data)
{
    qDebug() << "====================================================";
    qDebug() << "               LIVE message:";
    qDebug() << "====================================================";
    //qDebug() << data;
    QByteArray rawDataAll = QByteArray::fromBase64(data).toHex();
    qDebug() << rawDataAll;

    QList<QByteArray> deviceMessageList = splitMessage(rawDataAll);

    foreach (QByteArray rawData, deviceMessageList) {

        LiveMessage message;
        message.setRfAddress(rawData.mid(0,6));

        MaxDevice *device = getDeviceTypeFromRFAddress(message.rfAddress());

        switch (device->deviceType()) {
        case MaxDevice::DeviceWallThermostat:{
            QByteArray unknown = rawData.mid(6,2);

            QByteArray initCode = fillBin(QByteArray::number(rawData.mid(8,2).toInt(0,16),2),8);
            message.setInformationValid((bool)initCode.mid(3,1).toInt());
            message.setErrorOccured((bool)initCode.mid(4,1).toInt());
            message.setIsAnswereToCommand((bool)initCode.mid(5,1).toInt());
            message.setInitialized((bool)initCode.mid(6,1).toInt());

            QByteArray statusCode = fillBin(QByteArray::number(rawData.mid(10,2).toInt(0,16),2),8);
            message.setBatteryLow((bool)statusCode.mid(0,1).toInt());
            message.setLinkStatusOK(!(bool)statusCode.mid(1,1).toInt());
            message.setPanelLocked((bool)statusCode.mid(2,1).toInt());
            message.setGatewayKnown((bool)statusCode.mid(3,1).toInt());
            message.setDtsActive((bool)statusCode.mid(4,1).toInt());
            message.setDeviceMode(statusCode.right(2).toInt(0,2));

            message.setValvePosition((double)rawData.mid(12,2).toInt(0,16));

            // calculate current temperature and setpoint temperature
            // 00     00 00 00
            // 10
            QByteArray tempCode = fillBin(QByteArray::number(rawData.mid(14,2).toInt(0,16),2),8);
            message.setSetpointTemperatre((double)tempCode.right(6).toInt(0,2) / 2.0);
            double currentTemperature;
            if(tempCode.left(2) == "10"){
                currentTemperature = ((double)rawData.right(2).toInt(0,16) / 10.0) + 25.6;
            }else{
                currentTemperature = (double)rawData.right(2).toInt(0,16) / 10.0;
            }

            qDebug() << "                raw data | " << rawData;
            qDebug() << "             device type | " << device->deviceTypeString();
            qDebug() << "             device name | " << device->deviceName();
            qDebug() << "        RF address (hex) | " << message.rfAddress();
            qDebug() << "                 unknown | " << unknown;
            qDebug() << "                initCode | " << initCode;
            qDebug() << "       information valid | " << message.informationValid();
            qDebug() << "           error occured | " << message.errorOccured();
            qDebug() << " is answere to a command | " << message.isAnswereToCommand();
            qDebug() << "             initialized | " << message.initialized();
            qDebug() << "              statusCode | " << statusCode;
            qDebug() << "             battery low | " << message.batteryLow();
            qDebug() << "          link status OK | " << message.linkStatusOK();
            qDebug() << "            panel locked | " << message.panelLocked();
            qDebug() << "           gateway known | " << message.gatewayKnown();
            qDebug() << "     DST settings active | " << message.dtsActive();
            qDebug() << "             device mode | " << message.deviceModeString();
            qDebug() << "               temp Code | " << tempCode;
            qDebug() << "     Temperatur Setpoint | " << message.setpointTemperature();
            qDebug() << "            Current Temp | " << currentTemperature;
            qDebug() << "                 unknown | " << rawData.right(rawData.length() - 12);
            qDebug() << "-------------------------|-------------------------";



            emit liveMessageReady(message);
            break;
        }
        case MaxDevice::DeviceRadiatorThermostat:{
            QByteArray unknown = rawData.mid(6,2);

            QByteArray initCode = fillBin(QByteArray::number(rawData.mid(8,2).toInt(0,16),2),8);
            message.setInformationValid((bool)initCode.mid(3,1).toInt());
            message.setErrorOccured((bool)initCode.mid(4,1).toInt());
            message.setIsAnswereToCommand((bool)initCode.mid(5,1).toInt());
            message.setInitialized((bool)initCode.mid(6,1).toInt());

            QByteArray statusCode = fillBin(QByteArray::number(rawData.mid(10,2).toInt(0,16),2),8);
            message.setBatteryLow((bool)statusCode.mid(0,1).toInt());
            message.setLinkStatusOK(!(bool)statusCode.mid(1,1).toInt());
            message.setPanelLocked((bool)statusCode.mid(2,1).toInt());
            message.setGatewayKnown((bool)statusCode.mid(3,1).toInt());
            message.setDtsActive((bool)statusCode.mid(4,1).toInt());
            message.setDeviceMode(statusCode.right(2).toInt(0,2));

            message.setValvePosition((double)rawData.mid(12,2).toInt(0,16));
            message.setSetpointTemperatre((double)rawData.mid(14,2).toInt(0,16)/ 2.0);
            message.setDateTime(calculateDateTime(rawData.mid(18,4), rawData.mid(22,2)));

            qDebug() << "                raw data | " << rawData;
            qDebug() << "             device type | " << device->deviceTypeString();
            qDebug() << "             device name | " << device->deviceName();
            qDebug() << "        RF address (hex) | " << message.rfAddress();
            qDebug() << "                 unknown | " << unknown;
            qDebug() << "       information valid | " << message.informationValid();
            qDebug() << "           error occured | " << message.errorOccured();
            qDebug() << " is answere to a command | " << message.isAnswereToCommand();
            qDebug() << "             initialized | " << message.initialized();
            qDebug() << "             battery low | " << message.batteryLow();
            qDebug() << "          link status OK | " << message.linkStatusOK();
            qDebug() << "            panel locked | " << message.panelLocked();
            qDebug() << "           gateway known | " << message.gatewayKnown();
            qDebug() << "     DST settings active | " << message.dtsActive();
            qDebug() << "             device mode | " << message.deviceModeString();
            qDebug() << "          valve position | " << message.valvePosition() << "%";
            qDebug() << "     Temperatur Setpoint | " << message.setpointTemperature() << " deg C";
            qDebug() << "                 unknown | " << rawData.right(rawData.length() - 12);
            qDebug() << "-------------------------|-------------------------";

            emit liveMessageReady(message);
            break;
        }
        case MaxDevice::DeviceWindowContact:{
            QByteArray windowOpenRawData = fillBin(QByteArray::number(rawData.mid(10,2).toInt(0,16),2),8);
            bool windowOpen = (bool)windowOpenRawData.mid(5,1).toInt();

            qDebug() << "                raw data | " << rawData;
            qDebug() << "             device type | " << device->deviceTypeString();
            qDebug() << "             device name | " << device->deviceName();
            qDebug() << "        RF address (hex) | " << message.rfAddress();
            qDebug() << "        window open code | " << windowOpenRawData;
            qDebug() << "             window open | " << windowOpen;
            break;
        }
        default:
            break;
        }
    }
}

void MaxCube::parseWeeklyProgram(QByteArray data)
{
    for(int i=0; i < 7; i++){
        QByteArray dayData = data.left(52);
        //qDebug() << weekDayString(i);
        for(int i = 0; i < 52; i+=4){
            QByteArray element = fillBin(QByteArray::number(dayData.mid(i,4).toInt(0,16),2),16);
            int minutes = element.right(9).toInt(0,2) * 5;
            int hours = (minutes / 60) % 24;
            minutes = minutes % 60;
            QTime time = QTime(hours,minutes);
            //qDebug() << (double)element.left(7).toInt(0,2) / 2 << "\t" << "deg. until" << "\t" << time.toString("HH:mm");
        }


        data = data.right(data.length() - 52);
    }
    if(!data.isEmpty()){
        qDebug() << "                       ? | " << data;
    }
}

void MaxCube::decodeNewDeviceFoundMessage(QByteArray data)
{
    if(data.isEmpty()){
        return;
    }

    qDebug() << "====================================================";
    qDebug() << "               NEW DEVICE message:";
    qDebug() << "====================================================";
    qDebug() << "           Serial Number | " << QByteArray::fromBase64(data);

}

QDateTime MaxCube::calculateDateTime(QByteArray dateRaw, QByteArray timeRaw)
{
    QDate date;
    QTime time;
    date.setDate(dateRaw.left(2).toInt(0,16) + 2000, dateRaw.mid(2,2).toInt(0,16), dateRaw.right(2).toInt(0,16));
    time.setHMS(timeRaw.left(2).toInt(0,16), timeRaw.right(2).toInt(0,16), 0);

    return QDateTime(date,time);
}

QString MaxCube::deviceTypeString(int deviceType)
{
    QString deviceTypeString;

    switch (deviceType) {
    case MaxDevice::DeviceCube:
        deviceTypeString = "Cube";
        break;
    case MaxDevice::DeviceRadiatorThermostat:
        deviceTypeString = "Radiator Thermostat";
        break;
    case MaxDevice::DeviceRadiatorThermostatPlus:
        deviceTypeString = "Radiator Thermostat Plus";
        break;
    case MaxDevice::DeviceEcoButton:
        deviceTypeString = "Eco Button";
        break;
    case MaxDevice::DeviceWindowContact:
        deviceTypeString = "Window Contact";
        break;
    case MaxDevice::DeviceWallThermostat:
        deviceTypeString = "Wall Thermostat";
        break;
    default:
        deviceTypeString = "-";
        break;
    }

    return deviceTypeString;
}

QString MaxCube::weekDayString(int weekDay)
{
    QString weekDayString;

    switch (weekDay) {
    case Monday:
        weekDayString = "Monday";
        break;
    case Tuesday:
        weekDayString = "Tuesday";
        break;
    case Wednesday:
        weekDayString = "Wednesday";
        break;
    case Thursday:
        weekDayString = "Thursday";
        break;
    case Friday:
        weekDayString = "Friday";
        break;
    case Saturday:
        weekDayString = "Saturday";
        break;
    case Sunday:
        weekDayString = "Sunday";
        break;
    default:
        weekDayString = "-";
        break;
    }

    return weekDayString;
}

QByteArray MaxCube::fillBin(QByteArray data, int dataLength)
{
    QByteArray zeros;
    for(int i = 0; i < dataLength - data.length(); i++){
        zeros.append("0");
    }
    data = zeros.append(data);
    return data;
}

QList<QByteArray> MaxCube::splitMessage(QByteArray data)
{
    QList<QByteArray> messageList;
    while(!data.isEmpty()){
        int length = data.left(2).toInt(0,16)*2;
        messageList.append(data.mid(2,length));
        data = data.right(data.length() - (length+2));
    }
    qDebug() << messageList;
    return messageList;
}

MaxDevice *MaxCube::getDeviceTypeFromRFAddress(QByteArray rfAddress)
{
    foreach (MaxDevice *device, m_deviceList){
        if(device->rfAddress() == rfAddress){
            return device;
        }
    }
    return NULL;
}

void MaxCube::connected()
{
    qDebug() << "-> connected to cube " << m_serialNumber << m_hostAddress.toString();
    emit cubeConnectionStatusChanged(true);
}

void MaxCube::disconnected()
{
    m_cubeInitialized = false;
    qDebug() << "-> disconnected from cube " << m_serialNumber << m_hostAddress.toString();
    emit cubeConnectionStatusChanged(false);
}

void MaxCube::error(QAbstractSocket::SocketError error)
{
    qDebug() << "connection error (" << m_serialNumber << "): " << error;
    emit cubeConnectionStatusChanged(false);
}

void MaxCube::readData()
{
    QByteArray message;
    while(canReadLine()){
        QByteArray dataLine = readLine();
        message.append(dataLine);
    }
    emit cubeDataAvailable(message);
}

void MaxCube::processCubeData(const QByteArray &data)
{
    //qDebug() << "data" << data;
    if(data.startsWith("H")){
        decodeHelloMessage(data.right(data.length() -2 ));
        return;
    }
    // METADATA message
    if(data.startsWith("M")){
        decodeMetadataMessage(data.right(data.length() -2 ));
        return;
    }
    // CONFIG message
    if(data.startsWith("C")){
        QList<QByteArray> dataList = data.split('\r');
        foreach (QByteArray dataElement, dataList) {
            if(dataElement.startsWith("C")){
                decodeConfigMessage(dataElement.right(dataElement.length() -2 ));
            }
            if(dataElement.startsWith("\nC")){
                decodeConfigMessage(dataElement.right(dataElement.length() -3 ));
            }
        }
        return;
    }
    // LIVE message
    if(data.startsWith("L")){
        decodeDevicelistMessage(data.right(data.length() -2 ));
        return;
    }
    // NEWDEVICEFOUND message
    if(data.startsWith("N")){
        decodeNewDeviceFoundMessage(data.right(data.length() -2));
        return;
    }
    // ACK message
    if(data.startsWith("A")){
        qDebug() << "cube ACK!";
        emit cubeACK();
        return;
    }
    qDebug() << "  -> unknown message!!!!!!! from cube:" << data;
}

void MaxCube::enablePairingMode()
{
    qDebug() << "-------> enable pairing mode! press the boost button for min. 3 seconds";
    write("n:003c\r\n");
}

void MaxCube::disablePairingMode()
{
    qDebug() << " ----> disable pairing mode!";
    write("x:\r\n");
}

void MaxCube::refresh()
{
    if(m_cubeInitialized){
        write("l:\r\n");
    }
}

void MaxCube::customRequest(QByteArray data)
{
    qDebug() << " ----> custom request" << data;
    write(data + "\r\n");
}

