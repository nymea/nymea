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

#ifndef COAPOPTION_H
#define COAPOPTION_H

#include <QDebug>
#include <QObject>
#include <QByteArray>

#include "libnymea.h"

class LIBNYMEA_EXPORT CoapOption
{
    Q_GADGET

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
    Q_ENUM(Option)

    CoapOption();

    void setOption(const CoapOption::Option &option);
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
