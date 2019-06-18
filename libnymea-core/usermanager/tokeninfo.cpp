/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2017 Michael Zanetti <michael.zanetti@guh.io>            *
 *                                                                         *
 *  This file is part of nymea.                                            *
 *                                                                         *
 *  nymea is free software: you can redistribute it and/or modify          *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  nymea is distributed in the hope that it will be useful,               *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with nymea. If not, see <http://www.gnu.org/licenses/>.          *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*!
    \class nymeaserver::TokenInfo
    \brief This class holds information about an authentication token.

    \ingroup user
    \inmodule core

    The TokenInfo class holds information about a token used for authentication in the \l{nymeaserver::UserManager}{UserManager}.

    \sa TokenInfo, PushButtonDBusService
*/

#include "tokeninfo.h"

namespace nymeaserver {

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

}
