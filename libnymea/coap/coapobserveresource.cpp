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

/*!
    \class CoapObserveResource
    \brief Holds information about an observed resource.

    \ingroup coap-group
    \inmodule libnymea

    The CoapObserveResource class holds information about an observed resource.

    \sa Coap

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
