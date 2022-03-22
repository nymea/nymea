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

#ifndef NETWORKDEVICEINFO_H
#define NETWORKDEVICEINFO_H

#include <QDebug>
#include <QObject>
#include <QHostAddress>
#include <QNetworkInterface>

#include "libnymea.h"

class LIBNYMEA_EXPORT NetworkDeviceInfo
{
public:
    explicit NetworkDeviceInfo();
    explicit NetworkDeviceInfo(const QString &macAddress);

    QString macAddress() const;
    void setMacAddress(const QString &macAddress);

    QString macAddressManufacturer() const;
    void setMacAddressManufacturer(const QString &macAddressManufacturer);

    QHostAddress address() const;
    void setAddress(const QHostAddress &address);

    QString hostName() const;
    void setHostName(const QString &hostName);

    QNetworkInterface networkInterface() const;
    void setNetworkInterface(const QNetworkInterface &networkInterface);

    bool isValid() const;
    bool isComplete() const;

    bool operator==(const NetworkDeviceInfo &other) const;

private:
    QHostAddress m_address;
    QString m_macAddress;
    QString m_macAddressManufacturer;
    QString m_hostName;
    QNetworkInterface m_networkInterface;

    bool m_macAddressSet = false;
    bool m_macAddressManufacturerSet = false;
    bool m_addressSet = false;
    bool m_hostNameSet = false;
    bool m_networkInterfaceSet = false;
};


QDebug operator<<(QDebug debug, const NetworkDeviceInfo &networkDeviceInfo);


#endif // NETWORKDEVICEINFO_H
