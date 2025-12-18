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

#ifndef UPNPDISCOVERY_H
#define UPNPDISCOVERY_H

#include <QHostAddress>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <QUdpSocket>
#include <QUrl>

#include "hardwareresource.h"
#include "libnymea.h"
#include "upnpdevicedescriptor.h"
#include "upnpdiscoveryreply.h"

// Discovering UPnP devices reference: http://upnp.org/specs/arch/UPnP-arch-DeviceArchitecture-v1.1.pdf
// nymea basic device reference: http://upnp.org/specs/basic/UPnP-basic-Basic-v1-Device.pdf

class LIBNYMEA_EXPORT UpnpDiscovery : public HardwareResource
{
    Q_OBJECT

public:
    explicit UpnpDiscovery(QObject *parent = nullptr);
    virtual ~UpnpDiscovery() = default;

    virtual UpnpDiscoveryReply *discoverDevices(const QString &searchTarget = "ssdp:all", const QString &userAgent = QString(), const int &timeout = 5000) = 0;
    virtual void sendToMulticast(const QByteArray &data) = 0;

signals:
    void upnpNotify(const QByteArray &notifyMessage);
};

#endif // UPNPDISCOVERY_H
