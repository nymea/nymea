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

/*!
    \class CoapObserveResource
    \brief Holds information about an observed resource.

    \ingroup coap
    \inmodule libguh

    The CoapObserveResource class holds information about an observed resource.

    \sa Coap::notificationReceived()

*/

#include "coapobserveresource.h"

/*! Constructs a CoapObserveResource. */
CoapObserveResource::CoapObserveResource()
{
}

/*! Constructs a CoapObserveResource with the given \a url and \a token. */
CoapObserveResource::CoapObserveResource(const QUrl &url, const QByteArray &token):
    m_url(url),
    m_token(token)
{
}

/*! Constructs a copy of the given \a other \l{CoapObserveResource}. */
CoapObserveResource::CoapObserveResource(const CoapObserveResource &other)
{
    m_url = other.url();
    m_token = other.token();
}

/*! Returns the url of this \l{CoapObserveResource}. */
QUrl CoapObserveResource::url() const
{
    return m_url;
}

/*! Returns the token of this \l{CoapObserveResource}. */
QByteArray CoapObserveResource::token() const
{
    return m_token;
}
