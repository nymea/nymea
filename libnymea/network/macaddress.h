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

#ifndef MACADDRESS_H
#define MACADDRESS_H

#include <QHash>
#include <QString>
#include <QByteArray>

#include "libnymea.h"

class LIBNYMEA_EXPORT MacAddress
{
public:
    explicit MacAddress();
    explicit MacAddress(const QString &macAddress);
    explicit MacAddress(const QByteArray &macAddress);
    explicit MacAddress(unsigned char *rawData);
    MacAddress(const MacAddress &other);

    static MacAddress broadcast();
    static MacAddress fromString(const QString &macAddress);

    QString toString() const;

    QByteArray toByteArray() const;

    bool isNull() const;
    bool isValid() const;

    void clear();

    MacAddress &operator=(const MacAddress &other);

    bool operator<(const MacAddress &other) const;
    bool operator>(const MacAddress &other) const;
    bool operator==(const MacAddress &other) const;
    bool operator!=(const MacAddress &other) const;

private:
    QByteArray m_rawData = 0;

};


#if QT_VERSION < 0x0600000
using qhash_result_t = uint;
#else
using qhash_result_t = size_t;
#endif
inline qhash_result_t qHash(const MacAddress &macAddress, qhash_result_t seed)
{
    return qHash(macAddress.toByteArray(), seed);
}

QDebug operator<<(QDebug debug, const MacAddress &address);

#endif // MACADDRESS_H
