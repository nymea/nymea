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

/*!
    \class guhserver::PluginsResource
    \brief This subclass of \l{RestResource} processes the REST requests for the \tt Plugins namespace.

    \ingroup json
    \inmodule core

    This \l{RestResource} will be created in the \l{RestServer} and used to handle REST requests
    for the \tt {Plugins} namespace of the API.

    \code
        http://localhost:3333/api/v1/plugins
    \endcode

    \sa DevicePlugin, RestResource, RestServer
*/

#include "pluginsresource.h"
#include "httprequest.h"
#include "loggingcategories.h"
#include "guhcore.h"

#include <QJsonDocument>

namespace guhserver {

/*! Constructs a \l PluginsResource with the given \a parent. */
PluginsResource::PluginsResource(QObject *parent) :
    RestResource(parent)
{

}

/*! Returns the name of the \l{RestResource}. In this case \b plugins.

    \sa RestResource::name()
*/
QString PluginsResource::name() const
{
    return "plugins";
}

/*! This method will be used to process the given \a request and the given \a urlTokens. The request
    has to be in this namespace. Returns the resulting \l HttpReply.

    \sa HttpRequest, HttpReply, RestResource::proccessRequest()
*/
HttpReply *PluginsResource::proccessRequest(const HttpRequest &request, const QStringList &urlTokens)
{
    // get the main resource

    // /api/v1/plugins/{pluginId}/
    if (urlTokens.count() >= 4) {
        m_pluginId = PluginId(urlTokens.at(3));
        if (m_pluginId.isNull()) {
            qCWarning(dcRest) << "Could not parse PluginId:" << urlTokens.at(3);
            return createDeviceErrorReply(HttpReply::BadRequest, DeviceManager::DeviceErrorPluginNotFound);
        }
    }

    // check method
    HttpReply *reply;
    switch (request.method()) {
    case HttpRequest::Get:
        reply = proccessGetRequest(request, urlTokens);
        break;
    case HttpRequest::Put:
        reply = proccessPutRequest(request, urlTokens);
        break;
    case HttpRequest::Options:
        reply = proccessOptionsRequest(request, urlTokens);
        break;
    default:
        reply = createErrorReply(HttpReply::BadRequest);
        break;
    }
    return reply;
}

HttpReply *PluginsResource::proccessGetRequest(const HttpRequest &request, const QStringList &urlTokens)
{
    Q_UNUSED(request)

    // GET /api/v1/plugins
    if (urlTokens.count() == 3)
        return getPlugins();

    // GET /api/v1/plugins/{pluginId}
    if (urlTokens.count() == 4)
        return getPlugin(m_pluginId);

    // GET /api/v1/plugins/{pluginId}/configuration
    if (urlTokens.count() == 5 && urlTokens.at(4) == "configuration") {
        return getPluginConfiguration(m_pluginId);
    }

    return createErrorReply(HttpReply::NotImplemented);
}

HttpReply *PluginsResource::proccessPutRequest(const HttpRequest &request, const QStringList &urlTokens)
{
    // PUT /api/v1/plugins/{pluginId}/configuration
    if (urlTokens.count() == 5 && urlTokens.at(4) == "configuration") {
        return setPluginConfiguration(m_pluginId, request.payload());
    }

    return createErrorReply(HttpReply::NotImplemented);
}

HttpReply *PluginsResource::proccessOptionsRequest(const HttpRequest &request, const QStringList &urlTokens)
{
    Q_UNUSED(request)
    Q_UNUSED(urlTokens)
    return RestResource::createCorsSuccessReply();
}

HttpReply *PluginsResource::getPlugins() const
{
    qCDebug(dcRest) << "Get plugins";
    HttpReply *reply = createSuccessReply();
    reply->setHeader(HttpReply::ContentTypeHeader, "application/json; charset=\"utf-8\";");
    reply->setPayload(QJsonDocument::fromVariant(JsonTypes::packPlugins()).toJson());
    return reply;
}

HttpReply *PluginsResource::getPlugin(const PluginId &pluginId) const
{
    qCDebug(dcRest) << "Get plugin with id" << pluginId;
    HttpReply *reply = createSuccessReply();
    foreach (DevicePlugin *plugin, GuhCore::instance()->deviceManager()->plugins()) {
        if (plugin->pluginId() == pluginId) {
            reply->setHeader(HttpReply::ContentTypeHeader, "application/json; charset=\"utf-8\";");
            reply->setPayload(QJsonDocument::fromVariant(JsonTypes::packPlugin(plugin)).toJson());
            return reply;
        }
    }
    return createDeviceErrorReply(HttpReply::NotFound, DeviceManager::DeviceErrorPluginNotFound);
}

HttpReply *PluginsResource::getPluginConfiguration(const PluginId &pluginId) const
{
    qCDebug(dcRest) << "Get configuration of plugin with id" << pluginId.toString();

    DevicePlugin *plugin = 0;
    plugin = findPlugin(pluginId);
    if (!plugin)
        return createDeviceErrorReply(HttpReply::NotFound, DeviceManager::DeviceErrorPluginNotFound);

    QVariantList configurationParamsList;
    foreach (const Param &param, plugin->configuration()) {
        configurationParamsList.append(JsonTypes::packParam(param));
    }

    HttpReply *reply = createSuccessReply();
    reply->setHeader(HttpReply::ContentTypeHeader, "application/json; charset=\"utf-8\";");
    reply->setPayload(QJsonDocument::fromVariant(configurationParamsList).toJson());
    return reply;
}

HttpReply *PluginsResource::setPluginConfiguration(const PluginId &pluginId, const QByteArray &payload) const
{
    DevicePlugin *plugin = 0;
    plugin = findPlugin(pluginId);
    if (!plugin)
        return createDeviceErrorReply(HttpReply::NotFound, DeviceManager::DeviceErrorPluginNotFound);

    qCDebug(dcRest) << "Set configuration of plugin with id" << pluginId.toString();

    QPair<bool, QVariant> verification = RestResource::verifyPayload(payload);
    if (!verification.first)
        return createErrorReply(HttpReply::BadRequest);

    QVariantList configuration = verification.second.toList();
    ParamList pluginParams = JsonTypes::unpackParams(configuration);
    qCDebug(dcRest) << pluginParams;
    DeviceManager::DeviceError result = GuhCore::instance()->deviceManager()->setPluginConfig(pluginId, pluginParams);

    if (result != DeviceManager::DeviceErrorNoError)
        return createDeviceErrorReply(HttpReply::BadRequest, result);

    return createDeviceErrorReply(HttpReply::Ok, result);
}

DevicePlugin *PluginsResource::findPlugin(const PluginId &pluginId) const
{
    foreach (DevicePlugin *plugin, GuhCore::instance()->deviceManager()->plugins()) {
        if (plugin->pluginId() == pluginId) {
            return plugin;
        }
    }
    return 0;
}

}
