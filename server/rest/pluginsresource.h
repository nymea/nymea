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

#ifndef PLUGINSRESOURCE_H
#define PLUGINSRESOURCE_H

#include <QObject>
#include <QHash>

#include "jsontypes.h"
#include "restresource.h"
#include "network/httpreply.h"

class HttpRequest;

namespace guhserver {

class PluginsResource : public RestResource
{
    Q_OBJECT
public:
    explicit PluginsResource(QObject *parent = 0);

    QString name() const override;

    HttpReply *proccessRequest(const HttpRequest &request, const QStringList &urlTokens) override;

private:
    // Process method
    HttpReply *proccessGetRequest(const HttpRequest &request, const QStringList &urlTokens) override;
    HttpReply *proccessDeleteRequest(const HttpRequest &request, const QStringList &urlTokens) override;
    HttpReply *proccessPutRequest(const HttpRequest &request, const QStringList &urlTokens) override;
    HttpReply *proccessPostRequest(const HttpRequest &request, const QStringList &urlTokens) override;

    // Get methods

    // Delete methods

    // Post methods

    // Put methods

};

}

#endif // PLUGINSRESOURCE_H
