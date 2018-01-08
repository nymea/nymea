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

#ifndef UPNPDISCOVERYREQUEST_H
#define UPNPDISCOVERYREQUEST_H

#include <QObject>
#include <QDebug>
#include <QTimer>
#include <QMetaObject>

#include "upnpdiscoveryreplyimplementation.h"
#include "network/upnp/upnpdiscovery.h"
#include "network/upnp/upnpdevicedescriptor.h"

#include "typeutils.h"

class UpnpDiscovery;

namespace guhserver {

class UpnpDiscoveryRequest : public QObject
{
    Q_OBJECT
public:
    explicit UpnpDiscoveryRequest(UpnpDiscovery *upnpDiscovery, QPointer<UpnpDiscoveryReplyImplementation> reply);

    void discover(const int &timeout);
    void addDeviceDescriptor(const UpnpDeviceDescriptor &deviceDescriptor);
    QNetworkRequest createNetworkRequest(UpnpDeviceDescriptor deviveDescriptor);
    QList<UpnpDeviceDescriptor> deviceList() const;

    QPointer<UpnpDiscoveryReplyImplementation> reply();

private:
    UpnpDiscovery *m_upnpDiscovery;
    QByteArray m_ssdpSearchMessage;
    QPointer<UpnpDiscoveryReplyImplementation> m_reply;
    int m_totalTriggers = 0;
    int m_triggerCounter = 0;

    QTimer *m_timer = nullptr;
    QList<UpnpDeviceDescriptor> m_deviceList;

signals:
    void discoveryTimeout();

private slots:
    void onTimeout();

};

}

#endif // UPNPDISCOVERYREQUEST_H
