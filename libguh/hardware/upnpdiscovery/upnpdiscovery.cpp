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

#include "upnpdiscovery.h"

UpnpDiscovery::UpnpDiscovery(QObject *parent) :
    QUdpSocket(parent)
{
    // bind udp socket and join multicast group
    m_port = 1900;
    m_host = QHostAddress("239.255.255.250");

    setSocketOption(QAbstractSocket::MulticastTtlOption,QVariant(1));
    setSocketOption(QAbstractSocket::MulticastLoopbackOption,QVariant(1));

    if(!bind(QHostAddress::AnyIPv4,m_port,QUdpSocket::ShareAddress)){
        qWarning() << "ERROR: UPnP discovery could not bind to port " << m_port;
        return;
    }

    if(!joinMulticastGroup(m_host)){
        qWarning() << "ERROR: UPnP discovery could not join multicast group " << m_host;
        return;
    }

    m_deviceList.clear();
    m_informationRequestList.clear();

    // network access manager for requesting device information
    m_networkAccessManager = new QNetworkAccessManager(this);
    connect(m_networkAccessManager,SIGNAL(finished(QNetworkReply*)),this,SLOT(replyFinished(QNetworkReply*)));

    // discovery refresh timer
    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    connect(m_timer, &QTimer::timeout, this, &UpnpDiscovery::discoverTimeout);

    connect(this,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(error(QAbstractSocket::SocketError)));
    connect(this, &UpnpDiscovery::readyRead, this, &UpnpDiscovery::readData);

    qDebug() << "--> Successfully created UPnPDiscovery.";
}

bool UpnpDiscovery::discoverDevices(const QString &searchTarget, const QString &userAgent, const PluginId &pluginId)
{
    // clear the list...
    m_deviceList.clear();
    foreach (QNetworkReply* reply, m_informationRequestList.keys()) {
        m_informationRequestList.remove(reply);
        reply->deleteLater();
    }

    m_searchTarget = searchTarget;
    m_userAgent = userAgent;
    m_pluginId = pluginId;

    QByteArray ssdpSearchMessage = QByteArray("M-SEARCH * HTTP/1.1\r\n"
                                              "HOST:239.255.255.250:1900\r\n"
                                              "MAN:\"ssdp:discover\"\r\n"
                                              "MX:2\r\n"
                                              "ST: " + m_searchTarget.toUtf8() + "\r\n"
                                              "USR-AGENT: " + m_userAgent.toUtf8() + "\r\n\r\n");

    writeDatagram(ssdpSearchMessage,m_host,m_port);

    m_timer->start(3000);
    return true;
}

void UpnpDiscovery::requestDeviceInformation(const UpnpDeviceDescriptor &upnpDeviceDescriptor)
{
    QNetworkRequest deviceRequest;
    deviceRequest.setUrl(upnpDeviceDescriptor.location());
    deviceRequest.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("text/xml"));
    deviceRequest.setHeader(QNetworkRequest::UserAgentHeader,QVariant(m_userAgent));

    QNetworkReply *replay;
    replay = m_networkAccessManager->get(deviceRequest);
    m_informationRequestList.insert(replay, upnpDeviceDescriptor);
}

void UpnpDiscovery::error(QAbstractSocket::SocketError error)
{
    qWarning() << errorString() << error;
}

void UpnpDiscovery::readData()
{
    QByteArray data;
    QHostAddress hostAddress;
    QUrl location;

    // read the answere from the multicast
    while (hasPendingDatagrams()) {
        data.resize(pendingDatagramSize());
        readDatagram(data.data(), data.size(), &hostAddress);
    }
//    qDebug() << "-----------------------";
//    qDebug() << data;

    // if the data contains the HTTP OK header...
    if(data.contains("HTTP/1.1 200 OK")){
        const QStringList lines = QString(data).split("\r\n");
        foreach( const QString& line, lines){
            int separatorIndex = line.indexOf(':');
            QString key = line.left(separatorIndex).toUpper();
            QString value = line.mid(separatorIndex+1).trimmed();

            // get location
            if(key.contains("LOCATION")){
                location = QUrl(value);
            }
        }

        UpnpDeviceDescriptor upnpDeviceDescriptor;
        upnpDeviceDescriptor.setLocation(location);
        upnpDeviceDescriptor.setHostAddress(hostAddress);
        upnpDeviceDescriptor.setPort(location.port());

        requestDeviceInformation(upnpDeviceDescriptor);
    }
}

void UpnpDiscovery::replyFinished(QNetworkReply *reply)
{
    int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    switch (status) {
    case(200):{
        QByteArray data = reply->readAll();
        UpnpDeviceDescriptor upnpDeviceDescriptor = m_informationRequestList.take(reply);

        // parse XML data
        QXmlStreamReader xml(data);
        while(!xml.atEnd() && !xml.hasError()){
            xml.readNext();
            if(xml.isStartDocument()){
                continue;
            }
            if(xml.isStartElement()){
                if(xml.name().toString() == "device"){
                    while(!xml.atEnd()){
                        if(xml.name() == "deviceType" && xml.isStartElement()){
                            upnpDeviceDescriptor.setDeviceType(xml.readElementText());
                        }
                        if(xml.name() == "friendlyName" && xml.isStartElement()){
                            upnpDeviceDescriptor.setFriendlyName(xml.readElementText());
                        }
                        if(xml.name() == "manufacturer" && xml.isStartElement()){
                            upnpDeviceDescriptor.setManufacturer(xml.readElementText());
                        }
                        if(xml.name() == "manufacturerURL" && xml.isStartElement()){
                            upnpDeviceDescriptor.setManufacturerURL(QUrl(xml.readElementText()));
                        }
                        if(xml.name() == "modelDescription" && xml.isStartElement()){
                            upnpDeviceDescriptor.setModelDescription(xml.readElementText());
                        }
                        if(xml.name() == "modelName" && xml.isStartElement()){
                            upnpDeviceDescriptor.setModelName(xml.readElementText());
                        }
                        if(xml.name() == "modelNumber" && xml.isStartElement()){
                            upnpDeviceDescriptor.setModelNumber(xml.readElementText());
                        }
                        if(xml.name() == "modelURL" && xml.isStartElement()){
                            upnpDeviceDescriptor.setModelURL(QUrl(xml.readElementText()));
                        }
                        if(xml.name() == "serialNumber" && xml.isStartElement()){
                            upnpDeviceDescriptor.setSerialNumber(xml.readElementText());
                        }
                        if(xml.name() == "UDN" && xml.isStartElement()){
                            upnpDeviceDescriptor.setUuid(xml.readElementText());
                        }
                        if(xml.name() == "UPC" && xml.isStartElement()){
                            upnpDeviceDescriptor.setUpc(xml.readElementText());
                        }
                        xml.readNext();
                    }
                    xml.readNext();
                }
            }
        }

        // check if we allready have the device in the list
        bool isAlreadyInList = false;
        foreach (UpnpDeviceDescriptor deviceDescriptor, m_deviceList) {
            if(deviceDescriptor.uuid() == upnpDeviceDescriptor.uuid()){
                isAlreadyInList = true;
            }
        }
        if(!isAlreadyInList){
            m_deviceList.append(upnpDeviceDescriptor);
        }
        break;
    }
    default:
        qWarning() << "HTTP request error " << status;
        m_informationRequestList.remove(reply);
    }

    reply->deleteLater();
}

void UpnpDiscovery::discoverTimeout()
{
    emit discoveryFinished(m_deviceList, m_pluginId);
}
