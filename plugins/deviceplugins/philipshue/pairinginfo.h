/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
 *                                                                         *
 *  This file is part of guh.                                              *
 *                                                                         *
 *  This library is free software; you can redistribute it and/or          *
 *  modify it under the terms of the GNU Lesser General Public             *
 *  License as published by the Free Software Foundation; either           *
 *  version 2.1 of the License, or (at your option) any later version.     *
 *                                                                         *
 *  This library is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
 *  Lesser General Public License for more details.                        *
 *                                                                         *
 *  You should have received a copy of the GNU Lesser General Public       *
 *  License along with this library; If not, see                           *
 *  <http://www.gnu.org/licenses/>.                                        *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef PAIRINGINFO_H
#define PAIRINGINFO_H

#include <QObject>
#include <QHostAddress>

#include "typeutils.h"

class PairingInfo : public QObject
{
    Q_OBJECT
public:
    explicit PairingInfo(QObject *parent = 0);

    PairingTransactionId pairingTransactionId() const;
    void setPairingTransactionId(const PairingTransactionId &pairingTransactionId);

    QHostAddress host() const;
    void setHost(const QHostAddress &host);

    QString apiKey() const;
    void setApiKey(const QString &apiKey);

private:
    PairingTransactionId m_pairingTransactionId;
    QHostAddress m_host;
    QString m_apiKey;
};

#endif // PAIRINGINFO_H
