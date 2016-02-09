/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stuerz <simon.stuerz@guh.guru>                *
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
#include <QUrl>

#include "libguh.h"

class LIBGUH_EXPORT CoapObserveResource
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

#endif // COAPOBSERVERESOURCE_H
