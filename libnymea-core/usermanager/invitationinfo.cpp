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
    \class nymeaserver::InvitationInfo
    \brief This class holds information about a guest-login invitation.

    \ingroup user
    \inmodule core

    The InvitationInfo class holds information about a one-time invitation minted by an
    admin in the \l{nymeaserver::UserManager}{UserManager}. It never carries the clear
    one-time token, only its id.

    \sa InvitationInfo, TokenInfo
*/

#include "invitationinfo.h"

namespace nymeaserver {

InvitationInfo::InvitationInfo()
{

}

/*! Constructs a new invitation info with the given \a id, \a username, \a creationTime,
    \a expiryTime (invitation validity end, always set) and optional \a tokenValidityDuration
    (seconds from redemption; invalid means the redeemed token never expires). */
InvitationInfo::InvitationInfo(const QUuid &id, const QString &username, const QDateTime &creationTime,
                                const QDateTime &expiryTime, const QVariant &tokenValidityDuration):
    m_id(id),
    m_username(username),
    m_creationTime(creationTime),
    m_expiryTime(expiryTime),
    m_tokenValidityDuration(tokenValidityDuration)
{

}

/*! Returns the id of this InvitationInfo. This is not the one-time token. */
QUuid InvitationInfo::id() const
{
    return m_id;
}

/*! Returns the username this invitation was created for. */
QString InvitationInfo::username() const
{
    return m_username;
}

/*! Returns the creation time of this InvitationInfo. */
QDateTime InvitationInfo::creationTime() const
{
    return m_creationTime;
}

/*! Returns the absolute end of this invitation's own validity. Always set. */
QDateTime InvitationInfo::expiryTime() const
{
    return m_expiryTime;
}

/*! Returns the number of seconds the redeemed regular token remains valid, counted from a
    successful redemption, or an invalid QVariant if the redeemed token never expires. */
QVariant InvitationInfo::tokenValidityDuration() const
{
    return m_tokenValidityDuration;
}

QVariant InvitationInfoList::get(int index) const
{
    return QVariant::fromValue(at(index));
}

void InvitationInfoList::put(const QVariant &variant)
{
    append(variant.value<InvitationInfo>());
}

}
