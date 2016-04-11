/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015-2016 Simon Stuerz <simon.stuerz@guh.guru>           *
 *                                                                         *
 *  This file is part of QtCoap.                                           *
 *                                                                         *
 *  QtCoap is free software: you can redistribute it and/or modify         *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 3 of the License.                *
 *                                                                         *
 *  QtCoap is distributed in the hope that it will be useful,              *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the           *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with QtCoap. If not, see <http://www.gnu.org/licenses/>.         *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef COAPOBSERVERESOURCE_H
#define COAPOBSERVERESOURCE_H

#include <QObject>
#include <QHash>
#include <QUrl>

class CoapObserveResource
{

public:
    CoapObserveResource();
    CoapObserveResource(const QUrl &url, const QByteArray &token);
    CoapObserveResource(const CoapObserveResource &other);

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
