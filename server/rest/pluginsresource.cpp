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

#include "pluginsresource.h"
#include "network/httprequest.h"

namespace guhserver {

PluginsResource::PluginsResource(QObject *parent) :
    RestResource(parent)
{
}

HttpReply *PluginsResource::proccessRequest(const HttpRequest &request, const QStringList &urlTokens)
{
    Q_UNUSED(request)
    Q_UNUSED(urlTokens)

    return createErrorReply(HttpReply::NotImplemented);
}

HttpReply *PluginsResource::proccessGetRequest(const HttpRequest &request, const QStringList &urlTokens)
{
    Q_UNUSED(request)
    Q_UNUSED(urlTokens)

    return createErrorReply(HttpReply::NotImplemented);
}

HttpReply *PluginsResource::proccessDeleteRequest(const HttpRequest &request, const QStringList &urlTokens)
{
    Q_UNUSED(request)
    Q_UNUSED(urlTokens)

    return createErrorReply(HttpReply::NotImplemented);
}

HttpReply *PluginsResource::proccessPutRequest(const HttpRequest &request, const QStringList &urlTokens)
{
    Q_UNUSED(request)
    Q_UNUSED(urlTokens)

    return createErrorReply(HttpReply::NotImplemented);
}

HttpReply *PluginsResource::proccessPostRequest(const HttpRequest &request, const QStringList &urlTokens)
{
    Q_UNUSED(request)
    Q_UNUSED(urlTokens)

    return createErrorReply(HttpReply::NotImplemented);
}

}

