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

#include "devicesresource.h"
#include "jsontypes.h"
#include "guhcore.h"
#include "network/httpreply.h"
#include "network/httprequest.h"

#include <QJsonDocument>

namespace guhserver {

DevicesResource::DevicesResource(QObject *parent) :
    QObject(parent)
{
}

HttpReply DevicesResource::proccessDeviceRequest(const HttpRequest &request, const QStringList &urlTokens)
{
    DeviceId deviceId;
    StateTypeId stateTypeId;

    Device *device = 0;

    // first parse device, stateTypeId
    if (urlTokens.count() >= 4) {
        deviceId = DeviceId(urlTokens.at(3));
        if (deviceId.isNull()) {
            qCWarning(dcRest) << "Could not parse DeviceId:" << urlTokens.at(3);
            return HttpReply(HttpReply::BadRequest);
        }
        device = GuhCore::instance()->findConfiguredDevice(deviceId);
        if (!device) {
            qCWarning(dcRest) << "Could find any device with DeviceId:" << urlTokens.at(3);
            return HttpReply(HttpReply::NotFound);
        }

        // /api/v1/devices/{deviceId}/states/{stateTypeId}
        if (urlTokens.count() >= 6 && urlTokens.at(4) == "states") {
            stateTypeId = StateTypeId(urlTokens.at(5));
            if (stateTypeId.isNull()) {
                qCWarning(dcRest) << "Could not parse StateTypeId:" << urlTokens.at(5);
                return HttpReply(HttpReply::BadRequest);
            }

            if (!device->hasState(stateTypeId)){
                qCWarning(dcRest) << "This device has no StateTypeId:" << urlTokens.at(5);
                return HttpReply(HttpReply::NotFound);
            }
        }
    }

    // check methods
    if (request.method() == HttpRequest::Get) {

        // /api/v1/devices
        if (urlTokens.count() == 3)
            return getConfiguredDevices();

        // /api/v1/devices/{deviceId}
        if (urlTokens.count() == 4)
            return getConfiguredDevice(device);

        // /api/v1/devices/{deviceId}/states
        if (urlTokens.count() == 5 && urlTokens.at(4) == "states")
            return getDeviceStateValues(device);

        // /api/v1/devices/{deviceId}/states/{stateTypeId}
        if (urlTokens.count() == 6 && urlTokens.at(4) == "states")
            return getDeviceStateValue(device, stateTypeId);
    } else if (request.method() == HttpRequest::Delete) {

        // /api/v1/devices
        if (urlTokens.count() == 3)
            return HttpReply(HttpReply::BadRequest);

        if (urlTokens.count() == 4)
            return removeDevice(device);

    }

    return HttpReply(HttpReply::BadRequest);
}

HttpReply DevicesResource::getConfiguredDevices()
{
    HttpReply httpReply(HttpReply::Ok);
    httpReply.setHeader(HttpReply::ContentTypeHeader, "application/json; charset=\"utf-8\";");
    httpReply.setPayload(QJsonDocument::fromVariant(JsonTypes::packConfiguredDevices()).toJson());
    httpReply.packReply();
    return httpReply;
}

HttpReply DevicesResource::getConfiguredDevice(Device *device)
{
    HttpReply httpReply(HttpReply::Ok);
    httpReply.setHeader(HttpReply::ContentTypeHeader, "application/json; charset=\"utf-8\";");
    httpReply.setPayload(QJsonDocument::fromVariant(JsonTypes::packDevice(device)).toJson());
    httpReply.packReply();
    return httpReply;
}

HttpReply DevicesResource::getDeviceStateValues(Device *device)
{
    HttpReply httpReply(HttpReply::Ok);
    httpReply.setHeader(HttpReply::ContentTypeHeader, "application/json; charset=\"utf-8\";");
    httpReply.setPayload(QJsonDocument::fromVariant(JsonTypes::packDeviceStates(device)).toJson());
    httpReply.packReply();
    return httpReply;
}

HttpReply DevicesResource::getDeviceStateValue(Device *device, const StateTypeId &stateTypeId)
{
    HttpReply httpReply(HttpReply::Ok);
    httpReply.setHeader(HttpReply::ContentTypeHeader, "application/json; charset=\"utf-8\";");
    QVariantMap stateValue;
    stateValue.insert("value", device->state(stateTypeId).value());
    httpReply.setPayload(QJsonDocument::fromVariant(stateValue).toJson());
    httpReply.packReply();
    return httpReply;
}

HttpReply DevicesResource::removeDevice(Device *device)
{
    DeviceManager::DeviceError result = GuhCore::instance()->removeConfiguredDevice(device->id(), QHash<RuleId, RuleEngine::RemovePolicy>());

    // TODO: parse removepolicy query params

    if (result == DeviceManager::DeviceErrorNoError)
        return HttpReply(HttpReply::Ok);

    return HttpReply(HttpReply::Forbidden);
}

}
