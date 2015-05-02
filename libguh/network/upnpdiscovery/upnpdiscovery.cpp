/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stuerz <simon.stuerz@guh.guru>                *
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

/*!
  \class UpnpDiscovery
  \brief Allows to detect UPnP devices in the network.

  \ingroup hardware
  \inmodule libguh

  This resource allows plugins to discover UPnP devices in the network and receive notification messages. The resource
  will bind a UDP socket to the multicast 239.255.255.250 on port 1900.

  The communication was implementet using following documentation: \l{http://upnp.org/specs/arch/UPnP-arch-DeviceArchitecture-v1.1.pdf}

  \sa UpnpDevice, UpnpDeviceDescriptor
*/

/*!
 \fn UpnpDiscovery::discoveryFinished(const QList<UpnpDeviceDescriptor> &deviceDescriptorList, const PluginId & pluginId)
 This signal will be emitted if the discovery call from a \l{DevicePlugin}{Plugin} with the given \a pluginId is finished. The found devices
 will be passed with the \a deviceDescriptorList paramter.
 \sa DevicePlugin::upnpDiscoveryFinished()
 */

/*!
 \fn UpnpDiscovery::upnpNotify(const QByteArray &notifyMessage)
 This signal will be emitted when a UPnP NOTIFY message \a notifyMessage will be recognized.
 \sa DevicePlugin::upnpNotifyReceived()
 */


#include "upnpdiscovery.h"

/*! Construct the hardware resource UpnpDiscovery with the given \a parent. */
UpnpDiscovery::UpnpDiscovery(QObject *parent) :
    QUdpSocket(parent)
{
    // bind udp socket and join multicast group
    m_port = 1900;
    m_host = QHostAddress("239.255.255.250");

    setSocketOption(QAbstractSocket::MulticastTtlOption,QVariant(1));
    setSocketOption(QAbstractSocket::MulticastLoopbackOption,QVariant(1));

    if(!bind(QHostAddress::AnyIPv4, m_port, QUdpSocket::ShareAddress)){
        qWarning() << "ERROR: UPnP discovery could not bind to port" << m_port;
        return;
    }

    if(!joinMulticastGroup(m_host)){
        qWarning() << "ERROR: UPnP discovery could not join multicast group" << m_host;
        return;
    }

    // network access manager for requesting device information
    m_networkAccessManager = new QNetworkAccessManager(this);
    connect(m_networkAccessManager, &QNetworkAccessManager::finished, this, &UpnpDiscovery::replyFinished);

    connect(this,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(error(QAbstractSocket::SocketError)));
    connect(this, &UpnpDiscovery::readyRead, this, &UpnpDiscovery::readData);

    qDebug() << "--> UPnP discovery created successfully.";
}

/*! Returns false, if the \l{UpnpDiscovery} resource is not available. Returns true, if a device with
 * the given \a searchTarget, \a userAgent and \a pluginId can be discovered.*/
bool UpnpDiscovery::discoverDevices(const QString &searchTarget, const QString &userAgent, const PluginId &pluginId)
{
    if(state() != BoundState){
        qWarning() << "ERROR: UPnP not bound to port 1900";
        return false;
    }

    // create a new request
    UpnpDiscoveryRequest *request = new UpnpDiscoveryRequest(this, pluginId, searchTarget, userAgent);
    connect(request, &UpnpDiscoveryRequest::discoveryTimeout, this, &UpnpDiscovery::discoverTimeout);

    request->discover();
    m_discoverRequests.append(request);
    return true;
}


void UpnpDiscovery::requestDeviceInformation(const QNetworkRequest &networkRequest, const UpnpDeviceDescriptor &upnpDeviceDescriptor)
{
    QNetworkReply *replay;
    replay = m_networkAccessManager->get(networkRequest);
    m_informationRequestList.insert(replay, upnpDeviceDescriptor);
}

/*! This method will be called to send the SSDP message \a data to the UPnP multicast.*/
void UpnpDiscovery::sendToMulticast(const QByteArray &data)
{
    writeDatagram(data, m_host, m_port);
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

    //qDebug() << "======================";
    //qDebug() << data;

    if (data.contains("NOTIFY")) {
        emit upnpNotify(data);
        return;
    }

    // if the data contains the HTTP OK header...
    if (data.contains("HTTP/1.1 200 OK")) {
        const QStringList lines = QString(data).split("\r\n");
        foreach (const QString& line, lines) {
            int separatorIndex = line.indexOf(':');
            QString key = line.left(separatorIndex).toUpper();
            QString value = line.mid(separatorIndex+1).trimmed();

            // get location
            if (key.contains("LOCATION")) {
                location = QUrl(value);
            }
        }

        UpnpDeviceDescriptor upnpDeviceDescriptor;
        upnpDeviceDescriptor.setLocation(location);
        upnpDeviceDescriptor.setHostAddress(hostAddress);
        upnpDeviceDescriptor.setPort(location.port());

        foreach (UpnpDiscoveryRequest *upnpDiscoveryRequest, m_discoverRequests) {
            QNetworkRequest networkRequest = upnpDiscoveryRequest->createNetworkRequest(upnpDeviceDescriptor);
            requestDeviceInformation(networkRequest, upnpDeviceDescriptor);
        }
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
        while (!xml.atEnd() && !xml.hasError()) {
            xml.readNext();
            if (xml.isStartDocument()) {
                continue;
            }
            if (xml.isStartElement()) {
                if (xml.name().toString() == "device") {
                    while (!xml.atEnd()) {
                        if (xml.name() == "deviceType" && xml.isStartElement()) {
                            upnpDeviceDescriptor.setDeviceType(xml.readElementText());
                        }
                        if (xml.name() == "friendlyName" && xml.isStartElement()) {
                            upnpDeviceDescriptor.setFriendlyName(xml.readElementText());
                        }
                        if (xml.name() == "manufacturer" && xml.isStartElement()) {
                            upnpDeviceDescriptor.setManufacturer(xml.readElementText());
                        }
                        if (xml.name() == "manufacturerURL" && xml.isStartElement()) {
                            upnpDeviceDescriptor.setManufacturerURL(QUrl(xml.readElementText()));
                        }
                        if (xml.name() == "modelDescription" && xml.isStartElement()) {
                            upnpDeviceDescriptor.setModelDescription(xml.readElementText());
                        }
                        if (xml.name() == "modelName" && xml.isStartElement()) {
                            upnpDeviceDescriptor.setModelName(xml.readElementText());
                        }
                        if (xml.name() == "modelNumber" && xml.isStartElement()) {
                            upnpDeviceDescriptor.setModelNumber(xml.readElementText());
                        }
                        if (xml.name() == "modelURL" && xml.isStartElement()) {
                            upnpDeviceDescriptor.setModelURL(QUrl(xml.readElementText()));
                        }
                        if (xml.name() == "serialNumber" && xml.isStartElement()) {
                            upnpDeviceDescriptor.setSerialNumber(xml.readElementText());
                        }
                        if (xml.name() == "UDN" && xml.isStartElement()) {
                            upnpDeviceDescriptor.setUuid(xml.readElementText());
                        }
                        if (xml.name() == "uuid" && xml.isStartElement()) {
                            upnpDeviceDescriptor.setUuid(xml.readElementText());
                        }
                        if (xml.name() == "UPC" && xml.isStartElement()) {
                            upnpDeviceDescriptor.setUpc(xml.readElementText());
                        }
                        xml.readNext();
                    }
                    xml.readNext();
                }
            }
        }

        foreach (UpnpDiscoveryRequest *upnpDiscoveryRequest, m_discoverRequests) {
            upnpDiscoveryRequest->addDeviceDescriptor(upnpDeviceDescriptor);
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
    UpnpDiscoveryRequest *discoveryRequest = static_cast<UpnpDiscoveryRequest*>(sender());
    emit discoveryFinished(discoveryRequest->deviceList(), discoveryRequest->pluginId());

    m_discoverRequests.removeOne(discoveryRequest);
    delete discoveryRequest;
}
