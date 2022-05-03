/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2021, nymea GmbH
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

#ifndef ARPSOCKET_H
#define ARPSOCKET_H

#include <QDebug>
#include <QObject>
#include <QSocketNotifier>
#include <QLoggingCategory>
#include <QHostAddress>
#include <QNetworkInterface>

#include "libnymea.h"
#include "macaddress.h"

class LIBNYMEA_EXPORT ArpSocket : public QObject
{
    Q_OBJECT
public:
    explicit ArpSocket(QObject *parent = nullptr);

    // Send ARP request to all local networks
    bool sendRequest();

    // Send ARP request to a specific network interface with the given name
    bool sendRequest(const QString &interfaceName);

    // Send ARP request to a specific network interface
    bool sendRequest(const QNetworkInterface &networkInterface);

    // Send ARP request to a specific address within the network
    bool sendRequest(const QHostAddress &targetAddress);

    bool isOpen() const;

    bool openSocket();
    void closeSocket();

signals:
    void arpResponse(const QNetworkInterface &networkInterface, const QHostAddress &address, const MacAddress &macAddress);
    void arpRequestReceived(const QNetworkInterface &networkInterface, const QHostAddress &address, const MacAddress &macAddress);

private:
    QSocketNotifier *m_socketNotifier = nullptr;
    int m_socketDescriptor = -1;
    bool m_isOpen = false;

    bool sendRequestInternally(int networkInterfaceIndex, const MacAddress &senderMacAddress, const QHostAddress &senderHostAddress, const MacAddress &targetMacAddress, const QHostAddress &targetHostAddress);

    void processDataBuffer(unsigned char *receiveBuffer, int size);

    QHostAddress getHostAddressString(uint8_t *senderIpAddress);

    bool loadArpCache(const QNetworkInterface &interface = QNetworkInterface());

    void fillMacAddress(uint8_t *targetArray, const MacAddress &macAddress);
    void fillHostAddress(uint8_t *targetArray, const QHostAddress &hostAddress);

};

#endif // ARPSOCKET_H
