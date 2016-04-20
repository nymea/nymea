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

/*!
    \class CoapRequest
    \brief Represents a request to a CoAP server.

    \ingroup coap
    \inmodule libguh

*/

#include "coaprequest.h"

/*! Constructs a CoAP request for the given \a url. */
CoapRequest::CoapRequest(const QUrl &url) :
    m_url(url),
    m_contentType(CoapPdu::TextPlain),
    m_messageType(CoapPdu::Confirmable),
    m_statusCode(CoapPdu::Empty)
{
}

/*! Sets the URL of this CoAP request to the given \a url. */
void CoapRequest::setUrl(const QUrl &url)
{
    m_url = url;
}

/*! Returns the URL of this CoAP request. */
QUrl CoapRequest::url() const
{
    return m_url;
}

/*! Sets the \l{CoapPdu::ContentType}{ContentType} of this CoAP request to the given \a contentType.
 *  This method will only be used with the PUT and POST method and should describe the content format
 *  of the payload. */
void CoapRequest::setContentType(const CoapPdu::ContentType contentType)
{
    m_contentType = contentType;
}

/*! Returns the content format of the payload. */
CoapPdu::ContentType CoapRequest::contentType() const
{
    return m_contentType;
}

/*! Sets the \l{CoapPdu::MessageType}{MessageType} of this CoAP request to the given \a messageType. */
void CoapRequest::setMessageType(const CoapPdu::MessageType &messageType)
{
    m_messageType = messageType;
}

/*! Returns the message type of this CoapRequest. */
CoapPdu::MessageType CoapRequest::messageType() const
{
    return m_messageType;
}
