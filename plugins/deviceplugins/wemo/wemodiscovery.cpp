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

#include "wemodiscovery.h"

WemoDiscovery::WemoDiscovery(QObject *parent) :
    QUdpSocket(parent)
{
    m_timeout = new QTimer(this);
    m_timeout->setSingleShot(true);
    connect(m_timeout,SIGNAL(timeout()),this,SLOT(discoverTimeout()));

    m_manager = new QNetworkAccessManager(this);
    connect(m_manager,SIGNAL(finished(QNetworkReply*)),this,SLOT(replyFinished(QNetworkReply*)));

    m_port = 1900;
    m_host = QHostAddress("239.255.255.250");
    setSocketOption(QAbstractSocket::MulticastTtlOption,QVariant(1));
    setSocketOption(QAbstractSocket::MulticastLoopbackOption,QVariant(1));
    bind(QHostAddress::AnyIPv4,m_port,QUdpSocket::ShareAddress);

    if(!joinMulticastGroup(m_host)){
        qWarning() << "ERROR: could not join multicast group";
    }

    connect(this,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(error(QAbstractSocket::SocketError)));
    connect(this,SIGNAL(readyRead()),this,SLOT(readData()));
}

bool WemoDiscovery::checkXmlData(QByteArray data)
{
    QByteArray xmlOut;
    QXmlStreamReader reader(data);
    QXmlStreamWriter writer(&xmlOut);
    writer.setAutoFormatting(true);

    while (!reader.atEnd()) {
        reader.readNext();
        if (!reader.isWhitespace()) {
            writer.writeCurrentToken(reader);
        }
    }
    if(reader.hasError()){
        qDebug() << "ERROR reading XML device information:   " << reader.errorString();
        qDebug() << "--------------------------------------------";
        return false;
    }
    m_deviceInformationData = xmlOut;
    return true;
}

QString WemoDiscovery::printXmlData(QByteArray data)
{
    QString xmlOut;
    QXmlStreamReader reader(data);
    QXmlStreamWriter writer(&xmlOut);
    writer.setAutoFormatting(true);

    while (!reader.atEnd()) {
        reader.readNext();
        if (!reader.isWhitespace()) {
            writer.writeCurrentToken(reader);
        }
    }
    if(reader.hasError()){
        qDebug() << "ERROR reading XML device information:   " << reader.errorString();
        qDebug() << "--------------------------------------------";
    }
    return xmlOut;
}

void WemoDiscovery::error(QAbstractSocket::SocketError error)
{
    qWarning() << errorString() << error;
}

void WemoDiscovery::sendDiscoverMessage()
{
    QByteArray ssdpSearchMessage("M-SEARCH * HTTP/1.1\r\n"
                                 "HOST:239.255.255.250:1900\r\n"
                                 "ST:upnp:rootdevice\r\n"
                                 "MX:2\r\n"
                                 "MAN:\"ssdp:discover\"\r\n\r\n");
    writeDatagram(ssdpSearchMessage,m_host,m_port);
}

void WemoDiscovery::readData()
{
    QByteArray data;
    QHostAddress sender;
    quint16 udpPort;

    // read the answere from the multicast
    while (hasPendingDatagrams()) {
        data.resize(pendingDatagramSize());
        readDatagram(data.data(), data.size(), &sender, &udpPort);
    }

    if(data.contains("HTTP/1.1 200 OK")){
        const QStringList lines = QString(data).split("\r\n");

        QUrl location;
        QString uuid;

        foreach( const QString& line, lines){
            int separatorIndex = line.indexOf(':');
            QString key = line.left(separatorIndex).toUpper();
            QString value = line.mid(separatorIndex+1).trimmed();

            // get location
            if(key.contains("LOCATION")){
                location = QUrl(value);
            }

            // get uuid
            if(key.contains("USN")){
                int startIndex = value.indexOf(":");
                int endIndex = value.indexOf("::");
                uuid = value.mid(startIndex +1 ,(endIndex - startIndex)-1);
                // check if we found a socket...else return
                if(!uuid.startsWith("Socket-1_0")){
                    return;
                }
            }

            if(!location.isEmpty() && !uuid.isEmpty()){
                // check if we allready discovered this device
                foreach (WemoSwitch *device, m_deviceList) {
                    if(device->uuid() == uuid){
                        return;
                    }
                }

                // get port from location (it changes between 49152-5 so fare...)
                QByteArray locationData = location.toString().toUtf8();
                locationData = locationData.left(locationData.length() - 10);
                qDebug() << "locationData" << locationData;
                int port = locationData.right(5).toInt();


                WemoSwitch *device = new WemoSwitch(this);
                device->setHostAddress(sender);
                device->setUuid(uuid);
                device->setLocation(location);
                device->setPort(port);

                qDebug() << "--> UPnP searcher discovered wemo...";
                qDebug() << "location: " << device->location().toString();
                qDebug() << "ip: " << device->hostAddress().toString();
                qDebug() << "uuid: " << device->uuid();
                qDebug() << "port: " << device->port();
                qDebug() << "--------------------------------------------";

                m_deviceList.append(device);
                requestDeviceInformation(location);
            }
        }
    }
}

void WemoDiscovery::discoverTimeout()
{
    emit discoveryDone(m_deviceList);
}

void WemoDiscovery::requestDeviceInformation(QUrl location)
{
    m_manager->get(QNetworkRequest(location));
}

void WemoDiscovery::replyFinished(QNetworkReply *reply)
{
    int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    switch (status) {
    case(200):
        parseDeviceInformation(reply->readAll());
        break;
    default:
        qWarning() << "HTTP request error " << status;
        return;
    }
}

void WemoDiscovery::parseDeviceInformation(QByteArray data)
{

    QXmlStreamReader xml(data);

    QString name;
    QString uuid;
    QString modelName;
    QString modelDescription;
    QString serialNumber;
    QString deviceType;
    QString manufacturer;

    while(!xml.atEnd() && !xml.hasError()){
        xml.readNext();
        if(xml.isStartDocument()){
            continue;
        }
        if(xml.isStartElement()){
            if(xml.name().toString() == "device"){
                while(!xml.atEnd()){
                    if(xml.name() == "friendlyName" && xml.isStartElement()){
                        name = xml.readElementText();
                    }
                    if(xml.name() == "manufacturer" && xml.isStartElement()){
                        manufacturer = xml.readElementText();
                    }
                    if(xml.name() == "modelDescription" && xml.isStartElement()){
                        modelDescription = xml.readElementText();
                    }
                    if(xml.name() == "modelName" && xml.isStartElement()){
                        modelName = xml.readElementText();
                    }
                    if(xml.name() == "serialNumber" && xml.isStartElement()){
                        serialNumber = xml.readElementText();
                    }
                    if(xml.name() == "deviceType" && xml.isStartElement()){
                        deviceType = xml.readElementText();
                    }
                    if(xml.name() == "UDN" && xml.isStartElement()){
                        uuid = xml.readElementText();
                        if(uuid.startsWith("uuid:")){
                            uuid = uuid.right(uuid.length()-5);
                        }
                    }
                    xml.readNext();
                }
                xml.readNext();
            }
        }
    }
    foreach (WemoSwitch *device, m_deviceList) {
        // find our device with this uuid
        if(device->uuid() == uuid){
            device->setName(name);
            device->setModelName(modelName);
            device->setDeviceType(deviceType);
            device->setManufacturer(manufacturer);
            device->setModelDescription(modelDescription);
            device->setSerialNumber(serialNumber);

            qDebug() << "--> fetched Wemo information...";
            qDebug() << "name:              " << device->name();
            qDebug() << "model name:        " << device->modelName();
            qDebug() << "device type:       " << device->deviceType();
            qDebug() << "manufacturer:      " << device->manufacturer();
            qDebug() << "address:           " << device->hostAddress().toString();
            qDebug() << "location:          " << device->location().toString();
            qDebug() << "uuid:              " << device->uuid();
            qDebug() << "model description  " << device->modelDescription();
            qDebug() << "serial number      " << device->serialNumber();
            qDebug() << "--------------------------------------------";
        }
    }
}

void WemoDiscovery::discover(int timeout)
{
    m_deviceList.clear();
    m_timeout->stop();
    sendDiscoverMessage();
    m_timeout->start(timeout);
}
