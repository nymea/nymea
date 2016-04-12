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

#ifndef COAPREQUEST_H
#define COAPREQUEST_H

#include <QUrl>
#include <QHostAddress>

#include "coappdu.h"
#include "coapoption.h"

//class Coap;

class CoapRequest
{
//    friend class Coap;
public:
    CoapRequest(const QUrl &url = QUrl());

    void setUrl(const QUrl &url);
    QUrl url() const;

    void setContentType(const CoapPdu::ContentType contentType = CoapPdu::TextPlain);
    CoapPdu::ContentType contentType() const;

    void setMessageType(const CoapPdu::MessageType &messageType);
    CoapPdu::MessageType messageType() const;

private:
    QUrl m_url;
    CoapPdu::ContentType m_contentType;
    CoapPdu::MessageType m_messageType;
    CoapPdu::StatusCode m_statusCode;

    void setStatusCode(const CoapPdu::StatusCode &statusCode);
    CoapPdu::StatusCode statusCode();

};

#endif // COAPREQUEST_H
