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

#include "deviceclassesresource.h"
#include "httprequest.h"
#include "guhcore.h"

#include <QJsonDocument>

namespace guhserver {

DeviceClassesResource::DeviceClassesResource(QObject *parent) :
    RestResource(parent)
{
    connect(GuhCore::instance(), &GuhCore::devicesDiscovered, this, &DeviceClassesResource::devicesDiscovered, Qt::QueuedConnection);
}

QString DeviceClassesResource::name() const
{
    return "deviceclasses";
}

HttpReply *DeviceClassesResource::proccessRequest(const HttpRequest &request, const QStringList &urlTokens)
{

    // get the main resource

    // /api/v1/deviceclasses/{deviceClassId}/
    if (urlTokens.count() >= 4) {
        DeviceClassId deviceClassId = DeviceClassId(urlTokens.at(3));
        if (deviceClassId.isNull()) {
            qCWarning(dcRest) << "Could not parse DeviceClassId:" << urlTokens.at(3);
            return createErrorReply(HttpReply::BadRequest);
        }
        m_deviceClass = GuhCore::instance()->findDeviceClass(deviceClassId);
        if (!m_deviceClass.isValid()) {
            return createErrorReply(HttpReply::NotFound);
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
                    return createErrorReply(HttpReply::BadRequest);
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
            return createErrorReply(HttpReply::BadRequest);
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
            return createErrorReply(HttpReply::BadRequest);
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
            return createErrorReply(HttpReply::BadRequest);
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
    reply->setPayload(QJsonDocument::fromVariant(JsonTypes::packDeviceClass(m_deviceClass)).toJson());
    return reply;
}

HttpReply *DeviceClassesResource::getActionTypes()
{
    qCDebug(dcRest) << "Get action types for device class" << m_deviceClass.id();
    HttpReply *reply = createSuccessReply();
    reply->setPayload(QJsonDocument::fromVariant(JsonTypes::packActionTypes(m_deviceClass)).toJson());
    return reply;
}

HttpReply *DeviceClassesResource::getActionType(const ActionTypeId &actionTypeId)
{
    qCDebug(dcRest) << "Get action type with id" << actionTypeId;

    foreach (const ActionType &actionType, m_deviceClass.actionTypes()) {
        if (actionType.id() == actionTypeId) {
            HttpReply *reply = createSuccessReply();
            reply->setPayload(QJsonDocument::fromVariant(JsonTypes::packActionType(actionType)).toJson());
            return reply;
        }
    }
    return createErrorReply(HttpReply::NotFound);
}

HttpReply *DeviceClassesResource::getStateTypes()
{
    qCDebug(dcRest) << "Get state types for device class" << m_deviceClass.id();
    HttpReply *reply = createSuccessReply();
    reply->setPayload(QJsonDocument::fromVariant(JsonTypes::packStateTypes(m_deviceClass)).toJson());
    return reply;
}

HttpReply *DeviceClassesResource::getStateType(const StateTypeId &stateTypeId)
{
    qCDebug(dcRest) << "Get state type with id" << stateTypeId;

    foreach (const StateType &stateType, m_deviceClass.stateTypes()) {
        if (stateType.id() == stateTypeId) {
            HttpReply *reply = createSuccessReply();
            reply->setPayload(QJsonDocument::fromVariant(JsonTypes::packStateType(stateType)).toJson());
            return reply;
        }
    }
    return createErrorReply(HttpReply::NotFound);
}

HttpReply *DeviceClassesResource::getEventTypes()
{
    qCDebug(dcRest) << "Get event types for device class" << m_deviceClass.id();
    HttpReply *reply = createSuccessReply();
    reply->setPayload(QJsonDocument::fromVariant(JsonTypes::packEventTypes(m_deviceClass)).toJson());
    return reply;
}

HttpReply *DeviceClassesResource::getEventType(const EventTypeId &eventTypeId)
{
    qCDebug(dcRest) << "Get event type with id" << eventTypeId;

    foreach (const EventType &eventType, m_deviceClass.eventTypes()) {
        if (eventType.id() == eventTypeId) {
            HttpReply *reply = createSuccessReply();
            reply->setPayload(QJsonDocument::fromVariant(JsonTypes::packEventType(eventType)).toJson());
            return reply;
        }
    }
    return createErrorReply(HttpReply::NotFound);
}

HttpReply *DeviceClassesResource::getDiscoverdDevices(const ParamList &discoveryParams)
{
    qCDebug(dcRest) << "Discover devices for DeviceClass" << m_deviceClass.id();
    qCDebug(dcRest) << discoveryParams;

    DeviceManager::DeviceError status = GuhCore::instance()->discoverDevices(m_deviceClass.id(), discoveryParams);

    if (status == DeviceManager::DeviceErrorAsync) {
        HttpReply *reply = createAsyncReply();
        m_discoverRequests.insert(m_deviceClass.id(), reply);
        return reply;
    }

    if (status != DeviceManager::DeviceErrorNoError)
        return createErrorReply(HttpReply::InternalServerError);

    return createSuccessReply();
}

void DeviceClassesResource::devicesDiscovered(const DeviceClassId &deviceClassId, const QList<DeviceDescriptor> deviceDescriptors)
{
    if (!m_discoverRequests.contains(deviceClassId))
        return; // Not the discovery we are waiting for.

    qCDebug(dcRest) << "Discovery finished. Found" << deviceDescriptors.count() << "devices.";

    HttpReply *reply = m_discoverRequests.take(deviceClassId);
    reply->setPayload(QJsonDocument::fromVariant(JsonTypes::packDeviceDescriptors(deviceDescriptors)).toJson());
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
    reply->setPayload(QJsonDocument::fromVariant(JsonTypes::packSupportedDevices(vendorId)).toJson());
    return reply;
}

}
