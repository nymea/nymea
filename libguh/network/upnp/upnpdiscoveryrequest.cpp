/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
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

#include "upnpdiscoveryrequest.h"
#include "loggingcategories.h"

UpnpDiscoveryRequest::UpnpDiscoveryRequest(UpnpDiscovery *upnpDiscovery, PluginId pluginId, QString searchTarget, QString userAgent):
    QObject(upnpDiscovery),
    m_upnpDiscovery(upnpDiscovery),
    m_pluginId(pluginId),
    m_searchTarget(searchTarget),
    m_userAgent(userAgent)
{
    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    connect(m_timer, &QTimer::timeout, this, &UpnpDiscoveryRequest::discoveryTimeout);
}

void UpnpDiscoveryRequest::discover()
{
    QByteArray ssdpSearchMessage = QByteArray("M-SEARCH * HTTP/1.1\r\n"
                                              "HOST:239.255.255.250:1900\r\n"
                                              "MAN:\"ssdp:discover\"\r\n"
                                              "MX:4\r\n"
                                              "ST: " + m_searchTarget.toUtf8() + "\r\n"
                                              "USR-AGENT: " + m_userAgent.toUtf8() + "\r\n\r\n");

    m_upnpDiscovery->sendToMulticast(ssdpSearchMessage);
    qCDebug(dcHardware) << "--> UPnP discovery called.";

    m_timer->start(5000);
}

void UpnpDiscoveryRequest::addDeviceDescriptor(const UpnpDeviceDescriptor &deviceDescriptor)
{

    // check if we allready have the device in the list
    bool isAlreadyInList = false;
    foreach (UpnpDeviceDescriptor upnpDeviceDescriptor, m_deviceList) {
        if (upnpDeviceDescriptor.uuid() == deviceDescriptor.uuid()) {
            isAlreadyInList = true;
        }
    }
    if (!isAlreadyInList) {
        m_deviceList.append(deviceDescriptor);
    }
}

QNetworkRequest UpnpDiscoveryRequest::createNetworkRequest(UpnpDeviceDescriptor deviveDescriptor)
{
    QNetworkRequest deviceRequest;
    deviceRequest.setUrl(deviveDescriptor.location());
    deviceRequest.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("text/xml"));
    deviceRequest.setHeader(QNetworkRequest::UserAgentHeader,QVariant(m_userAgent));

    return deviceRequest;
}

QList<UpnpDeviceDescriptor> UpnpDiscoveryRequest::deviceList() const
{
    return m_deviceList;
}

PluginId UpnpDiscoveryRequest::pluginId() const
{
    return m_pluginId;
}

QString UpnpDiscoveryRequest::searchTarget() const
{
    return m_searchTarget;
}

QString UpnpDiscoveryRequest::userAgent() const
{
    return m_userAgent;
}
