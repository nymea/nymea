/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2024, nymea GmbH
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

#ifndef MACADDRESSINFO_H
#define MACADDRESSINFO_H

#include "macaddress.h"

class MacAddressInfo
{
public:
    explicit MacAddressInfo();
    explicit MacAddressInfo(const MacAddress &macAddress);
    explicit MacAddressInfo(const MacAddress &macAddress, const QString &vendorName);

    MacAddress macAddress() const;

    QString vendorName() const;
    void setVendorName(const QString &vendorName);

    bool isValid() const;
    bool isComplete() const;

    bool operator==(const MacAddressInfo &other) const;
    bool operator!=(const MacAddressInfo &other) const;

private:
    MacAddress m_macAddress;
    QString m_vendorName;

    bool m_vendorNameSet = false;
};

QDebug operator<<(QDebug debug, const MacAddressInfo &addressInfo);


#endif // MACADDRESSINFO_H
