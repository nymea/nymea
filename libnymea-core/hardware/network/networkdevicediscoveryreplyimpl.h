/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2022, nymea GmbH
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

#ifndef NETWORKDEVICEDISCOVERYREPLYIMPL_H
#define NETWORKDEVICEDISCOVERYREPLYIMPL_H

#include <QHash>
#include <QObject>

#include "network/networkdeviceinfo.h"
#include "network/networkdevicediscoveryreply.h"

namespace nymeaserver {

class NetworkDeviceDiscoveryReplyImpl : public NetworkDeviceDiscoveryReply
{
    Q_OBJECT

public:
    explicit NetworkDeviceDiscoveryReplyImpl(QObject *parent = nullptr);
    ~NetworkDeviceDiscoveryReplyImpl() override = default;

    NetworkDeviceInfos networkDeviceInfos() const override;
    NetworkDeviceInfos virtualNetworkDeviceInfos() const override;

    bool isFinished() const override;
    void setFinished(bool finished);

    // Add or update the network device info and verify if completed
    void processPingResponse(const QHostAddress &address, const QString &hostName);
    void processArpResponse(const QNetworkInterface &interface, const QHostAddress &address, const MacAddress &macAddress);
    void processMacManufacturer(const MacAddress &macAddress, const QString &manufacturer);

    void processDiscoveryFinished();

public slots:
    void addCompleteNetworkDeviceInfo(const NetworkDeviceInfo &networkDeviceInfo);
    void addVirtualNetworkDeviceInfo(const NetworkDeviceInfo &networkDeviceInfo);

private:
    NetworkDeviceInfos m_networkDeviceInfos; // Contains only complete and valid infos
    NetworkDeviceInfos m_virtualNetworkDeviceInfos; // Contains ping responses without ARP, like VPN devices

    QHash<MacAddress, NetworkDeviceInfo> m_networkDeviceCache;
    qint64 m_startTimestamp;

    bool m_isFinished = false;

    // Temporary cache for ping responses where the mac is not known yet (like VPN devices)
    QHash<QHostAddress, NetworkDeviceInfo> m_pingCache;

    QString macAddressFromHostAddress(const QHostAddress &address);
    bool hasHostAddress(const QHostAddress &address);

    void verifyComplete(const MacAddress &macAddress);


};

}

#endif // NETWORKDEVICEDISCOVERYREPLYIMPL_H
