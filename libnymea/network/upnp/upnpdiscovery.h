/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2020, nymea GmbH
* Contact: contact@nymea.io
*
* This file is part of nymea.
* This project including source code and documentation is protected by
* copyright law, and remains the property of nymea GmbH. All rights, including
* reproduction, publication, editing and translation, are reserved. The use of
* this project is subject to the terms of a license agreement to be concluded
* with nymea GmbH in accordance with the terms of use of nymea GmbH, available
* under https://nymea.io/license
*
* GNU Lesser General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU Lesser General Public License as published by the Free
* Software Foundation; version 3. This project is distributed in the hope that
* it will be useful, but WITHOUT ANY WARRANTY; without even the implied
* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef UPNPDISCOVERY_H
#define UPNPDISCOVERY_H

#include <QUdpSocket>
#include <QHostAddress>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>

#include "libnymea.h"
#include "hardwareresource.h"
#include "upnpdiscoveryreply.h"
#include "upnpdevicedescriptor.h"

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
