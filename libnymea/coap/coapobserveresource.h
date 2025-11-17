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

#ifndef COAPOBSERVERESOURCE_H
#define COAPOBSERVERESOURCE_H

#include <QObject>
#include <QHash>
#include <QUrl>

#include "libnymea.h"

class LIBNYMEA_EXPORT CoapObserveResource
{
public:
    CoapObserveResource();
    CoapObserveResource(const QUrl &url, const QByteArray &token);

    QUrl url() const;
    QByteArray token() const;

private:
    QUrl m_url;
    QByteArray m_token;

};

inline bool operator==(const CoapObserveResource &r1, const CoapObserveResource &r2)
{
    return r1.url() == r2.url() && r1.token() == r2.token();
}

inline uint qHash(const CoapObserveResource &key, uint seed)
{
    return qHash(key.url().toString(), seed);
}


#endif // COAPOBSERVERESOURCE_H
