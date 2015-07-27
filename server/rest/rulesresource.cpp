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

#include "rulesresource.h"

namespace guhserver {

RulesResource::RulesResource(QObject *parent) :
    RestResource(parent)
{
}

HttpReply *RulesResource::proccessRequest(const HttpRequest &request, const QStringList &urlTokens)
{
    Q_UNUSED(request)
    Q_UNUSED(urlTokens)

    return createErrorReply(HttpReply::NotImplemented);
}

HttpReply *RulesResource::proccessGetRequest(const HttpRequest &request, const QStringList &urlTokens)
{
    Q_UNUSED(request)
    Q_UNUSED(urlTokens)

    return createErrorReply(HttpReply::NotImplemented);}

HttpReply *RulesResource::proccessDeleteRequest(const HttpRequest &request, const QStringList &urlTokens)
{
    Q_UNUSED(request)
    Q_UNUSED(urlTokens)

    return createErrorReply(HttpReply::NotImplemented);
}

HttpReply *RulesResource::proccessPutRequest(const HttpRequest &request, const QStringList &urlTokens)
{
    Q_UNUSED(request)
    Q_UNUSED(urlTokens)

    return createErrorReply(HttpReply::NotImplemented);
}

HttpReply *RulesResource::proccessPostRequest(const HttpRequest &request, const QStringList &urlTokens)
{
    Q_UNUSED(request)
    Q_UNUSED(urlTokens)

    return createErrorReply(HttpReply::NotImplemented);
}

}

