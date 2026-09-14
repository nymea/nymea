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

#ifndef INVITATIONINFO_H
#define INVITATIONINFO_H

#include <QUuid>
#include <QDateTime>
#include <QMetaType>
#include <QVariant>

namespace nymeaserver {

class InvitationInfo
{
    Q_GADGET
    Q_PROPERTY(QUuid id READ id)
    Q_PROPERTY(QString username READ username)
    Q_PROPERTY(QDateTime creationTime READ creationTime)
    // Invitation expiry is always set, unlike TokenInfo.expiryTime.
    Q_PROPERTY(QDateTime expiryTime READ expiryTime)
    // Invalid QVariant means the redeemed token never expires. Following the established
    // State::minValue/maxValue idiom: a plain optional uint would always serialize (the
    // generic JsonHandler "isUser() + empty" check only omits QString/QUuid/QDateTime
    // specially; anything else only omits when the QVariant itself is invalid).
    Q_PROPERTY(QVariant tokenValidityDuration READ tokenValidityDuration USER true)

public:
    InvitationInfo();
    InvitationInfo(const QUuid &id, const QString &username, const QDateTime &creationTime,
                   const QDateTime &expiryTime, const QVariant &tokenValidityDuration = QVariant());

    QUuid id() const;
    QString username() const;
    QDateTime creationTime() const;
    QDateTime expiryTime() const;
    QVariant tokenValidityDuration() const;

private:
    QUuid m_id;
    QString m_username;
    QDateTime m_creationTime;
    QDateTime m_expiryTime;
    QVariant m_tokenValidityDuration;
};


class InvitationInfoList: public QList<InvitationInfo>
{
    Q_GADGET
    Q_PROPERTY(int count READ count)
public:
    Q_INVOKABLE QVariant get(int index) const;
    Q_INVOKABLE void put(const QVariant &variant);
};
}

Q_DECLARE_METATYPE(nymeaserver::InvitationInfo)
Q_DECLARE_METATYPE(nymeaserver::InvitationInfoList)
#endif // INVITATIONINFO_H
