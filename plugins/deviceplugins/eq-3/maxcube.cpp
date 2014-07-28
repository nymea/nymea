#include "maxcube.h"

MaxCube::MaxCube(QObject *parent, QString serialNumber, QHostAddress hostAdress, quint16 port):
    QTcpSocket(parent), m_serialNumber(serialNumber), m_hostAddress(hostAdress), m_port(port)
{
    connect(this,SIGNAL(connected()),this,SLOT(connected()));
    connect(this,SIGNAL(disconnected()),this,SLOT(disconnected()));
    connect(this,SIGNAL(readyRead()),this,SLOT(readData()));
    connect(this,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(error(QAbstractSocket::SocketError)));

    connect(this,SIGNAL(cubeDataAvailable(QByteArray)),this,SLOT(processCubeData(QByteArray)));
}

QString MaxCube::serialNumber()
{
    return m_serialNumber;
}

void MaxCube::setSerialNumber(QString serialNumber)
{
    m_serialNumber = serialNumber;
}

QByteArray MaxCube::rfAddress()
{
    return m_rfAddress;
}

void MaxCube::setRfAddress(QByteArray rfAddress)
{
    m_rfAddress = rfAddress;
}

int MaxCube::firmware()
{
    return m_firmware;
}

void MaxCube::setFirmware(int firmware)
{
    m_firmware = firmware;
}

QHostAddress MaxCube::hostAddress()
{
    return m_hostAddress;
}

void MaxCube::setHostAddress(QHostAddress hostAddress)
{
    m_hostAddress = hostAddress;
}

quint16 MaxCube::port()
{
    return m_port;
}

void MaxCube::setPort(quint16 port)
{
    m_port = port;
}

QByteArray MaxCube::httpConnectionId()
{
    return m_httpConnectionId;
}

void MaxCube::setHttpConnectionId(QByteArray httpConnectionId)
{
    m_httpConnectionId = httpConnectionId;
}

int MaxCube::freeMemorySlots()
{
    return m_freeMemorySlots;
}

void MaxCube::setFreeMemorySlots(int freeMemorySlots)
{
    m_freeMemorySlots = freeMemorySlots;
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

void MaxCube::parseHelloMessage(QByteArray data)
{

    QList<QByteArray> list = data.split(',');
    m_cubeDateTime = calculateDateTime(list.at(7),list.at(8));
    qDebug() << "====================================================";
    qDebug() << "HELLO message:";
    qDebug() << "====================================================";
    qDebug() << "           serial number | " << list.at(0);
    qDebug() << "        RF address (hex) | " << list.at(1);
    qDebug() << "                firmware | " << QString::number(list.at(2).toInt());
    qDebug() << "                       ? | " << list.at(3);
    qDebug() << "      HTTP connection id | " << list.at(4);
    qDebug() << "        duty cycle (hex) | " << list.at(5);
    qDebug() << " free memory slots (hex) | " << list.at(6);
    qDebug() << "               Cube date | " << m_cubeDateTime.date().toString("dd.MM.yyyy");
    qDebug() << "               Cube time | " << m_cubeDateTime.time().toString("HH:mm");
    qDebug() << "         State Cube Time | " << list.at(9);
    qDebug() << "             NTP counter | " << list.at(10);
}

void MaxCube::parseMetadataMessage(QByteArray data)
{
    QList<QByteArray> list = data.left(data.length()-2).split(',');
    QByteArray dataDecoded = QByteArray::fromBase64(list.at(2));
    qDebug() << "====================================================";
    qDebug() << "METADATA message:";
    qDebug() << "====================================================";
    //    qDebug() << "                   Index | " << list.at(0);
    //    qDebug() << "                   Count | " << list.at(1);
    //    qDebug() << "     Data Base64 encoded | " << list.at(2);
    //    qDebug() << "     Data Base64 decoded | " << dataDecoded;
    //    qDebug() << " Data Base64 decoded hex | " << dataDecoded.toHex();


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
        deviceRawData = deviceRawData.right(deviceRawData.length() - (30 + deviceNameLenght*2));
        //qDebug() << "rawdata left :" << deviceRawData;
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
        //qDebug() << "             Device Type | " << device->deviceType();
        qDebug() << "                 Room ID | " << device->roomId();
        qDebug() << "-------------------------|-------------------------";
    }
}

void MaxCube::parseConfigMessage(QByteArray data)
{
    QList<QByteArray> list = data.split(',');
    QByteArray rfAddress = list.at(0);
    QByteArray dataRaw = QByteArray::fromBase64(list.at(1)).toHex();
    //int lengthData = dataRaw.left(2).toInt(0,16);
    //QByteArray rfAddress = dataRaw.mid(2,6);
    int deviceType = dataRaw.mid(8,2).toInt(0,16);
    int roomId = dataRaw.mid(10,2).toInt(0,16);
    //QByteArray unknown = dataRaw.mid(12,6);

    QByteArray serialNumber = QByteArray::fromHex(dataRaw.mid(16,20));
    qDebug() << "====================================================";
    qDebug() << "CONFIG message:";
    qDebug() << "====================================================";
    qDebug() << "             device Type | " << deviceTypeString(deviceType);
    qDebug() << "           Serial Number | " << serialNumber;
    qDebug() << "        RF address (hex) | " << rfAddress;
    qDebug() << "                 Room ID | " << roomId;
    qDebug() << "-------------------------|-------------------------";

    switch (deviceType) {
    case MaxDevice::DeviceCube:{
        qDebug() << QByteArray::fromHex(dataRaw.mid(dataRaw.length() - 32));
        break;
    }
    case MaxDevice::DeviceEcoButton:
        break;
    case MaxDevice::DeviceWallThermostat:
        break;
    case MaxDevice::DeviceRadiatorThermostatPlus:
        break;
    case MaxDevice::DeviceRadiatorThermostat:{
        double confortTemp = dataRaw.mid(36,2).toInt(0,16)/2;
        double ecoTemp = dataRaw.mid(38,2).toInt(0,16)/2;
        double maxSetPointTemp = dataRaw.mid(40,2).toInt(0,16)/2;
        double minSetPointTemp = dataRaw.mid(42,2).toInt(0,16)/2;
        double offsetTemp = (dataRaw.mid(44,2).toInt(0,16) / 2 ) - 3.5;
        double windowOpenTemp = dataRaw.mid(46,2).toInt(0,16)/2;
        int windowOpenDuration = dataRaw.mid(48,2).toInt(0,16);
        // boost code
        QByteArray boostDurationCode = QByteArray::number(dataRaw.mid(50,2).toInt(0,16),2);
        qDebug() << boostDurationCode;
        int boostDuration = boostDurationCode.left(3).toInt(0,2);
        if(boostDuration = 7){
            boostDuration = 30;
        }else{
            boostDuration *= 5;
        }
        int valveValue = boostDurationCode.right(5).toInt(0,2);
        // day of week an time
        QByteArray dowTime = QByteArray::number(dataRaw.mid(52,2).toInt(0,16),2);
        double valveMaximumSettings = dataRaw.mid(54,2).toInt(0,16)*(double)100/255;
        double valveOffset = dataRaw.mid(56,2).toInt(0,16)*(double)100/255;

        qDebug() << "           Serial Number | " << serialNumber;
        qDebug() << "           Confort Temp. | " << confortTemp;
        qDebug() << "               Eco Temp. | " << ecoTemp;
        qDebug() << "    Max. Set Point Temp. | " << maxSetPointTemp;
        qDebug() << "    Min. Set Point Temp. | " << minSetPointTemp;
        qDebug() << "            Temp. Offset | " << offsetTemp;
        qDebug() << "       Window Open Temp. | " << windowOpenTemp;
        qDebug() << "   Window Open Duration  | " << windowOpenDuration;
        qDebug() << "         Boost Duration  | " << boostDuration << "min";
        qDebug() << "             Valve value | " << valveValue << "%";
        qDebug() << "    Day of week and time | " << dowTime;
        qDebug() << "  Valve Maximum Settings | " << valveMaximumSettings << "%";
        qDebug() << "            Valve Offset | " << valveOffset << "%";
        parseWeeklyProgram(dataRaw.right(dataRaw.length() - 58));

        break;
    }
    default:
        qWarning() << "unknown device type: " << deviceType;
        break;
    }
}

void MaxCube::parseDevicelistMessage(QByteArray data)
{
    QList<QByteArray> list = data.split(',');
    qDebug() << "====================================================";
    qDebug() << "DEVICELIST message:";
    qDebug() << "====================================================";
    foreach (const QByteArray &code, list) {
        QByteArray rawData = QByteArray::fromBase64(code).toHex();
        qDebug() << "                    Code | " << rawData.toBase64();
        qDebug() << "              Code (Hex) | " << rawData;
        qDebug() << "          Length of data | " << rawData.left(2).toInt(0,16);
        qDebug() << "        RF address (hex) | " << rawData.mid(2,6);
        qDebug() << "          Initialization | " << QByteArray::number(rawData.mid(10,2).toInt(0,16),2);
        qDebug() << "          Battery, ... , | " << QByteArray::number(rawData.mid(12,2).toInt(0,16),2);
        qDebug() << "          Valve Position | " << QByteArray::number(rawData.mid(14,2).toInt(0,16),2) << "%";
        qDebug() << "     Temperatur Setpoint | " << (double)rawData.mid(16,2).toInt(0,16) / 2.0 << "%";
        QDateTime dateTime = calculateDateTime(rawData.mid(18,4), rawData.mid(22,2));
        qDebug() << "               Cube date | " << dateTime.date().toString("dd.MM.yyyy");
        qDebug() << "               Cube time | " << dateTime.time().toString("HH:mm");
    }
}

void MaxCube::parseWeeklyProgram(QByteArray data)
{
    for(int i=0; i<7; i++){
        QByteArray dayData = data.left(52);
        qDebug() << dayData;
        data = data.right(data.length() - 52);
    }
    qDebug() << "data left" << data;
}

void MaxCube::parseNewDeviceFoundMessage(QByteArray data)
{
    if(data.isEmpty()){
        return;
    }

    qDebug() << "====================================================";
    qDebug() << "NEW DEVICE message:";
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

QByteArray MaxCube::fillBin(QByteArray data, int dataLength)
{

}

void MaxCube::connected()
{
    qDebug() << "-> connected to cube " << m_serialNumber << m_hostAddress.toString();
    emit cubeConnectionStatusChanged(true);
}

void MaxCube::disconnected()
{
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
        parseHelloMessage(data.right(data.length() -2 ));
        return;
    }
    // METADATA message
    if(data.startsWith("M")){
        parseMetadataMessage(data.right(data.length() -2 ));
        return;
    }
    // CONFIG message
    if(data.startsWith("C")){
        parseConfigMessage(data.right(data.length() -2 ));
        return;
    }
    // DEVICELIST message
    if(data.startsWith("L")){
        //customRequest("g:");
        parseDevicelistMessage(data.right(data.length() -2 ));
        return;
    }
    // NEWDEVICEFOUND message
    if(data.startsWith("N")){
        parseNewDeviceFoundMessage(data.right(data.length() -2));
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
    qDebug() << "refresh cube " << m_serialNumber;
    if(isWritable()){
        write("l:\r\n");
    }else{
        qDebug() << "ERROR: could not send to " << m_hostAddress.toString();
    }
}

void MaxCube::customRequest(QByteArray data)
{
    qDebug() << " ----> custom request" << data;
    write(data + "\r\n");
}

