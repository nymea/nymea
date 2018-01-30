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

#ifndef UPNPDISCOVERYIMPLEMENTATION_H
#define UPNPDISCOVERYIMPLEMENTATION_H

#include <QUrl>
#include <QTimer>
#include <QUdpSocket>
#include <QHostAddress>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QNetworkAccessManager>

#include "upnpdiscoveryrequest.h"

#include "network/upnp/upnpdiscovery.h"
#include "network/upnp/upnpdiscoveryreply.h"
#include "network/upnp/upnpdevicedescriptor.h"

// Discovering UPnP devices reference: http://upnp.org/specs/arch/UPnP-arch-DeviceArchitecture-v1.1.pdf
// guh basic device reference: http://upnp.org/specs/basic/UPnP-basic-Basic-v1-Device.pdf

namespace nymeaserver {

class UpnpDiscoveryImplementation : public UpnpDiscovery
{
    Q_OBJECT

public:
    explicit UpnpDiscoveryImplementation(QNetworkAccessManager *networkAccessManager, QObject *parent = nullptr);
    ~UpnpDiscoveryImplementation();

    UpnpDiscoveryReply *discoverDevices(const QString &searchTarget = "ssdp:all", const QString &userAgent = QString(), const int &timeout = 5000) override;
    void sendToMulticast(const QByteArray &data) override;

    bool available() const override;
    bool enabled() const override;

private:
    QUdpSocket *m_socket = nullptr;
    QHostAddress m_host;
    qint16 m_port;

    QTimer *m_notificationTimer = nullptr;

    QNetworkAccessManager *m_networkAccessManager = nullptr;

    QList<UpnpDiscoveryRequest *> m_discoverRequests;
    QHash<QNetworkReply*, UpnpDeviceDescriptor> m_informationRequestList;

    bool m_available = false;
    bool m_enabled = false;

    void requestDeviceInformation(const QNetworkRequest &networkRequest, const UpnpDeviceDescriptor &upnpDeviceDescriptor);
    void respondToSearchRequest(QHostAddress host, int port);

protected:
    void setEnabled(bool enabled) override;

private slots:
    void error(QAbstractSocket::SocketError error);
    void readData();
    void replyFinished();
    void notificationTimeout();
    void sendByeByeMessage();
    void sendAliveMessage();
    void discoverTimeout();

public slots:
    bool enable();
    bool disable();

};

}

#endif // UPNPDISCOVERYIMPLEMENTATION_H
