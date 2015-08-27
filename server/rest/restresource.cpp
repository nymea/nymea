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
    \class guhserver::RestResource
    \brief This class provides an interface for REST API resources.

    \ingroup api
    \inmodule core

    The \l{RestResource} class provides an interface for subclassing a new resource for
    the REST API. A resource represents an Object in the core like \l{Device}{Devices}
    or \l{DevicePlugin}{Plugins}. A \l{RestResource} will be identified by it's
    \l{RestResource::name()}{name}.

    Each subclass of \l{RestResource} has to implement the method \l{RestResource::proccessRequest()}
    where a valid \l{HttpRequest} will be processed.

    This name of the resource will define the endpoint of the an API call:
    \code
    http://localhost:3333/api/v1/<resourceName>
    \endcode

    \sa RestServer
*/

/*! \fn QString guhserver::RestResource::name() const;
    This method must be implemented in a subclass of \l{RestResource}. It returns
    the endpoint name of this \l{RestResource} i.e. if this method returns \tt "example"
    the API resource will be accessable with the URL:

    \code
    http://localhost:3333/api/v1/example
    \endcode

*/

/*! \fn HttpReply *guhserver::RestResource::proccessRequest(const HttpRequest &request, const QStringList &urlTokens)
    This method will be called from the \l{RestServer} once a \l{HttpRequest} \a request was identified to belong
    to this \l{RestResource}. The given \a urlTokens contain the full list of URL tokens from this request.
*/

#include "restresource.h"
#include "httprequest.h"
#include "loggingcategories.h"
#include "guhcore.h"

#include <QJsonDocument>
#include <QVariant>

namespace guhserver {

/*! Constructs a \l{RestResource} with the given \a parent. */
RestResource::RestResource(QObject *parent) :
    QObject(parent)
{
}

/*! Pure virtual destructor for this \l{RestResource}. */
RestResource::~RestResource()
{
}

/*! Returns the pointer to a new created \l{HttpReply} initialized with \l{HttpReply::Ok} and \l{HttpReply::TypeSync}.  */
HttpReply *RestResource::createSuccessReply()
{
    HttpReply *reply = new HttpReply(HttpReply::Ok, HttpReply::TypeSync);
    reply->setHeader(HttpReply::ContentTypeHeader, "application/json; charset=\"utf-8\";");
    return reply;
}

HttpReply *RestResource::createCorsSuccessReply()
{
    HttpReply *reply = RestResource::createSuccessReply();
    reply->setHeader(HttpReply::ContentTypeHeader, "text/plain");
    reply->setRawHeader("Access-Control-Allow-Methods", "PUT, POST, GET, DELETE, OPTIONS");
    reply->setRawHeader("Access-Control-Allow-Headers", "Origin, Content-Type, Accept");
    reply->setRawHeader("Access-Control-Max-Age", "1728000");
    reply->setCloseConnection(true);
    return reply;
}

/*! Returns the pointer to a new created error \l{HttpReply} initialized with the given \a statusCode and \l{HttpReply::TypeSync}.  */
HttpReply *RestResource::createErrorReply(const HttpReply::HttpStatusCode &statusCode)
{
    HttpReply *reply = new HttpReply(statusCode, HttpReply::TypeSync);
    reply->setPayload(QByteArray::number(reply->httpStatusCode()) + " " + reply->httpReasonPhrase());
    return reply;
}

/*! Returns the pointer to a new created \l{HttpReply} initialized with \l{HttpReply::Ok} and \l{HttpReply::TypeAsync}.  */
HttpReply *RestResource::createAsyncReply()
{
    HttpReply *reply = new HttpReply(HttpReply::Ok, HttpReply::TypeAsync);
    return reply;
}

/*! This method can be used from every \l{RestResource} in order to verify if the \a payload of a
    \l{HttpRequest} is a valid JSON document. Returns \tt true and the valid \e QVariant if there
    was no error while parsing JSON. Returns \tt false and an invalid \e QVariant if the \a payload
    could not be parsed.
*/
QPair<bool, QVariant> RestResource::verifyPayload(const QByteArray &payload)
{
    QVariant data;
    if (!payload.isEmpty()) {
        QJsonParseError error;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(payload, &error);

        if(error.error != QJsonParseError::NoError) {
            qCWarning(dcRest) << "Failed to parse JSON payload" << payload << ":" << error.errorString();
            return QPair<bool, QVariant>(true, QVariant());
        }

        data = jsonDoc.toVariant();
    }
    return QPair<bool, QVariant>(true, data);
}

HttpReply *RestResource::proccessPostRequest(const HttpRequest &request, const QStringList &urlTokens)
{
    Q_UNUSED(request)
    Q_UNUSED(urlTokens)
    return createErrorReply(HttpReply::NotImplemented);
}

HttpReply *RestResource::proccessOptionsRequest(const HttpRequest &request, const QStringList &urlTokens)
{
    Q_UNUSED(request)
    Q_UNUSED(urlTokens)
    return createErrorReply(HttpReply::NotImplemented);
}

HttpReply *RestResource::proccessPutRequest(const HttpRequest &request, const QStringList &urlTokens)
{
    Q_UNUSED(request)
    Q_UNUSED(urlTokens)
    return createErrorReply(HttpReply::NotImplemented);
}

HttpReply *RestResource::proccessDeleteRequest(const HttpRequest &request, const QStringList &urlTokens)
{
    Q_UNUSED(request)
    Q_UNUSED(urlTokens)
    return createErrorReply(HttpReply::NotImplemented);
}

HttpReply *RestResource::proccessGetRequest(const HttpRequest &request, const QStringList &urlTokens)
{
    Q_UNUSED(request)
    Q_UNUSED(urlTokens)
    return createErrorReply(HttpReply::NotImplemented);
}

}
