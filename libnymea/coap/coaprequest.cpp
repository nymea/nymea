/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2020, nymea GmbH
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

/*!
    \class CoapRequest
    \brief Represents a request to a CoAP server.

    \ingroup coap-group
    \inmodule libnymea

*/

#include "coaprequest.h"

/*! Constructs a CoAP request for the given \a url. */
CoapRequest::CoapRequest(const QUrl &url) :
    m_url(url),
    m_contentType(CoapPdu::TextPlain),
    m_messageType(CoapPdu::Confirmable),
    m_reqRspCode(CoapPdu::Empty)
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
