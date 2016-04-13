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

#ifndef COAPOPTION_H
#define COAPOPTION_H

#include <QDebug>
#include <QObject>
#include <QByteArray>

class CoapOption
{
    Q_GADGET
    Q_ENUMS(Option)

public:
    // Options format: https://tools.ietf.org/html/rfc7252#section-3.1
    enum Option {
        IfMatch       = 1,
        UriHost       = 3,
        ETag          = 4,
        IfNoneMatch   = 5,
        Observe       = 6, // (Observe) https://tools.ietf.org/html/rfc7641
        UriPort       = 7,
        LocationPath  = 8,
        UriPath       = 11,
        ContentFormat = 12,
        MaxAge        = 14,
        UriQuery      = 15,
        Accept        = 17,
        LocationQuery = 20,
        Block2        = 23, // (Block) https://tools.ietf.org/html/draft-ietf-core-block-18
        Block1        = 27, // (Block)
        ProxyUri      = 35,
        ProxyScheme   = 39,
        Size1         = 60
    };

    CoapOption();
    CoapOption(const Option &option, const QByteArray &data);

    void setOption(const Option &option);
    Option option() const;

    void setData(const QByteArray &data);
    QByteArray data() const;

private:
    Option m_option;
    QByteArray m_data;
};

Q_DECLARE_METATYPE(CoapOption)

QDebug operator<<(QDebug debug, const CoapOption &coapOption);

#endif // COAPOPTION_H
