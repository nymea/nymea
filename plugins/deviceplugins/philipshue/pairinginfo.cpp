/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stuerz <simon.stuerz@guh.guru>                *
 *                                                                         *
 *  This file is part of guh.                                              *
 *                                                                         *
 *  Guh is free software: you can redistribute it and/or modify            *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  Guh is distributed in the hope that it will be useful,                 *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with guh. If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "pairinginfo.h"

PairingInfo::PairingInfo(QObject *parent) :
    QObject(parent)
{
}

PairingTransactionId PairingInfo::pairingTransactionId() const
{
    return m_pairingTransactionId;
}

void PairingInfo::setPairingTransactionId(const PairingTransactionId &pairingTransactionId)
{
    m_pairingTransactionId = pairingTransactionId;
}

QHostAddress PairingInfo::host() const
{
    return m_host;
}

void PairingInfo::setHost(const QHostAddress &host)
{
    m_host = host;
}

QString PairingInfo::apiKey() const
{
    return m_apiKey;
}

void PairingInfo::setApiKey(const QString &apiKey)
{
    m_apiKey = apiKey;
}
