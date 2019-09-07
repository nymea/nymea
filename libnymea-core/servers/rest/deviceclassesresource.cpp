/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
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
    \class nymeaserver::DeviceClassesResource
    \brief This subclass of \l{RestResource} processes the REST requests for the \tt DeviceClasses namespace.

    \ingroup json
    \inmodule core

    This \l{RestResource} will be created in the \l{RestServer} and used to handle REST requests
    for the \tt {DeviceClasses} namespace of the API.

    \code
        http://localhost:3333/api/v1/deviceclasses
    \endcode

    \sa DeviceClass, RestResource, RestServer
*/

#include "deviceclassesresource.h"
#include "servers/httprequest.h"
#include "nymeacore.h"

#include <QJsonDocument>

namespace nymeaserver {

/*! Constructs a \l DeviceClassesResource with the given \a parent. */
DeviceClassesResource::DeviceClassesResource(QObject *parent) :
    RestResource(parent)
{
    connect(NymeaCore::instance(), &NymeaCore::devicesDiscovered, this, &DeviceClassesResource::devicesDiscovered, Qt::QueuedConnection);
}

/*! Returns the name of the \l{RestResource}. In this case \b deviceclasses.

    \sa RestResource::name()
*/
QString DeviceClassesResource::name() const
{
    return "deviceclasses";
}

/*! This method will be used to process the given \a request and the given \a urlTokens. The request
    has to be in this namespace. Returns the resulting \l HttpReply.

    \sa HttpRequest, HttpReply, RestResource::proccessRequest()
*/
HttpReply *DeviceClassesResource::proccessRequest(const HttpRequest &request, const QStringList &urlTokens)
{

    // get the main resource

    // /api/v1/deviceclasses/{deviceClassId}/
    if (urlTokens.count() >= 4) {
        DeviceClassId deviceClassId = DeviceClassId(urlTokens.at(3));
        if (deviceClassId.isNull()) {
            qCWarning(dcRest) << "Could not parse DeviceClassId:" << urlTokens.at(3);
            return createDeviceErrorReply(HttpReply::BadRequest, Device::DeviceErrorDeviceClassNotFound);
        }
        m_deviceClass = NymeaCore::instance()->deviceManager()->findDeviceClass(deviceClassId);
        if (!m_deviceClass.isValid()) {
            qCWarning(dcRest) << "DeviceClassId" << deviceClassId.toString() << "not found";
            return createDeviceErrorReply(HttpReply::NotFound, Device::DeviceErrorDeviceClassNotFound);
        }
    }

    // check method
    HttpReply *reply;
    switch (request.method()) {
    case HttpRequest::Get:
        reply = proccessGetRequest(request, urlTokens);
        break;
    default:
        reply = createErrorReply(HttpReply::BadRequest);
        break;
    }
    return reply;
}

HttpReply *DeviceClassesResource::proccessGetRequest(const HttpRequest &request, const QStringList &urlTokens)
{
    Q_UNUSED(request)

    // GET /api/v1/deviceclasses?vendorId="{vendorId}"
    if (urlTokens.count() == 3) {
        VendorId vendorId;
        if (request.url().hasQuery()) {
            if (request.urlQuery().hasQueryItem("vendorId")) {
                vendorId = VendorId(request.urlQuery().queryItemValue("vendorId"));
                if (vendorId.isNull()) {
                    qCWarning(dcRest) << "Could not parse VendorId:" << request.urlQuery().queryItemValue("vendorId");
                    return createDeviceErrorReply(HttpReply::BadRequest, Device::DeviceErrorVendorNotFound);
                }
            }
        }
        return getDeviceClasses(vendorId);
    }

    // GET /api/v1/deviceclasses/{deviceClassId}
    if (urlTokens.count() == 4)
        return getDeviceClass();

    // GET /api/v1/deviceclasses/{deviceClassId}/actiontypes
    if (urlTokens.count() == 5 && urlTokens.at(4) == "actiontypes")
        return getActionTypes();

    // GET /api/v1/deviceclasses/{deviceClassId}/actiontypes/{actionTypeId}
    if (urlTokens.count() == 6 && urlTokens.at(4) == "actiontypes") {
        ActionTypeId actionTypeId = ActionTypeId(urlTokens.at(5));
        if (actionTypeId.isNull()) {
            qCWarning(dcRest) << "Could not parse ActionTypeId:" << urlTokens.at(5);
            return createDeviceErrorReply(HttpReply::BadRequest, Device::DeviceErrorActionTypeNotFound);
        }
        return getActionType(actionTypeId);
    }

    // GET /api/v1/deviceclasses/{deviceClassId}/statetypes
    if (urlTokens.count() == 5 && urlTokens.at(4) == "statetypes")
        return getStateTypes();

    // GET /api/v1/deviceclasses/{deviceClassId}/statetypes/{stateTypeId}
    if (urlTokens.count() == 6 && urlTokens.at(4) == "statetypes") {
        StateTypeId stateTypeId = StateTypeId(urlTokens.at(5));
        if (stateTypeId.isNull()) {
            qCWarning(dcRest) << "Could not parse StateTypeId:" << urlTokens.at(5);
            return createDeviceErrorReply(HttpReply::BadRequest, Device::DeviceErrorStateTypeNotFound);
        }
        return getStateType(stateTypeId);
    }

    // GET /api/v1/deviceclasses/{deviceClassId}/eventtypes
    if (urlTokens.count() == 5 && urlTokens.at(4) == "eventtypes")
        return getEventTypes();

    // GET /api/v1/deviceclasses/{deviceClassId}/eventtypes/{eventTypeId}
    if (urlTokens.count() == 6 && urlTokens.at(4) == "eventtypes") {
        EventTypeId eventTypeId = EventTypeId(urlTokens.at(5));
        if (eventTypeId.isNull()) {
            qCWarning(dcRest) << "Could not parse EventTypeId:" << urlTokens.at(5);
            return createDeviceErrorReply(HttpReply::BadRequest, Device::DeviceErrorEventTypeNotFound);
        }
        return getEventType(eventTypeId);
    }

    // GET /api/v1/deviceclasses/{deviceClassId}/discover?name=paramName&value=paramValue
    if (urlTokens.count() == 5 && urlTokens.at(4) == "discover") {
        ParamList params;
        if (request.urlQuery().hasQueryItem("params")) {
            QString paramMapString = request.urlQuery().queryItemValue("params");

            QPair<bool, QVariant> verification = verifyPayload(paramMapString.toUtf8());
            if (!verification.first)
                return createErrorReply(HttpReply::BadRequest);

            params = JsonTypes::unpackParams(verification.second.toList());
        }
        return getDiscoverdDevices(params);
    }

    return createErrorReply(HttpReply::BadRequest);
}

HttpReply *DeviceClassesResource::getDeviceClass()
{
    qCDebug(dcRest) << "Get device class with id " << m_deviceClass.id();
    HttpReply *reply = createSuccessReply();
    reply->setHeader(HttpReply::ContentTypeHeader, "application/json; charset=\"utf-8\";");
    reply->setPayload(QJsonDocument::fromVariant(JsonTypes::packDeviceClass(m_deviceClass, NymeaCore::instance()->configuration()->locale())).toJson());
    return reply;
}

HttpReply *DeviceClassesResource::getActionTypes()
{
    qCDebug(dcRest) << "Get action types for device class" << m_deviceClass.id();
    HttpReply *reply = createSuccessReply();
    reply->setHeader(HttpReply::ContentTypeHeader, "application/json; charset=\"utf-8\";");
    reply->setPayload(QJsonDocument::fromVariant(JsonTypes::packActionTypes(m_deviceClass, NymeaCore::instance()->configuration()->locale())).toJson());
    return reply;
}

HttpReply *DeviceClassesResource::getActionType(const ActionTypeId &actionTypeId)
{
    qCDebug(dcRest) << "Get action type with id" << actionTypeId;

    foreach (const ActionType &actionType, m_deviceClass.actionTypes()) {
        if (actionType.id() == actionTypeId) {
            HttpReply *reply = createSuccessReply();
            reply->setHeader(HttpReply::ContentTypeHeader, "application/json; charset=\"utf-8\";");
            reply->setPayload(QJsonDocument::fromVariant(JsonTypes::packActionType(actionType, m_deviceClass.pluginId(), NymeaCore::instance()->configuration()->locale())).toJson());
            return reply;
        }
    }
    return createDeviceErrorReply(HttpReply::NotFound, Device::DeviceErrorActionTypeNotFound);
}

HttpReply *DeviceClassesResource::getStateTypes()
{
    qCDebug(dcRest) << "Get state types for device class" << m_deviceClass.id();
    HttpReply *reply = createSuccessReply();
    reply->setHeader(HttpReply::ContentTypeHeader, "application/json; charset=\"utf-8\";");
    reply->setPayload(QJsonDocument::fromVariant(JsonTypes::packStateTypes(m_deviceClass, NymeaCore::instance()->configuration()->locale())).toJson());
    return reply;
}

HttpReply *DeviceClassesResource::getStateType(const StateTypeId &stateTypeId)
{
    qCDebug(dcRest) << "Get state type with id" << stateTypeId;

    foreach (const StateType &stateType, m_deviceClass.stateTypes()) {
        if (stateType.id() == stateTypeId) {
            HttpReply *reply = createSuccessReply();
            reply->setHeader(HttpReply::ContentTypeHeader, "application/json; charset=\"utf-8\";");
            reply->setPayload(QJsonDocument::fromVariant(JsonTypes::packStateType(stateType, m_deviceClass.pluginId(), NymeaCore::instance()->configuration()->locale())).toJson());
            return reply;
        }
    }
    return createDeviceErrorReply(HttpReply::NotFound, Device::DeviceErrorStateTypeNotFound);
}

HttpReply *DeviceClassesResource::getEventTypes()
{
    qCDebug(dcRest) << "Get event types for device class" << m_deviceClass.id();
    HttpReply *reply = createSuccessReply();
    reply->setHeader(HttpReply::ContentTypeHeader, "application/json; charset=\"utf-8\";");
    reply->setPayload(QJsonDocument::fromVariant(JsonTypes::packEventTypes(m_deviceClass, NymeaCore::instance()->configuration()->locale())).toJson());
    return reply;
}

HttpReply *DeviceClassesResource::getEventType(const EventTypeId &eventTypeId)
{
    qCDebug(dcRest) << "Get event type with id" << eventTypeId;

    foreach (const EventType &eventType, m_deviceClass.eventTypes()) {
        if (eventType.id() == eventTypeId) {
            HttpReply *reply = createSuccessReply();
            reply->setHeader(HttpReply::ContentTypeHeader, "application/json; charset=\"utf-8\";");
            reply->setPayload(QJsonDocument::fromVariant(JsonTypes::packEventType(eventType, m_deviceClass.pluginId(), NymeaCore::instance()->configuration()->locale())).toJson());
            return reply;
        }
    }
    return createDeviceErrorReply(HttpReply::NotFound, Device::DeviceErrorEventTypeNotFound);
}

HttpReply *DeviceClassesResource::getDiscoverdDevices(const ParamList &discoveryParams)
{
    qCDebug(dcRest) << "Discover devices for DeviceClass" << m_deviceClass.id();
    qCDebug(dcRest) << discoveryParams;

    DeviceDiscoveryInfo discoveryInfo = NymeaCore::instance()->deviceManager()->discoverDevices(m_deviceClass.id(), discoveryParams);

    if (discoveryInfo.status() == Device::DeviceErrorAsync) {
        HttpReply *reply = createAsyncReply();
        m_discoverRequests.insert(discoveryInfo.id(), reply);
        return reply;
    }

    if (discoveryInfo.status() != Device::DeviceErrorNoError)
        return createDeviceErrorReply(HttpReply::InternalServerError, discoveryInfo.status());

    return createSuccessReply();
}

void DeviceClassesResource::devicesDiscovered(const DeviceDiscoveryInfo &deviceDiscoveryInfo)
{
    if (!m_discoverRequests.contains(deviceDiscoveryInfo.id()))
        return; // Not the discovery we are waiting for.

    qCDebug(dcRest) << "Discovery finished. Found" << deviceDiscoveryInfo.deviceDescriptors().count() << "devices.";

    if (m_discoverRequests.value(deviceDiscoveryInfo.id()).isNull()) {
        qCWarning(dcRest) << "Async reply for discovery does not exist any more (timeout).";
        return;
    }

    HttpReply *reply = m_discoverRequests.take(deviceDiscoveryInfo.id());
    reply->setHeader(HttpReply::ContentTypeHeader, "application/json; charset=\"utf-8\";");
    reply->setPayload(QJsonDocument::fromVariant(JsonTypes::packDeviceDescriptors(deviceDiscoveryInfo.deviceDescriptors())).toJson());
    reply->finished();
}

HttpReply *DeviceClassesResource::getDeviceClasses(const VendorId &vendorId)
{
    if (vendorId == VendorId()) {
        qCDebug(dcRest) << "Get all device classes.";
    } else {
        qCDebug(dcRest) << "Get device classes for vendor" << vendorId.toString();
    }

    HttpReply *reply = createSuccessReply();
    reply->setHeader(HttpReply::ContentTypeHeader, "application/json; charset=\"utf-8\";");
    reply->setPayload(QJsonDocument::fromVariant(JsonTypes::packSupportedDevices(vendorId, NymeaCore::instance()->configuration()->locale())).toJson());
    return reply;
}

}
