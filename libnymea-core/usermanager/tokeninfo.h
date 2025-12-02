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

#ifndef TOKENINFO_H
#define TOKENINFO_H

#include <QUuid>
#include <QDateTime>
#include <QMetaType>
#include <QVariant>

namespace nymeaserver {

class TokenInfo
{
    Q_GADGET
    Q_PROPERTY(QUuid id READ id)
    Q_PROPERTY(QString username READ username)
    Q_PROPERTY(QDateTime creationTime READ creationTime)
    Q_PROPERTY(QString deviceName READ deviceName)

public:
    TokenInfo();
    TokenInfo(const QUuid &id, const QString &username, const QDateTime &creationTime, const QString &deviceName);

    QUuid id() const;
    QString username() const;
    QDateTime creationTime() const;
    QString deviceName() const;

private:
    QUuid m_id;
    QString m_username;
    QDateTime m_creationTime;
    QString m_deviceName;
};


class TokenInfoList: public QList<TokenInfo>
{
    Q_GADGET
    Q_PROPERTY(int count READ count)
public:
    Q_INVOKABLE QVariant get(int index) const;
    Q_INVOKABLE void put(const QVariant &variant);
};
}

Q_DECLARE_METATYPE(nymeaserver::TokenInfo)
Q_DECLARE_METATYPE(nymeaserver::TokenInfoList)
#endif // TOKENINFO_H
