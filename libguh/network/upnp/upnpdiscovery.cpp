/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
 *                                                                         *
 *  This file is part of guh.                                              *
 *                                                                         *
 *  This library is free software; you can redistribute it and/or          *
 *  modify it under the terms of the GNU Lesser General Public             *
 *  License as published by the Free Software Foundation; either           *
 *  version 2.1 of the License, or (at your option) any later version.     *
 *                                                                         *
 *  This library is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
 *  Lesser General Public License for more details.                        *
 *                                                                         *
 *  You should have received a copy of the GNU Lesser General Public       *
 *  License along with this library; If not, see                           *
 *  <http://www.gnu.org/licenses/>.                                        *
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
#include "loggingcategories.h"
#include "guhsettings.h"

#include <QNetworkInterface>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

/*! Construct the hardware resource UpnpDiscovery with the given \a parent. */
UpnpDiscovery::UpnpDiscovery(QNetworkAccessManager *networkAccessManager, QObject *parent) :
    HardwareResource(HardwareResource::TypeUpnpDisovery, "UPnP discovery", parent),
    m_networkAccessManager(networkAccessManager)
{
    m_notificationTimer = new QTimer(this);
    m_notificationTimer->setInterval(30000);
    m_notificationTimer->setSingleShot(false);
    connect(m_notificationTimer, &QTimer::timeout, this, &UpnpDiscovery::notificationTimeout);

    setAvailable(true);
    qCDebug(dcDeviceManager) << "-->" << name() << "created successfully.";
}

/*! Destruct this \l{UpnpDiscovery} object. */
UpnpDiscovery::~UpnpDiscovery()
{
    qCDebug(dcApplication) << "Shutting down \"UPnP Server\"";
    sendByeByeMessage();
    m_socket->waitForBytesWritten();
    m_socket->close();
}

/*! Returns false, if the \l{UpnpDiscovery} resource is not available. Returns true, if a device with
 * the given \a searchTarget, \a userAgent and \a pluginId can be discovered.*/
bool UpnpDiscovery::discoverDevices(const QString &searchTarget, const QString &userAgent, const PluginId &pluginId)
{
    if(m_socket->state() != QUdpSocket::BoundState){
        qCWarning(dcHardware) << "UPnP not bound to port 1900";
        return false;
    }

    qCDebug(dcHardware) << name() << "discover" << searchTarget << userAgent;

    // create a new request
    UpnpDiscoveryRequest *request = new UpnpDiscoveryRequest(this, pluginId, searchTarget, userAgent);
    connect(request, &UpnpDiscoveryRequest::discoveryTimeout, this, &UpnpDiscovery::discoverTimeout);

    request->discover();
    m_discoverRequests.append(request);
    return true;
}


void UpnpDiscovery::requestDeviceInformation(const QNetworkRequest &networkRequest, const UpnpDeviceDescriptor &upnpDeviceDescriptor)
{
    QNetworkReply *replay = m_networkAccessManager->get(networkRequest);
    m_informationRequestList.insert(replay, upnpDeviceDescriptor);
}

void UpnpDiscovery::respondToSearchRequest(QHostAddress host, int port)
{
    // TODO: Once DeviceManager (and with that this can be moved into the server, use GuhCore's configuration manager instead of parsing the config here...
    GuhSettings globalSettings(GuhSettings::SettingsRoleGlobal);
    globalSettings.beginGroup("guhd");
    QByteArray uuid = globalSettings.value("uuid", QUuid()).toByteArray();
    globalSettings.endGroup();

    globalSettings.beginGroup("WebServer");
    int serverPort = -1;
    bool useSsl = false;
    foreach (const QString &group, globalSettings.childGroups()) {
        globalSettings.beginGroup(group);
        QHostAddress serverInterface = QHostAddress(globalSettings.value("address").toString());
        if (serverInterface == host || serverInterface == QHostAddress("0.0.0.0")) {
            serverPort = globalSettings.value("port", -1).toInt();
            useSsl = globalSettings.value("sslEnabled", true).toBool();
        }
        globalSettings.endGroup();
    }
    globalSettings.endGroup();

    if (serverPort == -1) {
        qCWarning(dcConnection) << "No matching WebServer configuration found. Discarding UPnP request!";
        return;
    }

    foreach (const QNetworkInterface &interface,  QNetworkInterface::allInterfaces()) {
        foreach (QNetworkAddressEntry entry, interface.addressEntries()) {
            // check IPv4
            if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol) {
                // check subnet
                if (host.isInSubnet(QHostAddress::parseSubnet(entry.ip().toString() + "/24"))) {
                    QString locationString;
                    if (useSsl) {
                        locationString = "https://" + entry.ip().toString() + ":" + QString::number(serverPort) + "/server.xml";
                    } else {
                        locationString = "http://" + entry.ip().toString() + ":" + QString::number(serverPort) + "/server.xml";
                    }

                    // http://upnp.org/specs/basic/UPnP-basic-Basic-v1-Device.pdf
                    QByteArray rootdeviceResponseMessage = QByteArray("HTTP/1.1 200 OK\r\n"
                                                                      "CACHE-CONTROL: max-age=1900\r\n"
                                                                      "DATE: " + QDateTime::currentDateTime().toString("ddd, dd MMM yyyy hh:mm:ss").toUtf8() + " GMT\r\n"
                                                                      "EXT:\r\n"
                                                                      "CONTENT-LENGTH:0\r\n"
                                                                      "LOCATION: " + locationString.toUtf8() + "\r\n"
                                                                      "SERVER: guh/" + QByteArray(GUH_VERSION_STRING) + " UPnP/1.1 \r\n"
                                                                      "ST:upnp:rootdevice\r\n"
                                                                      "USN:uuid:" + uuid + "::urn:schemas-upnp-org:device:Basic:1\r\n"
                                                                      "\r\n");

                    //qCDebug(dcHardware) << QString("Sending response to %1:%2\n").arg(host.toString()).arg(port);
                    m_socket->writeDatagram(rootdeviceResponseMessage, host, port);
                }
            }
        }
    }
}

/*! This method will be called to send the SSDP message \a data to the UPnP multicast.*/
void UpnpDiscovery::sendToMulticast(const QByteArray &data)
{
    m_socket->writeDatagram(data, m_host, m_port);
}

void UpnpDiscovery::error(QAbstractSocket::SocketError error)
{
    qCWarning(dcHardware) << "UPnP socket error:" << error << m_socket->errorString();
}

void UpnpDiscovery::readData()
{
    QByteArray data;
    quint16 port;
    QHostAddress hostAddress;
    QUrl location;

    // read the answere from the multicast
    while (m_socket->hasPendingDatagrams()) {
        data.resize(m_socket->pendingDatagramSize());
        m_socket->readDatagram(data.data(), data.size(), &hostAddress, &port);
    }

    if (data.contains("M-SEARCH") && !QNetworkInterface::allAddresses().contains(hostAddress)) {
        respondToSearchRequest(hostAddress, port);
        return;
    }

    if (data.contains("NOTIFY") && !QNetworkInterface::allAddresses().contains(hostAddress)) {
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
            if (key.contains("LOCATION") || key.contains("location") || key.contains("Location")) {
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
        qCWarning(dcHardware) << "HTTP request error" << reply->request().url().toString() << status;
        m_informationRequestList.remove(reply);
    }

    reply->deleteLater();
}

void UpnpDiscovery::notificationTimeout()
{
    sendAliveMessage();
}

void UpnpDiscovery::sendByeByeMessage()
{
    // TODO: Once DeviceManager (and with that this can be moved into the server, use GuhCore's configuration manager instead of parsing the config here...
    GuhSettings globalSettings(GuhSettings::SettingsRoleGlobal);
    globalSettings.beginGroup("guhd");
    QByteArray uuid = globalSettings.value("uuid", QUuid()).toByteArray();
    globalSettings.endGroup();

    globalSettings.beginGroup("WebServer");
    foreach (const QString &group, globalSettings.childGroups()) {
        globalSettings.beginGroup(group);
        QHostAddress serverInterface = QHostAddress(globalSettings.value("address").toString());
        int serverPort = globalSettings.value("port", -1).toInt();
        bool useSsl = globalSettings.value("sslEnabled", true).toBool();
        globalSettings.endGroup();

        foreach (const QNetworkInterface &interface,  QNetworkInterface::allInterfaces()) {
            foreach (QNetworkAddressEntry entry, interface.addressEntries()) {
                if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol && (serverInterface == QHostAddress("0.0.0.0") || entry.ip() == serverInterface)) {

                    QString locationString;
                    if (useSsl) {
                        locationString = "https://" + entry.ip().toString() + ":" + QString::number(serverPort) + "/server.xml";
                    } else {
                        locationString = "http://" + entry.ip().toString() + ":" + QString::number(serverPort) + "/server.xml";
                    }

                    // http://upnp.org/specs/basic/UPnP-basic-Basic-v1-Device.pdf
                    QByteArray byebyeMessage = QByteArray("NOTIFY * HTTP/1.1\r\n"
                                                                      "HOST:239.255.255.250:1900\r\n"
                                                                      "CACHE-CONTROL: max-age=1900\r\n"
                                                                      "LOCATION: " + locationString.toUtf8() + "\r\n"
                                                                      "NT:urn:schemas-upnp-org:device:Basic:1\r\n"
                                                                      "USN:uuid:" + uuid + "::urn:schemas-upnp-org:device:Basic:1\r\n"
                                                                      "NTS: ssdp:byebye\r\n"
                                                                      "SERVER: guh/" + QByteArray(GUH_VERSION_STRING) + " UPnP/1.1 \r\n"
                                                                      "\r\n");

                    sendToMulticast(byebyeMessage);
                }
            }
        }
    }
    globalSettings.endGroup();
}

void UpnpDiscovery::sendAliveMessage()
{
    // TODO: Once DeviceManager (and with that this can be moved into the server, use GuhCore's configuration manager instead of parsing the config here...
    GuhSettings globalSettings(GuhSettings::SettingsRoleGlobal);
    globalSettings.beginGroup("guhd");
    QByteArray uuid = globalSettings.value("uuid", QUuid()).toByteArray();
    globalSettings.endGroup();

    globalSettings.beginGroup("WebServer");
    foreach (const QString &group, globalSettings.childGroups()) {
        globalSettings.beginGroup(group);
        QHostAddress serverInterface = QHostAddress(globalSettings.value("address").toString());
        int serverPort = globalSettings.value("port", -1).toInt();
        bool useSsl = globalSettings.value("sslEnabled", true).toBool();
        globalSettings.endGroup();

        foreach (const QNetworkInterface &interface,  QNetworkInterface::allInterfaces()) {
            foreach (QNetworkAddressEntry entry, interface.addressEntries()) {
                if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol && (serverInterface == QHostAddress("0.0.0.0") || entry.ip() == serverInterface)) {

                    QString locationString;
                    if (useSsl) {
                        locationString = "https://" + entry.ip().toString() + ":" + QString::number(serverPort) + "/server.xml";
                    } else {
                        locationString = "http://" + entry.ip().toString() + ":" + QString::number(serverPort) + "/server.xml";
                    }

                    // http://upnp.org/specs/basic/UPnP-basic-Basic-v1-Device.pdf
                    QByteArray aliveMessage = QByteArray("NOTIFY * HTTP/1.1\r\n"
                                                                      "HOST:239.255.255.250:1900\r\n"
                                                                      "CACHE-CONTROL: max-age=1900\r\n"
                                                                      "LOCATION: " + locationString.toUtf8() + "\r\n"
                                                                      "NT:urn:schemas-upnp-org:device:Basic:1\r\n"
                                                                      "USN:uuid:" + uuid + "::urn:schemas-upnp-org:device:Basic:1\r\n"
                                                                      "NTS: ssdp:alive\r\n"
                                                                      "SERVER: guh/" + QByteArray(GUH_VERSION_STRING) + " UPnP/1.1 \r\n"
                                                                      "\r\n");

                    sendToMulticast(aliveMessage);
                }
            }
        }
    }
    globalSettings.endGroup();
}

void UpnpDiscovery::discoverTimeout()
{
    UpnpDiscoveryRequest *discoveryRequest = static_cast<UpnpDiscoveryRequest*>(sender());
    emit discoveryFinished(discoveryRequest->deviceList(), discoveryRequest->pluginId());

    m_discoverRequests.removeOne(discoveryRequest);
    delete discoveryRequest;
}

bool UpnpDiscovery::enable()
{
    // Clean up
    if (m_socket) {
        delete m_socket;
        m_socket = nullptr;
    }

    // Bind udp socket and join multicast group
    m_socket = new QUdpSocket(this);
    m_port = 1900;
    m_host = QHostAddress("239.255.255.250");

    m_socket->setSocketOption(QAbstractSocket::MulticastTtlOption,QVariant(1));
    m_socket->setSocketOption(QAbstractSocket::MulticastLoopbackOption,QVariant(1));

    if(!m_socket->bind(QHostAddress::AnyIPv4, m_port, QUdpSocket::ShareAddress)){
        qCWarning(dcHardware()) << name() << "could not bind to port" << m_port;
        delete m_socket;
        m_socket = nullptr;
        return false;
    }

    if(!m_socket->joinMulticastGroup(m_host)){
        qCWarning(dcHardware()) <<  name() << "could not join multicast group" << m_host;
        delete m_socket;
        m_socket = nullptr;
        return false;
    }

    connect(m_socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(error(QAbstractSocket::SocketError)));
    connect(m_socket, &QUdpSocket::readyRead, this, &UpnpDiscovery::readData);

    m_notificationTimer->start();

    sendAliveMessage();
    sendAliveMessage();

    setEnabled(true);
    return true;
}

bool UpnpDiscovery::disable()
{
    // TODO:
    setEnabled(false);
    return true;
}
