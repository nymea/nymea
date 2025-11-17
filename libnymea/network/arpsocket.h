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
