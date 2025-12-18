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

#ifndef MACADDRESS_H
#define MACADDRESS_H

#include <QByteArray>
#include <QHash>
#include <QString>

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
