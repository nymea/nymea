/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
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

#include "jsonrpc/jsontypes.h"
#include "restresource.h"
#include "httpreply.h"

namespace guhserver {

class HttpRequest;

class PluginsResource : public RestResource
{
    Q_OBJECT
public:
    explicit PluginsResource(QObject *parent = 0);

    QString name() const override;

    HttpReply *proccessRequest(const HttpRequest &request, const QStringList &urlTokens) override;

private:
    PluginId m_pluginId;

    // Process method
    HttpReply *proccessGetRequest(const HttpRequest &request, const QStringList &urlTokens) override;
    HttpReply *proccessPutRequest(const HttpRequest &request, const QStringList &urlTokens) override;
    HttpReply *proccessOptionsRequest(const HttpRequest &request, const QStringList &urlTokens) override;

    // Get methods
    HttpReply *getPlugins() const;
    HttpReply *getPlugin(const PluginId &pluginId) const;
    HttpReply *getPluginConfiguration(const PluginId &pluginId) const;
    HttpReply *setPluginConfiguration(const PluginId &pluginId, const QByteArray &payload) const;

    // Put methods


    DevicePlugin *findPlugin(const PluginId &pluginId) const;
};

}

#endif // PLUGINSRESOURCE_H
