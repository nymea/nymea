/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2016 Simon Stürz <simon.stuerz@guh.io>                   *
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

#include "cloudjsonreply.h"

namespace guhserver {

CloudJsonReply::CloudJsonReply(int commandId, QString nameSpace, QString method, QVariantMap params, QObject *parent) :
    QObject(parent),
    m_commandId(commandId),
    m_nameSpace(nameSpace),
    m_method(method),
    m_params(params)
{

}

int CloudJsonReply::commandId() const
{
    return m_commandId;
}

QString CloudJsonReply::nameSpace() const
{
    return m_nameSpace;
}

QString CloudJsonReply::method() const
{
    return m_method;
}

QVariantMap CloudJsonReply::params() const
{
    return m_params;
}

QVariantMap CloudJsonReply::requestMap()
{
    QVariantMap request;
    request.insert("id", m_commandId);
    request.insert("method", m_nameSpace + "." + m_method);
    if (!m_params.isEmpty())
        request.insert("params", m_params);

    return request;
}

}
