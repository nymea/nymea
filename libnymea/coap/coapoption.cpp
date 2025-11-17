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
    \class CoapOption
    \brief Represents the option of a \l{CoapPdu}.

    \ingroup coap-group
    \inmodule libnymea

    The CoapOption class provides an easy way to create / parse CoAP options of a \l{CoapPdu} (Protocol Data Unit). An options
    can be compared with a HTTP header.

*/

/*! \enum CoapOption::Option
    Represents the known CoAP options according to \l{https://tools.ietf.org/html/rfc7252#section-3.1}{RFC7252}.

    \value IfMatch
    \value UriHost
    \value ETag
    \value IfNoneMatch
    \value Observe
        \l{https://tools.ietf.org/html/rfc7641}

    \value UriPort
    \value LocationPath
    \value UriPath
    \value ContentFormat
    \value MaxAge
    \value UriQuery
    \value Accept
    \value LocationQuery
    \value Block2
        \l{https://tools.ietf.org/html/draft-ietf-core-block-18}

    \value Block1
        \l{https://tools.ietf.org/html/draft-ietf-core-block-18}

    \value ProxyUri
    \value ProxyScheme
    \value Size1

*/


#include "coapoption.h"

#include <QMetaEnum>

/*! Constructs a CoapOption. */
CoapOption::CoapOption()
{

}

/*! Sets the \a option of this CoapOption to the given parameter. */
void CoapOption::setOption(const CoapOption::Option &option)
{
    m_option = option;
}

/*! Returns the option value of this CoapOption. */
CoapOption::Option CoapOption::option() const
{
    return m_option;
}

/*! Sets the data of this CoapOption to the given \a data. */
void CoapOption::setData(const QByteArray &data)
{
    m_data = data;
}

/*! Returns the data of this CoapOption. */
QByteArray CoapOption::data() const
{
    return m_data;
}

#include "coappdu.h"

/*! Writes the data of the given \a coapOption to \a dbg.

    \sa CoapOption
*/
QDebug operator<<(QDebug debug, const CoapOption &coapOption)
{
    QDebugStateSaver saver(debug);
    const QMetaObject &metaObject = CoapOption::staticMetaObject;
    QMetaEnum optionEnum = metaObject.enumerator(metaObject.indexOfEnumerator("Option"));

    switch (coapOption.option()) {
    case CoapOption::ETag:
        debug.nospace() << "CoapOption(" << optionEnum.valueToKey(coapOption.option()) << "): " << "0x" + coapOption.data().toHex() << '\n';
        break;
    case CoapOption::UriHost:
        debug.nospace() << "CoapOption(" << optionEnum.valueToKey(coapOption.option()) << "): " << coapOption.data() << '\n';
        break;
    case CoapOption::UriPath:
        debug.nospace() << "CoapOption(" << optionEnum.valueToKey(coapOption.option()) << "): " << coapOption.data() << '\n';
        break;
    case CoapOption::UriQuery:
        debug.nospace() << "CoapOption(" << optionEnum.valueToKey(coapOption.option()) << "): " << coapOption.data() << '\n';
        break;
    case CoapOption::ContentFormat: {
        const QMetaObject &pduMetaObject = CoapPdu::staticMetaObject;
        QMetaEnum contentEnum = pduMetaObject.enumerator(pduMetaObject.indexOfEnumerator("ContentType"));
        debug.nospace() << "CoapOption(" << optionEnum.valueToKey(coapOption.option()) << "): " << contentEnum.valueToKey(static_cast<CoapPdu::ContentType>(coapOption.data().toHex().toInt(0, 16))) << '\n';
        break;
    }
    case CoapOption::Block1: {
        // SZX = size exponent
        CoapPduBlock block(coapOption.data());
        debug.nospace() << "CoapOption(" << optionEnum.valueToKey(coapOption.option()) << "): " << coapOption.data().toHex() << " Block #" << block.blockNumber() << ", More flag = " << block.moreFlag() << ", SZX:" << block.blockSize() << '\n';
        break;
    }
    case CoapOption::Block2: {
        // SZX = size exponent
        CoapPduBlock block(coapOption.data());
        debug.nospace() << "CoapOption(" << optionEnum.valueToKey(coapOption.option()) << "): " << coapOption.data().toHex() << " Block #" << block.blockNumber() << ", More flag = " << block.moreFlag() << ", SZX:" << block.blockSize() << '\n';
        break;
    }
    case CoapOption::Observe: {
        debug.nospace() << "CoapOption(" << optionEnum.valueToKey(coapOption.option()) << "): " << coapOption.data().toHex().toInt(0, 16) << '\n';
        break;
    }
    default:
        QString optionName = optionEnum.valueToKey(coapOption.option());
        if (optionName.isNull()) {
            debug.nospace() << "CoapOption(" << "Unknown" << "): " << "value = " << coapOption.option() << " -> " << coapOption.data() << " = " << "0x" + coapOption.data().toHex() << '\n';
        } else {
            debug.nospace() << "CoapOption(" << optionName << "): " << coapOption.data() << " = " << "0x" + coapOption.data().toHex() << '\n';
        }
        break;
    }

    return debug;
}
