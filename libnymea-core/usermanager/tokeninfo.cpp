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
    \class nymeaserver::TokenInfo
    \brief This class holds information about an authentication token.

    \ingroup user
    \inmodule core

    The TokenInfo class holds information about a token used for authentication in the \l{nymeaserver::UserManager}{UserManager}.

    \sa TokenInfo, PushButtonDBusService
*/

#include "tokeninfo.h"

#include <QVariant>

namespace nymeaserver {

TokenInfo::TokenInfo()
{

}

/*! Constructs a new token info with the given \a id, \a username, \a creationTime and \a deviceName. */
TokenInfo::TokenInfo(const QUuid &id, const QString &username, const QDateTime &creationTime, const QString &deviceName):
    m_id(id),
    m_username(username),
    m_creationTime(creationTime),
    m_deviceName(deviceName)
{

}

/*! Returns the id of this TokenInfo. */
QUuid TokenInfo::id() const
{
    return m_id;
}

/*! Returns the userename of this TokenInfo. */
QString TokenInfo::username() const
{
    return m_username;
}

/*! Returns the creation time of this TokenInfo. */
QDateTime TokenInfo::creationTime() const
{
    return m_creationTime;
}

/*! Returns the device name of this TokenInfo. */
QString TokenInfo::deviceName() const
{
    return m_deviceName;
}

QVariant TokenInfoList::get(int index) const
{
    return QVariant::fromValue(at(index));
}

void TokenInfoList::put(const QVariant &variant)
{
    append(variant.value<TokenInfo>());
}

}
