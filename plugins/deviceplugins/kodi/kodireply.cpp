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

#include "kodireply.h"

KodiReply::KodiReply()
{
}

KodiReply::KodiReply(const QString &method, const QVariantMap &params, const ActionId &actionId) :
    m_method(method),
    m_params(params),
    m_actionId(actionId)
{
}

void KodiReply::setActionId(const ActionId &actionId)
{
    m_actionId = actionId;
}

ActionId KodiReply::actionId() const
{
    return m_actionId;
}

void KodiReply::setMethod(const QString &method)
{
    m_method = method;
}

QString KodiReply::method() const
{
    return m_method;
}

void KodiReply::setParams(const QVariantMap &params)
{
    m_params = params;
}

QVariantMap KodiReply::params() const
{
    return m_params;
}
