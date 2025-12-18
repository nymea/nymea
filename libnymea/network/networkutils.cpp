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

#include "networkutils.h"

NetworkUtils::NetworkUtils() {}

QNetworkInterface NetworkUtils::getInterfaceForHostaddress(const QHostAddress &address)
{
    foreach (const QNetworkInterface &networkInterface, QNetworkInterface::allInterfaces()) {
        foreach (const QNetworkAddressEntry &entry, networkInterface.addressEntries()) {
            // Only IPv4
            if (entry.ip().protocol() != QAbstractSocket::IPv4Protocol)
                continue;

            if (address.isInSubnet(entry.ip(), entry.prefixLength())) {
                return networkInterface;
            }
        }
    }

    return QNetworkInterface();
}

QNetworkInterface NetworkUtils::getInterfaceForMacAddress(const QString &macAddress)
{
    foreach (const QNetworkInterface &networkInterface, QNetworkInterface::allInterfaces()) {
        if (networkInterface.hardwareAddress().toLower() == macAddress.toLower()) {
            return networkInterface;
        }
    }

    return QNetworkInterface();
}

QNetworkInterface NetworkUtils::getInterfaceForMacAddress(const MacAddress &macAddress)
{
    foreach (const QNetworkInterface &networkInterface, QNetworkInterface::allInterfaces()) {
        if (MacAddress(networkInterface.hardwareAddress()) == macAddress) {
            return networkInterface;
        }
    }

    return QNetworkInterface();
}
