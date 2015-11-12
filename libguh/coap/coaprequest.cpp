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

#include "coaprequest.h"

CoapRequest::CoapRequest(const QUrl &url) :
    m_url(url),
    m_contentType(CoapPdu::TextPlain),
    m_messageType(CoapPdu::Confirmable),
    m_statusCode(CoapPdu::Empty)
{
}

void CoapRequest::setUrl(const QUrl &url)
{
    m_url = url;
}

QUrl CoapRequest::url() const
{
    return m_url;
}

void CoapRequest::setContentType(const CoapPdu::ContentType contentType)
{
    m_contentType = contentType;
}

CoapPdu::ContentType CoapRequest::contentType() const
{
    return m_contentType;
}

void CoapRequest::setMessageType(const CoapPdu::MessageType &messageType)
{
    m_messageType = messageType;
}

CoapPdu::MessageType CoapRequest::messageType() const
{
    return m_messageType;
}

void CoapRequest::setStatusCode(const CoapPdu::StatusCode &statusCode)
{
    m_statusCode = statusCode;
}

CoapPdu::StatusCode CoapRequest::statusCode()
{
    return m_statusCode;
}



