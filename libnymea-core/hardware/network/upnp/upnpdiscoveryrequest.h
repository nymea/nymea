// SPDX-License-Identifier: LGPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2025, chargebyte austria GmbH
*
* This file is part of nymea.
*
* nymea is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* as published by the Free Software Foundation, either version 3
* of the License, or (at your option) any later version.
*
* nymea is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with nymea. If not, see <https://www.gnu.org/licenses/>.
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef UPNPDISCOVERYREQUEST_H
#define UPNPDISCOVERYREQUEST_H

#include <QDebug>
#include <QMetaObject>
#include <QObject>
#include <QPointer>
#include <QTimer>

#include "network/upnp/upnpdevicedescriptor.h"
#include "network/upnp/upnpdiscovery.h"
#include "upnpdiscoveryreplyimplementation.h"

#include "typeutils.h"

class UpnpDiscovery;

namespace nymeaserver {

class UpnpDiscoveryRequest : public QObject
{
    Q_OBJECT
public:
    explicit UpnpDiscoveryRequest(UpnpDiscovery *upnpDiscovery, QPointer<UpnpDiscoveryReplyImplementation> reply);

    void discover(int timeout);
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

} // namespace nymeaserver

#endif // UPNPDISCOVERYREQUEST_H
