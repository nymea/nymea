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

#ifndef UPNPDISCOVERY_H
#define UPNPDISCOVERY_H

#include <QUdpSocket>
#include <QHostAddress>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>

#include "libguh.h"
#include "devicemanager.h"
#include "hardwareresource.h"
#include "upnpdiscoveryrequest.h"
#include "upnpdevicedescriptor.h"

// Discovering UPnP devices reference: http://upnp.org/specs/arch/UPnP-arch-DeviceArchitecture-v1.1.pdf
// guh basic device reference: http://upnp.org/specs/basic/UPnP-basic-Basic-v1-Device.pdf

class UpnpDiscoveryRequest;

class LIBGUH_EXPORT UpnpDiscovery : public HardwareResource
{
    Q_OBJECT

    friend class HardwareManager;

public:
    bool discoverDevices(const QString &searchTarget = "ssdp:all", const QString &userAgent = "", const PluginId &pluginId = PluginId());
    void sendToMulticast(const QByteArray &data);

private:
    explicit UpnpDiscovery(QNetworkAccessManager *networkAccessManager, QObject *parent = 0);
    ~UpnpDiscovery();

    QUdpSocket *m_socket = nullptr;
    QHostAddress m_host;
    qint16 m_port;

    QTimer *m_notificationTimer = nullptr;

    QNetworkAccessManager *m_networkAccessManager = nullptr;

    QList<UpnpDiscoveryRequest *> m_discoverRequests;
    QHash<QNetworkReply*,UpnpDeviceDescriptor> m_informationRequestList;

    void requestDeviceInformation(const QNetworkRequest &networkRequest, const UpnpDeviceDescriptor &upnpDeviceDescriptor);
    void respondToSearchRequest(QHostAddress host, int port);

signals:
    void discoveryFinished(const QList<UpnpDeviceDescriptor> &deviceDescriptorList, const PluginId & pluginId);
    void upnpNotify(const QByteArray &notifyMessage);

private slots:
    void error(QAbstractSocket::SocketError error);
    void readData();
    void replyFinished(QNetworkReply *reply);
    void notificationTimeout();
    void sendByeByeMessage();
    void sendAliveMessage();
    void discoverTimeout();

public slots:
    bool enable();
    bool disable();
};

#endif // UPNPDISCOVERY_H
