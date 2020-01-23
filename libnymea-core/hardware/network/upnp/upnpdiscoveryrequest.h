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
* GNU General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU General Public License as published by the Free Software
* Foundation, GNU version 3. This project is distributed in the hope that it
* will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
* of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
* Public License for more details.
*
* You should have received a copy of the GNU General Public License along with
* this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef UPNPDISCOVERYREQUEST_H
#define UPNPDISCOVERYREQUEST_H

#include <QObject>
#include <QDebug>
#include <QTimer>
#include <QMetaObject>
#include <QPointer>

#include "upnpdiscoveryreplyimplementation.h"
#include "network/upnp/upnpdiscovery.h"
#include "network/upnp/upnpdevicedescriptor.h"

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

}

#endif // UPNPDISCOVERYREQUEST_H
