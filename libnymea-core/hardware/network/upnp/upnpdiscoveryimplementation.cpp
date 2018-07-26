/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015-2018 Simon Stürz <simon.stuerz@guh.io>              *
 *                                                                         *
 *  This file is part of nymea.                                            *
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
  \class nymeaserver::UpnpDiscoveryImplementation
  \brief Allows to detect UPnP devices in the network.

  \ingroup hardware
  \inmodule libnymea

  This resource allows plugins to discover UPnP devices in the network and receive notification messages. The resource
  will bind a UDP socket to the multicast 239.255.255.250 on port 1900.

  The communication was implementet using following documentation: \l{http://upnp.org/specs/arch/UPnP-arch-DeviceArchitecture-v1.1.pdf}

  \sa UpnpDevice, UpnpDeviceDescriptor
*/


#include "nymeasettings.h"
#include "loggingcategories.h"

#include "upnpdiscoveryimplementation.h"
#include "upnpdiscoveryreplyimplementation.h"

#include <QMetaObject>
#include <QNetworkInterface>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QNetworkConfiguration>
#include <QNetworkConfigurationManager>

namespace nymeaserver {

/*! Construct the hardware resource UpnpDiscoveryImplementation with the given \a parent. */
UpnpDiscoveryImplementation::UpnpDiscoveryImplementation(QNetworkAccessManager *networkAccessManager, QObject *parent) :
    UpnpDiscovery(parent),
    m_networkAccessManager(networkAccessManager)
{
    m_notificationTimer = new QTimer(this);
    m_notificationTimer->setInterval(30000);
    m_notificationTimer->setSingleShot(false);
    connect(m_notificationTimer, &QTimer::timeout, this, &UpnpDiscoveryImplementation::notificationTimeout);

    QNetworkConfigurationManager *configManager = new QNetworkConfigurationManager(this);
    connect(configManager, &QNetworkConfigurationManager::configurationAdded, this, &UpnpDiscoveryImplementation::networkConfigurationChanged);
    connect(configManager, &QNetworkConfigurationManager::configurationRemoved, this, &UpnpDiscoveryImplementation::networkConfigurationChanged);
    connect(configManager, &QNetworkConfigurationManager::configurationChanged, this, &UpnpDiscoveryImplementation::networkConfigurationChanged);

    m_available = true;

    qCDebug(dcHardware()) << "-->" << name() << "created successfully.";
}

/*! Destruct this \l{UpnpDiscoveryImplementation} object. */
UpnpDiscoveryImplementation::~UpnpDiscoveryImplementation()
{
    sendByeByeMessage();
    if (m_socket) {
        m_socket->waitForBytesWritten(1000);
        m_socket->close();
    }
}

UpnpDiscoveryReply *UpnpDiscoveryImplementation::discoverDevices(const QString &searchTarget, const QString &userAgent, const int &timeout)
{
    // Create the reply for this discovery request
    QPointer<UpnpDiscoveryReplyImplementation> reply = new UpnpDiscoveryReplyImplementation(searchTarget, userAgent, this);

    if (!available()) {
        qCWarning(dcUpnp()) << name() << "is not avilable.";
        reply->setError(UpnpDiscoveryReplyImplementation::UpnpDiscoveryReplyErrorNotAvailable);
        reply->setFinished();
        return reply.data();
    }

    if (!enabled()) {
        qCWarning(dcUpnp()) << name() << "is not enabled.";
        reply->setError(UpnpDiscoveryReplyImplementation::UpnpDiscoveryReplyErrorNotEnabled);
        reply->setFinished();
        return reply.data();
    }

    qCDebug(dcHardware) << name() << "discover" << searchTarget << userAgent;

    // Looks good so far, lets start a request
    UpnpDiscoveryRequest *request = new UpnpDiscoveryRequest(this, reply.data());
    connect(request, &UpnpDiscoveryRequest::discoveryTimeout, this, &UpnpDiscoveryImplementation::discoverTimeout);
    request->discover(timeout);
    m_discoverRequests.append(request);
    return reply.data();
}

void UpnpDiscoveryImplementation::requestDeviceInformation(const QNetworkRequest &networkRequest, const UpnpDeviceDescriptor &upnpDeviceDescriptor)
{
    QNetworkReply *replay = m_networkAccessManager->get(networkRequest);
    connect(replay, &QNetworkReply::finished, this, &UpnpDiscoveryImplementation::replyFinished);
    m_informationRequestList.insert(replay, upnpDeviceDescriptor);
}

void UpnpDiscoveryImplementation::respondToSearchRequest(QHostAddress host, int port)
{
    // TODO: Once DeviceManager (and with that this can be moved into the server, use NymeaCore's configuration manager instead of parsing the config here...
    NymeaSettings globalSettings(NymeaSettings::SettingsRoleGlobal);
    globalSettings.beginGroup("nymead");
    QByteArray uuid = globalSettings.value("uuid", QUuid()).toString().remove(QRegExp("[{}]")).toUtf8();
    globalSettings.endGroup();

    globalSettings.beginGroup("WebServer");
    int serverPort = -1;
    bool useSSL = false;
    foreach (const QString &group, globalSettings.childGroups()) {
        globalSettings.beginGroup(group);
        QHostAddress serverInterface = QHostAddress(globalSettings.value("address").toString());
        bool ssl = globalSettings.value("sslEnabled", true).toBool();
        int port = globalSettings.value("port", -1).toInt();
        globalSettings.endGroup();

        // We prefer unencrypted WebServers in this case. Most UPnP clients will bail out on our certificate...
        // However, if there is none without encryption, still use one with, so that at least our own clients find it.
        if (serverPort == -1) { // Don't have a better option yet. Use this.
            serverPort = port;
            useSSL = ssl;
            continue;
        }
        if (!ssl && useSSL) { // There is one without ssl. Use that.
            serverPort = port;
            useSSL = false;
            break;
        }
    }
    globalSettings.endGroup();

    if (serverPort == -1) {
        qCWarning(dcUpnp()) << "No matching WebServer configuration found. Discarding UPnP request! UPnP requires a plaintext webserver.";
        return;
    }
    if (useSSL) {
        qCWarning(dcUpnp()) << "Could not find a WebServer without SSL. Using one with SSL. This will not work with many clients.";
    }

    foreach (const QNetworkInterface &interface,  QNetworkInterface::allInterfaces()) {
        foreach (QNetworkAddressEntry entry, interface.addressEntries()) {
            // check IPv4
            if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol) {
                // check subnet
                if (host.isInSubnet(QHostAddress::parseSubnet(entry.ip().toString() + "/24"))) {
                    QString locationString;
                    if (useSSL) {
                        locationString = "https://" + entry.ip().toString() + ":" + QString::number(serverPort) + "/server.xml";
                    } else {
                        locationString = "http://" + entry.ip().toString() + ":" + QString::number(serverPort) + "/server.xml";
                    }

                    // http://upnp.org/specs/basic/UPnP-basic-Basic-v1-Device.pdf
                    QByteArray rootdeviceResponseMessage = QByteArray("HTTP/1.1 200 OK\r\n"
                                                                      "CACHE-CONTROL: max-age=1900\r\n"
                                                                      "DATE: " + QDateTime::currentDateTime().toString("ddd, dd MMM yyyy hh:mm:ss").toUtf8() + " GMT\r\n"
                                                                      "EXT:\r\n"
                                                                      "LOCATION: " + locationString.toUtf8() + "\r\n"
                                                                      "SERVER: nymea/" + QByteArray(NYMEA_VERSION_STRING) + " UPnP/1.1 \r\n"
                                                                      "ST: upnp:rootdevice\r\n"
                                                                      "USN: uuid:" + uuid + "::urn:schemas-upnp-org:device:Basic:1\r\n"
                                                                      "\r\n");

                    qCDebug(dcUpnp()) << QString("Sending response to %1:%2").arg(host.toString()).arg(port);
                    m_socket->writeDatagram(rootdeviceResponseMessage, host, port);
                }
            }
        }
    }
}

/*! This method will be called to send the SSDP message \a data to the UPnP multicast.*/
void UpnpDiscoveryImplementation::sendToMulticast(const QByteArray &data)
{
    if (!m_socket)
        return;

    m_socket->writeDatagram(data, m_host, m_port);
}

bool UpnpDiscoveryImplementation::available() const
{
    return m_available;
}

bool UpnpDiscoveryImplementation::enabled() const
{
    return m_enabled;
}

void UpnpDiscoveryImplementation::setEnabled(bool enabled)
{
    if (m_enabled == enabled)
        return;

    qCDebug(dcUpnp()) << "Enabling UPnP server";
    m_enabled = enabled;
    emit enabledChanged(m_enabled);

    if (enabled) {
        enable();
    } else {
        disable();
    }

}

void UpnpDiscoveryImplementation::error(QAbstractSocket::SocketError error)
{
    qCWarning(dcHardware) << name() << "socket error:" << error << m_socket->errorString();
}

void UpnpDiscoveryImplementation::readData()
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
        qCDebug(dcUpnp()) << "UPnP discovery request received";
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

void UpnpDiscoveryImplementation::replyFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply *>(sender());
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
        qCWarning(dcHardware) << name() << "HTTP request error" << reply->request().url().toString() << status;
        m_informationRequestList.remove(reply);
    }

    reply->deleteLater();
}

void UpnpDiscoveryImplementation::notificationTimeout()
{
    sendAliveMessage();
}

void UpnpDiscoveryImplementation::sendByeByeMessage()
{
    // TODO: Once DeviceManager (and with that this can be moved into the server, use NymeaCore's configuration manager instead of parsing the config here...
    NymeaSettings globalSettings(NymeaSettings::SettingsRoleGlobal);
    globalSettings.beginGroup("nymead");
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
                                                                      "SERVER: nymea/" + QByteArray(NYMEA_VERSION_STRING) + " UPnP/1.1 \r\n"
                                                                      "\r\n");

                    sendToMulticast(byebyeMessage);
                }
            }
        }
    }
    globalSettings.endGroup();
}

void UpnpDiscoveryImplementation::sendAliveMessage()
{
    // TODO: Once DeviceManager (and with that this) can be moved into the server, use NymeaCore's configuration manager instead of parsing the config here...
    NymeaSettings globalSettings(NymeaSettings::SettingsRoleGlobal);
    globalSettings.beginGroup("nymead");
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
                                                                      "SERVER: nymea/" + QByteArray(NYMEA_VERSION_STRING) + " UPnP/1.1 \r\n"
                                                                      "\r\n");

                    sendToMulticast(aliveMessage);
                }
            }
        }
    }
    globalSettings.endGroup();
}

void UpnpDiscoveryImplementation::discoverTimeout()
{
    UpnpDiscoveryRequest *discoveryRequest = static_cast<UpnpDiscoveryRequest*>(sender());
    QPointer<UpnpDiscoveryReplyImplementation> reply = discoveryRequest->reply();

    if (reply.isNull()) {
        qCWarning(dcUpnp()) << name() << "Reply does not exist any more. Please don't delete the reply before it has finished.";
    }  else {
        reply->setDeviceDescriptors(discoveryRequest->deviceList());
        reply->setError(UpnpDiscoveryReplyImplementation::UpnpDiscoveryReplyErrorNoError);
        reply->setFinished();
    }

    m_discoverRequests.removeOne(discoveryRequest);
    delete discoveryRequest;
}

void UpnpDiscoveryImplementation::networkConfigurationChanged(const QNetworkConfiguration &config)
{
    Q_UNUSED(config)
    if (m_enabled) {
        disable();
        enable();
    }
}

bool UpnpDiscoveryImplementation::enable()
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
        qCWarning(dcUpnp()) << name() << "could not bind to port" << m_port;
        m_available = false;
        emit availableChanged(false);
        delete m_socket;
        m_socket = nullptr;
        return false;
    }

    if(!m_socket->joinMulticastGroup(m_host)){
        qCWarning(dcUpnp()) << name() << "could not join multicast group" << m_host;
        m_available = false;
        emit availableChanged(false);
        delete m_socket;
        m_socket = nullptr;
        return false;
    }

    connect(m_socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(error(QAbstractSocket::SocketError)));
    connect(m_socket, &QUdpSocket::readyRead, this, &UpnpDiscoveryImplementation::readData);

    m_notificationTimer->start();

    sendAliveMessage();
    sendAliveMessage();

    setEnabled(true);

    qCDebug(dcHardware()) << "";

    return true;
}

bool UpnpDiscoveryImplementation::disable()
{
    if (!m_socket) {
        return false;
    }
    sendByeByeMessage();
    m_socket->waitForBytesWritten();
    m_socket->close();
    delete m_socket;
    m_socket = nullptr;

    m_notificationTimer->stop();

    setEnabled(false);
    return true;
}

}
